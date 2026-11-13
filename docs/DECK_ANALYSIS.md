# Deck analysis (development version)

Each deck has Calculate BPM, Calculate key and Refresh tags buttons. Commands apply
to its loaded original file/subsong, not the current playlist selection and not the
tempo-adjusted/transposed output. This does not change transport or playback position.

Install foo_bpm and foo_musical_key to use their respective analysis buttons. DJ Decks
dispatches their public context commands. It does not include their analysis engines.
Normal plugin progress/result windows, cancellation, skip-existing options and tag-write
preferences apply. Save/confirm BPM results in the plugin's dialog when required.
Key analysis may store results only in foobar's database unless file writing is enabled.

Host metadata notifications refresh loaded deck values; Refresh tags also reads the
current database manually. No decoder reload is needed. Supported metadata names are
BPM/TEMPO and INITIALKEY/INITIAL KEY/KEY (case-insensitive). Non-default names used by
analyser preferences must be changed to supported names for DJ Decks to display them.
New BPM values also refresh the beat-grid calculation; they do not automatically change
deck speed, transposition or manual grid offsets.

Public command identifier references:
- [foo_bpm](https://github.com/stengerh/foo_bpm/blob/master/foo_bpm/guid.h)
- [foo_musical_key](https://github.com/PEERSOFTdev/foo_musical_key/blob/master/src/guids.h)

Command availability depends on the installed versions. Live-host invocation and
completion still need testing. No analysis was launched during development.
