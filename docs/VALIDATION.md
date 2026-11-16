# Release validation — 0.40.0-beta.1

The user confirmed the installed x64 version working. Both architectures compile and pass full native suites for release. New tests exercise the sample-peak limiter across five rates and four ceilings, stereo linkage, impulse protection, exact finite bypass, release, chunk invariance and extreme/invalid samples. Actual analyser and clear methods run against mock services/dialog answers to check GUID/provider/subsong, release-before-dispatch, unavailable/error behavior, confirmation cancellation and loaded/other-list preservation. Hidden playlist/settings/tooltip tests cover new commands/layout. Real-host x86 and actual external Smart Tempo analysis/tag writing still require external verification. No foobar access, capture or monitoring is used for release work.

Historical validation follows for context.

# Release validation — 0.38.1-beta.1

The user confirmed active playlist distribution and Ctrl+A in x64. Both architectures are compiled and checked with native suites for release; external real-host x86 feedback remains welcome. New regression cases cover alternating empty/odd/even/20,000-item lists, limits, duplicates/subsongs, unchanged inputs and actual Ctrl+A handling on two independent hidden playlist windows. No live foobar host, capture or monitoring is used.

Previous release validation follows for context.

# Release validation — 0.37.0-beta.1

The user confirmed x64 installation and the new session-file workflow working. Both architectures compile and pass native release regression suites. These tests cover the audio processing cases, queue/drop/persistence models, actual controller methods with mock services/readers, hidden settings/tooltip/control windows, parent-routed contrast drawing, loop bounds/Q, atomic file IO, malformed imports, recovery prerequisites and asynchronous preparation. No live x86 host installation/loading is claimed; external feedback remains welcome.

Publication checks compare every complete-source archive entry to both build trees and each packaged DLL to its architecture-specific build. Foobar is not accessed, launched, captured or monitored by release work. The historical local validation notes below describe development checks rather than missing release artifacts.

# Local candidate — 0.37.0-beta.1, x64 only

Targeted session tests run the actual capture/apply methods with mock readers, strict document parsing and actual atomic IO on generated temporary files. Coverage includes Unicode/BOM, ordering/duplicates/subsongs, version/type/size/queue limits, original-file preservation on a locked target, capacity checks before mutation, lists-only playback preservation, complete paused restoration, missing/empty decks, async import intent and the actual existing preparation scheduler/load wrapper. Existing settings/bindings and loop/storage/layout regressions pass. File chooser and real media loading remain for user verification in foobar. No host capture, monitoring, physical audio or scrobbles are used; x86 is not compiled.

# Local candidate — 0.36.2-beta.1, x64 only

Only x64 is compiled/tested for this change; x86 is deliberately deferred at the user’s request. Settings regressions now dispatch WM_DRAWITEM through the real content-parent window, reproducing the previously missed routing defect. Loop regressions execute the actual Q activation function, with mock readers/grid inputs, and check persistence, transport preservation, unavailable-grid rollback, A-only and final-line handling. Existing native loop/storage/layout tests are included. No foobar access/capture/monitoring; live confirmation remains for the user.

# Local candidate — 0.36.1-beta.1

Targeted tooltip and settings regressions cover the actual tooltip registration, delay/style policy, actual bitmap reuse/resize, hidden native window lifecycle, bound toggle behavior and bright text pixels. The optional old/new renderer benchmark checks pixel identity under a small dirty clip. Native tests do not establish live-host hover latency; user confirmation remains needed. No host, audio-device output, screen capture or monitoring is used.

# Local candidate — 0.36.0-beta.1

`scripts/test-settings.ps1 -Architecture x64|x86` builds its own hidden windows, executes the actual SettingsValues/SettingsChanged methods with mock configuration/services, and checks all six bilingual categories, command routing, custom-value retention, safe cue selection, trim/fade/Q, window reuse, resize, focus, close/reopen and owner teardown. No host, physical audio device, scrobble or capture is used. Live UI validation in foobar remains for the user. Existing loop regressions are retained.

# Local candidate — 0.35.2-beta.1

Targeted loop regression suite now also checks beat snapping and hidden control layout. Coverage includes the shared display-grid selector, non-zero phases, current-zoom strides, admissible source bounds, actual snapped A/B edits and saves, no-grid failure preserving previous points, persistence IO and existing loop boundaries. The unchanged audio-processing engine retains prior validation. Live-host checking of the Q button remains for the user. No foobar access, capture or monitoring.

# Local candidate — 0.35.1-beta.1

Both architectures are checked with `scripts/test-loop-storage.ps1 -Architecture x64|x86`: actual persistence/UI controls, original atomic file IO (including failed replacement preserving the existing file), restart simulation through fresh JSON parsing, Unicode/subsong separation, A-only points, overwrite, deletion, malformed and trimmed bounds, lock-free save and non-fatal write failure. Existing loop-boundary regressions are included. No host was accessed, launched, captured or monitored. Live persistence validation in foobar remains for the user.

# Local candidate — 0.35.0-beta.1

x64 full native suite passed; the final loop command regression also passed after adding non-fatal notices and the completed-fade activation guard. Both architectures compile. The final x86 full native suite also passed. A/B loop listening quality remains to be checked in the host by the user.

Coverage: actual A/B commands, independent points, minimum interval, pause/play retention, unavailable seeking, active/completed fade guard, preserving playback errors, seek-outside disable, AUTO wait/resume, 5400 shared-block simulations across 18 rate/speed/original/stem combinations, lookahead reset and hidden control layout. Existing audio processing cases remain included. No foobar access, capture or monitoring in these checks.

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
