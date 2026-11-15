# Extraits au casque depuis les playlists — 0.33.0-beta.1

Chaque entrée des deux playlists flottantes possède un petit bouton casque à
gauche. Un clic écoute un extrait sans charger le morceau dans un deck, sans
modifier sa position, la sélection de la playlist ou le mix envoyé aux enceintes.
Le bouton devient un carré Stop pendant la préécoute ; un clic sur ce Stop arrête l’extrait ; cliquer
sur un autre morceau le remplace. Le double-clic sur ce bouton ne charge pas le deck.
Le texte de l’entrée conserve ses commandes habituelles de sélection et de chargement.

## Réglages

Dans **⚙ → Préécoute casque…** :

- **Sortie casque** : choisir un périphérique stéréo distinct des enceintes.
  Si aucune sortie n’est configurée, le premier clic ouvre ce réglage, puis il faut
  cliquer de nouveau sur l’entrée pour lancer son extrait.
- **Volume casque** : le même volume indépendant que pour les boutons casque des decks.
- **Extrait depuis les playlists** : choix commun et mémorisé pour les deux listes :
  **8 s du début**, **15 s du début**, **8 s du milieu**, **15 s du milieu**,
  **8 s du début + 8 s du milieu**. Par défaut : 8 s du début.
- **Arrêter l’extrait de playlist** : interruption explicite.

Le milieu désigne une fenêtre centrée sur la moitié du morceau. Exemple pour une
piste de 100 s : les 8 s du milieu vont de 46 à 54 s. Les extraits trop longs
sont limités à la durée disponible. Sur un morceau court, le mode combiné évite de
répéter la partie déjà écoutée : une piste de 10 s donne les 8 premières secondes,
puis les 2 restantes. Une durée inconnue interdit le milieu ; un décodeur sans
positionnement ne peut pas fournir un extrait central.

## Fonctionnement et limites

Une petite fenêtre flottante assortie au deck apparaît **à côté de la playlist
utilisée**, sans prendre le focus. Si la place manque, elle passe à gauche, puis
au-dessus ou au-dessous, en restant dans l’écran. Elle montre une forme d’onde
grossière **du morceau entier**, un trait vertical et les positions dans l’extrait
et dans le morceau. En mode début + milieu, le trait saute à la partie centrale
sur la même vue du morceau entier.
Elle se ferme à la fin, à l’arrêt, sur erreur ou à la fermeture du panneau.
Sa croix arrête également la préécoute. Cliquer sur une autre entrée réutilise
la fenêtre avec le nouveau morceau.

**Cliquer sur la forme d’onde** relance la préécoute à cet endroit du morceau,
y compris dans une zone dont la forme n’est pas encore calculée. La durée choisie
reste 8 ou 15 s ; pour le mode début + milieu, le clic lance **16 s continues**
depuis ce point. La fin du morceau limite la durée sans retour au début. Le même
fichier et la même sous-piste restent utilisés, sans charger ou déplacer les decks.
La fenêtre et son aperçu sont conservés ; les anciens blocs audio sont annulés.
La sortie casque est vidée et réutilisée lorsqu’elle le permet, sans reconnexion
systématique à chaque clic. Les en-têtes et la zone de texte ne déclenchent pas de
positionnement. La croix et le bouton Stop de playlist restent disponibles.
Le décodeur doit permettre le positionnement ; le délai du casque reste perceptible.

L’écoute démarre sans attendre cette analyse. Un travailleur séparé de basse
priorité ouvre un lecteur supplémentaire et examine de courtes portions réparties
dans 32 zones du morceau, en commençant par le début, la fin et le milieu. Il ne
parcourt pas tout le fichier : c’est un **échantillonnage indicatif**, qui peut manquer
des attaques ou silences entre les portions examinées. Les zones non encore calculées
restent vides. Les échantillons réellement préécoutés enrichissent aussi la vue.
Un budget de 2 s est vérifié entre les opérations du décodeur ; une opération lente
ou un stockage bloqué ne sont pas interrompus par ce délai à eux seuls. Une analyse
incomplète reste partielle et peut reprendre lors d’une prochaine préécoute.
Un cache en mémoire conserve jusqu’à huit résultats, distingués par chemin,
sous-piste, taille et date de modification. Il est perdu en fermant le panneau.
La vue est limitée à 1 024 pics et son rafraîchissement à 20 fois/s. L’analyse est
annulée à l’arrêt, au changement d’extrait et avant l’écriture des tags. Les fichiers
sans durée connue ou sans possibilité de positionnement n’ont pas d’aperçu complet.
Le trait tient compte du tampon annoncé par la sortie, mais pas nécessairement
de tout le délai matériel/Bluetooth : sa position reste indicative. L’effet sur
les performances et l’alignement perçu doivent encore être confirmés à l’écoute.

