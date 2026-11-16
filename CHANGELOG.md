# Changelog

## 0.40.0-beta.1 — 2026-11-16 (x64 / x86)

Public beta with both architectures, installable packages, complete matching sources and SHA-256 checksums. Includes the previously unpublished 0.39.0 limiter, detailed below. The user confirmed the installed x64 version working. Both native suites are verified for release; external x86 host feedback remains welcome. Smart Tempo compatibility is verified by its public command GUID and mock-service dispatch; actual analysis/tag writing with an installed version still needs separate verification.

- Escape hides either floating playlist, including when a button has focus; during a drag/selection gesture it cancels the gesture first. Existing lists and deck playback are retained.
- Added themed Empty list / Vider la liste button. Confirmation precedes clearing the whole chosen queue, even with no selection. Music files and loaded audio are retained; this deck's AUTO/preload and owned preview stop, A/B alternation stops, the other queue is preserved, and the empty queue is saved.
- Settings → Audio: BPM analyser Auto / foo_smart_tempo / foo_bpm. Auto prefers the registered Smart Tempo analysis command, falling back to foo_bpm only if Smart Tempo is unavailable. Explicit selection fails visibly rather than switching silently.
- Smart Tempo dispatch uses its public analysis GUID, preserving track subsong and the existing file-release-before-analysis workflow. Optional external component; no analyser code/binary is bundled. Save/validate BPM in the external UI then Refresh tags and manually resume the deck. Key analysis remains foo_musical_key.
- Includes the previously unpublished 0.39.0 limiter. User-confirmed x64 operation; actual Smart Tempo execution/tag writing is not independently established by the native regression suite.


## 0.39.0-beta.1 — previously unpublished development milestone

- Master sample-peak limiter after crossfader, deck volumes, ReplayGain and original/stem mixing; enabled by default at -1 dBFS. Settings → Audio: bypass and -0.3 / -1 / -3 / -6 dBFS ceilings, saved in the profile.
- Stereo-linked instantaneous attack with 80 ms exponential release, no added lookahead latency, no allocation or locking in the sample loop. The headphone Cue/playlist-preview path is unchanged.
- Compact master indicator beside the crossfader: output peak bar, green LIM ready, amber gain reduction in dB held 350 ms, grey bypass, red CLIP held one second for pre-output sample overload with bypass or invalid samples. Invalid samples are sanitized in either mode.
- This limits sample peaks, not intersample/true peaks, headphone output or downstream device processing. It is not an automatic loudness normalizer. This historical development milestone was included in 0.40.0; see current validation above.


## 0.38.1-beta.1 — 2026-11-15 (x64 / x86)

Public beta with both architectures, installable packages, complete matching sources and SHA-256 checksums. Includes the previously unpublished 0.38.0 changes below. The user confirmed active-playlist distribution and Ctrl+A in x64. Both native suites are checked for release; x86 real-host feedback remains welcome. New user-supplied screenshot in both READMEs.

- Ctrl+A selects every track in the focused floating deck playlist, independently for A and B. Handles empty lists and suppresses the translated control character beep.


## 0.38.0-beta.1 — previously unpublished development milestone

- Settings → Playlists: distribute the entire active foobar playlist alternately to A (1,3,5…) and B (2,4,6…), preserving order, duplicates and subsongs.
- Shows source playlist name and per-deck counts before replacing both lists. Does not modify the source playlist, loaded tracks, playback position or mixer; disables AUTO/previews using the existing playlists-only import path.
- Validates list/storage limits before changes and creates a recovery snapshot; aborts if the backup fails. Restore through Settings → Playlists → Restore before the last import or distribution.
- Empty/absent/oversized playlists and pending deck loads leave existing state unchanged. No automatic track loading or playback start.


## 0.37.0-beta.1 — 2026-11-15 (x64 / x86)

Public beta with both architectures, installable packages, matching complete sources and SHA-256 checksums. Includes every previously unpublished milestone from 0.35.0 through 0.37.0, detailed below: independent/persistent A/B loops, Q alignment of existing points, six-tab settings, tooltip/contrast corrections and session-file save/load/recovery.

The user confirmed the x64 build and session-file workflow working. Both architectures compile and pass native release regression suites; x86 real-host testing remains requested. Stems remain experimental. Historical notes about x86 deferral below describe the local development stage; this release now includes x86.

- Settings → Playlists: export both playlists or a full deck session to versioned UTF-8 JSON; import with an explicit lists-only/full choice, plus one-level recovery of the state before import. No audio files are copied.
- Preserve queue order, duplicates/subsongs, loaded paths/positions, per-deck loop points/manual grids, speed/range, pitch/key lock, Q, volume and crossfader/fade settings. Include saved markers for referenced playlist tracks. Full imports restore paused with loops/AUTO/Cue off; stems/profile output settings are not restored.
- Validate complete documents and merged marker-library capacity before applying. Atomic same-folder file replacement preserves an existing file on failure. Save a recovery snapshot before applying; abort if that save fails. Reject unsupported schemas, malformed data and limits without changing live lists/decks.
- Lists-only imports keep loaded deck playback and reconcile queue current pointers. Full imports close the previous readers safely, retain unavailable references and hand restored positions/per-deck point overrides through asynchronous silence preparation.
- Targeted x64 tests cover actual capture/apply with mock readers, generated Unicode file IO, failed replacement/temporary cleanup, metadata/order/subsong retention, missing/empty/pending decks, validated mutation boundaries and actual existing preparation/loop/settings paths. x86 was deferred during this local development stage; see release validation above.


## 0.36.2-beta.1 — previously unpublished development milestone

