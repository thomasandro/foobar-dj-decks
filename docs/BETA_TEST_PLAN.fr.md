# Fiche de validation bêta — DJ Decks 0.28.0-beta.1

## État au 15 novembre 2026

Déclarations de l’utilisateur : installation x64, lecture sur de vrais morceaux et
sauvegarde/restauration validées. Stems jugés relativement satisfaisants, conservés
comme expérimentaux. Les lenteurs préexistaient au travail sur le composant et ne
sont pas un blocage démontré de DJ Decks. Retour x86 de Fabcore encore attendu.
Ces validations ne remplissent pas automatiquement chaque cas détaillé ci-dessous.
Le paquet bêta a ensuite été installé avec sauvegarde et son fonctionnement confirmé par l’utilisateur.


Objectif : confirmer les fonctions annoncées en conditions réelles. Cette fiche
est à remplir manuellement ; aucun résultat automatique ne remplace ces essais.
Les scripts de développement n'ouvrent, ne surveillent et ne capturent pas foobar.

## Configuration du testeur

- Testeur / date :
- Version de DJ Decks :
- Version de foobar et architecture **x86 ou x64** :
- Version de Windows / mémoire :
- Sortie audio et mode partagé/exclusif :
- CPU / GPU / pilote NVIDIA si stems :
- Python / moteur de stems installé : oui/non ; CPU/CUDA :
- Profil portable vierge ou profil habituel :
- Durée de la session :

Noter chaque ligne **OK / ÉCHEC / NON TESTÉ / SANS OBJET**. En cas d'échec,
noter les étapes, le résultat attendu, le résultat constaté et sa fréquence.
Ne joindre ni musique, ni sessions, ni chemins personnels ; une capture n'est
pas nécessaire. Comparer avant/après avec le même morceau et les mêmes réglages.

## Installation et démarrage

| Essai | Résultat / observations |
|---|---|
| Profil portable vierge : installer le paquet correspondant à l'architecture de foobar, ajouter le panneau et redémarrer | |
| Sans moteur Python : les fonctions audio ordinaires fonctionnent ; STEMS explique l'installation facultative | |
| Mise à jour depuis la version précédente : playlists et préférences retrouvées | |
| Trois ouvertures/fermetures normales ; durée de fermeture notée, absence de blocage | |
| Retirer/réajouter le panneau, puis désinstaller le composant dans le profil de test | |

## Lecture et sauvegarde

| Essai | Résultat / observations |
|---|---|
| Charger deux morceaux, lecture/pause séparée et commune, début et déplacement dans le morceau | |
| FLAC, MP3, Ogg/Opus, WAV ; fichier absent ou illisible traité sans boucle de chargement | |
| Glisser un album/sélection depuis les listes utilisées ; ordre et doublons conservés | |
| Deux playlists flottantes : ouvrir, déplacer, redimensionner, réordonner, supprimer puis fermer ; la musique source reste présente | |
| Format par défaut Artiste - Titre (durée), quatre préréglages, format personnalisé/aperçu, Annuler et conservation après redémarrage | |
| Numéros de position facultatifs, tag tracknumber explicite et fichier sans titre : affichage cohérent avec les métadonnées disponibles | |
| Glisser A → B et B → A : copie, Maj pour déplacer, sélection multiple, insertion au milieu/en fin et liste vide ; Échap et dépôt extérieur sans modification | |
| Copier/déplacer le morceau courant sans interrompre sa lecture ; ordre et sous-pistes conservés après redémarrage | |
| Fermeture normale/redémarrage : listes, morceaux, positions en pause, grilles, langue, thème et couleurs retrouvés | |
| Modifier des tags après Arrêter/libérer ; actualiser les tags puis reprendre à la position retenue | |

## Mixage et qualité à l'écoute

| Essai | Résultat / observations |
|---|---|
| Volumes et crossfader dans les deux sens, sans à-coups ; niveaux d'écoute raisonnables (pas de limiteur) | |
| Plages ±8/±16/±50 %, tonalité fixe oui/non et transposition ; retour au neutre | |
| Calage BPM exact, moitié et double, y compris quand une cible sort de la plage | |
| Tonalité/Camelot, compatibilité et calage harmonique cohérents avec les tags et transpositions | |
| Grille manuelle, conservation après rechargement et SYNC ponctuel sur un morceau à tempo stable | |
| AUTO indépendant : fin de morceau et fin de liste ; noter les coupures (sans promesse de gapless) | |
| Alternance A/B : plusieurs transitions avec fondus 0/5/15 s, pause pendant le fondu et reprise manuelle | |

## ReplayGain et scrobbling

