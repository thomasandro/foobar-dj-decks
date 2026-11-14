# Validation — 2026-11-14

Release: **0.18.0-alpha.1**, based on locally installed 0.18.0-dev.
Release preparation does not launch, capture or monitor foobar. The user's development
installation was updated separately on request; this does not validate a clean profile.

## User feedback and screenshot

The user approved the 0.17.1-dev interface and explicitly supplied its screenshot for
publication. No new host capture or monitoring was performed. The image is preserved
unchanged at docs/images/dj-decks-0.17.1.png. Tooltip/tag-writing behavior is not
inferred from that screenshot.

## 0.18.0 scope

Added compact-footer non-overlap assertions and pixel comparisons distinguishing normal,
hover and pressed rendering. No host was launched or captured. The existing screenshot
documents 0.17.1-dev, not this release. Live pointer/capture behavior and manual release
still require host validation.

## Automated checks

- Release x64 build from the public tree; original SDK/third-party licensing preserved.
- Native drop-parser, BPM-matching, beat-grid/phase, language and grid-storage tests.
- Key parsing/standard-Camelot-Open Key conversions, 24-key round trips, transposition
  and all 576 pairs of Camelot compatibility states, including wraparound/unknown keys.
- BPM limits for ±8%, ±16%, ±50%, matching an already adjusted reference, out-of-range
  rejection and clamping. Old sessions keep the default ±16% range.
- Actual FillDeck/SoundTouch processing: 30 synthetic stereo sine-wave cases combining
  speeds 0.5/0.84/1/1.16/1.5, key lock/vinyl and pitches −12/0/+7 semitones. Duration
  tolerance 25 ms, frequency tolerance 2 Hz; this is not a music-quality assessment.
- File-reader release regression: decoder ownership dropped, pending audio cleared and
  position retained while awaiting explicit resume after tag analysis.
- Automatic harmonic match across key pairs and existing/reference transpositions.
- Offscreen skin rendering and non-overlapping control layouts at 900x650, 1280x720,
  1920x1080. No foobar window or screen capture is used.
- Package/source ZIP integrity, payload/source equality, hashes and privacy checks.

## Runtime checks still pending

- Clean portable-profile package installation/loading and all new UI controls.
- foo_bpm and foo_musical_key command availability, dialogs, cancellation and asynchronous
  completion in the user's installed versions; supported tag-name configuration.
- Session/range/transposition restoration in a fresh host profile, output backends,
  panel removal/unload and extended startup/shutdown testing.
- Listening quality and phase behavior on real music, especially at extreme settings.

Earlier development screenshots showed key tags and a low-contrast badge; this was
corrected in 0.14.1-dev. No automated live-host result is inferred from those screenshots.
No guarantee of live-DJ stability or compatibility with all foobar 2.x versions is made.
