# Session files / Fichiers de session — 0.37.0

## Français

Dans **Réglages → Playlists**, quatre commandes sont disponibles :

- **Enregistrer les deux playlists dans un fichier…** : ordre, doublons et sous-pistes des listes A et B ; références aux morceaux, sans audio copié.
- **Enregistrer une session complète (playlists + platines)…** : les deux listes et les morceaux chargés, positions, points A/B (y compris A seul), grilles manuelles, vitesse/plage, transposition, tonalité fixe, Q, volumes, crossfader et durée/courbe de fondu. Les points et grilles enregistrés des morceaux présents dans les listes sont également inclus.
- **Charger une sauvegarde…** : sélectionner un fichier `.dj-decks.json` ou `.json`. Pour une session complète, choisir Oui pour restaurer aussi les platines, Non pour les listes seules, ou Annuler. Les deux listes sont remplacées, pas fusionnées.
- **Revenir à la session avant le dernier chargement…** : ouvrir la sauvegarde de sécurité conservée dans le profil. Une seule sauvegarde précédente est conservée, pas un historique illimité.

**Listes seules** : les morceaux actuellement chargés et leur lecture sont conservés ; l’extrait casque et les automatismes sont arrêtés. Le repère « Actuel » est rapproché du morceau réellement chargé, si présent dans la nouvelle liste.

**Session complète** : les platines sont restaurées en pause, les boucles et AUTO restent désactivés, et Cue A/B est coupé. Les points sont en temps source ; activer ensuite la boucle et Lecture si souhaité. Les fichiers sont ouverts avec les décodeurs foobar habituels. Le préchargement/suppression des silences actuel du profil reste applicable ; les positions sont bornées à la durée utile et les points incompatibles sont ignorés. Laisser finir un chargement de deck avant d’importer ou d’exporter une session complète.

Les références à un fichier indisponible restent dans la sauvegarde et les listes ; sa platine reste vide/en erreur, sans reprendre l’ancien morceau. Reconnecter le disque ou vérifier les chemins puis recharger le fichier de session. Les fichiers audio doivent rester accessibles aux mêmes chemins ; il n’y a ni copie de musique ni relocalisation automatique.

Les réglages généraux du profil (apparence, format d’affichage, ReplayGain, périphériques de sortie, configuration foo_scrobble) ne sont pas remplacés. Les stems ne sont pas embarqués/réactivés : les platines repartent du son original ; leur préparation et cache restent locaux. Les tags BPM/tonalité sont relus depuis la bibliothèque/fichier.

## Fiabilité et confidentialité

Format JSON UTF-8 versionné (`format: foo_dj.session`, `version: 1`), avec chemins et numéros de sous-piste explicites. Le fichier est validé avant modification de la session. Limites : 10 000 entrées par liste, 3 Mio combinés pour les références de playlists, 16 Mio pour le fichier ; les bibliothèques de repères fusionnées doivent rester dans leur limite de stockage locale de 4 Mio chacune. Un fichier invalide est refusé sans modifier les platines/listes.

L’écriture passe par un fichier temporaire unique dans le même dossier, vidé sur disque puis remplacé atomiquement ; si l’écriture/remplacement échoue, l’ancien fichier est conservé et le temporaire créé est supprimé. Avant de charger, l’état actuel est enregistré dans `foo_dj-before-import.json` du profil ; si cette sauvegarde de sécurité échoue, le chargement est annulé.

Les JSON contiennent des chemins locaux et les références de morceaux : les vérifier/masquer avant partage. Les fichiers de session et la récupération locale sont exclus de l’archive des sources. Aucun transfert réseau, scrobble, lecture ou préparation de stems n’est déclenché par les tests ; l’utilisateur confirme ensuite le fonctionnement dans foobar.

## English

**Settings → Playlists** offers: save both playlists, save a complete session, load a saved file, and restore the session before the last import. Files are versioned UTF-8 JSON, normally `.dj-decks.json`. No music is embedded. Complete sessions include loaded tracks/source positions, loop points, manual grids, tempo/range, pitch/key lock, Q, volumes, crossfader/fade settings and saved markers for referenced playlist tracks.

Loading replaces both playlists. A full-session file prompts whether to also restore decks. List-only loading preserves deck playback while stopping previews/AUTO; full loading restores paused, with loops/AUTO and Cue A/B disabled. Source paths must remain accessible. Missing references remain recorded; unavailable decks do not resume an old track. Let pending deck loads complete first.

General profile settings and output devices are unchanged. Stems are not embedded/reactivated; original audio is loaded. Tags are reread normally. Current silence/preload options still apply, so restored positions/points are bounded to actual source/useful duration.

The entire document is validated before applying; limits are 10,000 entries per list, 3 MiB combined playlist references, 16 MiB file size and 4 MiB per merged local marker library. Saving uses a unique temporary file and atomic replacement; failed replacement preserves the old file. Import first saves a one-level recovery snapshot `foo_dj-before-import.json` in the profile, and aborts if that safety save fails. JSON files include private local paths; review before sharing.
