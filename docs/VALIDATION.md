# Current validation — 0.34.1-beta.1

User-confirmed: x64 installation, real-track playback, save/restore, and successful
operation after installing this beta package with backup. Native automated tests
pass; no foobar host was launched, captured or monitored by release checks.
Stems remain experimental. Both x64 and x86 builds and native regression suites pass. External x86 host testing is still requested. The release process does not access foobar.
The corrected quoted playlist format was verified by the user in foobar; the mock
formatter test validates callback/Unicode handling, not foobar's script parser.


## 0.27 playlists

248 model copy/move cases cover insertion boundaries, subsongs, duplicates, active indices and capacity rollback. Hidden-window tests dispatch same-list/cross-list drops, Shift moves, cancellation and cleanup. The default format and one-line sanitization are checked. Real pointer hit testing and the title-format editor still need manual testing in foobar. No host was opened or captured.


## 0.26 listening options

The actual ScrobbleTick method passes with a mock service: opt-in, host toggle, unavailable service/metadata, normal-player conflict, independent decks and no repeated attempt after provider success/failure. No Last.fm requests are issued by these tests. Output-time qualification and SDK ReplayGain scale/fallback/peak cases pass separately. Actual interoperability with foo_scrobble inside the host remains unverified; see docs/SCROBBLING_REPLAYGAIN.md.


## 0.25 cache manager

Native cleanup tests cover byte counts, protected keys, open mapped-file readers,
worker byte locks, unknown/read-only data, repeat clear and source/model/job
preservation. Separate native tests reject Windows junctions at the cache root and
both application-owned parent locations. The native/audio suite and real CUDA
preparation/cache/cancellation tests pass. No host was launched or captured.
Dialog behavior, user playback during cleanup and long sessions still require
manual validation using [the beta sheet](BETA_TEST_PLAN.fr.md).

## Earlier public validation

Public x64 rebuild and native tests cover the actual automatic playback controller,
fade duration/pause/direction/curves, 192 queue reorder cases, serialization, themed
hidden windows, owner-draw states, layout and 30 synthetic audio cases. No foobar
host was opened or captured for this publication. The user supplied the screenshot;
it is illustration, not proof of automated transition validation.

Actual cross-app OLE behavior, real-music transition quality, clean-profile install,
session restoration under varied profiles, long sessions and shutdown/unload remain
pending. Same-deck auto-next is not gapless; A/B automatic fades remain experimental.

Previous validation record follows for traceability:

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

## 0.24 public preparation

Same native suite as 0.24.0-dev, plus five isolated backend tests (CPU unavailable GPU,
CUDA success, GPU failure/retry, CPU error, both errors). Real installed CUDA processing
and a simulated CUDA error followed by real CPU inference produced four valid stems
whose sum reconstructed synthetic input within 1.5e-8. These checks never launch foobar.
The supplied screenshot illustrates the development build, not an automated host test.
