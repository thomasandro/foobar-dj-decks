# DJ Decks 0.18.0-alpha.1 — experimental prerelease

Improvements since 0.17.1-alpha.1, following community feedback:

- **Button feedback:** brighter hover state and depressed press appearance, keeping the glossy black style.
- **Compact lower strip:** common transport, crossfader and FR/EN menu on one row.
  Recovers **60 pixels** for the deck/waveform area; version is in the language tooltip.
- **Stop/Release file button on each deck:** closes its decoder and waveform readers,
  preserving position/settings. Press **Play** after tag editing to reopen at that position.
  If both decks use the same file, both are released. Released decks show **FREE/LIBRE**.

This releases only DJ Decks readers; other applications/components may still hold a file.
Multi-track drops still load the first track; per-deck queues are not implemented yet.

Install the **win-x64.fb2k-component** asset via Preferences → Components → Install.
Matching complete source/dependencies: **source.zip**. Distribute alongside the binary.
Checksums: **SHA256SUMS.txt**.

[Detailed changelog](https://github.com/thomasandro/foobar-dj-decks/blob/main/CHANGELOG.md)
· [Français](https://github.com/thomasandro/foobar-dj-decks/blob/main/README.fr.md)

**Experimental and unsigned; Windows x64 / foobar2000 2.x / Default UI.**
Build and native tests passed, including offscreen interaction-state/geometry checks,
reader release/position retention and 30 synthetic audio duration/frequency cases.
Live-host interaction, tag-writing regression, clean-profile installation, real-music
quality and extended shutdown/unload checks remain pending. No limiter/headphone cue;
beat SYNC remains experimental. Slow shutdown has been observed and remains under investigation.

[Known issues](https://github.com/thomasandro/foobar-dj-decks/blob/main/docs/KNOWN_ISSUES.md)
· [Validation](https://github.com/thomasandro/foobar-dj-decks/blob/main/docs/VALIDATION.md)
