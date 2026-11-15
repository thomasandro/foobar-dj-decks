# DJ Decks 0.28.0-beta.1 — first x64 beta

New since the public 0.24.0-alpha.1 release:

- Stem cache manager: occupied size and safe cleanup of unused entries, preserving
  active stems, preparations, models and source music.
- Optional ReplayGain (Off / Track / Album) and optional foo_scrobble 1.6.x adapter,
  disabled by default. Wider real-world scrobbling feedback is welcome.
- Custom foobar-style playlist formatting, four presets and optional position
  numbers. Correct default: `%artist% - %title% '('%length%')'`.
- Drag between floating playlists: copy, or Shift to move. Preserve order,
  duplicates, subsongs and the currently playing item.
- Retained queue metadata, asynchronous metadata reload and duration-hook fixes.

On upgrade, select Settings → Playlist display → Artist - Title (duration) again
if an older saved format lacks the quoted parentheses. Custom formats are preserved.

**Stems remain experimental.** Separation needs preparation before playback,
with optional NVIDIA CUDA and CPU fallback; it is not real-time separation.
Preparation time and audio artifacts depend on the machine and material.

The user confirms x64 installation, real-track playback and save/restore, plus
successful operation after installing the packaged beta. Native tests pass.
The x86 beta still awaits Fabcore's feedback; this release is x64 only.

## Downloads

- [Installable x64 component](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.28.0-beta.1/foo_dj-0.28.0-beta.1-win-x64.fb2k-component)
- [Matching complete source](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.28.0-beta.1/foo_dj-0.28.0-beta.1-source.zip)
- [Optional stems installer](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.28.0-beta.1/foo_dj-stems-engine-0.28.0-beta.1.zip)
- SHA256SUMS.txt attached to this release.

The engine installer downloads Python dependencies and the model separately.
The component works without it. Full development history, including unpublished
0.25–0.27.3 milestones, is in [CHANGELOG.md](https://github.com/thomasandro/foobar-dj-decks/blob/v0.28.0-beta.1/CHANGELOG.md).

Thanks to StyxCrosser for the continued testing and feedback, and Fabcore for
helping with x86 testing!
