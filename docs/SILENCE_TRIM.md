# Silences de début/fin et préchargement — 0.34.0-beta.1

## Français

Dans **⚙ → Silences / préchargement…**, activer indépendamment la suppression du
silence au début et à la fin. Les deux options sont désactivées par défaut. Le seuil
initial est **−60 dBFS**, réglable de −96 à −30 dBFS par préréglages. Le seuil porte
sur les échantillons décodés d’origine, avant ReplayGain, volume, crossfader et stems.
Un signal au-dessus du seuil sur n’importe quel canal est conservé.

**Les réglages s’appliquent aux prochains chargements.** Pour essayer sur les morceaux
déjà chargés, les recharger depuis leurs playlists après avoir choisi les options.
Le composant ne déplace pas un morceau en cours lorsqu’on change le seuil. Un seuil
plus élevé peut couper une introduction ou une fin très douce : commencer à −60,
puis diminuer le seuil si nécessaire. 0 dBFS serait le maximum numérique et ne serait
pas un réglage approprié pour détecter le silence.

Seules les bordures sont examinées ; les pauses à l’intérieur du morceau restent
intactes. Une marge de **20 ms** est gardée avant/après le son détecté. Les fichiers,
tags et durées originales ne sont pas modifiés. Les positions affichées restent celles
du fichier d’origine ; les boutons Début/relecture reviennent au début utile détecté.
L’option concerne le son des decks, pas les extraits de préécoute de playlist.

## Préparation et lecture automatique

- Lorsqu’une option est active, le chargement prépare le morceau en arrière-plan :
  ouverture du décodeur, détection des bordures et environ **250 ms de son source**
  déjà décodé, dans une limite de blocs/mémoire. Le deck reste en pause. Un clic
  Lecture pendant « Préparation des silences… » démarre dès que le morceau est prêt.
- En AUTO indépendant ou alterné, le morceau suivant de chaque liste concernée est
  préparé à l’avance. Les fondus A/B se déclenchent par rapport à la **fin utile**,
  en tenant compte de la vitesse. La limite de fin est appliquée dans la production
  audio, et non uniquement au rythme du rafraîchissement de l’interface.
- Si la préparation est trop lente, l’enchaînement attend le résultat ; il peut donc
  rester un blanc. Le changement de morceau et SoundTouch ne sont pas garantis
  parfaitement gapless. Ce préchargement réduit les lectures/calculs au moment de
  la transition, mais ne constitue pas une promesse d’absence de coupure.
- Sans option de silence active, le chemin de chargement précédent reste utilisé.
- Les lecteurs préchargés peuvent tenir ouverts les fichiers suivants. Utiliser
  **Arrêter/libérer** avant de modifier des tags : cela annule les préparations et
  libère leurs lecteurs. Ils ne reprennent pas automatiquement après cette libération
  jusqu’à une action explicite de lecture, chargement ou activation d’AUTO.
- Les lecteurs devenus inutiles après désactivation d’AUTO/fin de liste sont libérés.
  Les résultats ne sont pas enregistrés dans un cache disque.

## Limites et validation

La recherche examine au maximum les **60 premières et 60 dernières secondes**, de
façon exhaustive dans ces zones, pas à partir de la forme d’onde échantillonnée de
préécoute. Si aucun son n’est trouvé dans une zone, sa bordure d’origine est conservée.
Une piste entièrement silencieuse n’est donc pas supprimée ni sautée en boucle.
La durée doit être connue et le décodeur doit permettre le positionnement ; sinon
le morceau est conservé sans découpe. La qualité du positionnement dépend du décodeur.

Les préchargements sont bornés au suivant de chaque liste active et annulables ; la
réactivité d’une opération d’entrée/sortie bloquée dépend néanmoins du décodeur et du
stockage. Les réglages sont mémorisés. Un chargement encore en attente à la fermeture
est sauvegardé comme cible de restauration, avec lecture restaurée en pause.

Tests sur données simulées : seuils/canaux, marges, pauses internes, morceaux muets,
formats non positionnables, annulation, limite de fin selon la vitesse ; exécution
des vrais ordonnanceur/chargement différé et contrôleur AUTO avec décodeur simulé.
Aucun fichier musical personnel, périphérique réel ou foobar ouvert par les tests.
L’écoute réelle, la fluidité et les cas de stockage lent restent à confirmer.

## English

**Settings → Silence trimming / preload** independently enables leading/trailing
silence removal. Both default off; threshold defaults to **−60 dBFS**, with −96…−30
presets. Settings apply to newly loaded tracks; reload existing decks to try them.
Detection uses original decoded samples before gains/processing, across all channels.
Only edges are trimmed, with 20 ms guards; internal pauses, files and tags are untouched.

When enabled, preparation runs in the background and decodes roughly 250 ms ahead.
AUTO prepares the next queued track and uses the trimmed source end for speed-aware
fades. It waits if preparation is not ready; this is not a gapless guarantee. Stop/Release
cancels preloads and prevents automatic reopening until explicit playback/load/AUTO.

Scan limits are the first/last 60 seconds. Unknown duration, missing seek support or
no sound found preserve the corresponding original edge. Entirely silent tracks are
not removed. Native tests use simulated decoders/audio; live listening is still needed.
