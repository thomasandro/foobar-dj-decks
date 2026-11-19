# DJ Decks 0.43.0-beta.1 — temporary MIDI jog speed correction (x64 / x86)

- Select seek or temporary speed correction independently for A/B in Settings → MIDI, after selecting A/B jog. Seek remains the default; existing relative mappings are retained.
- Maximum ±0.5/1/2/4/8%, default ±2%. Each signed tick adds one quarter of the maximum. Forward accelerates, backward slows; inversion supported. Hold 120 ms, return over 180 ms. Audio-thread expiry prevents a stalled UI from leaving correction applied.
- Base tempo, selected range and saved session speed stay unchanged. No seeks or automatic play in bend mode; existing key lock applies. Paused/pending/unloaded decks ignore bend.
- Transient JOG percentage beside base speed; transport/load/MIDI changes clear bend. Mode and maximum saved independently per deck.

No scratch/touch/HID/LED support. Installed x64 version confirmed working by the owner. I have not tested x86 in foobar. Broader physical controller testing is welcome. No foobar capture or surveillance.
