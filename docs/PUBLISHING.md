# Publication policy

Repository: https://github.com/thomasandro/foobar-dj-decks
First alpha: v0.12.0-alpha.1 (2026-11-13).
The owner authorized GitHub publication; no Hydrogenaudio posting was requested.

Future releases should:
- Keep the README/changelog and experimental limitations accurate.
- Build and test from the public source tree; validate the actual package in a clean
  portable x64 foobar profile before claiming host compatibility/stability.
- Distribute the matching full-source ZIP alongside the component and SHA-256 hashes.
- Preserve the original-code MIT license and separate upstream notices, including LGPL SoundTouch.
- Exclude user profiles, sessions, music, dumps, personal paths and private debug files.
- Use prerelease status until runtime validation and shutdown problems are resolved.
- Obtain the owner's instruction before publishing releases or a forum announcement.

The local-only development versions 0.1–0.12.0-dev are grouped under the first alpha
in CHANGELOG.md. Public preparation changed packaging/build hygiene and component
metadata only; the installed development profile was not modified.
