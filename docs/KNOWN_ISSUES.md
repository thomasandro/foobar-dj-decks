# Known issues and scope

## Release blockers / checks still outstanding

- Install and load the actual alpha package in a clean portable x64 foobar profile.
  No host has been launched by the release-preparation scripts.
- Investigate shutdown occasionally exceeding 40 seconds. Normal-close waits timed
  out repeatedly during development; some closes completed later and others were
  forced with the user's approval. Cause and attribution remain unproven.
- Startup freezes were observed even with DJ Decks unloaded. One cleared after
  closing the companion app, then did not recur immediately on reopening it.
  This does not establish a cause or clear this component of involvement.
- Runtime validation of French/English switching, saved language, startup restoration,
  artwork, output-device changes and manual-grid persistence in a fresh profile.

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

- Default UI element only; no native Columns UI panel, no headphone/cue routing.
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
