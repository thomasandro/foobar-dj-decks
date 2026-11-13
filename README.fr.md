# DJ Decks pour foobar2000 (`foo_dj`)

[English](README.md) · [Historique](CHANGELOG.md) · [Compilation](docs/BUILD.md) · [Limites connues](docs/KNOWN_ISSUES.md) · [Discussion sur Hydrogenaudio](https://hydrogenaudio.org/index.php/topic,130167.0.html)

Deux platines indépendantes dans foobar2000, avec mixage et formes d’onde.
**0.12.0-alpha.1 est une préversion alpha expérimentale.**

[Télécharger l’alpha et les sources correspondantes](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.12.0-alpha.1)
Le développement a été assisté par IA ; les revues du moteur audio et des threads
ainsi que les retours reproductibles sont bienvenus.

## Fonctions

- Chargement, lecture/pause et positionnement indépendants ; bouton commun pour les deux états.
- Volumes séparés, crossfader à puissance constante, variations de gain lissées.
- Formes d’onde avec zoom, clic pour se déplacer et indication de la partie déjà jouée.
- Pochettes ; titre et artiste sur deux lignes défilantes ; interface français/anglais.
- Affichage du BPM renseigné dans les tags ; vitesse de −16 % à +16 %.
- Mode vinyle ou tempo avec tonalité préservée par SoundTouch.
- Égalisation des BPM ; premier calage ponctuel des battements, expérimental.
- Repères estimés/manuels continus ; grille indicative en pointillés si la détection est incertaine.
- Réglage manuel de grille, « Temps ici » et retour Auto, mémorisés par fichier/sous-piste.
- Morceaux et positions restaurés en pause après fermeture normale.
- Glisser-déposer natif de foobar/Explorateur ; formats WebView2 compatibles facultatifs.

## Installation

Windows x64, foobar2000 2.x x64, Default User Interface. Les essais de développement
ont été faits avec foobar2000 2.26.3 preview du 8 novembre 2026. Les autres versions
2.x ne sont pas encore validées. Aucune version x86/ARM64/macOS ni intégration native
Columns UI n’est fournie.

Pour le premier essai, utiliser un profil portable séparé. Installer
`foo_dj-0.12.0-alpha.1-win-x64.fb2k-component` via
**Préférences → Composants → Installer…**, puis redémarrer. Dans l’édition de disposition
de Default UI, ajouter **DJ Decks** (Utilitaires). Un grand panneau est préférable ;
commencer vers 900 × 650 px. WebView2, SMP et Smooth Browser ne sont pas nécessaires. Le composant utilise
`shared.dll` fourni par foobar et le runtime C++ Microsoft. Si Windows signale
MSVCP140/VCRUNTIME140 manquant, utiliser le
[runtime C++ x64 officiel Microsoft](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).

La préversion comprend le paquet installable, les sources complètes correspondantes
et les sommes SHA-256. L’installation et le chargement dans un profil portable
vierge n’ont pas encore été validés. Les scripts créent les paquets locaux dans `dist/`.

## Utilisation

Glisser un morceau sur chaque platine ; une sélection multiple charge le premier.
Le bouton commun inverse les deux états : lecture des deux, pause des deux, ou échange
de la platine en lecture si une seule joue. Régler volumes et crossfader ; aucun
limiteur de sortie n’est prévu.

Molette sur la vitesse : pas de 0,1 % ; Maj+molette : 0,01 % ; clic droit : 0 %.
« Tonalité fixe : OUI » conserve la hauteur ; le mode vinyle la fait varier.
« BPM = A/B » égale le BPM effectif de l’autre platine, dans la plage ±16 % et avec
deux tags valides. « SYNC = A/B » ajuste aussi la position de la platine choisie,
sans changer lecture/pause. C’est un calage ponctuel, sans suivi continu ni détection
du premier temps de mesure. Les pointillés n’attestent pas un calage et ne valident
pas une grille automatique pour SYNC.

Pour une grille manuelle : pause, zoom, clic sur une attaque, puis « Temps ici ».
Décaler de ±10 ms avec les boutons ou à la molette ; Maj donne un pas de 1 ms.
« Auto » retire le réglage du morceau. SYNC utilise les grilles manuelles.
Un décalage de grille ne corrige ni un mauvais BPM ni les variations de tempo.

## Limites et données

La fermeture a parfois dépassé 40 secondes dans le profil de développement ; la cause
n’est pas isolée. Cette alpha ne doit pas être présentée comme stable pour un usage
public en direct. La détection des battements peut refuser de vrais morceaux. Pas de
préécoute casque, d’égaliseur, de limiteur, de boucles ou de suivi de synchronisation.

Les platines utilisent les décodeurs installés et la sortie configurée, mais suivent
leur propre transport : ne pas supposer une intégration avec les DSP, scrobbles et
informations « en cours » du lecteur principal. Un périphérique exclusif peut être
en conflit avec la lecture principale. Voir les [limites connues](docs/KNOWN_ISSUES.md).

Les fichiers de profil `foo_dj-session.json` et `foo_dj-grids.json` contiennent des
chemins de musique personnels et des réglages. Ne pas les publier sans les expurger.
La langue reste dans la configuration foobar. La recherche de pochettes suit les
fournisseurs configurés dans foobar, qui peuvent avoir leur propre activité réseau.
DJ Decks n’ajoute ni télémétrie ni recherche automatique de mises à jour.

Les rapports utiles précisent les versions, l’architecture, la sortie audio et des
étapes de reproduction, si possible dans un profil vierge. Ne pas joindre de fichiers
musicaux ou de sessions personnelles par défaut. [Licences](THIRD_PARTY_NOTICES.md).
