# Settings window / Fenêtre de réglages — 0.36.0

## Français

Cliquer sur l’engrenage du bandeau inférieur ouvre une fenêtre flottante, indépendante et non bloquante. Un deuxième clic la remet au premier plan. Fermer cette fenêtre ne ferme pas les platines ; elle se ferme avec le panneau DJ Decks. Le dernier onglet est conservé pendant la session. Les modifications sont appliquées immédiatement : pas de bouton Appliquer ni d’annulation globale.

- **Apparence** : français/anglais, thème brillant ou plat, couleurs des deux platines.
- **Audio** : ReplayGain désactivé/piste/album, scrobbling via foo_scrobble et diagnostic.
- **Casque** : sortie séparée, actualisation des périphériques, volume 0–100 %, durée/type d’extrait, Cue A/B, arrêt d’extrait, libération et nouvelle tentative de sortie. La sortie du mix principal est exclue ; une sortie mémorisée indisponible est indiquée et n’est pas remplacée silencieusement.
- **Playlists** : formats prédéfinis, éditeur de format personnalisé, numéros de position et relecture des métadonnées.
- **Lecture** : durée et courbe du fondu, Q indépendant sur chaque platine, suppression des silences de début/fin et seuil. Les options de silences concernent les prochains chargements ; les modes AUTO restent disponibles directement sur les decks.
- **Stems** : ouverture du gestionnaire de taille/nettoyage du cache, explications sur le moteur facultatif et le statut expérimental.

Les réglages existants sont repris ; ouvrir un onglet ne remet rien à zéro. Les dialogues spécialisés de format, diagnostic et cache restent accessibles depuis les onglets. Les menus locaux des decks restent des raccourcis. La fenêtre utilise un fond sombre, des contrôles natifs et un accent de la couleur A. Elle peut être déplacée et redimensionnée ; les pages longues défilent. Échap ferme, Tab parcourt les commandes, Ctrl+Tab change d’onglet. Il n’y a pas de surveillance/polling ajouté : actualisation à l’ouverture, au choix d’un onglet ou après une modification. Si un réglage est changé depuis le deck, rouvrir par l’engrenage ou changer d’onglet pour rafraîchir sa valeur.

## English

The lower-strip gear opens an owned, modeless window with six tabs: **Appearance, Audio, Headphones, Playlists, Playback, Stems**. Changes apply immediately; there is no global Apply/Cancel transaction. Existing choices and custom values are retained. A second gear click reuses the window. Closing settings leaves decks running; destroying the panel closes settings. The last tab is remembered for the session.

The tabs expose the former global menu options plus fade duration/curve and per-deck loop Q. Headphones include endpoint refresh, volume, excerpt mode, Cue A/B and stop/release/retry. The current master output is excluded; unavailable saved endpoints remain explicitly identified. Specialized custom-format, diagnostic and protected-cache dialogs remain available. Local deck menus remain shortcuts.

The dark native window supports resize, scrolling, Escape, Tab and Ctrl+Tab. It refreshes on opening, tab selection and settings changes; no new monitoring timer is installed. Reopen via the gear or switch tabs to refresh values changed elsewhere. Silence changes apply to new loads, scrobbling remains opt-in and stems remain experimental.

## Correctif 0.36.1 / contrast fix

Les boutons et cases utilisent désormais un dessin explicite avec texte clair, état coché et repère de focus, pour rester lisibles avec les thèmes sombres Windows. Les changements restent immédiats. / Buttons and toggles explicitly draw light text, checked state and keyboard focus to preserve contrast independently of native dark-theme text colors.

## Correctif 0.36.2

Les notifications de dessin des cases/boutons sont désormais transmises depuis leur zone de contenu à la fenêtre de réglages. Le test de contraste emprunte ce même trajet, au lieu d’appeler directement le dessin. Activer Q depuis Lecture recale aussi les points A/B déjà posés et les sauvegarde. / Content-parent draw notifications are now forwarded correctly; the contrast test follows that actual route. Enabling Q also resnaps and saves existing points.

## Sessions (0.37.0)

L’onglet Playlists propose désormais l’enregistrement des deux listes, l’enregistrement des listes avec l’état des platines, le chargement et la récupération de l’état avant chargement. Voir [SESSION_FILES.md](SESSION_FILES.md). / The Playlists tab now includes save/load/recovery session actions; see SESSION_FILES.md.


### Category defaults (0.40.2)

The seven category buttons follow the selected Glossy/Flat theme and deck A color. Ctrl+Tab / Ctrl+Shift+Tab switches categories; Tab and Shift+Tab move focus.

Appearance, Audio, Headphones, Playlists and Playback each offer a confirmed reset to their defaults. Language is retained. Resets preserve music, playlist contents, deck positions and A/B points. Headphone reset stops cue/excerpts and clears the selected device; Audio reset disables scrobbling/ReplayGain and restores the enabled -1 dBFS limiter and Auto sample rate (rate change after restart). Stems cache cleanup remains separate and is never triggered by reset.


### MIDI (0.41.0)

Seventh category: optional controller input, MIDI Learn, saved command assignments, soft takeover and inverted continuous controls. MIDI is off by default. See [MIDI guide / guide MIDI](MIDI.md) for the 24 actions, supported messages and first-version limits.
