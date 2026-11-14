# DJ Decks for foobar2000 (`foo_dj`)

[Français](README.fr.md) · [Changelog](CHANGELOG.md) · [Build](docs/BUILD.md) · [Known issues](docs/KNOWN_ISSUES.md) · [Hydrogenaudio discussion](https://hydrogenaudio.org/index.php/topic,130167.0.html)

Two independent DJ-style decks inside foobar2000, with a mixer and waveforms.
**0.20.0-alpha.1 is an experimental alpha release.**

## ⬇ Download the component

**[Download DJ Decks 0.20.0-alpha.1 for Windows x64](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.20.0-alpha.1/foo_dj-0.20.0-alpha.1-win-x64.fb2k-component)**

Installable `.fb2k-component` package for foobar2000 2.x x64.
In foobar2000: **Preferences → Components → Install…**, then restart.

[Download the alpha and matching source](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.20.0-alpha.1)
Development was AI-assisted; audio and threading review and reproducible bug reports
are particularly welcome.

![DJ Decks glossy cyan/orange interface](docs/images/dj-decks-playlists-0.20.png)

*User-supplied screenshot of the themed floating playlists; the AUTO menu is not shown.*

Thanks to [StyxCrosser](https://hydrogenaudio.org/index.php/topic,130167.msg1090621.html#msg1090621)
for suggesting playlist popups and for the UI/file-release feedback.

## Features

- Independent load, play/pause and seek for A and B; shared start/pause/swap button.
- Per-deck volume; equal-power or linear crossfader with smoothed gain changes.
- Waveforms with zoom, elapsed/remaining-region shading and clickable seek.
- Front covers, artist/title on separate scrolling lines, French/English interface.
- Tagged BPM display; selectable ±8%, ±16% or ±50% speed range, vinyl or SoundTouch key-lock mode.
- One-click tagged-BPM matching and experimental beat-phase SYNC within the selected range.
- Independent pitch transposition from −12 to +12 semitones, without changing tempo.
- One-click harmonic key matching: same key or relative major/minor, nearest transposition.
- Glossy cyan/orange casings, black icon buttons with hover/pressed feedback and bilingual tooltips.
- Compact lower strip with common transport, crossfader and FR/EN menu.
- Explicit Stop/Release file button on each deck; Play reopens at the retained position.
- Camelot + standard musical key labels, e.g. 8A (Am), and colored compatibility badges.
- Per-deck BPM/key analysis through optional foo_bpm / foo_musical_key components;
  metadata refresh without reloading audio. [Analysis details](docs/DECK_ANALYSIS.md).
- Solid estimated/manual beat markers; dotted indicative BPM grids when detection is uncertain.
- Manual grid shift, “Beat here”, Auto reset; grids remembered per file and subsong.
- Loaded tracks and positions restored paused after a normal close.
- Per-deck saved playlists with two-line Current/Next previews and independent themed floating windows.
- Ordered multi-track drops, multi-selection reorder/removal and double-click loading. [Playlist guide](docs/PLAYLISTS.md).
- AUTO: independent auto-next or alternating A/B with 0–30 second fades, equal-power/linear curve. [Automatic playback](docs/AUTOMATIC_PLAYBACK.md).
- Native foobar/Explorer drag-and-drop; optional compatible Smooth Playlist/Browser text payloads.

## Requirements and installation

Windows x64 and foobar2000 2.x x64, Default User Interface. Development use was on
foobar2000 2.26.3 preview (2026-11-08); compatibility with other 2.x versions has
not been validated. There is no x86, ARM64, macOS or native Columns UI build.

Prefer an isolated portable profile for the first test. Install
`foo_dj-0.20.0-alpha.1-win-x64.fb2k-component` with
**Preferences → Components → Install…**, then restart. In Default UI layout editing
mode, add **DJ Decks** (Utilities). A large panel works best; start around 900 × 650 px.
This component does not require WebView2, Spider Monkey Panel or Smooth Browser.
It uses the host-provided `shared.dll` and Microsoft C++ runtime. If Windows reports
a missing MSVCP140/VCRUNTIME140 dependency, install the official
[Microsoft x64 C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).
Do not download individual DLLs from third-party sites.

The release includes the installable component, matching full-source ZIP and SHA-256
checksums. Install/load on a clean portable profile has not yet been validated.
For local builds and packaging, see [build instructions](docs/BUILD.md).

## Quick use

Drop tracks on each deck to append them to its saved playlist; an empty deck loads
the first incoming track paused. Click **≡** or the Current/Next preview for its
floating playlist. Double-click to load; Delete removes entries, never files.
Use **AUTO** for auto-next or alternating A/B and fade settings. These automatic
modes are experimental; same-deck auto-next is not gapless. Play each
independently or use the common button. It toggles both states, so with only one
deck playing it pauses that deck and starts the other. Set crossfader and volumes
before listening to both together; there is no output limiter.

Speed wheel: 0.1% steps; Shift+wheel: 0.01%; right-click: neutral. **Key lock: ON**
changes tempo while preserving pitch; OFF is vinyl-style rate change.

**BPM = A/B** matches the effective tagged BPM of the other deck. Both tags must
be valid and the required speed must fit the destination deck's selected range. **SYNC = A/B** also seeks the selected
deck slightly to match estimated beat phase; it keeps play/pause states. It is
one-shot, not continuous tracking or downbeat/bar detection. Dotted grids are
indicative only and do not satisfy SYNC's automatic confidence requirement.

Click the range button (±16% by default) for ±8%, ±16% or ±50%. A narrower
range clamps the current speed if needed. The range is saved per deck.
Use the transposition −/+ buttons for semitone steps; click the centre to reset.
This changes pitch without changing tempo. The transposed tagged key is displayed.
The note-arrow button matches the other deck's effective tagged key (or its compatible
relative when major/minor modes differ). Non-neutral vinyl speed requires key lock.
Hover over icons for explanations in the selected interface language.
Analysis pauses/releases the file for tag writing; after saving the plugin results,
press Play to reopen at the retained position. Refresh tags shows raw/effective BPM.
Green/OK means a usual Camelot pair; amber/~ means another pair; grey/? means
unknown metadata or non-neutral vinyl speed. These are harmonic hints, not guarantees.

For manual grids, pause, zoom, click a transient, then **Beat here**. Use ±10 ms
buttons or the grid-row wheel (Shift: 1 ms). **Auto** removes the override. Manual
grids are used by SYNC. A phase shift cannot correct an inaccurate BPM or a varying tempo.

## Limits worth knowing

Shutdown has sometimes taken over 40 seconds in the development profile; its cause
has not been isolated. Do not present this alpha as production-stable. Beat detection
can reject real music, particularly rubato or weak/percussion-free material. Source
sample-rate conversion is linear. No headphone cue output, EQ, limiter, beat-looping
or master/slave continuous sync is implemented.

The decks decode through installed foobar inputs and instantiate the configured output,
but run outside the normal playback transport. Do not assume normal DSP/scrobbling or
now-playing integration. Main playback may contend for an exclusive audio device.
See [known issues](docs/KNOWN_ISSUES.md) before testing.

## State, privacy and reporting

`foo_dj-session.json` in the foobar profile stores local file paths, subsongs,
positions, speed and key-lock state. `foo_dj-grids.json` stores manual phase overrides.
Language is stored in foobar's configuration. These are local files and must not be
attached to public reports without redacting private paths. Cover lookup follows
foobar's configured artwork providers; other installed providers may have their own
network behavior. DJ Decks adds no update checker or telemetry.

For issues, include the component/foobar versions, architecture, output backend,
reproduction steps and whether the issue occurs in a clean profile. Do not attach music
files or private sessions by default. [Licenses and source](THIRD_PARTY_NOTICES.md).
