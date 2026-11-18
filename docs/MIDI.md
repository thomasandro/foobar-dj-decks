# MIDI Learn — 0.42.0-beta.1 (public prerelease)

MIDI input is disabled by default. The owner confirms the installed x64 0.42 release working. Both full native suites and final MIDI tests pass using simulated inputs. The owner has not tested this release in x86 foobar; compatibility with other physical controllers/jog protocols and drivers remains to be confirmed.

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
- Relative CC encoders and seek-only jogs are now supported as described below. No scratch, temporary pitch bend, jog touch sensor, motor faders, LED feedback, paired high-resolution CC, proprietary HID, SysEx, MIDI clock or MIDI 2.0 mapping. Absolute/relative choice is explicit; the software does not guess the protocol.
- Up to 16 saved input profiles, 26 actions each. Port names/manufacturer/product and duplicate-name occurrence identify a saved input. Identical ports cannot be distinguished reliably if their enumeration order changes; verify the selected port after reconnecting identical units. Never silently switches to another input.
- Only one DJ Decks panel per foobar process can own MIDI input at a time, to avoid duplicate transport commands. Disable MIDI in that panel before enabling it elsewhere. Mappings are profile settings, not part of music tags or exported DJ session files.
- No device polling or automatic reconnect loop. Refresh/retry after disconnect/reconnect. If the controller floods the bounded queue, MIDI input stops with a visible status; retry explicitly. No outgoing MIDI and no hardware access during tests.
- Windows WinMM interface; a compatible Windows MIDI input driver is required. Reference: [Microsoft midiInOpen](https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/nf-mmeapi-midiinopen). Driver callbacks only enqueue data; transport and settings updates execute on the UI thread.


## Encodeurs relatifs et jogs — 0.42

Dans **Réglages → MIDI**, choisir une commande continue (volume, vitesse, crossfader ou Jog A/B), puis **Format du contrôle** avant MIDI Learn. Pour une association CC existante, changer le format suffit ; pour passer d’une association Pitch Bend absolue à un encodeur relatif, apprendre un CC. Les boutons restent en mode absolu et les associations 0.41 sont conservées comme telles.

| Format affiché | Pas positif | Pas négatif | Neutre / repos |
|---|---:|---:|---:|
| 1 / 127 — complément à deux | 1 | 127 | 0 |
| 65 / 63 — décalage 64 | 65 | 63 | 64 ou 0 |
| 1 / 65 — signe/amplitude | 1 | 65 | 0 ou 64 |
| 17 / 15 — décalage 16 | 17 | 15 | 16 ou 0 ; valeurs >31 ignorées |

Ces noms montrent le sens positif/négatif, pas la marque du contrôleur. Choisir le format indiqué par son manuel ou sa configuration. **Inverser** échange les sens. La documentation [Arturia](https://support.arturia.com/hc/en-us/articles/4405741265810-How-to-switch-between-Relative-Absolute-Mode-with-KeyLab-controllers) décrit les variantes 65/63, 1/127 et 17/15 ; [Ableton Push 2](https://github.com/Ableton/push-interface/blob/main/doc/AbletonPush2MIDIDisplayInterface.asc) documente les incréments en complément à deux.

Pour volume/crossfader/vitesse, la sensibilité indique la part de la plage par pas : 0,1 / 0,25 / 0,5 / 1 / 2 / 5 %. Par exemple, 0,5 % d’une plage de vitesse ±16 % donne 0,16 point de pourcentage par pas. Le relatif part de la valeur actuelle et n’utilise pas le rattrapage. Les pas multiples envoyés par le contrôleur sont pris en compte, plafonnés à 16 par message.

Pour une roue, choisir **A : jog (déplacement)** ou **B : jog (déplacement)**, puis le format relatif CC et **Déplacement jog par pas** : 10, 25, 50, 100, 250 ou 1000 ms (50 ms par défaut). Apprendre en tournant la roue, puis tester lentement dans les deux sens. Un repositionnement par platine et par cycle UI regroupe les crans, avec un déplacement cumulé plafonné à 30 s par cycle.

Le jog déplace la position même en pause ; il ne lance pas la lecture, ne change pas le BPM et ne produit pas de scratch. En lecture, les repositionnements peuvent produire des clics ou des interruptions selon le décodeur et la sortie. La suppression des silences reste respectée et sortir d’une boucle active la désactive, comme un clic dans la forme d’onde. Les lecteurs déchargés et les chargements en attente ne sont pas rouverts par le jog. Les réglages AUTO sont conservés selon les règles existantes du repositionnement.

Les formats/sensibilités sont sauvegardés par association. La version v2 est stockée séparément du format v1 de la 0.41 : une ancienne DLL retrouve ses anciennes associations v1, mais pas les changements faits dans la 0.42. Le contrôleur doit envoyer des CC relatifs ; les jogs en HID, données propriétaires, Pitch Bend relatif ou CC absolus circulaires ne sont pas décodés ici.

## Relative encoders and jogs (English)

Select a continuous target, choose **Control message format**, sensitivity and Learn. Supported relative CC formats are explicitly 1/127 two’s complement, 65/63 offset 64, 1/65 signed magnitude, or 17/15 offset 16. Invert to reverse direction. No automatic detection. Legacy mappings remain absolute; relative controls bypass pickup and adjust the current value, clamped to its range.

Use **A/B: jog (seek)** for navigation, with 10–1000 ms per step. Steps coalesce into at most one seek per deck per UI tick; no automatic playback, scratch or temporary tempo bend. Seeking can audibly interrupt playback, respects trim bounds and disables an active loop if the seek exits it. Pending loads/unloaded readers are skipped. Physical jog protocols still need validation on each controller; proprietary HID/touch data is unsupported.
