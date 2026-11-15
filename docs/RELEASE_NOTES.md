# DJ Decks 0.37.0-beta.1 — 2026-11-15

Beta for foobar2000 2.x Default UI, in x64 and x86. Choose the architecture of foobar, even on 64-bit Windows.

## New since the public 0.34.1-beta.1

- Independent **A/B loops** on both decks: A, B, enable/disable and clear, with source times and waveform markers. Points persist per file/subsong across restarts.
- Per-deck **Q** snaps new points to the displayed grid. Enabling it also snaps and saves existing A/B points or A alone. Manual/estimated/dotted grids use the same displayed phase and zoom stride. Missing grids leave existing points unchanged; dotted lines remain indicative.
- A modeless **six-tab settings window** replaces the gear popup: Appearance, Audio, Headphones, Playlists, Playback and Stems. Changes apply immediately and existing choices are retained.
- More responsive tooltip drawing through bounded repaint regions, clipped rendering and a reused bitmap. Tooltip animation/fade disabled locally, with 300 ms initial and 75 ms reshow delay. Corrected settings check/button drawing and contrast.
- **Save/load JSON session files** from Settings → Playlists: both lists alone, or loaded deck paths/positions/points/manual grids/tempo/pitch/volumes/mixer. Preserve ordering, duplicates and subsongs; no audio is embedded. Full restore is paused with loops/AUTO/Cue off. A recovery snapshot is saved before import, and invalid files are rejected before changing the live lists/decks.

## Downloads

- [64-bit component](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-win-x64.fb2k-component)
- [32-bit component](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-win-x86.fb2k-component)
- [Complete matching source for both architectures](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-source.zip)

The optional stems installer and SHA-256 checksums are attached. See [all detailed intermediate versions](../CHANGELOG.md), [loops](AB_LOOPS.md), [settings](SETTINGS.md), [session files](SESSION_FILES.md) and [known issues](KNOWN_ISSUES.md).

## Validation and limits

The user confirmed the installed x64 build/session-file workflow working. x64 and x86 builds and native release tests pass; wider real-host x86 feedback is still requested. Release verification does not open, capture or monitor foobar.

**Stems remain experimental** and prepared in advance. The supported optional Python/PyTorch engine requires 64-bit Windows, including with foobar x86. Stems/cache/media are not embedded or reactivated by session import. General profile/output settings stay local; source audio must remain accessible at the saved paths. Session files contain private local paths, so review before sharing.

Q follows displayed lines rather than a full musical beat tracker; zoom changes which lines are available. Decoder seeks and processing resets can produce loop-edge clicks/artifacts. Bluetooth latency, indicative grids, silence-scan bounds and lack of a limiter remain documented limitations. x86 has a smaller address space for long tracks/stems.

Thanks to StyxCrosser for testing/suggestions and Fabcore for the 32-bit request.
