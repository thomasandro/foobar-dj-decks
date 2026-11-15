# DJ Decks 0.28.0-beta.1 — première bêta x64

Première bêta x64 du 15 novembre 2026. Paquet installé avec sauvegarde et
fonctionnement confirmé par l’utilisateur avant publication. La version x86
reste en attente du retour de Fabcore.

## Ce qui change depuis l’alpha publique 0.24.0-alpha.1

- Gestion du cache des stems : taille occupée et suppression des entrées inutilisées,
  en préservant les stems actifs, les préparations et les fichiers musicaux.
- ReplayGain facultatif : désactivé, piste ou album. Intégration facultative avec
  foo_scrobble 1.6.x, désactivée par défaut ; des retours en conditions réelles restent
  souhaités pour cet adaptateur.
- Formats de playlist personnalisables avec les expressions de foobar, quatre
  préréglages et numérotation facultative. Le format par défaut corrigé est :
  `%artist% - %title% '('%length%')'`.
- Glisser-déposer d’une playlist à l’autre : copie, ou déplacement avec Maj.
- Conservation des métadonnées des listes, relecture asynchrone et correction de
  la gestion des champs de durée.

Le changelog conserve le détail des étapes de développement 0.25 à 0.27.3.

## Stems expérimentaux

La séparation batterie/basse/instruments/voix reste en phase de test. Le moteur
facultatif utilise Demucs, avec préparation préalable et cache ; ce n’est pas une
séparation en temps réel. La durée dépend du matériel et du morceau. CUDA est
préféré lorsqu’il est disponible, avec repli CPU. Des artefacts de séparation sont
possibles. Les contrôles d’activation, volume et Solo restent disponibles sur chaque
platine. Le composant fonctionne sans installer ce moteur.

## Installation ou mise à jour

1. Installer `foo_dj-0.28.0-beta.1-win-x64.fb2k-component` dans foobar2000 2.x **x64**,
   via Préférences → Composants → Installer…, puis redémarrer.
2. Ajouter DJ Decks à la disposition Default UI si nécessaire.
3. Sur une installation existante, si la durée manque encore, sélectionner de nouveau
   **⚙ → Affichage des playlists → Artiste - Titre (durée)**. Les formats déjà
   enregistrés sont conservés ; les parenthèses du format personnalisé doivent être
   protégées comme dans l’exemple ci-dessus.

Le paquet moteur `foo_dj-stems-engine-0.28.0-beta.1.zip` est facultatif et contient
l’installateur, pas les dépendances Python ni le modèle : ils sont téléchargés
séparément lors de son installation.

## Validation et limites

L’utilisateur confirme l’installation x64, la lecture de vrais morceaux et la
sauvegarde/restauration sur son installation. Il indique que les lenteurs observées
préexistaient au travail sur le composant ; elles ne sont pas retenues comme un
blocage démontré propre à DJ Decks. Les stems sont jugés relativement satisfaisants
et restent explicitement expérimentaux.

Le paquet bêta a également été installé avec sauvegarde et son fonctionnement
a été confirmé par l’utilisateur. Les tests automatisés s’exécutent
sans lancer foobar et ne remplacent ni l’écoute ni les essais de son interface.
Le test de formatage simulé ne valide pas le parseur de foobar ; la correction des
parenthèses provient de la vérification de l’utilisateur dans l’hôte.

Pas de bêta x86 validée à ce stade. Pas de préécoute casque dédiée, de limiteur,
de suivi continu de synchronisation ou de garantie d’enchaînement sans coupure.

Le paquet installable, ses sources complètes correspondantes et les sommes SHA-256
sont fournis ensemble. Aucun lancement, capture ou surveillance de foobar n’est
effectué par la préparation.
