# Limiteur du mix principal — 0.39.0-beta.1

Le limiteur est activé par défaut à **−1 dBFS**. Dans **Réglages → Audio**, on peut le désactiver ou choisir un plafond de **−0,3 / −1 / −3 / −6 dBFS**. Ces choix sont mémorisés dans le profil foobar, indépendamment des fichiers de session.

Il traite la somme des platines après volumes, crossfader et ReplayGain, y compris les stems. Les deux canaux reçoivent le même gain afin de conserver l’image stéréo. Attaque immédiate, relâchement exponentiel de 80 ms, aucune latence de lookahead ajoutée.

À droite du crossfader :
- **LIM vert** : limiteur actif ; barre de crête en sortie.
- **Orange et −x dB** : réduction de gain appliquée (maintien visuel de 350 ms).
- **LIM OFF gris** : limiteur désactivé.
- **CLIP rouge** : dépassement de 0 dBFS avec le limiteur désactivé, ou échantillons non finis ; maintien une seconde. Les échantillons non finis sont remplacés par zéro.

La crête et le voyant concernent les blocs envoyés à la sortie ; ils ne constituent pas une mesure du signal analogique. La préécoute casque et les extraits de playlist restent indépendants.

Ce limiteur porte sur les **crêtes échantillonnées**, pas sur les crêtes inter-échantillons (true peak). Il ne normalise pas le volume des morceaux. Un niveau trop fort maintenu peut produire un pompage ou une distorsion audible ; réduire les volumes des platines si la réduction est importante. Le traitement après le composant ou dans le périphérique peut encore modifier le niveau.

Validation par signaux synthétiques et tests natifs, sans lancement, capture ni surveillance de foobar. L’écoute et l’affichage dans l’hôte restent à confirmer par l’utilisateur.