- Fix blank/invisible settings toggles and action buttons: their content host now forwards WM_DRAWITEM/WM_MEASUREITEM to the owner-draw handler. The previous contrast test bypassed that parent route; the regression now sends drawing through the real control parent.
- Enabling Q from the deck or Playback tab now quantizes existing A/B points (or A alone) to the displayed beat grid and immediately persists them. Disabling preserves positions. Missing/invalid grids or an impossible pair leave points/Q unchanged with a non-fatal notice.
- Preserve playback/pause and active-loop state. If an active loop’s new bounds exclude the current position, seek its original/stem reader to the new A and clear lookahead. Keep a valid distinct pair near track end when an earlier A grid line is needed.
- Targeted x64 tests cover parent-routed drawing pixels, bound toggle clicks, snapped point persistence, paused/active loops, invalid grids, A-only points, near-end bounds and the existing loop/storage regressions. x86 compilation explicitly deferred at user request.


## 0.36.1-beta.1 — previously unpublished development milestone

- Tooltip responsiveness: invalidate only old/new button feedback rectangles for hover/press/leave; reuse the panel paint bitmap until resize. Skip GDI+ skin/icon work outside the dirty clip. Preserve existing visible rendering.
- Disable tooltip animation/fade locally, set initial delay to 300 ms and button-to-button reshow delay to 75 ms, and retain cached tool registration until layout/language changes. No global Windows settings or monitoring added.
- Fix unreadable dark settings toggles/buttons with explicitly drawn light labels, check marks and keyboard focus. Checkbox click state is driven by the existing bound setting rather than native owner-draw check state. Applies to all tabs, including Playback.
- Regressions exercise actual tooltip setup, stable registration, delays, cached bitmap reuse/resize, owner-drawn text pixels, toggle clicks and existing settings bindings. A separate offscreen benchmark compares the old/new clipped renderer and checks identical visible pixels; it is not a live-host latency test.


## 0.36.0-beta.1 — previously unpublished development milestone

- Replace the lower gear popup with one owned, modeless, dark settings window: Appearance, Audio, Headphones, Playlists, Playback and Stems tabs; immediate changes and preserved existing settings.
- Expose the existing controls directly, including safe headphone endpoint selection/refresh, continuous 0–100% cue volume, preview modes, format presets/custom editor, ReplayGain/scrobble, silence trim and protected stem-cache manager. Add shared fade controls and per-deck Q alongside local shortcuts.
- Reuse the floating window, remember the current tab during the session, support resize/scroll and keyboard navigation, and destroy it with the owning panel. Refresh is event-driven; no new polling timer, host capture or audio-device test is introduced.
- Preserve unknown/custom stored fade, threshold, format and unavailable output choices without resetting them on display. Exclude/recheck the master cue endpoint and reuse the existing trim-preparation cancellation/adoption path.
- Hidden-window regressions exercise all categories in both languages, actual settings row/binding methods with mock services, keyboard/focus layout, reuse, close/reopen, owned teardown and current-value retention.


## 0.35.2-beta.1 — previously unpublished development milestone

- Per-deck Q option beside A/B controls: snap newly set A/B points to the nearest displayed beat-grid line, using the current zoom stride. Off by default, persisted independently in foobar configuration. Existing saved points are not moved.
- Share grid selection/phase between rendering and quantization: manual, accepted estimated or dotted indicative BPM fallback. Respect track/trim bounds and minimum A/B interval without moving a point off-grid. Refuse missing/unusable grids with a non-fatal notice.
- Keep snapped positions in source seconds and persist them through the existing point library. Later tempo/grid/zoom changes do not silently move saved points.
- Native tests cover snapped UI point saves, invalid-grid preservation, nonzero grid phases, coarse visible strides, constrained B placement, missing/invalid grids, rendering geometry and existing loop/persistence behavior.


## 0.35.1-beta.1 — previously unpublished development milestone

- Persist A/B points per canonical file path and subsong in profile `foo_dj-loops.json`, including partial A-only points. Restore on loading on either deck, after normal close/restart, always with looping disabled. Last edit wins for a shared track.
- Save immediately after A/B edits and clear, outside the audio mutex; clear removes the stored record. Reuse atomic temporary-file replacement and retry on normal session save. Report write failures without cancelling playback/AUTO.
- Validate stored types, finite positions, minimum interval and current duration/trim bounds; ignore malformed/missing records safely. No audio/tag modifications.
- Targeted native regressions exercise the actual file reader/writer, failed replacement preserving the previous file, JSON round trips, Unicode/subsongs, partial points, overwrite/clear, invalid bounds and actual UI save calls outside the audio mutex.


## 0.35.0-beta.1 — previously unpublished development milestone

- Independent manual A/B loops on both decks: set A/B, enable/disable, clear; minimum 100 ms. Preserve play/pause when enabling; seek to A. Show source times, waveform markers and bilingual tooltips in the selected theme.
- Split shared audio output blocks at loop boundaries, seek the original or stem reader to A and clear processing/lookahead buffers. Preserve the other deck clock, tempo/key/volume/cue behavior; no UI timer controls loop timing.
- Preserve AUTO mode while a loop holds progression; alternating mode waits for both loops to be disabled. Refuse enabling during an in-progress automatic fade. Seeks outside disable the loop, inside retain it; loading resets points. Session-only points, no beat quantization or seamless-click guarantee.
- Keep invalid loop-point notices separate from playback errors, so a point-setting mistake does not cancel AUTO. Also guard the completed-but-not-yet-adopted fade state.
- Harden native test failure detection for negative process exits; allow asynchronous preview completion a generous test-only timeout and use a short unique temporary cache fixture path.
- Regression tests cover actual loop commands, AUTO waiting/resumption, seek behavior, source/stem buffer resets and independent boundaries across three output rates and three speeds, plus hidden layout checks. No foobar access, capture or monitoring.


## 0.34.1-beta.1 — 2026-11-15

