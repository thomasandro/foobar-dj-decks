# Changelog

## 0.12.0-alpha.1 — 2026-11-13

First public alpha snapshot. The local-only development iterations 0.1 through
0.12.0-dev are grouped here rather than presented as public releases.

- Two independent decks, shared output mixer, crossfader and separate volumes.
- Waveforms, zoom, seek, artwork, artist/title scrolling and session restore.
- Tagged BPM, ±16% rate/tempo, SoundTouch key lock, one-click BPM matching.
- Experimental local beat phase SYNC, conservative rejection, indicative/manual grids.
- Per-track manual phase persistence and French/English interface selection.
- Public-source preparation: portable include paths, MIT/third-party notices, source
  bundle, x64 package tooling, tests and documentation. Unused SDK sample dialogs
  removed from this public build; /GS enabled; private PDB path suppressed.

This candidate is not a stability release. Clean-profile load and long-running
host/audio tests have not been completed. See docs/KNOWN_ISSUES.md and docs/VALIDATION.md.
