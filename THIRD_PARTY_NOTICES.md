# Third-party notices

The optional foo_scrobble adapter bundles the upstream Track, UnixClock and
ScrobbleService interface declarations from gix/foo_scrobble commit
bc9e53db898f3293240a2477732ef909cb9477eb (Copyright Nico Rieck, MIT).
The Track header include was adapted to this SDK; the service GUID is defined in
the adapter. See `licenses/foo_scrobble-MIT.txt` and
`sdk/foobar2000/foo_dj/third_party/foo_scrobble`. The installed foo_scrobble component
itself is external and is not redistributed or modified.

Original DJ Decks code is offered under the MIT license in [LICENSE](LICENSE).
The component began from the foobar2000 SDK sample. This is an altered derivative,
not an official foobar2000 component. Original SDK and dependency notices remain
applicable to their code. No upstream author endorsement is claimed.

| Dependency | Version / provenance | License | Included source |
|---|---|---|---|
| foobar2000 SDK | SDK header reports 2026-10-01, API 81 | SDK redistribution license | `sdk/foobar2000` |
| pfc | bundled with that SDK | zlib-style | `sdk/pfc` |
| libPPUI | bundled with that SDK | zlib-style | `sdk/libPPUI` |
| Windows Template Library | local WTL headers identify 10.0 | Microsoft Public License | `third_party/WTL` |
| SoundTouch | 2.3.3, official source archive | LGPL-2.1-or-later | `sdk/foobar2000/foo_dj/third_party/soundtouch` |
| picojson | 1.3.0, upstream release header | BSD-2-Clause | `sdk/foobar2000/foo_dj/picojson.h` |

License texts are preserved in [licenses](licenses). WTL and SoundTouch also
retain their license files beside their sources. SoundTouch copyright:
Olli Parviainen, 2001–2024. SDK, pfc and libPPUI: Peter Pawlowski; WTL:
Microsoft Corporation / WTL Team; picojson: Cybozu Labs and Kazuho Oku.

SoundTouch is statically linked. Distribute the matching complete source archive
alongside every binary release, including the build scripts and bundled inputs.
The source kit is sufficient to rebuild the application with a modified SoundTouch
using the documented Microsoft toolchain; no proprietary DJ Decks object is needed.
Reverse engineering for debugging modifications of the LGPL-covered library is
permitted as required by LGPL 2.1. Changes made here to upstream SDK projects are
Visual Studio retargeting to v145 and portable build settings, not upstream releases.

The SDK-provided `shared-x64.lib` is a build input, covered by the SDK license;
its SDK sources and project are also included. It is the only prebuilt library
kept in the source tree. Microsoft Visual Studio, ATL, the Windows SDK, the C++
runtime and foobar2000 itself are external prerequisites, not redistributed here.

Upstream: [foobar2000 SDK](https://www.foobar2000.org/SDK),
[SoundTouch](https://soundtouch.surina.net/),
[picojson](https://github.com/kazuho/picojson/tree/v1.3.0),
[WTL](https://sourceforge.net/projects/wtl/).

Optional stems engine (0.21.0-dev): Demucs 4.0.1 / htdemucs and PyTorch/torchaudio, NumPy and soundfile are installed separately by the optional installer. Their libraries and weights are not bundled in the component/source archives. Demucs code is MIT-licensed; retain each external package's installed upstream notices. The worker adapter is original component code. Upstream: https://github.com/facebookresearch/demucs/tree/v4.0.1 .