Public beta for x64 and x86, with installable packages and a common complete source archive.
Includes all previously unpublished changes from 0.28.0-beta.2 through 0.34.1-beta.1, detailed below: configurable headphone cue, playlist excerpt previews, clickable approximate whole-track waveforms, optional silence trimming/preload, multi-track Shift moves and AUTO seek correction. Stems remain experimental. x64 operation confirmed by the user; x86 compilation/native tests verified, broader host feedback requested.

- Do not cancel A/B alternation when seeking within a deck or returning to start.
  Previously SeekDeck called the generic CancelAutomation before positioning,
  resetting the user's AUTO mode even on ordinary waveform clicks.
- Preserve play/pause, playlist indices and preparation/arming state. During an
  active crossfade, preserve direction/current gain and replan remaining time
  using updated source positions, trimmed ends and speeds. Pending/empty deck
  handling remains non-disruptive; seeking itself does not start paused decks.
- Regression executes the actual SeekDeck method with mock readers. Reproduces
  the previous mode reset, then checks ordinary/paused seeks, pending/empty decks,
  queue preservation, trimmed start and continuous in-progress fade replanning.
  Native tests run without foobar, personal music, screenshots or physical audio.


## 0.34.0-beta.1 — local candidate, not published

- Settings → Silence trimming / preload: independently enable leading/trailing
  silence removal (both off initially). Default threshold -60 dBFS, selectable
  presets -96…-30. Settings affect newly loaded tracks; source files/tags unchanged.
- Scan decoded source samples across all channels, before gain/stems processing.
  Preserve internal pauses and 20ms edge guards. Bound scanning to the first/last
  60s; unknown duration, missing seek support or no detected sound preserve the
  corresponding original edge, including entirely silent tracks.
- Prepare enabled loads in a cancelable low-priority worker: decoder, trim bounds
  and a bounded approximately 250ms source buffer. Adopt on the UI thread; preserve
  queued Play/seek intent and restore pending targets paused across sessions.
- AUTO warms the next item per active queue. Wait for preparation readiness and
  use useful source end for speed-aware fades. Enforce trimmed tail in audio-frame
  consumption, rebase source-position clocks on output-rate initialization, and
  return to trimmed start for replay. This does not guarantee gapless transitions.
- Stop/Release cancels and releases preloads and suspends their automatic reopening
  until explicit playback/load/AUTO. Discard obsolete/end-of-list/manual-mode warm
  readers. Disable analyser/stem actions during a pending track change so they do
  not act on the previous file. Audio/stem rendering retains original time axes.
- Tests cover actual detection, thresholds/channels, guarded edges/internal pauses,
  silent/unsupported tracks, cancellation and speed-aware caps. Execute the actual
  preparation scheduler/load wrapper and AUTO controller using mock decoders:
  warm next, Play/seek intent, readiness gating, suspended release, normal/unknown
  exceptions and disabled legacy loads. Full native/audio suite passed; no foobar,
  user music or physical output opened. Live transition quality remains manual.
- Bilingual guide: docs/SILENCE_TRIM.md. Not installed/published by packaging alone.


## 0.33.0-beta.1 — local candidate, not published

- Make the headphone overview waveform clickable. Map its plot area to full source
  duration and restart the current file/subsong at that position, independently of
  deck transports, loaded items, playlists, master audio and scrobbling.
- Keep the selected 8/15-second length. The combined preset becomes one continuous
  16-second excerpt after a click. Clamp near EOF without wrapping to the beginning;
  seek failures follow the existing preview error/cleanup path.
- Retain the window/overview on seek; immediately move the indicator and preserve
  request generation fencing against stale audio/visual updates. Resume partial
  overview sampling using the existing cache. Same cue endpoint flushes/reuses its
  output instead of reopening it for every click, unless flushing fails.
- Add a hand cursor over the plot and bilingual click hint. Ignore clicks in header,
  margins and status text; closing/stopping and automatic completion still work.
- Test plot bounds/callbacks with hidden windows, actual preview offsets/durations
  with mock decoding, near-EOF clamping, retained visuals and stale generations,
  plus same-endpoint reuse with mock audio output. No foobar, music file or real
  output opened by the test harness. Live interaction/latency remain manual checks.


## 0.32.0-beta.1 — local candidate, not published

- Anchor the preview window beside the originating floating playlist, choosing
  right/left/below/above according to monitor work area and clamping to the screen.
  Reposition when switching playlist; keep the window nonactivating and draggable.
- Plot the whole source track and its source-time playhead. Middle excerpts and
  combined begin/middle previews retain their correct positions on this full axis.
- Optional low-priority overview worker samples 32 distributed regions, with a
  two-second budget checked between decoder calls. Audio does not wait for it;
  partial results are shown progressively. This is approximate sampling, not a
  full peak scan or a guaranteed deadline on an individual slow decoder/I/O call.
- Keep up to eight in-memory overviews keyed by path/subsong/file size/mtime;
  reuse complete results without reopening a sampling decoder and resume partial
  results. Cancel sampling on replacement, stop, tag-writing handoff and teardown.
- Preserve already published overview data when playback metadata arrives later;
  fence all updates by request generation. Actual preview samples enrich the
  whole-track view. UI storage/refresh remain bounded to 1024 bins / 20Hz.
- Tests cover adjacent/work-area placement, full-axis/middle offsets, stale updates,
  overview-before-metadata ordering, sampled coverage, decoder/cache reuse and
  bounded cache eviction using hidden windows and mocked inputs only. Real-device
  performance and perceived latency remain to be checked by the user.


## 0.31.0-beta.1 — local candidate, not published

- Add a small, owned, draggable headphone-preview waveform window matching the
  source deck color. Open without stealing focus; reuse for a replacement excerpt.
  Close on completion, stop, failure or panel teardown; its close button stops cue.
