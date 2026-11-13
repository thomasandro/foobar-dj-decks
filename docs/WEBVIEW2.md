# Optional WebView2 drag payloads

Native Album List, playlists/library and Explorer drops are supported without WebView2.
The public component also accepts explicit UTF-16 CF_UNICODETEXT payloads exported by
WebView2 HTML drags. It accepts only the following tagged JSON forms, not arbitrary text.

```text
foobar-playlist-tracks-v1
{"type":"foobar-playlist-tracks-v1","items":[{"path":"file://C:/Music/example.flac","subsong":0}]}
```

or `foobar-browser-tracks-v1` as both first line and JSON type. Only the first item
is loaded; path and subsong are preserved. The parser bounds input size and rejects
invalid subsongs, embedded NULs and malformed payloads. An advertised `text/plain`
type alone is insufficient: the contents must match the protocol.

Smooth Playlist already exports the playlist form in the development setup. A local
Smooth Browser adapter was prepared separately. Neither panel, its configuration,
SMP relay nor the user's library index is included here or required by DJ Decks.
A source panel must implement export itself; compatibility with arbitrary panels
is not claimed. This alpha's real cross-window behavior still needs fresh-profile testing.
