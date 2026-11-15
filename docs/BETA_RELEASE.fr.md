# DJ Decks 0.37.0-beta.1 — bêta x64 et x86

Nouveautés depuis 0.34.1-beta.1 : boucles A/B indépendantes et points sauvegardés par morceau/sous-piste ; bouton Q pour caler les nouveaux points et recaler les points existants ; fenêtre de réglages à six onglets ; infobulles allégées et correction du dessin/contraste des cases ; fichiers JSON pour enregistrer/charger les deux playlists et l’état des platines, avec récupération avant chargement.

Choisir le paquet correspondant à foobar : [x64](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-win-x64.fb2k-component) ou [x86](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-win-x86.fb2k-component). [Sources communes complètes](https://github.com/thomasandro/foobar-dj-decks/releases/download/v0.37.0-beta.1/foo_dj-0.37.0-beta.1-source.zip).

L’utilisateur confirme le fonctionnement x64 et les fichiers de session. Les deux architectures sont compilées et leurs tests natifs passent ; le retour en conditions réelles x86 reste attendu. Les stems restent expérimentaux. La restauration complète revient en pause, boucles/AUTO/Cue coupés. Aucun audio/stem n’est embarqué : les morceaux doivent rester accessibles aux mêmes chemins. Ne pas partager un JSON sans vérifier ses chemins privés.

Voir [boucles/Q](AB_LOOPS.md), [réglages](SETTINGS.md), [fichiers de session](SESSION_FILES.md) et [changelog détaillé](../CHANGELOG.md), y compris les versions intermédiaires non publiées.