- Build the excerpt waveform progressively from already decoded samples. No extra
  decoder, full-file scan, cache or wait before playback. Bound storage to 1024 peak
  bins and UI refresh to 20Hz. Future waveform bins remain blank until decoded.
- Show the vertical playhead, excerpt elapsed/duration and source-track position;
  concatenate beginning/middle sections in combined mode. Estimate playhead from
  submitted frames minus cue queue/backend latency. Unreported Bluetooth/device
  delay is not compensated and live perceived alignment remains unverified.
- Fence visual updates by request generation; reset on replacement. Test bounded
  peak extraction, offsets, invalid samples, latency/end bounds and stale updates.
  Hidden native window tests exercise painting, dragging hit-test, close callback
  and destruction; mock actual decoding confirms no additional decoder is opened.
- No foobar access/capture, real output or personal music used by tests. Guide:
  docs/PLAYLIST_PREVIEW.md. Live performance/listening confirmation remains manual.


## 0.30.1-beta.1 — local candidate, not published

- Increase playlist excerpts from 5/10 seconds to 8/15 seconds in all five modes,
  including first 8 + middle 8. Keep the saved preset index, short-track clipping,
  centered middle windows and overlap avoidance.
- Show a solid Stop icon instead of headphones on the currently previewed playlist
  entry. Clicking it stops the excerpt; headphones return after completion,
  cancellation or failure through the existing active-row state.
- Update French/English labels, usage guide and duration/offset regression tests.
  Check active-row Stop rendering offscreen, without any foobar capture or access.


## 0.30.0-beta.1 — local candidate, not published

- Add a small headphone button before every row in both floating playlists. Listen
  without loading/seeking a deck or modifying the master mix/list selection. A
  second click stops that track; another row replaces the single shared preview.
- Headphone settings: five saved excerpt presets shared by both lists: first 5/10 s,
  centered middle 5/10 s, or first 5 + middle 5. Clamp short tracks and avoid overlap
  in combined mode; require known duration and seek support for middle excerpts.
- Separate original-file decoder, headphone volume and explicit selected endpoint.
  Previews temporarily override deck cue, then restore it. No stems, deck processing,
  ReplayGain or scrobbling for excerpts. Preserve subsongs; local files only.
- Latest-request cancellation with a generation fence prevents stale audio entering
  a newly configured cue route. Stop/release the reader before analysis tag-writing
  handoff and during panel shutdown. Errors release the input and restore deck cue.
- Native row-button tests preserve selection, suppress deck-loading on double-click
  and cancel outside releases. Test the actual preview method with mock input/sink:
  exact sample counts, centered offsets, stereo/fades, short tracks, cancellation,
  missing output, stale generations, subsongs and decoder cleanup. Runner tests cover
  replacement/errors/recovery. No real audio device, user music or foobar access.
- Includes unpublished configurable headphone routing from 0.29 below and the
  beta.2 multi-drag fix. Guide: docs/PLAYLIST_PREVIEW.md. Live listening still pending.


## 0.29.0-beta.1 — local candidate, not published

- Add a per-deck headphone icon and a Settings → Headphone cue menu. Enumerate
  foobar output endpoints, persist the chosen backend/device and independent
  0–100% cue volume (initially 25%). Cue switches reset off at panel startup.
- Tap the same decoded frames before deck volume and crossfader, after tempo,
  transposition, stems and optional ReplayGain. No additional decoder or transport;
  cue-only listening does not earn scrobbles. Both cue decks are mixed at half gain.
- Separate output worker, bounded 200ms queue, nonwaiting producer and streaming
  sample-rate conversion for forced output rates. Overflow drops old cue audio;
  it never back-pressures the master. No automatic clock/latency correction.
- Reject the exact master endpoint pair; never fall back to the master on failure.
  Show output errors in cue settings, allow retry/device change and explicit release.
  Available endpoints depend on installed foobar outputs; no fixed hardware names.
- First implementation uses a separate stereo output and the existing deck playhead;
  no custom multichannel routing or independent paused-deck preview. Bluetooth may
  add latency; acoustic beat alignment between separate devices is not guaranteed.
- Mock-output tests exercise real cue worker/gains/resampler/queue: silent master
  with audible cue, rate conversion across packet boundaries, bounded backlog,
  stalls, absent/shared/broken endpoints, recovery and shutdown. No physical output
  or foobar host is opened. Live compatibility and listening remain to be confirmed.
- Includes the beta.2 Shift multi-drag fix below. See docs/HEADPHONE_CUE.md.


## 0.28.0-beta.2 — local candidate, not published

- Preserve the complete existing selection when Shift is held before starting a
  drag on a selected playlist row. Previously the native listbox processed that
  mouse-down as a selection edit before the drag payload was captured, potentially
  leaving only one item to move. Copy drags and Shift pressed during a drag keep
  their behavior; move/copy is still decided at release.
- Defer native Shift-click selection until release when there was no drag, so
  range selection remains available. Plain clicks still select a single row;
  Ctrl-click selection remains native. Escape cancels without dropping entries.
- Hidden-window regression reproduces the original selection loss and now passes
  with contiguous/noncontiguous selections, Shift before/during drag, cancellation,
  plain clicks and comparison with native Shift-click selection. No foobar host
  is opened, captured or monitored. Real-host gesture confirmation remains manual.


## 0.28.0-beta.1 — 2026-11-15 (first public x64 beta)

First x64 beta, including previously unpublished 0.25–0.27.3 changes detailed below.
The packaged beta was installed with backup and confirmed working by the user before publication.

- User confirms x64 installation, real-track playback and session save/restore.
  Slow starts/closes predate the component work according to the user and are not
  a demonstrated component-specific blocker. x86 beta awaits Fabcore's feedback.
- Stem separation remains experimental, including preparation, CPU/CUDA fallback,
  mute/gain/Solo and caching. Mark this in the guides and preparation tooltip.
