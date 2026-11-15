# Known issues and scope

## A/B loops and Q

Points persist per track/subsong; enabling Q aligns them to displayed grid lines at the current zoom. Dotted grids have indicative phase. This is not continuous beat tracking, and decoder seeks/processing resets can produce loop-edge clicks/artifacts. AUTO waits while looping; see [AB_LOOPS.md](AB_LOOPS.md).


## Beta status — 0.37.0-beta.1 x64 / x86

- User-confirmed: x64 installation, playback on real tracks, save/restore.
  The installed x64 0.37.0 beta and session-file workflow were confirmed working by the user.
- Slow starts/closes predate this component work according to the user; no
  component-specific cause has been demonstrated, so this is not a beta blocker.
- Both architectures compile and pass native tests. External x86 host feedback is still requested.
- Stems remain experimental. Preparation time, separation artifacts and recovery
  from GPU/decoder errors depend on the machine and source material.
- Additional profiles/devices, narrow panels/DPI, output reconfiguration and the
  foo_scrobble service adapter still benefit from wider beta testing.

## Session files

- Session files reference audio paths; no media/stems are embedded or relocated. Full restore is paused, loops/AUTO/Cue off; stems return to original audio.
- Current silence settings bound restored positions/points. Only a one-level pre-import recovery snapshot is kept. Review private paths before sharing. See [SESSION_FILES.md](SESSION_FILES.md).

## Audio and synchronization

- No limiter. Correlated full-scale decks can exceed 0 dBFS at the crossfader centre.
- Linear source sample-rate conversion. SoundTouch supplies tempo/rate processing;
  key lock can produce audible artifacts and adds processing latency.
- Seek/phase SYNC is not sample-accurate; SoundTouch, decoding and output buffering
  can affect perceived phase. Synthetic tests are not a guarantee on music.
- Automatic phase fitting uses local broadband RMS rises and the tagged BPM, not
  a full beat-tracking/downbeat model. Real dance tracks can be rejected. Piano,
  rubato, syncopation, tempo drift and incorrect/half/double BPM tags are problematic.
- Dotted grids show indicative tagged spacing; their phase is uncertain. Manual
  grids are user anchors, not proof of correct tempo or meter. SYNC is one-shot.
- Auto BPM/SYNC considers half/double equivalents; no multi-anchor tempo map or automatic BPM-tag rewriting.
- Loading/seek operations and worker joins still have synchronous paths; slow
  storage, artwork providers or decoder calls can delay the UI and shutdown.
- Waveforms are rebuilt for each load and held in memory; very long files are costly.

## New controls and metadata

- External BPM/key command invocation and completion still need live-host tests.
- Custom analyser tag names must match the supported fields.
- Harmonic compatibility uses tags, not audio measurements; extreme tempo and pitch settings can introduce artifacts.
- New controls need live layout validation in narrow panels and at different DPI scales.

## Integration

- Default UI element only; no native Columns UI panel. Separate stereo headphone cue is available (see HEADPHONE_CUE.md); wider real-device testing remains welcome. No custom multichannel cue matrix.
- Separate output instance can contend with normal playback/exclusive outputs.
  The deck engine continues sending silence when idle after first start.
- Standard playback callbacks, DSP chains, replaygain, main volume, scrobbling and
  “now playing” integrations must not be assumed to control or report these decks.
- Multichannel input currently takes the first two channels; it is not a proper downmix.
  Mid-stream audio format changes end deck playback.
- The two-deck session and language are shared for the component, not isolated per
  panel instance. Multiple simultaneous DJ Decks panels are not a supported use case.
- User artwork providers, decoder formats and output backends vary; only limited
  development-profile use is known. No claim of broad device/format compatibility.
- Output-device reconfiguration and removal/unload during active decoding need testing.

## Files and reporting

No user sessions, music, personal paths, private screenshots, debug dumps or development logs
belong in a source release. Redact those from issue reports. Retained profile JSON
is not automatically removed on component uninstall.
Screenshots explicitly supplied for publication are included under docs/images.
