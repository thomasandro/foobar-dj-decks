# Headphone cue / Préécoute casque — 0.29.0-beta.1

## Français

Cette première version utilise une **sortie stéréo distincte**, choisie par chaque
utilisateur. Aucun nom de carte ou de casque n’est imposé. La sortie principale
reste celle configurée dans foobar2000 ; DJ Decks ne change pas ce réglage global.

1. Laisser la sortie principale de foobar sur les enceintes, par exemple la MOTU.
2. Ouvrir **⚙ → Préécoute casque… → Sortie casque**, et choisir explicitement le
   casque parmi les sorties proposées par les composants audio installés dans foobar.
   Le périphérique doit déjà être connecté et disponible dans Windows.
3. Régler **Volume casque**, indépendant du volume des decks et du crossfader.
   Le réglage initial est 25 % ; 0 % coupe le son. Sortie et volume sont mémorisés.
4. Activer le bouton avec le pictogramme casque sur A, B ou les deux. Les boutons
   sont désactivés à chaque ouverture du panneau ; sélectionner une sortie ne lance
   ni la lecture ni l’écoute automatiquement. Au premier clic sans sortie configurée,
   choisir la sortie, puis cliquer de nouveau pour activer le deck.
5. Lancer la lecture du deck à préparer et positionner le crossfader pour le garder
   hors du mix principal. La préécoute suit le transport du deck : lecture/pause,
   déplacement, vitesse, transposition et stems. Elle prend son signal après ces
   traitements et le ReplayGain éventuel, mais **avant volume du deck et crossfader**.
   Activer le casque ne rend pas automatiquement le deck muet dans les enceintes.

Lorsque les deux casques sont actifs, A et B sont additionnés avec une atténuation
de moitié pour chaque deck. Il n’y a pas de limiteur. Le temps d’écoute uniquement
au casque n’est pas ajouté au scrobbling des platines audibles dans le mix principal.

**Désactiver la préécoute / libérer la sortie** ferme uniquement la sortie casque.
Une erreur casque est affichée dans le menu de préécoute ; choisir une autre sortie
ou **Réessayer la sortie sélectionnée** pour reprendre. Aucun retour automatique
vers la sortie principale n’est effectué en cas d’erreur. Le couple module/sortie
identique au mix principal est refusé. Des alias peuvent désigner le même matériel :
choisir un périphérique nommé explicitement plutôt qu’une sortie système par défaut.

### Limites de cette première version

- Ce n’est pas une deuxième tête de lecture indépendante sur le même deck. Pour
  écouter un morceau préparé, faire jouer le deck en le retirant du mix principal.
- Le menu propose les sorties **exposées par foobar**. Il ne fournit pas encore une
  matrice de canaux personnalisée, par exemple master 1/2 et casque 3/4 d’un seul
  périphérique ASIO. Des sorties stéréo distinctes déjà exposées sont utilisables.
- Deux périphériques ont des horloges et des latences différentes. Le Bluetooth
  convient à l’écoute de préparation, mais la synchronisation acoustique exacte
  avec les enceintes n’est pas garantie. Choisir la sortie stéréo musicale du casque,
  lorsqu’elle est distinguée d’une sortie mains libres. Voir la documentation
  [Bluetooth audio de Microsoft](https://learn.microsoft.com/en-us/windows-hardware/drivers/bluetooth/bluetooth-classic-audio)
  et les [différences de latence LE Audio](https://support.microsoft.com/en-us/windows/hardware/bluetooth/configuring-bluetooth-le-audio-quality-settings-on-windows-11).
- La sortie principale cadence la lecture. La file de préécoute est limitée à
  200 ms : en cas de retard, les blocs anciens sont abandonnés, ce qui peut provoquer
  une discontinuité au casque. Pas de compensation automatique de dérive ou de délai.
- La conversion entre fréquences différentes est linéaire et continue entre blocs.
  Un backend qui n’accepte pas la stéréo ou le format fourni peut être refusé.
- Une sortie casque défaillante n’est jamais attendue par la production audio
  principale. La fermeture finale dépend néanmoins du comportement des pilotes.
- Les tests utilisent des sorties simulées, sans ouvrir de périphérique réel.
  La validation dans foobar et l’écoute restent à faire par l’utilisateur.

## English

Choose a separate stereo endpoint in **Settings → Headphone cue → Headphone output**.
Keep the normal foobar output assigned to the speakers. Set the independent cue
volume, then enable the headphone button on A, B or both. Device and volume persist;
cue buttons reset off when the panel opens. No device is hard-coded.

Cue follows the deck's existing playback/seek/tempo/pitch/stems and optional
ReplayGain, **before deck volume and crossfader**. Play the deck and keep it out of
the master using the crossfader. Enabling cue does not mute the master contribution.
With both decks selected, each cue signal is halved. No limiter or separate cue
playhead is provided, and cue-only listening does not count towards scrobbling.

The cue output runs separately with a bounded queue and never waits in the master
audio producer. Errors appear in the cue settings menu: select another endpoint or
retry; there is no fallback to the master. Exact master endpoint pairs are rejected,
but aliases can refer to the same hardware. Prefer an explicit named device.

Bluetooth and separate device clocks can introduce delay/drift. Old queued cue
blocks are discarded on overflow, so headphone glitches are possible. No automatic
latency compensation, clock-drift correction or custom multichannel/ASIO channel
matrix is included. Linear streaming resampling handles differing forced rates.
The master remains the playback clock. Mock-output tests do not establish real
driver compatibility or listening quality; live testing remains manual.
