# DJ Decks 0.15.0-alpha.1 — experimental prerelease

Second public alpha for foobar2000 Windows x64 / Default UI.

New since 0.12.0-alpha.1:
- Camelot and standard key labels, e.g. **8A (Am)**, reading foobar database/file tags.
- Colored harmonic compatibility hints: green/OK, amber/~, grey/? when uncertain.
- Independent **−12 to +12 semitone transposition**, without changing tempo.
- Per-deck **Calculate BPM**, **Calculate key**, **Refresh tags**; the analysis buttons
  require optional **foo_bpm** and **foo_musical_key**, respectively. Their dialogs,
  validation and tag-writing preferences still apply; live invocation needs testing.
- Per-deck **±8%, ±16%, ±50% speed ranges**, respected by BPM matching and beat SYNC.
- Persisted transposition/range and improved key-badge contrast.

Install `foo_dj-0.15.0-alpha.1-win-x64.fb2k-component` via Preferences → Components → Install.
The matching `foo_dj-0.15.0-alpha.1-source.zip` includes the complete source and bundled
dependencies. SHA-256 hashes are provided in `SHA256SUMS.txt`.

[Detailed changelog, including unpublished development versions](https://github.com/thomasandro/foobar-dj-decks/blob/main/CHANGELOG.md)

**Experimental, unsigned, not a stability release.** Build and native tests passed,
including 30 synthetic audio duration/frequency cases. Clean-profile install/load,
real-music listening tests, external analysis commands and extended runtime/shutdown
checks remain pending. Slow shutdown has been observed. No limiter or headphone cue;
automatic beat SYNC remains experimental. See [known issues](https://github.com/thomasandro/foobar-dj-decks/blob/main/docs/KNOWN_ISSUES.md).
