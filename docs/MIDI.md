# MIDI Learn — 0.41.0-beta.1 (first public implementation)

MIDI input is disabled by default. The owner confirms MIDI Learn working in x64 foobar. Both architectures pass native automated tests using simulated MIDI devices; the owner has not tested this release in x86 foobar. Broader physical controller/driver testing remains welcome.

## Français

1. Brancher le contrôleur puis ouvrir **Réglages → MIDI**.
2. Choisir son entrée et activer **Activer l’entrée MIDI**. Utiliser **Actualiser / réessayer la connexion** après un branchement ou une erreur. Fermer l’entrée dans les autres logiciels si le pilote la réserve exclusivement.
3. Choisir la **Commande à associer**, puis **MIDI Learn**. Appuyer sur un bouton ou déplacer le potentiomètre. Pour un bouton, relâcher après apprentissage. Le message utilisé pour apprendre ne déclenche pas la commande ; les messages suivants peuvent agir.
4. L’association (Note/CC/Pitch Bend, numéro, canal 1–16) apparaît et se sauvegarde automatiquement par entrée dans le profil foobar. Changer de commande annule un apprentissage en cours. Le bouton **Annuler l’apprentissage** est aussi disponible.
5. **Rattrapage** est activé par défaut pour volumes, vitesse et crossfader : rejoindre ou traverser la valeur affichée pour prendre le contrôle. Un changement à la souris ou par AUTO réarme le rattrapage. Le désactiver permet une réponse immédiate avec saut possible. **Inverser** change le sens du contrôle choisi.
6. **Supprimer cette association** retire la commande choisie. **Effacer les associations de cette entrée** demande confirmation et conserve celles des autres entrées. Un même message ne pilote qu’une commande : le réapprendre ailleurs remplace son ancienne association.

Commandes : crossfader, volumes et vitesses A/B (dans la plage choisie), lecture/pause A/B et bouton commun, début, Décharger, Cue casque, points A/B et activation des boucles, calage BPM et tonalité. Les commandes gardent leurs prérequis : morceau chargé, boucle valide, tags BPM/tonalité, sortie casque configurée. Décharger peut aussi fermer l’autre deck s’il lit le même fichier. MIDI utilise les mêmes règles que les clics.

## English

Open **Settings → MIDI**, select an input and enable MIDI. Choose a target command, click Learn, then press a button or move a knob/fader. Release buttons after learning. The learned message is consumed, not executed. Later messages may control the target. Assignments save automatically per input. Cancel learning or choose another command to disarm Learn. Remove one assignment, or confirm clearing the selected input’s mappings. Other inputs are preserved. Reusing a physical message moves it to the newly selected action.

Soft takeover defaults on for continuous controls: move near/across the current software value before taking control. Mouse/AUTO changes re-arm pickup. Disable it for immediate response, or invert the selected continuous mapping. Tempo follows the deck’s selected ±8/16/50% range. Cue requires a configured headphone output; loop/BPM/key commands retain their normal preconditions.

## Limits and connection behavior

- MIDI 1.0 short messages: Note buttons (Note Off / Note On velocity zero release), CC buttons (rising threshold 64), absolute CC knobs/faders (0–127), Pitch Bend (0–16383). Buttons should send both press and release; latch-only/toggle-only controller modes may require reconfiguration in the controller.
- No relative encoders, high-resolution paired CC, jog wheels, scratching, motor faders, output LEDs, SysEx, MIDI clock or MIDI 2.0 mapping. A relative encoder can look like an absolute CC: configure absolute mode on the controller where available.
- Up to 16 saved input profiles, 24 actions each. Port names/manufacturer/product and duplicate-name occurrence identify a saved input. Identical ports cannot be distinguished reliably if their enumeration order changes; verify the selected port after reconnecting identical units. Never silently switches to another input.
- Only one DJ Decks panel per foobar process can own MIDI input at a time, to avoid duplicate transport commands. Disable MIDI in that panel before enabling it elsewhere. Mappings are profile settings, not part of music tags or exported DJ session files.
- No device polling or automatic reconnect loop. Refresh/retry after disconnect/reconnect. If the controller floods the bounded queue, MIDI input stops with a visible status; retry explicitly. No outgoing MIDI and no hardware access during tests.
- Windows WinMM interface; a compatible Windows MIDI input driver is required. Reference: [Microsoft midiInOpen](https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/nf-mmeapi-midiinopen). Driver callbacks only enqueue data; transport and settings updates execute on the UI thread.
