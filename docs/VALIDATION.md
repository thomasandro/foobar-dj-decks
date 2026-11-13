# Validation — 2026-11-13

Release: **0.15.0-alpha.1**, based on locally installed 0.15.0-dev.
Release preparation does not launch, capture or monitor foobar. The user's development
installation was updated separately on request; this does not validate a clean profile.

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
