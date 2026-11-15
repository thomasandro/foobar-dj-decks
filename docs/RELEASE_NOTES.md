# DJ Decks 0.38.1-beta.1 — 2026-11-15

Beta for foobar2000 2.x / Default UI, x64 and x86. Choose the architecture of foobar.

## New since 0.37.0-beta.1

- Settings → Playlists → Distribute active foobar playlist: copies its entire current order, tracks 1/3/5… to A and 2/4/6… to B. Preserves duplicates/subsongs; source playlist is unchanged.
- Confirmation shows the source name and counts before replacing both deck playlists. A recovery snapshot is required before changes; restore it from the same settings tab. Only one recovery snapshot is retained, shared with session imports.
- Loaded deck tracks, playback position and mixer are retained. AUTO and playlist previews stop; nothing starts automatically. Load the first entry of each new list and choose the desired AUTO modes to play through from the beginning.
- Empty/absent/oversized playlists or pending deck loads leave state unchanged. At most 20,000 entries total, 10,000 per deck and 3 MiB serialized references.
- Ctrl+A selects all tracks in the focused floating playlist, independently on A and B, including correct empty-list behavior.
- Updated screenshot provided by the user.

The x64 workflow is user-confirmed. Both architectures compile and pass native regression tests; real-host x86 feedback is welcome. No foobar host is accessed or launched for release checks.

Stems remain experimental and offline-prepared. The optional Python/PyTorch engine requires 64-bit Windows even with foobar x86. The component works without it. Q/SYNC depend on the grid; loops may click at edges. There is no limiter. Session files reference audio paths and do not embed tracks/stems; review private paths before sharing.

[Detailed changelog](../CHANGELOG.md), including unpublished 0.38.0.
