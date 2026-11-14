# DJ Decks 0.24.0-alpha.1 — stems, GPU acceleration and appearance settings

New since **0.20.0-alpha.1**:

- **Local stems:** drums, bass, instruments and vocals; mute, volume and Solo for each part on both decks. Preparation has progress/cancellation and a reusable cache.
- **NVIDIA GPU acceleration:** automatic CUDA selection with CPU fallback if unavailable or if GPU processing fails. Same htdemucs model and float32 settings.
- **Half/double BPM matching:** exact, half and double BPM candidates, selecting the nearest speed inside the chosen range; beat SYNC follows the matching grid.
- **Appearance:** Glossy or subdued Flat theme, eight independent deck colors and a gear Settings menu. Clearer BPM/key icons and corrected header text clipping.
- **Floating playlists:** integrated themed title and close button, with dragging/resizing and saved lists retained.

Thanks, **[StyxCrosser](https://hydrogenaudio.org/index.php/topic,130167.msg1090691.html#msg1090691)** — my first, best and so far only tester! Of course your requests get special attention. :)
The Flat theme, header fixes and clearer musical symbols follow your suggestions.

![Stems, customized deck colors and floating playlist — user-supplied screenshot](https://raw.githubusercontent.com/thomasandro/foobar-dj-decks/v0.24.0-alpha.1/docs/images/dj-decks-stems-0.24.png)

## Downloads and setup

- [Installable component — Windows x64](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.24.0-alpha.1/foo_dj-0.24.0-alpha.1-win-x64.fb2k-component): Preferences → Components → Install, then restart.
- [Optional stems engine installer](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.24.0-alpha.1/foo_dj-stems-engine-0.24.0-alpha.1.zip): Python 3.10–3.12 required. Extract and run `./install-stems.ps1 -DownloadModel` in PowerShell. Downloads pinned dependencies and the model; CPU fallback included. Use `-CPUOnly` for the smaller CPU installation. Existing CPU engines need this installer rerun to enable CUDA.
- Complete matching source ZIP and SHA-256 checksums are attached. Python packages and model weights are downloaded separately, not bundled.

[Detailed changelog, including unpublished 0.20.1–0.24.0 milestones](https://github.com/thomasandro/foobar-dj-decks/blob/v0.24.0-alpha.1/CHANGELOG.md) · [Stems setup and limits](https://github.com/thomasandro/foobar-dj-decks/blob/v0.24.0-alpha.1/docs/STEMS.md) · [Appearance](https://github.com/thomasandro/foobar-dj-decks/blob/v0.24.0-alpha.1/docs/APPEARANCE.md)

## Performance and validation

GTX 1070 Ti, same synthetic 30-second excerpt, htdemucs/float32, three runs per mode:
median preparation **53.04 s CPU (two threads) → 3.01 s CUDA**, about **17.6×** faster.
First result including process initialization: **73.98 s → 13.92 s**. These are offline
predecoded-WAV measurements, not a real-time streaming guarantee or a music-quality test.

Release x64 build/native tests passed, including 60 actual audio-processing cases,
stem mute/solo combinations, real CUDA preparation, cache/cancellation and real CPU
separation after an injected CUDA error. The supplied screenshot is from the development
interface. This publication did not launch, install into, monitor or capture foobar.

**Experimental, unsigned; foobar2000 2.x x64 / Default UI.** Stems are approximate,
fully prepared before use, limited to 30 minutes/1 GiB decoded input and stored in a
cache without automatic eviction (~85 MB/minute). No limiter/headphone cue; SYNC is
experimental. Clean-profile and extended runtime/shutdown validation remain incomplete.
