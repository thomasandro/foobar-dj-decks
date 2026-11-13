# Changelog

## 0.15.0-alpha.1 — 2026-11-13 (public prerelease)

Second public alpha, collecting the local development steps 0.12.1-dev through
0.15.0-dev listed individually below. Those development versions were not separate
GitHub releases. The public build changes the version label and release documentation
from 0.15.0-dev; no further audio/UI feature changes are introduced here.

- Musical key display in Camelot and standard notation; metadata database fallback.
- Per-deck harmonic compatibility colors and improved badge contrast.
- Independent semitone transposition, with updated displayed key and compatibility.
- Analysis buttons using optional foo_bpm / foo_musical_key and live metadata refresh.
- Selectable ±8%, ±16% and ±50% speed ranges, including BPM matching and beat SYNC.
- Session persistence for transposition and selected speed range.
- Native tests passed, including 30 synthetic duration/frequency cases up to ±50%.

Experimental and unsigned. Clean-profile installation, real-audio quality, external
analysis command execution and extended runtime/shutdown validation remain pending.
No claim of stable live-DJ operation is made. See docs/VALIDATION.md.

## 0.15.0-dev — local development, not separately published

- Independent speed-range menus on both decks: +/-8%, +/-16% (default), +/-50%.
- Slider, wheel, BPM matching and beat SYNC respect the destination deck's range.
- Switching to a narrower range clamps the current speed to its nearest limit.
- Range persists per deck in the session; older sessions default to +/-16%.

## 0.14.1-dev — local development, not separately published

- Fix low-contrast key badges: use a dark background with colored text and border
  in every compatibility state on both decks.

## 0.14.0-dev — local development, not separately published

- Per-deck Calculate BPM (foo_bpm), Calculate key (foo_musical_key) and Refresh tags.
- Dispatch public context commands with only the loaded file/subsong, independently
  of playlist selection. Missing commands show a message; dependencies remain optional.
- React to host metadata-change notifications for loaded tracks, without reloading
  decoders or seeking. Analysis dialogs, tag writes and skip-existing settings remain
  controlled by the analysing components. Supported fields: BPM/TEMPO and
  INITIALKEY/INITIAL KEY/KEY; custom field names must use one of these to be displayed.
- Integration compiled, but invocation and completion in the live host remain untested.

## 0.13.0-dev — local development, not separately published

- Independent per-deck transposition, -12 to +12 semitones, via SoundTouch.
  Minus/plus buttons change by one semitone; the centre button resets to zero.
- Transposition changes pitch without changing the selected tempo or BPM.
  In vinyl mode, rate-induced pitch change is applied in addition.
- Save/restore transposition in the session. Camelot/standard key labels and
  compatibility use the transposed tagged key; vinyl at non-neutral speed
  continues to mark compatibility as uncertain.
- Audio tests cover tempo, key-lock/vinyl and transposition combinations.

## 0.12.3-dev — local development, not separately published

- Per-deck musical key badge, showing Camelot and standard notation, e.g. 8A (Am).
- Green/OK for usual Camelot pairs (same key, adjacent same-mode, relative major/minor),
  amber/~ for other pairs and grey/? for missing/unknown keys or pitch-changing vinyl speed.
- Compatibility describes tagged keys, not measured audio or a guarantee of a good mix.

## 0.12.2-dev — local development, not separately published

- Read key metadata from foobar's database first, then fall back to decoder file tags.

## 0.12.1-dev — local development, not separately published

- Display the tagged musical key beside BPM on both decks, converting standard
  major/minor, Camelot and Open Key notation to Camelot where recognized.
- Read INITIALKEY, Initial Key and KEY at track load; retain unknown tag text.
- This is the original tagged key, not a live pitch estimate in vinyl mode.

## 0.12.0-alpha.1 — 2026-11-13

First public alpha snapshot. The local-only development iterations 0.1 through
0.12.0-dev are grouped here rather than presented as public releases.

- Two independent decks, shared output mixer, crossfader and separate volumes.
- Waveforms, zoom, seek, artwork, artist/title scrolling and session restore.
- Tagged BPM, ±16% rate/tempo, SoundTouch key lock, one-click BPM matching.
- Experimental local beat phase SYNC, conservative rejection, indicative/manual grids.
- Per-track manual phase persistence and French/English interface selection.
- Public-source preparation: portable include paths, MIT/third-party notices, source
  bundle, x64 package tooling, tests and documentation. Unused SDK sample dialogs
  removed from this public build; /GS enabled; private PDB path suppressed.

This candidate is not a stability release. Clean-profile load and long-running
host/audio tests have not been completed. See docs/KNOWN_ISSUES.md and docs/VALIDATION.md.

## Earlier local development history (before the first public alpha)

The following milestones are reconstructed from retained source history and development
scripts. They are not GitHub releases; descriptions do not imply every host test passed.
Where only a feature milestone survives, no extra patch-level history is invented.

| Local version | Change |
| --- | --- |
| 0.12.0-dev | French/English interface selector and persisted language. |
| 0.11.3-dev | Manual beat-grid anchors, millisecond shifts, Auto reset and per-track persistence. |
| 0.11.2-dev | More visible beat markers, sparse overview spacing and indicative dotted grids. |
| 0.11.1-dev | First waveform beat markers and confidence-related grid labels. |
| 0.11.0-dev | Experimental one-shot beat-phase SYNC in addition to BPM matching. |
| 0.10.0-dev | SoundTouch tempo processing with key lock and optional vinyl mode. |
| 0.9.1-dev | One-click BPM matching to the other deck, within the original ±16% range. |
| 0.9.0-dev | Per-deck speed control, effective BPM display and persisted speed. |
| 0.8.10-dev | Display BPM from track metadata. |
| 0.8.9-dev | Scrolling title/artist for long labels. |
| 0.8.8-dev | Separate title/artist lines and larger cover art. |
| 0.8.7-dev | Prefer title/artist tags over the filename. |
| 0.8.6-dev | Per-deck cover lookup and artwork display. |
| 0.8.5-dev | Session persistence correction with profile JSON fallback. |
| 0.8.4-dev | Shared transport button toggles both states, including opposite states. |
| 0.8.3-dev | Shared start button for both decks. |
| 0.8.2-dev | Save/restore loaded tracks and positions, restored paused. |
| 0.8.1-dev | Improved waveform relief/contrast and UTF-8 build settings. |
| 0.8.0-dev | Dark mixer appearance with cyan/orange accents. |
| 0.7.2-dev | Compatible WebView2 text drag/drop payloads alongside native foobar drops. |
| 0.7.1-dev | Cooperative cancellation of waveform decoding before worker join. |
| 0.7 | Drag-and-drop track loading. |
| 0.6.2 | Smoother waveform scrolling. |
| 0.6.1 | Reduced flicker and visible component version. |
| 0.6 | Per-deck volume, waveform, zoom and seeking. |
| 0.5 | Crossfader. |
| 0.4 | Both decks play through foobar's audio output. |
| 0.3 | Audio decoding, duration and format. |
| 0.2 | Deck A / Deck B panel UI. |
| 0.1 | Initial loadable component. |
