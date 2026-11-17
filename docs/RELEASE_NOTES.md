# DJ Decks 0.40.3-beta.1 — corrections and settings improvements (x64 / x86)

Thanks to StyxCrosser and Fabcore for their testing and reports.

- Auto master sample rate now follows the first playing track at engine start instead of an unconditional 48 kHz fallback. Settings → Audio offers Auto / 44.1 / 48 / 96 / 192 kHz; restart foobar after changing it. A rate forced by the output takes priority.
- Corrected settings scrollbar and wheel routing, including fractional wheel input over closed combo boxes.
- Bounded glossy-surface cache and 30 fps animation with coalesced volume/crossfader repaints, intended to reduce UI CPU cost.
- Unload replaces Release; retained position and resume behavior are unchanged.
- Settings category buttons follow the selected Glossy/Flat theme and deck A accent. Confirmed per-category defaults preserve tracks, playlist contents, positions, loop points and cache files; language is retained.
- Settings option updates retain existing controls where possible; category rebuilds batch repaint to reduce flicker.

The installed x64 release works on my setup. **I have not tested this release in x86 foobar.** Both full native test suites pass, but Fabcore’s output-device, scrolling and CPU retest is still needed. No real-host CPU benchmark is claimed. Stems remain experimental.

[Detailed changelog](../CHANGELOG.md) · [Download](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.40.3-beta.1)