- Since the last public alpha: stem-cache size and safe cleanup, optional ReplayGain
  and foo_scrobble integration, custom playlist formats and optional numbering,
  cross-playlist copy/move, retained metadata and asynchronous reload.
  Wider field testing remains needed, particularly the scrobbling service adapter.
- Supply an x64 installable package, matching full source, optional stem installer
  and SHA-256 checksums. No x86 beta is bundled or claimed.


- Quote literal parentheses in all playlist presets, the initial saved-format
  default and the custom-format example: `%artist% - %title% '('%length%')'`.
  The user confirmed this resolves the missing durations in the running host.
  Existing custom formats are preserved. Select the corrected preset to replace
  a previously saved unquoted default.
- Correct the preset regression expectation. The mock label test does not execute
  foobar's title-format parser and did not detect this syntax issue; its scope is
  limited to callback wiring and Unicode conversion.

## 0.27.3-dev — local development, not published

- Handle null-terminated title-format field names (SIZE_MAX) in the playlist
  duration hook. Previously constructing a string with this sentinel could throw,
  and the exception was hidden by the filename fallback. Leave unrelated fields
  to foobar's formatter. This is a reproduced defect; confirmation in the live
  host is still required for the reported missing durations.
- Regression tests cover the actual duration hook with explicit and sentinel
  lengths, plus the actual QueueLabel method with a mock metadata/format provider:
  accented artist names, minutes, hours, unknown duration and an intentionally
  duration-free custom format. No user music or running foobar is accessed.

## 0.27.2-dev — local development, not published

- Retain foobar metadata handles for both playlists and keep native drop handles
  alive until insertion. Rebuild both retained lists before releasing old references,
  including when the last occurrence moves A/B. Previously only paths/subsongs were
  retained, and missing/incomplete metadata was never explicitly loaded.
- Read missing title/artist/duration information asynchronously through foobar's
  metadata loader for local items only. No tag writing or synchronous decoder calls
  in playlist formatting. Deduplicate requests, serialize batches and do not retry
  inaccessible/untagged files indefinitely. Removed entries prune retry bookkeeping.
- Settings → Playlist display → Reload playlist metadata forces a background
  refresh of both lists. Refresh labels when queued tracks' metadata changes and
  retain the user's current selection. Orphan completion callbacks on panel close.
- Tests execute the real metadata scheduling/retention method with a mock registry:
  native handles, last-copy A/B moves, missing/partial tags and duration, subsongs,
  duplicate requests, force reload, retries and failure cleanup. Live-host rendering
  remains to be checked; no user music is read by the test harness.

## 0.27.1-dev — local development, not published

- Recognize foo_scrobble registry module names with or without .dll, path prefixes,
  case/whitespace differences and the Scrobble component name. Keep the 1.6.x version
  gate and exact service GUID check. The generic refusal now distinguishes missing
  component, unsupported version and unavailable ScrobbleService, reporting the
  detected version/module; Settings also offers a one-shot diagnostic.
- Explicit playlist duration hook for `%length%` and `%length_seconds%`, shared
  by preview and displayed labels. Use cached track length, falling back to the
  matching loaded deck's decoder length. Preserve unknown-length semantics and
  custom expressions that intentionally omit duration. No decoder is opened for
  list formatting. Do not bypass formatting solely because cached file_info is absent.
- Native tests cover detection variants/version rejection and the actual duration
  hook, including long/unknown tracks. Live service discovery/display remain to be
  confirmed by the user; no real scrobbles or foobar captures are made by tests.

## 0.27.0-dev — local development, not published

- Settings → Playlist display: four title-format presets and a free-form foobar
  title-format editor with expression validation and a cached-metadata preview.
  Default: `%artist% - %title% (%length%)`. The format is saved and shared by both
  floating lists and the Current/Next previews, without changing tags or list order.
- Automatically added playlist-position numbers are now optional and off by default.
  Track-number tags can be requested explicitly in a custom format or preset.
  Missing cached metadata still falls back to filename/subsong, without a decoder.
- Drag selections between the two floating lists: copy by default, Shift to move.
  Preserve source order, duplicates, subsongs and current-entry indices. Drop at the
  destination row or end; an empty destination deck loads its first new entry paused.
- Keep same-list reordering, add target-list edge scrolling, Escape/capture-loss
  cancellation and ignore releases outside the two lists. Never remove source
  entries until destination capacity and serialized-size limits are validated.
- Native tests cover 248 model-transfer cases, atomic capacity rejection, hidden
  window dispatch in both modes, cancellation, teardown and display defaults.
  Real host pointer interaction and format-dialog behavior remain manual checks.

## 0.26.0-dev — local development, not published

- Settings: optional ReplayGain Off / Track / Album with gain + peak processing,
  SDK fallback behavior, zero extra preamp and smoothed changes. Apply once to each
  deck, including stems, without altering source files, caches or global preferences.
- Optional foo_scrobble 1.6.x adapter, disabled by default. Counts audible output
  time per deck and queues qualifying artist/title-tagged tracks through its existing
  authenticated asynchronous service. Honors the global Scrobble tracks switch.
- Pauses, seeks, inaudible decks and muted/zero-volume stems cannot earn time.
  Suppress the deck path while normal foobar playback holds the same item. One
  queue attempt per playback occurrence; no competing Now Playing notifications.
- Version/service availability checks; direct-adapter metadata and filtering limits
  are documented. Custom foo_scrobble mappings/skip/library filters are not inherited.
- Native listening-policy tests, actual ScrobbleTick mock-backend tests and actual
  SDK ReplayGain scale cases. No real submission, host capture or monitoring.
- [Usage and remaining manual validation](docs/SCROBBLING_REPLAYGAIN.md).

## 0.25.0-dev — local development, not published

