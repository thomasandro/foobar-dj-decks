# Candidate validation — 2026-11-13

Version: **0.12.0-alpha.1**, built separately from the installed development component.
GitHub publication was authorized on 2026-11-13. No Hydrogenaudio post was made.
No foobar host was launched, installed into, captured or monitored during release preparation.

## Completed

- Clean local source snapshot, excluding user profiles, music, artwork, old builds,
  private real-audio fixtures, dumps and development logs.
- Release/x64 build from the public tree with VS 18 / MSVC v145, ATL and Windows SDK.
  All SDK support projects and bundled SoundTouch were built from this tree.
- Native tests: validated drop protocol, tempo matching in both directions/range
  failures, synthetic onset phase fitting, silence/irregular-data rejection,
  marker spacing across zoom/scroll, manual grid shifts/anchors and SYNC math.
- Win32 grid-file persistence helpers: Unicode paths, separate subsongs, replacing
  a saved file and removing an override for Auto mode.
- French/English dictionary: labels, numeric placeholders, existing error translation
  and unchanged original metadata text.
- Actual FillDeck/SoundTouch functions extracted into an x64-style double-source fixture:
  all six duration/frequency cases passed for 440 Hz stereo at 84%, 100%, 116%,
  with and without key lock; output was drained at EOF.
- Static PE checks: x64 machine 0x8664, `foobar2000_get_interface` export, candidate
  version in the image; PDB reference is `foo_dj.pdb`, without a private build path.
- Dependency inspection: host `shared.dll`, Windows libraries/GDI+ and MSVC runtime.
  SoundTouch is static; no separate SoundTouch DLL is required.
- ZIP CRC/integrity, safe forward-slash paths, `x64/foo_dj.dll` architecture payload,
  payload equality with the build, license inclusion and no user-state files.
- Matching full-source archive entries compared byte-for-byte to the public tree.
- Private-path/content scan found none of the development profile, user-folder,
  private music-root or personal-email markers in the public source or DLL.

The audio tests measured around 439.8–440.0 Hz in key-lock mode at the non-neutral
speeds; vinyl-mode results were around 369.8 and 510.0 Hz as expected. These checks
are synthetic, not listening tests or guarantees of host behavior.

## Runtime checks still pending for this experimental alpha

1. Install the actual `.fb2k-component` into a fresh portable **x64** foobar profile.
   Confirm its alpha version, add the Default UI element and load two disposable
   local audio fixtures. Do not use a large personal profile for this first check.
2. Validate native/Explorer drops, independent/common transport, covers, labels,
   FR/EN live switching and persisted language/session/manual grids.
3. Listen to key lock and exercise SYNC/manual correction on suitable steady music.
   Confirm that uncertain automatic grids are rejected and identified honestly.
4. Test normal close/reopen, panel removal/unload, idle engine behavior and output
   backends. Investigate the previously observed slow shutdown.
5. Review the MIT choice, full-source companion, unsigned status and public wording.

These checks remain **unrun**, not passed. The current development profile is
unchanged. No claim is made of sample-accurate phase, live-DJ reliability, broad
format/device coverage, or compatibility with all foobar 2.x versions.
