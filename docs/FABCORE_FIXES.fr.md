# Correctifs pour le retour x86 de Fabcore — 0.40.1-beta.1

## Fréquence

Le moteur ne choisit plus systématiquement 48 kHz en l’absence de fréquence imposée par la sortie. Auto utilise la fréquence source du premier morceau en lecture au démarrage du moteur, sinon du premier chargé, sinon 44,1 kHz. La fréquence reste ensuite fixe pendant la vie du moteur, pour mixer des sources différentes. Si le périphérique impose sa fréquence par l’API foobar, cette contrainte reste prioritaire.

**Réglages → Audio → Fréquence du mix principal** permet Auto, 44100, 48000, 96000 ou 192000 Hz. Une modification s’applique après redémarrage de foobar. Pour reproduire le cas signalé, choisir 44100 Hz avec une sortie compatible ou laisser Auto avec un premier morceau 44,1 kHz. Aucun test de périphérique physique n’a été effectué.

## Réglages

Nouveau conteneur de défilement Windows dédié ; déplacement groupé des contrôles sans repeindre chaque élément synchroniquement. Barre de défilement de largeur stable, flèches/pages et position finale du curseur prises en charge. La molette fonctionne sur le fond et les contrôles ; sur une liste déroulante fermée, elle défile dans la page sans changer sa valeur. Une liste déroulante ouverte conserve son fonctionnement normal. Les petits mouvements de molette sont cumulés.

## Charge graphique

Les fonds brillants sont rasterisés une fois puis réutilisés, y compris leurs états normal/survol/enfoncé. Cache limité à 16 Mio et 96 entrées par fil d’interface ; rendu direct si une surface est trop grande ou la création du cache échoue. Thème plat conservé. Rafraîchissement d’animation plafonné à 30 images/s et demandes de dessin regroupées lors du glissement volume/crossfader. Les valeurs audio sont mises à jour immédiatement.

## Essai demandé au testeur

- Démarrer avec un morceau 44,1 kHz, vérifier la fréquence affichée et la lecture ; préciser le nom du périphérique et du mode de sortie si l’erreur persiste.
- Dans les réglages Audio/Playlists, essayer flèches, glissement de la barre et molette sur le fond puis sur les contrôles.
- Manipuler volume/crossfader et survoler les boutons, à vide puis pendant la lecture ; comparer Brillant et Plat.

Les tests natifs sont exécutés sans lancer, capturer ni surveiller foobar. La réduction du travail de dessin et les limites du cache sont testées ; le taux CPU réel et le comportement sur le matériel de Fabcore doivent être confirmés par lui. Les demandes cosmétiques de StyxCrosser sont hors du périmètre de ce correctif.
