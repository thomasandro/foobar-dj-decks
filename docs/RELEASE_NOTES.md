# DJ Decks 0.42.0-beta.1 — relative MIDI encoders and jog navigation (x64 / x86)

- Four explicit relative CC formats: 1/127 two’s complement, 65/63 offset 64, 1/65 signed magnitude and 17/15 offset 16. No automatic protocol detection; inversion reverses direction.
- Relative volume/crossfader/tempo changes use the current software value, skip absolute soft takeover and clamp to the allowed range. Per-step sensitivity: 0.1 / 0.25 / 0.5 / 1 / 2 / 5 percent of the control range.
- Jog A/B seek navigation, paused or playing, at 10 / 25 / 50 / 100 / 250 / 1000 ms per step (default 50 ms). Movement coalesces to one seek per deck per UI tick; up to 16 steps per message and 30 seconds accumulated per tick.
- Jog does not start playback, scratch audio or temporarily bend tempo. Existing seek rules apply: trim limits, AUTO intent retained, and an active loop is disabled if seeking outside it. Pending loads and unloaded readers are skipped. Repeated seeks can audibly interrupt playback depending on the decoder/output.
- Legacy 0.41 assignments retain their IDs and absolute mode. Version 2 stores format/sensitivity separately from the old v1 configuration, preserving prior mappings for rollback. Changes made in 0.42 do not appear when reverting to 0.41.

Setup: Settings → MIDI → choose a continuous target or A/B jog → choose the controller’s relative format and sensitivity → MIDI Learn → turn the control. Existing CC mappings can change format directly; an absolute Pitch Bend mapping must be relearned as CC for relative mode.

The owner confirms the installed x64 release working. Both full native suites and final MIDI regressions pass, using simulated input and a mock panel for dispatch/jog aggregation. **The owner has not tested this release in x86 foobar.** Broader hardware/driver tests remain welcome. No proprietary HID, jog touch sensor, SysEx, LED/motor output, MIDI clock or scratch support. Stems remain experimental and prepared offline.

[Download](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.42.0-beta.1) · [MIDI guide](MIDI.md) · [Changelog](../CHANGELOG.md)
