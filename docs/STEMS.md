# Local stems — 0.24.0-dev

Experimental first version: drums, bass, other instruments and vocals. These are
estimated from a finished mix; they are not the original studio multitracks.

## Use / Utilisation

- Load a local track, then click **STEMS** on its deck. Preparation runs outside
  foobar in a hidden Python/Demucs process. The percent button shows progress;
  click it again to cancel. Only one preparation runs at a time. Playback can continue.
- Completion only makes the cache available; it does not switch the playing audio.
  Click the four buttons **DRUM/BASS/INST/VOC** (French: **BAT/BASSE/INST/VOIX**) to
  use the stems and toggle that part. Lit = kept; dim = cut. The first click cuts
  the chosen stem and keeps the other three. All four cut means silence.
- Below each stem: **S** toggles Solo and the percentage bar sets volume **0–100%**.
  Drag the bar, use the wheel (5% steps, Shift: 1%), or right-click to reset to 100%.
  Multiple solos combine. Solo overrides the mute mask temporarily; clearing all
  solos restores the previous mutes. A zero-volume solo remains silent. Levels and
  solos are independent between decks; volume alone does not activate stems/unmute.
  Loading a new track resets all stem volumes to 100% and clears solos. Original audio
  clears solos; reactivating stems retains the current track's volume settings.
- Once ready, the STEMS menu offers **Enable all four stems** and **Original audio**.
  The original file is never overwritten. Activating stems or returning to the
  original repositions/rebuffers at the current playhead; a short interruption can occur.
- The four streams share one source clock, are mixed before SoundTouch and then
  use the deck's normal speed, key lock, transposition, volume and crossfader.
  Muting has a short gain ramp, plus normal decoder/processor/output buffering latency.
- Seeking, beat SYNC, replay and EOF operate on the same stem timeline. Each deck
  has independent masks and position, even if both use the same cached track.
- Changing tracks resets to original audio with all stems enabled by default.
  Cached tracks do not activate automatically; no background playlist separation
  is performed. Auto-next/A-B playback therefore loads the next track's original audio.
- Stop/Release and tag analysis cancel any preparation of that file and release
  stem mappings. Preparing/activating stems cannot reopen an explicitly released
  decoder. After tag editing, explicitly resume first; playback resumes with original audio.

**Français :** STEMS prépare les quatre parties ; cliquer sur la progression annule.
Une fois prêt, BAT/BASSE/INST/VOIX conserve ou coupe chaque partie. Cliquer sur STEMS
permet d'activer les quatre ou de revenir au son original. Sous chaque stem, S active le Solo (plusieurs possibles) et le pourcentage règle son volume : glisser, molette, Maj pour affiner, clic droit pour 100 %. Un seul calcul à la fois.
La préparation ne modifie jamais les fichiers musicaux et n'active pas le résultat
en cours de lecture sans action explicite. Après changement de morceau, le son original
est utilisé ; les stems déjà préparés restent disponibles en cache.

## Engine installation

Optional external runtime: Python 3.10–3.12, Demucs 4.0.1, PyTorch/torchaudio 2.5.1
(CPU or CUDA 11.8), NumPy 1.26.4 and soundfile 0.13.1. Run the supplied **install-stems.ps1**
from its stems folder, optionally with **-DownloadModel**. It creates a dedicated
venv under `%LOCALAPPDATA%\DJDecks\stems-engine` and copies the worker there.
It does not alter the global Python installation or the foobar profile.

The official htdemucs model is downloaded once (about 80 MiB). Models/packages use
the network for installation/download; audio stays on the machine. Preparation is
automatic: CUDA is preferred when available, otherwise CPU (two PyTorch threads).
A GPU error retries once with a fresh CPU model, including out-of-memory errors.
The installer detects NVIDIA GPUs; **-CPUOnly** keeps a smaller CPU-only runtime.
Existing CPU installations need the installer rerun to obtain CUDA packages.
The worker logs the backend and fallback reason; existing caches remain reusable.
The process retains below-normal priority. Duration depends on hardware and track;
preparation still completes before use, with no real-time guarantee.
No AI inference library or model is loaded into foobar itself.

## Cache / limits

Location: `%LOCALAPPDATA%\DJDecks\stems-cache\v1`. Cache keys include local path,
subsong, file size, modification time and the fixed engine/model version. Tag writes
can invalidate the cache even if the audio itself is unchanged. Changing separation
parameters in a future worker requires a new engine/cache version.

Four raw float32 stereo files at 44.1 kHz, about **84.7 MB per minute** total. The
native player memory-maps them instead of reading the entire track into RAM. Caches
are reused on subsequent loads. No automatic eviction or cache manager in this version;
close/release the decks before manually removing generated cache entries.

Only local files, up to 30 minutes and 1 GiB decoded export input, are supported.
Subsong export uses foobar's own decoder, including installed input components.
Multichannel export uses the first two channels, matching the existing deck engine.
The original waveform, tags and artwork remain the reference; separate stem waveforms
are not implemented. The four stem buttons toggle mute; each has a volume and Solo.

Results are validated for identity, format, duration and size before becoming ready.
Only a complete result is published atomically; cancelled/failed staging data is
discarded. An invalid prior cache is quarantined rather than silently reused.
Closing/unloading the panel cancels its job and kills its own worker via a Windows
job object. Only the worker's log/NUL handles are inherited, never the host's readers.

Separation is imperfect. Residual mixture correction makes all four enabled stems
reconstruct the resampled decoded input; it does not improve isolation quality.
No limiter is added. Live quality, responsiveness, codecs/subsongs, tag-writing and
long-track memory use require further manual validation in foobar.

## Verification

The native suite covers 16 masks, stereo summation, gain ramps, independent clocks,
seek/EOF, malformed caches, source/subsong invalidation and WAV export. Actual FillDeck
tests include 30 original audio cases plus 30 stem cases for speed 0.5–1.5, key lock,
vinyl and -12/0/+7 semitone transposition. Release closes the mapped stem readers.

An optional integration test runs the real native export → installed Python Demucs
→ atomic cache → native reader chain on synthetic stereo audio, checks reconstruction,
cache hits, export cancellation and worker-process cancellation. It never opens foobar.
Run the native suite with `DJ_STEM_ENGINE` set to the engine root to include it.

Upstream: https://github.com/facebookresearch/demucs (v4.0.1),
https://pytorch.org/ and https://python-soundfile.readthedocs.io/.