- Un seul extrait à la fois, partagé entre les deux playlists. Il a priorité sur
  la préécoute des decks ; celle-ci reprend ensuite si ses boutons sont activés.
  Le mix principal, la lecture des decks et leur scrobbling continuent normalement.
- Lecture du fichier original, à sa vitesse et sa tonalité d’origine, sans stems,
  transposition ou ReplayGain du deck. L’extrait ne compte pas comme un scrobble.
- Décodage séparé, en arrière-plan, avec annulation des demandes précédentes et
  protection contre l’arrivée tardive de leurs anciens blocs audio.
- Les fichiers et sous-pistes locaux pris en charge par les décodeurs installés
  sont acceptés. Les flux réseau ne sont pas pris en charge par cette première version.
- Le lecteur ferme le fichier après l’extrait, en cas d’erreur ou d’annulation.
  Avant les commandes d’analyse/écriture de tags de DJ Decks, l’extrait est arrêté
  et son lecteur libéré. Une application externe peut néanmoins encore tenir le fichier.
- Supprimer ou réordonner une entrée ne change pas le morceau copié par la demande
  d’extrait. Les doublons du même fichier/sous-piste peuvent partager son témoin actif.
- Le tampon et la latence du périphérique, notamment Bluetooth, restent audibles.
  Voir [la préécoute casque](HEADPHONE_CUE.md) pour le routage et ses limites.
- Tests de boutons cachés, de planification, d’annulation et du vrai chemin de
  décodage avec entrées/sorties simulées : aucune musique personnelle, sortie audio
  réelle ou fenêtre foobar ouverte par les tests. L’écoute réelle reste à valider.

## English

Click the small headphone button before any floating-playlist row to preview it
without loading or moving either deck. Clicking the same track again stops it;
another track replaces it. The shared, saved mode in **Settings → Headphone cue →
Playlist excerpt** offers first 5/15 seconds, middle 5/15 seconds, or first 5 plus
middle 8. Middle windows are centered; short tracks are clipped without repeating
overlap in combined mode. The default is first 8 seconds.

One excerpt plays at a time on the selected headphone output and independent cue
volume. Deck cue is temporarily suspended and resumes afterwards; the master mix
and deck transport are unaffected. Excerpts use the original file, without deck
stems/tempo/pitch/ReplayGain processing or scrobbling. Local supported files/subsongs
only; middle previews need a known duration and seeking support. Cancellation,
replacement and tag-analysis handoff release the preview decoder. Physical-device
compatibility, buffering and Bluetooth delay still require live listening tests.

The floating window is placed beside the originating playlist (left/above/below
when necessary). It shows a **sampled whole-track overview**, not a concatenated
excerpt waveform; the playhead follows source time and jumps to the middle part.
A separate low-priority decoder samples up to 32 distributed regions without
delaying audio startup. A two-second budget is checked between decoder operations;
slow individual calls can exceed it. Missing regions remain partial. This is an
approximation, not an exhaustive peak scan. Up to eight size/mtime-keyed results
are cached in memory. Stop/replacement/tag analysis cancel this worker too.

Click the waveform to restart a cue excerpt at that source position, without
loading or moving a deck. Length stays 8/15 seconds; combined mode becomes one
continuous 16-second excerpt after a click. Near EOF it is shortened, never wrapped.
The overview/window are retained, old audio generations rejected, and the same cue
endpoint flushed/reused when possible. Seeking support and device latency still apply.
