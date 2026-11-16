# External BPM analysers — 0.40.0

Settings → Audio → BPM analyser: **Auto**, **foo_smart_tempo**, **foo_bpm**. The existing BPM analysis button on either deck uses the selected provider. Auto prefers Smart Tempo when its automatic analysis command is registered, otherwise foo_bpm. Explicit choices do not fall back silently. No available selected command: a message appears before audio readers are released.

The integration calls the registered context command on the loaded file/subsong after the existing release-for-tags step. It does not copy or embed the analyser, change its preferences, benchmark its accuracy or install it. Its dialogs and tag-writing/overwrite choices remain in force. The tagged field must be **BPM** for DJ Decks to read it. Save the result, then use **Refresh tags**; resume playback manually once analysis/tag writing finishes. Other components/apps can still hold the file.

Key analysis is unchanged (foo_musical_key). Component versions without the expected public command GUID report unavailable; no menu-name guessing is performed.

Source verification: [Smart Tempo context command](https://github.com/lxsdd/foo_smart_tempo-public/blob/ac4945f7816d05f5a5a3cef5f4147eb86d8080d7/src/foo_smart_tempo/bpm_contextmenu_item.cpp) and [GUID definition](https://github.com/lxsdd/foo_smart_tempo-public/blob/ac4945f7816d05f5a5a3cef5f4147eb86d8080d7/src/foo_smart_tempo/guid.h). Automatic analysis GUID: `51fa6f82-7669-4090-997c-8507c6ec2e30`. The public source candidate may differ from an installed historical release; native tests use simulated registered services. Actual execution/completion and tag writing with the user's installed version remain to be checked in foobar.

## Français

Dans **Réglages → Audio → Analyseur BPM**, choisir Automatique, foo_smart_tempo ou foo_bpm. Automatique préfère Smart Tempo disponible, sinon foo_bpm. Un choix explicite manquant affiche une erreur sans changer d’analyseur. Installer/configurer le composant externe séparément ; aucun téléchargement ni aucune installation ne sont effectués par DJ Decks. Conserver **BPM** comme nom du tag de sortie. Valider/enregistrer le résultat, **Actualiser les tags**, puis reprendre la lecture manuellement. Le retour de StyxCrosser sur la précision n’est pas une mesure réalisée par DJ Decks.
