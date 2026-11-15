# Tooltip and dark-label fix — 0.36.1-beta.1

Reported: slow/stuttering tooltips since 0.35.1 and black-on-near-black items in Playback settings. The tooltip setup function was unchanged between archived 0.35.0 and 0.35.1 sources; this does not rule out the user’s observed timing. Source review found whole-panel hover repaints, unconditional GDI+ work for clipped-away controls, and repeated full-size bitmap allocation.

The patch narrows button feedback repaint regions, skips clipped surfaces/icons and reuses the panel bitmap until resize. Tooltip animation/fade is disabled for DJ Decks only. Delay: 300 ms initially, 75 ms when moving between buttons, 14 s auto-hide. Geometry/language changes rebuild registration; ordinary paints do not. No Windows-wide preference is changed.

Settings buttons/toggles draw their own light text/check marks/focus, avoiding theme-dependent black text. Existing values, immediate application, keyboard/checkbox behavior and six categories are retained.

Verification uses hidden test windows and generated in-memory pixels, never foobar or screen capture. The actual tooltip setup and paint methods are extracted from component source. The benchmark (`DJ_HOVER_BENCH=1` with `scripts/test-tooltips.ps1`) includes the archived renderer as a test-only reference; it compares clipped drawing work, not actual tooltip latency, decoding or host responsiveness. Real-host improvement remains to be checked by the user after installation.

The final 0.36.2 correction also forwards WM_DRAWITEM from the settings content host to the owner renderer; this is included in 0.37.0 and required for visible checkbox/action labels. The contrast regression follows that parent route.
