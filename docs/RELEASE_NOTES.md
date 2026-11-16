# DJ Decks 0.40.0-beta.1 — 2026-11-16

Beta for foobar2000 2.x / Default UI, x64 and x86. Choose the package matching foobar's architecture.

## New since 0.38.1

- Master sample-peak limiter, enabled by default at -1 dBFS. Settings → Audio: bypass or -0.3 / -1 / -3 / -6 dBFS ceilings. Stereo-linked immediate attack and 80 ms release, without added lookahead latency. Processes the main mix after crossfader/volume/ReplayGain and original/stem mixing; headphone cue and previews remain independent.
- Master output peak bar beside the crossfader. Green LIM: active; amber and negative dB: gain reduction (350 ms visual hold); grey LIM OFF: bypass; red CLIP: sample overload with bypass or invalid input (one second hold). Invalid samples are sanitized in either mode.
- Escape hides either floating playlist, including with button focus; during a drag gesture it cancels the gesture first. Lists and playback are retained.
- Empty list / Vider la liste: confirmation before clearing the entire selected deck queue, even with no selected rows. It keeps music files and the loaded track, stops that deck's AUTO/preload and preview plus A/B alternation, preserves the other queue, and saves the empty list. Export a session before clearing if you want a backup; there is no dedicated undo for clear.
- Settings → Audio → BPM analyser: Auto / foo_smart_tempo / foo_bpm. Auto prefers Smart Tempo's registered analysis command, otherwise foo_bpm. Explicit selection never silently changes provider. The file is released before dispatch, preserving subsong and existing tag-writing workflow. Keep BPM as the analyser's output tag, save/validate the result, Refresh tags and manually resume the deck. Key analysis is unchanged.

## Validation and limits

The installed x64 version is user-confirmed. Both builds pass native regression suites, including limiter impulses/rates/stereo/bypass/release, actual analyser dispatch with mock services, clear/cancel behavior and hidden playlist/settings/tooltips tests. No live x86 foobar host loading is claimed. Actual Smart Tempo execution/tag writing with installed releases is not established by simulated-service tests; versions lacking the expected public command report unavailable. No analyser code or binary is bundled or installed.

The limiter controls sample peaks, not intersample/true peaks, headphones or downstream device processing; sustained heavy limiting may audibly pump/distort. It is not loudness normalization. Stems remain experimental and offline-prepared, with an optional Python/PyTorch engine requiring 64-bit Windows even with foobar x86. Beat/grid alignment remains experimental; loops may click at edges. Profile limiter/provider choices are independent of session files.

[Changelog including unpublished 0.39.0](../CHANGELOG.md) · [Limiter guide](MASTER_LIMITER.fr.md) · [BPM analysers](BPM_ANALYSERS.md)
