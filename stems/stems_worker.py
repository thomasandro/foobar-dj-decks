"""Local-only Demucs adapter. Job paths are supplied by the native component.
No source music is uploaded or modified; only the model download uses the network.
"""
from pathlib import Path
import argparse, contextlib, hashlib, json, math, os, shutil, sys, time, uuid

ENGINE = "demucs-4.0.1-htdemucs-v1"
NAMES = ("drums", "bass", "other", "vocals")

def atomic_json(path, value):
    path = Path(path)
    tmp = path.with_suffix(path.suffix + ".tmp")
    with tmp.open("w", encoding="utf-8") as f:
        json.dump(value, f, ensure_ascii=True); f.flush(); os.fsync(f.fileno())
    os.replace(tmp, path)

def valid_cache(path, key):
    try:
        m = json.loads((Path(path)/"manifest.json").read_text())
        assert m["engine"] == ENGINE and m["key"] == key and m["rate"] == 44100 and m["channels"] == 2
        n = m["frames"]
        assert type(n) is int and 0 < n <= 44100*1800
        return all((Path(path)/(name+".f32")).stat().st_size == n*8 for name in NAMES)
    except (OSError, ValueError, AssertionError, KeyError, TypeError):
        return False

def load_model():
    import torch
    from demucs.pretrained import get_model
    torch.set_num_threads(2)
    # Each engine has its own model directory, independently of any other Python tools.
    torch.hub.set_dir(str(Path(__file__).resolve().parent/"models"))
    return get_model("htdemucs").eval()

def run_inference(normalized, update):
    """Prefer CUDA, retry once with a fresh CPU model on any CUDA failure."""
    import gc, torch
    from demucs.apply import apply_model
    model = load_model()
    sources = list(model.sources)
    device = "cpu"
    try:
        if torch.cuda.is_available():
            device = "cuda"
    except Exception as exc:
        print("CUDA detection failed; using CPU:", exc, flush=True)

    def run(current, backend):
        update("separating", 3, "GPU (CUDA)" if backend == "cuda" else "CPU")
        print("Stem backend:", backend, flush=True)
        with torch.inference_mode():
            result = apply_model(current, normalized[None], shifts=0, split=True,
                                 overlap=.25, progress=True, device=backend, num_workers=0)[0]
            # Finish the CUDA work before declaring success or publishing files.
            if backend == "cuda":
                torch.cuda.synchronize()
            result = result.cpu()
            if not torch.isfinite(result).all():
                raise RuntimeError("Model returned non-finite samples")
            return result

    try:
        return run(model, device), device, sources
    except Exception as exc:
        if device != "cuda":
            raise
        print("CUDA failed; retrying on CPU:", exc, flush=True)
    # Outside the exception handler so its traceback no longer retains GPU tensors.
    del model
    gc.collect()
    try:
        torch.cuda.empty_cache()
    except Exception:
        pass
    update("loading_model", 1, "GPU unavailable; retrying on CPU")
    return run(load_model(), "cpu"), "cpu", sources

def separate(job_file):
    job_file = Path(job_file).resolve()
    work = job_file.parent
    status = work/"status.json"
    def update(phase, progress, message=""):
        atomic_json(status, dict(phase=phase, progress=int(progress), message=message))
    try:
        job = json.loads(job_file.read_text(encoding="utf-8-sig"))
        key = job["key"]
        if job.get("protocol") != 1 or len(key) != 64 or any(c not in "0123456789abcdef" for c in key):
            raise ValueError("Invalid stem job")
        target = Path(job["output"]).resolve()
        if target.name != key or target.parent != work.parent:
            raise ValueError("Output must be the sibling cache key")
        if valid_cache(target, key):
            update("ready", 100); return
        # OS lock is released even if the native component cancels/kills the worker.
        import msvcrt
        lock = (work.parent/(key+".lock")).open("a+b")
        lock.write(b"0"); lock.flush(); lock.seek(0)
        try:
            msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
        except OSError:
            raise RuntimeError("This track is already being prepared")
        with contextlib.closing(lock):
            if valid_cache(target, key):
                update("ready", 100); return
            update("loading_model", 1, "Loading model (first use may download it)")
            import torch, numpy as np, soundfile as sf
            from demucs.audio import convert_audio
            import demucs.apply as da
            audio, rate = sf.read(work/"input.wav", dtype="float32", always_2d=True)
            if len(audio) == 0 or len(audio)/rate > 1800 or not np.isfinite(audio).all():
                raise ValueError("Empty/invalid input or duration exceeds 30 minutes")
            wav = convert_audio(torch.from_numpy(audio.T.copy()), rate, 44100, 2)
            del audio
            mean = wav.mean(0).mean(); std = wav.mean(0).std()
            if not torch.isfinite(std) or std < 1e-8:
                std = torch.tensor(1.)
            normalized = (wav-mean)/std
            total = int(wav.shape[-1])
            update("separating", 3)
            # v4.0.1 reports chunk progress through tqdm; adapt its iterator locally.
            # No monkeypatch reaches the host: this interpreter handles one job and exits.
            original_tqdm = da.tqdm.tqdm
            def progress_iter(iterable, **kwargs):
                size = len(iterable)
                for i, item in enumerate(iterable):
                    yield item
                    update("separating", 3+int(85*(i+1)/max(1,size)))
            da.tqdm.tqdm = progress_iter
            try:
                stems, backend, sources = run_inference(normalized, update)
            finally:
                da.tqdm.tqdm = original_tqdm
            stems = stems*std+mean
            # Mixture consistency: all four enabled reconstruct the decoded input.
            # Distribute model residual across stems without independently normalizing them.
            stems += ((wav-stems.sum(0))/4)[None]
            if stems.shape != (4, 2, total) or not torch.isfinite(stems).all():
                raise RuntimeError("Model returned invalid samples")
            output = work/"result"
            output.mkdir(exist_ok=True)
            for i, name in enumerate(NAMES):
                update("writing", 90+i*2)
                data = stems[sources.index(name)].T.contiguous().numpy().astype("<f4", copy=False)
                with (output/(name+".f32")).open("wb") as f:
                    data.tofile(f); f.flush(); os.fsync(f.fileno())
            atomic_json(output/"manifest.json", dict(protocol=1,engine=ENGINE,key=key,rate=44100,channels=2,frames=total,backend=backend))
            if not valid_cache(output,key):
                raise RuntimeError("Cache validation failed")
            # Publish only complete results. Keep an invalid existing cache as a quarantine.
            if target.exists():
                target.rename(target.with_name(key+".invalid-"+uuid.uuid4().hex[:8]))
            output.rename(target)
            update("ready",100)
    except Exception as exc:
        atomic_json(status,dict(phase="error",progress=0,message=str(exc)))
        print(str(exc),file=sys.stderr)
        raise

if __name__ == "__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--job", type=Path)
    parser.add_argument("--download-model", action="store_true")
    args=parser.parse_args()
    if args.download_model:
        model=load_model(); print("Ready:",ENGINE,model.sources)
    elif args.job:
        separate(args.job)
    else:
        parser.error("--job or --download-model is required")