| Essai | Résultat / observations |
|---|---|
| ReplayGain désactivé : niveau habituel ; Piste/Album : niveaux conformes aux tags, sans à-coup au changement | |
| Absence de tags et gain Album absent : repli cohérent ; Actualiser tags met à jour le gain | |
| Même correction sur le son original et les stems ; réglage conservé après redémarrage | |
| Scrobbling coupé par défaut : aucune entrée DJ Decks envoyée | |
| foo_scrobble absent ou incompatible : activation refusée clairement, lecture normale intacte | |
| Avec foo_scrobble 1.6.x configuré, test manuel volontaire : une entrée après la durée audible requise ; noter la réception Last.fm séparément de la mise en file | |
| Pause, déplacement, platine coupée au crossfader et stems tous muets : pas de crédit d'écoute | |
| Désactiver dans DJ Decks ou dans Playback → Scrobble tracks ; les écoutes non envoyées sont abandonnées | |
| Même morceau dans le lecteur principal : pas de second chemin d'envoi depuis la platine ; nouvelle lecture après EOF testée | |
| Mode hors connexion puis reconnexion : vérifier la file de foo_scrobble et l'absence de répétition involontaire | |

Les envois de ce tableau sont des tests **manuels volontaires** : les tests
automatiques n'envoient rien à Last.fm. Le pont utilise les tags ordinaires ;
les mappings, expressions d'exclusion et filtres « bibliothèque uniquement » de
foo_scrobble ne sont pas repris. Voir [les limites](SCROBBLING_REPLAYGAIN.md).

## Stems et gestion du cache (essais)

| Essai | Résultat / observations |
|---|---|
| Nouveau morceau court : préparation CPU seule puis CUDA si disponible ; durée et appareil notés | |
| Annuler une préparation, changer de morceau et Arrêter/libérer pendant une préparation ; aucun résultat incomplet proposé | |
| Activer les quatre stems, mute, volumes, plusieurs Solo ; revenir au son original | |
| Vitesse, transposition, déplacement et SYNC avec stems sur les deux platines | |
| Réutiliser un cache existant après redémarrage, sans nouvelle séparation | |
| Engrenage → Cache des stems : taille des stems préparés et nombre de morceaux plausibles ; Actualiser fonctionne | |
| Vider le cache inutilisé pendant qu'une platine utilise des stems : ceux-ci restent audibles et sont conservés | |
| Vider pendant une préparation : celle-ci reste protégée et peut finir ou être annulée | |
| Son original ou Arrêter/libérer sur les deux decks, rouvrir le gestionnaire puis vider : caches libérés ; nouvelle préparation possible | |
| Après nettoyage : musique source, modèle IA et réglages intacts ; second nettoyage sans erreur | |
| Cache verrouillé/accès refusé/disque plein : message compréhensible, pas de blocage ; test sur copie/profil de test uniquement | |

La taille affichée concerne les dossiers de stems préparés à nom de clé reconnu,
hors travaux temporaires et moteur/modèle IA. Les caches utilisés, en préparation,
inaccessibles ou contenant des données inconnues sont conservés. Les suppressions
concernent seulement les fichiers générés ; les stems peuvent être recalculés.
Sur Windows 64 bits, foobar x86 peut utiliser le moteur Python x64 séparé. Aucun
moteur PyTorch x86 n'est fourni pour Windows 32 bits ; l'espace mémoire du foobar
x86 limite davantage les longs morceaux. Ne pas provoquer volontairement une
panne GPU/pilote sur la machine d'écoute : les replis sont aussi testés hors hôte.

## Session prolongée

Au moins une session de **deux heures**, avec deux decks, plusieurs morceaux,
transitions, changements de réglages et stems si disponibles. Noter les bruits,
blocages, erreurs, croissance mémoire observée manuellement et durée de fermeture.
Après un incident, vérifier la restauration au prochain lancement ; ne pas
forcer une fermeture uniquement pour remplir cette fiche.

## Rapport d'incident

- Résumé et gravité : blocage / perte de données / problème audio / interface.
- Configuration, versions et fonction active.
- Étapes numérotées, attendu, observé, fréquence (ex. 3 essais sur 3).
- Contournement éventuel ; avec et sans stems ; profil vierge si possible.
- Message d'erreur exact, en masquant les chemins personnels.

## Décision bêta

- Installation/chargement confirmés séparément en x64 et x86.
- Aucun blocage majeur, perte de données ou défaut audio reproductible non résolu.
- Fermetures et restauration fiables sur les configurations testées.
- Cas essentiels ci-dessus testés ; absences et limites restantes documentées.
- Retour d'au moins un testeur externe sur le paquet candidat.

Décision : préparation de la **bêta x64 autorisée par l’utilisateur** sur la base de ses validations. La validation x86 reste indépendante et en attente ; les essais détaillés non renseignés restent à effectuer au fil de la bêta.