- Gear Settings → Stem cache opens a French/English manager showing prepared-stem
  size/count, Refresh size and Clear unused cache, with freed-space/result reporting.
- Scan/delete run on a below-normal background thread outside the audio mutex.
  Active stems and preparations are retained; select Original audio or Stop/Release
  on both decks and reopen the manager to make their caches eligible too.
- Cleanup only touches five expected files in direct 64-hex cache folders.
  Worker byte locks and file-handle preflight protect busy entries, including other
  processes. Linked, read-only and unknown data remain; no recursive deletion.
  Source music, the model/runtime, sessions and temporary jobs are preserved.
- Refresh both decks' cache availability after closing the manager. Original audio
  is not automatically replaced or stopped. There is no automatic cache eviction.
- Added native cleanup tests and a manual pre-beta test sheet, separating automated
  evidence from outstanding installation, listening and long-session validation.

## 0.24.0-alpha.1 — 2026-11-14 (public prerelease)

Sixth public alpha, based on 0.24.0-dev. Includes every previously unpublished
milestone from 0.20.1 through 0.24.0, detailed below. Public version/packaging
changes do not replace the locally installed development component.

- Four local stems: drums, bass, instruments and vocals; independent mute,
  0–100% gain and additive Solo on each deck, with smooth level transitions.
- External Demucs worker with progress, cancellation and persistent validated cache.
  NVIDIA CUDA is preferred automatically; GPU detection/inference errors retry on CPU.
  Optional Python engine installer is supplied separately; no audio is uploaded.
- BPM matching and beat SYNC consider half/double equivalents as well as exact BPM,
  selecting the closest valid speed in the chosen range; exact wins ties.
- Glossy/Flat themes, eight deck colors, saved settings, corrected status clipping,
  clearer BPM/key analysis and matching icons, dedicated gear Settings menu.
- Floating playlists have integrated themed titles/close controls and retain resize,
  title dragging, keyboard actions and saved list contents.
- Updated French/English READMEs, direct downloads, installation guide and supplied
  screenshot. Thanks to StyxCrosser for sustained testing and interface suggestions.

Validation: release x64 build/native suite, 60 real audio-processing cases,
256 stem mute/solo combinations, actual CUDA preparation/cache/cancellation and
real CPU preparation after simulated CUDA failure. GTX 1070 Ti synthetic 30-second
benchmark (three runs): 53.04 s CPU/two threads vs 3.01 s CUDA median preparation.
First result including process initialization: 73.98 s CPU vs 13.92 s CUDA.
This is an offline preparation benchmark, not real-time playback or listening validation.
Separation remains approximate and completes before activation; cache has no automatic
eviction. The release remains experimental/unsigned and Windows x64 / Default UI only.

## 0.24.0-dev — local development, not published

- Stem preparation automatically uses NVIDIA CUDA when available, with the same
  htdemucs model, float32 precision and separation settings as the CPU engine.
- CUDA detection failure, insufficient GPU memory, kernel/driver errors or invalid
  GPU samples trigger one retry using a fresh CPU model. CPU failures remain errors;
  partial results are never published. Backend and fallback reasons are logged.
- The optional installer selects PyTorch 2.5.1 CUDA 11.8 for detected NVIDIA GPUs,
  with a CPU package fallback on installation failure and an explicit -CPUOnly option.
  Existing CPU-only installations continue to work; rerun the installer to enable CUDA.
- Cache identity stays unchanged: existing stems remain usable. Playback, stem
  levels, Solo and source music are unchanged; preparation still completes before use.
- GTX 1070 Ti benchmark on a synthetic 30-second excerpt: median preparation
  53.04 s CPU (two threads) versus 3.01 s CUDA over three runs; not a live-DJ guarantee.

## 0.23.2-dev — local development, not published

- Replace the FR/EN footer label with a vector gear and bilingual Settings tooltip.
- Settings groups Language in its own submenu alongside Theme and Deck A/B colors.
- Existing saved preferences are retained; no audio or playback changes.

## 0.23.1-dev — local development, not published

- More explicit BPM/key pictograms inspired by StyxCrosser's musical-symbol suggestion:
  musical note + BPM, flat mark + KEY for analysis; magnifier denotes calculation.
- BPM/key matching use a musical mark + equals sign and BPM/KEY label, distinguishing
  matching from analysis. Existing bilingual action tooltips remain in place.
- Musical marks are vector-drawn, independent of Unicode music font availability;
  glyph render tests check distinctness at small/large sizes in both themes.
- No audio behavior changes. No foobar capture, monitoring or launch for verification.

## 0.23.0-dev — local development, not published

- FR/EN options menu now includes Glossy or Flat/subdued theme and independent
  deck color selections: cyan, orange, blue, purple, green, rose/red, gold and grey.
- Theme and palette choices persist with foobar configuration and apply to decks,
  waveforms, floating playlist headers, buttons and selection colors immediately.
- Flat mode uses a neutral dark backdrop and subtle accent borders, without glossy
  gradients, reflections or shadows; hover and pressed feedback remain distinct.
- Deck status text uses measured glyph width, extra right padding and a separate
  title rectangle. Long headings use ellipsis rather than overlapping status text.
- Native tests cover palette routing, both themes/states and FR/EN status fit at
  several font sizes and panel widths, without any host/window capture.

## 0.22.0-dev — local development, not published

- Auto BPM compares the other deck's effective BPM at ×1, ×½ and ×2. Choose the
  in-range candidate closest to the destination's current speed; exact wins ties.
- Both BPM matching and beat SYNC use this selection. SYNC aligns against the
  corresponding virtual reference grid, retaining its original phase anchor.
- Speed labels show ½×A/B or 2×A/B while the effective BPMs have that relationship.
  Tags, waveforms and saved manual grids are not rewritten.
