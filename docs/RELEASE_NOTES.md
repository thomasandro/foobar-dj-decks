# DJ Decks 0.34.1-beta.1 — 2026-11-15

Beta packages for foobar2000 2.x Default UI, in x64 and x86. Choose the architecture of foobar, even on 64-bit Windows. Both packages share the complete matching source archive and detailed changelog.

Since the previous public 0.28.0-beta.1:

- Configurable separate stereo headphone output and cue volume, with per-deck Cue A/B.
- Independent playlist excerpt previews: 8 or 15 seconds from the start/middle, or 8+8 seconds. Click the active Stop icon to stop.
- Floating approximate whole-track preview waveform near the playlist; click to preview another position. A click in combined mode plays 16 continuous seconds.
- Optional leading/trailing silence trimming and asynchronous preparation/preload for AUTO. Both options default off; default threshold -60 dBFS, adjustable. Settings apply to newly loaded tracks; source files are unchanged.
- Multi-track Shift drag-and-drop moves preserve the full selection.
- Seeking or returning to start preserves AUTO alternation; active fades retain their direction and current gain.

## Validation and limits

User-confirmed x64 operation. Both architectures compile and pass native automated tests, without opening or monitoring foobar. x86 real-host feedback is still requested. Stems remain experimental; the optional Python engine is a separate installation (64-bit Windows required for the supported engine).

Bluetooth cue latency is device-dependent. Cue is a separate stereo output, not a custom multichannel/ASIO matrix. Cue A/B follows deck transport; playlist excerpts are independent. Whole-track preview waveforms are sampled approximations. Silence detection examines at most the first/last 60 seconds and keeps original edges when a safe result is unavailable; it is not a gapless guarantee. No limiter is included.

See CHANGELOG.md for every intermediate version and docs/HEADPHONE_CUE.md, PLAYLIST_PREVIEW.md, SILENCE_TRIM.md and KNOWN_ISSUES.md for details.
