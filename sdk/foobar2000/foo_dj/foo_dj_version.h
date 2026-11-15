#pragma once

// Numero de version de foo_dj.
// A MODIFIER ICI SEULEMENT : il est repris automatiquement dans la liste des composants de
// foobar2000 (Preferences > Composants) et affiche en petit en bas a droite du panneau.
//
// Historique :
//   0.1    composant vide qui se charge dans foobar2000
//   0.2    panneau Deck A / Deck B (interface seulement)
//   0.3    decodage des fichiers (duree et format)
//   0.4    lecture des deux decks sur la sortie audio de foobar2000
//   0.5    crossfader
//   0.6    volume par deck, forme d'onde avec zoom, positionnement dans le morceau
//   0.6.1  affichage sans scintillement, numero de version affiche
//   0.6.2  defilement de la forme d'onde plus fluide
//   0.7    chargement des morceaux par glisser-deposer (plus de bouton Charger)

#define FOO_DJ_VERSION "0.38.1-beta.1"

// Meme texte en "grands caracteres" (L"..."), pour l'affichage dans le panneau.
#define FOO_DJ_WIDE_(x) L##x
#define FOO_DJ_WIDE(x) FOO_DJ_WIDE_(x)
#define FOO_DJ_VERSION_W FOO_DJ_WIDE(FOO_DJ_VERSION)
