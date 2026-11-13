# Build and package (Windows x64)

## Prerequisites

- Visual Studio 2026 / MSVC v145 with Desktop development with C++, ATL for x64,
  and a Windows SDK. The prepared build was tested with VS 18 Community / MSVC 14.52.
- PowerShell and Git (Git is only needed for repository work).
- foobar2000 SDK 2026-10-01, WTL, SoundTouch and picojson sources/build inputs are
  included; there is no dependency download step and no absolute developer include path.

From the repository root:

```powershell
.\scriptsuild.ps1
.\scripts	est.ps1
.\scripts\package.ps1 -SkipBuild -SkipTests
```

Or `scripts/package.ps1` performs all three steps. `-SkipBuild/-SkipTests` are for
an already verified local build only. No script installs components, opens foobar,
reads a user's profile, captures a window, publishes, or creates an online repository.

Outputs: `build/bin/foo_dj.dll`, then the installable, matching full-source archive
and SHA-256 checksums in `dist/`. `VSWHERE` / `MSBuildPath` parameters can select a
specific build installation; see the scripts. Default target is Release/x64 only.
Other architectures/toolsets are not claimed supported.

The source keeps the SDK's relative project layout. `sdk/Directory.Build.targets`
adds the bundled WTL headers and records only the PDB filename in the release DLL.
Do not publish PDBs, local build logs or intermediate objects.

## Tests

The native suite covers drop-payload validation, tagged tempo matching, beat phase
and marker math, manual grids, session-file helpers, translations, and the actual
FillDeck/SoundTouch processing functions extracted into an isolated audio fixture.
Audio tests use a synthetic 440 Hz stereo signal and check duration and frequency
with vinyl/key-lock modes at 84%, 100% and 116%, including draining the output at EOF.
The x64 fixture uses double-precision source samples converted to SoundTouch floats.

Tests cannot establish foobar UI correctness, decoder/thread safety, perceptual quality,
shutdown behavior or long-run stability. See VALIDATION.md for the actual run status.

## Modifying SoundTouch

Edit the bundled SoundTouch sources, rebuild with `scripts/build.ps1`, run the suite,
then package again. All application source and necessary non-system build inputs
are supplied, so no unpublished DJ Decks object files are required to relink.
The Microsoft toolchain and foobar host remain external prerequisites.
