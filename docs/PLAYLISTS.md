# Per-deck playlists — 0.20.0-alpha.1

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
AUTO menu; see [automatic playback](AUTOMATIC_PLAYBACK.md). Lists do not export
items by OLE drag to other windows. Drag reordering is within each floating list.

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
de chaque deck. Le déplacement par glisser d'une fenêtre vers l'autre n'est pas
inclus. Les fenêtres utilisent les contrôles
Windows redessinés : fond sombre, bord cyan/orange, boutons noirs brillants et
sélection teintée comme les decks. La barre de titre et le défilement restent Windows.
