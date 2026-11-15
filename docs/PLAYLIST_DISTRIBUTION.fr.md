# Répartir la playlist active — 0.38.0-beta.1

Dans l’engrenage **Réglages → Playlists**, choisir **Répartir la playlist active de foobar : A / B…**.

La playlist active entière est copiée, dans son ordre affiché : pistes 1, 3, 5… vers A, pistes 2, 4, 6… vers B. Les doublons et sous-pistes sont conservés, sans modifier la playlist source. Le dialogue affiche son nom et les nombres de pistes avant de remplacer les deux listes des decks.

Les morceaux déjà chargés, leur position et le mixage restent conservés. Aucun nouveau morceau ne démarre. AUTO et les préécoutes des playlists sont arrêtés. Pour écouter les nouvelles listes depuis le début, charger leur première piste puis choisir le mode AUTO voulu sur les decks.

Une sauvegarde complète de récupération est créée avant remplacement. Utiliser **Revenir avant le dernier chargement ou répartition…** pour la restaurer (une seule sauvegarde de récupération, partagée avec les imports de session). L’enregistrement manuel d’une session permet de conserver plusieurs sauvegardes.

Aucune modification si la playlist est absente, vide, trop volumineuse, si un deck est encore en cours de chargement ou si la sauvegarde de sécurité échoue. Limites : 20 000 entrées au total, 10 000 par deck et 3 Mio de références sérialisées.

Validation : compilation et tests natifs sans lancement de foobar ; essai dans l’hôte à effectuer par l’utilisateur.