- Tests cover both directions, adjusted reference speed, all ranges, ties, outside
  candidates, invalid values and continued half/double phase alignment over time.

## 0.21.1-dev — local development, not published

- Per-stem 0–100% volume and Solo on both decks: drag/wheel for level, right-click
  resets to 100%; Shift+wheel for finer steps. Multiple solos can be combined.
- Solo temporarily overrides mutes without destroying the prior mute mask or levels.
  Clearing solos restores the previous mix. New tracks reset levels and solos;
  Original clears solos, while Enable all four also clears solos explicitly.
- Smooth gain ramps before SoundTouch, no cache/model change or additional inference.
- Native tests cover all 256 mute/solo combinations with varied gains, zero/invalid
  levels, deck independence, restoring mutes, control geometry and existing audio cases.

## 0.21.0-dev — local development, not published

- Local four-stem preparation: drums, bass, other instruments and vocals, with
  progress/cancellation and a reusable source/subsong/version-keyed cache.
- Per-deck STEMS menu and four on/off buttons; explicit all-stems/original selection.
- Float PCM streams share one clock and mix before the existing SoundTouch path.
  Preserve speed, pitch/key lock, seek/SYNC, volume and crossfade; smooth mute ramps.
- External isolated Demucs 4.0.1/htdemucs CPU worker; no AI libraries in the host.
  Atomic complete-cache publication, bounded validation, source-change checks,
  one job at a time, own-process lifetime job and restricted handle inheritance.
- Cancel preparation on source change/release/unload; do not reopen tagged/released files.
  Local input export uses installed foobar decoders and retains subsong identity.
- Installer for an optional dedicated engine; no source music modification/upload.
- Native cache/mask tests, 60 actual audio cases and real external-model integration
  on synthetic stereo audio. See [stems usage and limits](docs/STEMS.md).
- First version: CPU only, 30-minute/1-GiB export limits, no automatic cache eviction,
  no stem solo/gain, and original audio on newly loaded tracks. Live-host checks pending.

## 0.20.1-dev — local development, not published

- Replace the floating playlists' Windows title/frame with an integrated glossy
  black header, cyan/orange Playlist A/B title and matching hover/pressed close button.
- Preserve title dragging, all eight edge/corner resize directions, minimum size
  and standard system close. The close button hides the window without clearing
  entries or invoking any deck/audio action.
- Hidden-window regression tests cover caption-free styles, resize/title hit tests,
  header/list separation and close-button preservation of list contents.
- No foobar host was accessed or captured; live-host appearance remains to be checked.

## 0.20.0-alpha.1 — 2026-11-14 (public prerelease)

Fifth public alpha, based on 0.20.0-dev. Includes the previously unpublished 0.19.0,
0.19.1 and 0.20.0 development milestones detailed below. Version/docs packaging
changes only; the installed development build is not changed by this publication.

- Saved per-deck playlists: Current/Next previews, two independent floating windows,
  multi-track drops, multi-selection reordering/removal and double-click loading.
- Floating playlists share dark rows, cyan/orange accents and glossy black buttons.
- Independent playlist auto-next or alternating A/B; refill the silent deck for the
  next transition, finish remaining entries if the other playlist is exhausted.
- Fade duration presets 0–30 seconds; equal-power/linear curve; audio-frame clock
  pauses with playback/output stalls, with speed-aware end triggering.
- Manual takeover and released-file/error handling; saved auto-next/fade preferences.
- User-supplied playlist screenshot in both READMEs and release materials.
- Thanks to StyxCrosser for the popup-playlist suggestion and earlier interface/file
  feedback: https://hydrogenaudio.org/index.php/topic,130167.msg1090621.html#msg1090621

Build/native tests passed. Same-deck auto-next is not gapless. Real-host automatic
transitions, long sessions, clean-profile installation and shutdown/unload still
need validation. The supplied image illustrates playlists; AUTO controls are not shown.


## 0.20.0-dev — local development, not published

- AUTO menu on each deck: independent playlist auto-next or mutually exclusive
  alternating A/B playback, refilling the silent deck with its next queue entry.
- Shared crossfade duration presets 0–30 seconds; equal-power or linear curve.
- Speed-aware end trigger and audio-frame-driven fade, suspended during pause/stalls.
- Alternating transport pauses/resumes both decks during overlaps; remaining entries
  finish sequentially if the other playlist is exhausted. No automatic repeat.
- Manual crossfader/seek/load/release cancels alternation. Released readers remain
  released, and failed incoming tracks stop automation without repeated retries.
- Persist independent auto-next, duration and curve; alternation is explicitly armed
  each session. Actual controller, fade, layout and existing audio regression tests.
- [Usage and validation limits](docs/AUTOMATIC_PLAYBACK.md): independent auto-next
  is not gapless, alternating transitions are experimental and need live-host testing.

## 0.19.1-dev — local development, not published

- Floating playlists now share the decks' dark background, glossy cyan/orange rim,
  black glossy buttons with hover/pressed feedback, Segoe UI and tinted selected rows.
- Alternating dark rows, accent selection marker, ellipsized titles and keyboard focus.
- Preserve native multi-selection, drag/drop and keyboard behavior. Button widths adapt
  to the window. Standard Windows title bar and scrollbar remain.
- Native offscreen owner-draw tests cover both accent colors, unselected row contrast
  and distinct pressed button rendering, without accessing or capturing foobar.

## 0.19.0-dev — local development, not published

- Two-line Current/Next playlist preview and black open-list button on each deck.
- Independent modeless floating playlist windows; both can be open simultaneously.
- Ordered multi-item drops from native foobar, compatible Smooth Playlist/Browser
  payloads and Explorer files. Appends on deck drops, inserts at the drop row in lists.
- Multi-selection drag reordering, Move up/down, Remove/Delete and double-click/Enter
  loading. Removal affects entries only and does not interrupt the loaded track.
