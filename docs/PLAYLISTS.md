# Per-deck playlists — 0.27.2-dev

Playlist metadata handles are retained. Missing/incomplete local-track artist/title/duration data is read asynchronously by foobar, once per item unless explicitly retried. Use Settings → Playlist display → Reload playlist metadata to refresh both lists from their files. Labels refresh when the read completes; selections are retained. This is read-only metadata loading, not playback/decoding in the paint path. Remote items are not probed automatically and unavailable files are not retried in a loop.


Duration fields are supplied explicitly from foobar's cached length, with a fallback to the corresponding loaded deck. Unknown duration appears as ?; no file is opened solely for formatting. Custom formats without %length% remain without a duration.


Experimental public alpha; installation is manual.

Each deck has a two-line preview: **Current / Next** (Actuel / Suivant).
Click either line or the black **≡** button to open its independent, resizable
floating playlist. Both windows can stay open together. Closing a window hides it;
the list is retained. Double-click a track or select it and press **Load / Charger**
or Enter to load it, paused, on that deck. Use the deck Play control to start it.

Drop multiple tracks from foobar playlists/library, compatible Smooth Playlist or
Smooth Browser text payloads, or individual files from Explorer. Order and subsong
indices are retained; duplicates are allowed. A drop on a deck appends to its list.
On an empty deck the first incoming track is loaded, paused; adding to an already
loaded deck does not interrupt its track. In a floating list, drop at the desired
insertion point. Explorer folders are ignored; folder expansion is not implemented.

Select one or several entries (Ctrl/Shift), drag within the list to reorder, or use
**Move up / Move down**. **Remove / Supprimer** or Delete removes only list entries,
never music files. Removing the loaded entry does not stop or unload its audio.
The Current line still shows the loaded track; Next points to the first list entry.
Loading a different entry stops the previous deck track and leaves the new one paused.
Automatic advancement and alternating A/B playback are available in the deck's
AUTO menu; see [automatic playback](AUTOMATIC_PLAYBACK.md). Drag between the two
DJ Decks floating lists copies the selection; hold Shift while releasing to move
it instead. Source order, duplicates and subsongs are retained. Drop at the desired
insertion boundary, or below the last entry to append. Existing playing audio is
not stopped by a transfer. Escape cancels; dropping outside the lists does nothing.
The destination scrolls near its top/bottom edges while dragging. External OLE
export to unrelated apps/foobar panels is not implemented by this internal transfer.

## Display format

Gear Settings → **Playlist display** offers four presets and **Custom format**.
When updating an older installation, select the Artist - Title (duration) preset again if its saved format lacks quoted parentheses. Custom expressions are preserved.

The shared default is `%artist% - %title% '('%length%')'`: Artist - Title (duration).
Use ordinary foobar title-format fields, conditional sections/functions, e.g.
`$if2(%artist%,?) - %title% '('%length%')'`, or include `%tracknumber%` explicitly.
The editor validates the expression and previews the first available playlist item.
Cancel leaves the saved format intact. The setting applies to both floating lists
and their Current/Next previews; it never changes metadata or queue contents.

**Show playlist position numbers** is a separate saved option, off by default.
These ordinal numbers are different from the track-number tag. An existing number
inside a TITLE tag or filename is not stripped. Items without cached metadata use
the filename/subsong fallback, so filenames can still contain their own numbers.
No synchronous file/decoder access occurs while formatting the list. Output is kept on one line.
The current-entry triangle remains a playback marker outside the chosen format.
Playlist-context fields such as `%list_index%` are not supplied by metadata-only
formatting; use the position-number option instead.

Lists, ordering, duplicates, subsongs and current occurrence are saved with the
existing deck session in `foo_dj-session.json` in the foobar profile. Legacy sessions
seed each list with its loaded track; explicitly emptied new lists stay empty after
restart. Missing/offline paths remain listed. File removal is never performed.
Limits: 10,000 entries per deck, 3 MiB total serialized list data. Metadata previews
use foobar's cached tags, with filename fallback, without opening audio readers.
Window position/size and selected rows are not persisted in this first version.

Build and native tests cover serialization, invalid entries, Unicode/subsongs,
duplicates, deletion, all 192 five-item reorder/selection cases, hidden independent
window controls, double-click/keyboard callbacks, multi-selection drag and teardown.
Existing graphics/layout and audio regression tests also pass. These tests use no
foobar host and take no screenshots. Actual cross-application OLE drops and restart
restoration in the user's foobar profile remain to be checked manually after installation.

## Français

Deux lignes **Actuel / Suivant** sur chaque platine. Cliquer dessus ou sur **≡**
ouvre sa fenêtre flottante ; les deux fenêtres peuvent rester ouvertes ensemble.
Glisser une sélection ou un album ajoute tous les morceaux, dans l'ordre. Dans la
fenêtre, déposer à l'emplacement voulu ; déplacer les entrées par glisser-déposer
ou avec **Monter / Descendre**. Ctrl/Maj permet la sélection multiple.

Double-clic, Entrée ou **Charger** charge le morceau en pause sur sa platine.
**Supprimer** ou Suppr retire uniquement les entrées, jamais les fichiers, et ne
coupe pas le morceau déjà chargé. Les deux listes et leur ordre sont sauvegardés.
L'enchaînement automatique et l'alternance A/B sont proposés dans le menu AUTO
de chaque deck. Glisser d'une fenêtre de playlist vers l'autre **copie** la sélection ;
maintenir **Maj au relâchement** pour la déplacer. Échap annule. L'ordre et les
sous-pistes sont conservés, et les fichiers musicaux ne sont jamais supprimés.
Le morceau déjà en lecture continue même si son entrée est déplacée.

Dans **⚙ → Affichage des playlists**, choisir un préréglage ou **Format personnalisé**.
Après mise à jour, sélectionner de nouveau le préréglage Artiste - Titre (durée) si l’ancien format enregistré ne protégeait pas les parenthèses. Les formats personnalisés sont conservés.

Le défaut est **Artiste - Titre (durée)** : `%artist% - %title% '('%length%')'`.
Le format est sauvegardé, validé par le moteur de foobar et commun aux deux listes.
Les numéros de position ajoutés par le plugin sont facultatifs. Un numéro déjà
contenu dans le titre ou le nom du fichier reste présent ; le format ne réécrit
pas les tags. En l'absence de métadonnées en cache, le nom de fichier reste le repli.

Les fenêtres utilisent les contrôles
Windows redessinés : fond sombre, bord cyan/orange, boutons noirs brillants et
sélection teintée comme les decks. Le titre et la croix sont intégrés au bandeau brillant : glisser le bandeau pour déplacer la fenêtre, tirer les bords pour la redimensionner. La croix masque la fenêtre sans vider la liste. Le défilement reste Windows.
