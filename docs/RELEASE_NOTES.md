# DJ Decks 0.12.0-alpha.1 — experimental prerelease

First public alpha of a two-deck DJ component for foobar2000 Windows x64 / Default UI.
Includes independent transport, waveforms/artwork, a mixer, ±16% tempo/rate,
SoundTouch key lock, tagged-BPM matching, experimental beat SYNC, manual grids,
session restoration and a French/English interface.

Installable: `foo_dj-0.12.0-alpha.1-win-x64.fb2k-component`.
Matching complete source: `foo_dj-0.12.0-alpha.1-source.zip`.
Checksums: `SHA256SUMS.txt`. Distribute source alongside the binary package.

Experimental and unsigned. Automatic beat detection is conservative and may reject
music. Grids can be manually anchored; dotted grids are indicative only. There is
no limiter/headphone cue routing. Slow shutdown has been observed and needs investigation.

The standalone build and native tests passed, but a clean portable-profile install/load
has not yet been validated. Do not mark this as a stable release. Clean-profile installation and extended runtime checks remain pending; see
[known issues](https://github.com/thomasandro/foobar-dj-decks/blob/main/docs/KNOWN_ISSUES.md) and
[validation](https://github.com/thomasandro/foobar-dj-decks/blob/main/docs/VALIDATION.md).