- Persist both lists, duplicate occurrences, order and subsongs in the existing session.
  Migrate old loaded tracks; preserve explicitly emptied new lists and offline paths.
- Save JSON and write/flush session files outside the audio-engine mutex.
- Added queue-model and hidden native-window regression tests; no foobar access.
- See [playlist usage and current limits](docs/PLAYLISTS.md). Live-host OLE integration
  and restart restoration remain pending manual validation. No auto-advance in this build.

## 0.18.0-alpha.1 — 2026-11-14 (public prerelease)

Fourth public alpha, based on 0.18.0-dev (detailed below), responding to community
feedback. Public packaging changes version/documentation only.

- Visible button hover highlights and depressed press appearance, preserving black faces.
- Single-row lower strip: common transport, crossfader, FR/EN menu; 60 pixels recovered
  for deck/waveform space. Version is available in the language-menu tooltip.
- Separate Stop/Release file button on each deck; decoder and waveform readers close,
  position/settings remain, and Play reopens at the retained position. Both decks are
  released if they use the same physical file. FREE/LIBRE indicates released state.
- Native tests include non-overlapping footer/control geometry, distinct offscreen
  normal/hover/pressed rendering, reader release and 30 synthetic audio cases.

Still experimental/unsigned. Live-host interaction, tag-writing regression, clean-profile
installation and extended shutdown/unload validation remain pending. No new screenshot
was taken; the README image is explicitly identified as the earlier 0.17.1 interface.


## 0.18.0-dev — local development, not separately published

- Button hover highlights and depressed press appearance; mouse-leave/capture loss
  clears interaction state. Existing commands still act on mouse down.
- Compact single-row lower strip: common transport, crossfader and FR/EN menu.
  Recover 60 pixels of deck/waveform height; version remains in the language tooltip.
- Explicit per-deck Stop/Release button closes decoder and waveform readers while
  retaining position/settings. Also releases another deck using the same file.
  Press Play to reopen at the retained position. Released decks show FREE/LIBRE.
- Geometry tests cover footer/control separation; offscreen pixel checks distinguish
  normal, hover and pressed states. Host interaction remains to be verified by the user.

## 0.17.1-alpha.1 — 2026-11-14 (public prerelease)

Third public alpha, collecting the local milestones 0.16.0-dev, 0.16.1-dev,
0.17.0-dev and 0.17.1-dev detailed below. They were not separate GitHub releases.
The public build changes the version label/documentation from 0.17.1-dev.

- Automatic harmonic key matching using the nearest semitone transposition, without
  changing tempo: same-mode exact matching or opposite-mode relative major/minor.
- Release loaded file readers before BPM/key analysis so external plugins can write tags;
  keep decks paused until explicit Play, reopening at the retained position.
- Visible tag-refresh feedback with raw/effective BPM and key values.
- Glossy cyan/orange molded-plastic casings, black raised vector-icon buttons,
  native bilingual tooltips, larger pictograms and compact aligned control rows.
- User-supplied screenshot added to both READMEs and release assets; appearance approved
  by the user. This does not establish broad host stability or audio-quality validation.
- Native regression/audio/graphics tests passed; known runtime limits remain documented.


## 0.17.1-dev — local development, not separately published

- Closer to the approved glossy-plastic concept: raised rounded rims, curved broad
  reflections, lower-edge highlights and soft button shadows.
- Replace stretched action bars with compact, taller black buttons and larger icons.
- Put grid tools, transposition, harmonic matching and key lock on one aligned row.
  Keep separate beat/BPM matching controls beside the speed settings.
- Preserve numeric values and bilingual tooltips; adapt hit areas to the new layout.

## 0.17.0-dev — local development, not separately published

- Glossy molded-plastic appearance: cyan/orange deck casings, black rounded buttons,
  dark metadata/waveform displays and high-contrast numerical values.
- Vector pictograms for transport, BPM/key analysis, metadata refresh, beat-grid tools,
  key lock, harmonic matching, beat SYNC and BPM matching. Existing commands retained.
- Native French/English hover tooltips for controls, sliders, range and language.
  Tool rectangles follow panel resizing; tooltip resources are destroyed with the panel.
- Crisp procedural icon rendering without raster scaling, WebView2 or external assets.
- Offscreen skin/geometry tests at 900x650, 1280x720 and 1920x1080. Live-host
  appearance/tooltips remain for user verification; no host capture or monitoring.

## 0.16.1-dev — local development, not separately published

- Before external BPM/key analysis, pause and release deck decoders plus waveform
  readers for every deck holding the same physical file (including different subsongs).
- Keep the file released through asynchronous analysis and result/tag-write dialogs.
  After saving or cancelling, press Play to reopen at the retained position.
  Seeking/SYNC do not reopen a released decoder prematurely.
- Retain speed, range, transposition, metadata and manual grids. Rebuild interrupted
  waveforms after explicit resume; no automatic restart or completion polling.
- Refresh tags now reads fresh file BPM with database fallback, preserves database
  key metadata, and shows tagged/effective BPM and key values instead of silent success.
- Reader-release regression tests verify decoder ownership is dropped, audio buffers
  cleared and position retained. Live-host tag-writing still requires verification.

## 0.16.0-dev — local development, not separately published

- Per-deck KEY = A/B (TON = A/B in French) chooses the nearest transposition
  setting to match the other deck's effective tagged key, including its transposition.
- Same-mode keys match exactly; opposite modes target the relative major/minor
  sharing the same Camelot number. Transposition cannot change major into minor.
- Does not modify speed, BPM, position, transport or the reference deck.
- Rejects missing/unknown tags and non-neutral vinyl pitch; enable key lock first.
- Matching respects the +/-12 semitone range and saves the selected transposition.

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
