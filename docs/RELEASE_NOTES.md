# DJ Decks 0.41.0-beta.1 — MIDI Learn (x64 / x86)

First MIDI input implementation, off by default, in Settings → MIDI.

- Select an input, enable MIDI, select a command and click MIDI Learn; press a button or move a knob. The learned message is consumed, not executed. Release buttons after learning.
- 24 assignable actions: crossfader, deck volume/tempo, play/pause/common transport, start, unload, Cue, set/toggle A/B loops, BPM matching and key matching.
- Saved per-input assignments, cancellation, per-command removal, confirmed input-map clear, inverted continuous controls and soft takeover enabled by default.
- MIDI 1.0 Note/CC buttons, absolute 7-bit CC and 14-bit Pitch Bend. No relative encoders, jog/scratch, paired high-resolution CC, MIDI clock, SysEx or LED output.
- Refresh/retry after input changes; no device polling. One MIDI-owning panel per foobar process. Bounded input queue; overload stops MIDI until retry. Existing mouse operation remains available.

The owner confirms x64 installation and MIDI Learn working. Both builds pass full native regression suites; simulated WinMM tests cover input parsing, Learn, button edges, takeover, persistence, queue limits, failures and shutdown, and the actual 24-action dispatcher is checked with a mock panel. **The owner has not tested this release in x86 foobar.** Broader controller/driver testing remains welcome. Stems remain experimental and prepared offline.

[Download](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.41.0-beta.1) · [MIDI guide](MIDI.md) · [Changelog](../CHANGELOG.md)
