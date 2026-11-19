# DJ Decks 0.40.0-beta.1 — bêta x64 et x86

Depuis 0.38.1 : limiteur du mix principal activé par défaut à −1 dBFS, plafond réglable et voyant LIM/CLIP ; Échap pour masquer les playlists ; bouton Vider la liste avec confirmation ; sélection de l’analyseur BPM (Automatique, foo_smart_tempo ou foo_bpm). Le changelog détaille également la 0.39.0 non publiée séparément.

Le limiteur concerne les crêtes échantillonnées du mix, pas le true peak ni la préécoute casque. Vider conserve le morceau chargé et les fichiers musicaux, arrête AUTO/préécoute associés et l’alternance A/B ; l’autre liste est conservée. Pas d’annulation dédiée : exporter une session si besoin avant de vider.

Smart Tempo est facultatif et doit être installé séparément ; aucune installation n’est effectuée par DJ Decks. Automatique préfère sa commande disponible, sinon foo_bpm. Enregistrer le résultat dans BPM, Actualiser les tags puis reprendre manuellement. L’appel a été vérifié avec services simulés ; l’exécution réelle et l’écriture des tags restent à confirmer selon la version installée.

L’utilisateur confirme le fonctionnement de la version x64 installée. Les deux architectures sont compilées et testées nativement ; retours réels x86 bienvenus. Stems expérimentaux, préparation hors temps réel et moteur facultatif nécessitant Windows 64 bits même avec foobar x86.

[Release GitHub](https://github.com/thomasandro/foobar-dj-decks/releases/tag/v0.40.0-beta.1) · [Changelog](../CHANGELOG.md) · [Limiteur](MASTER_LIMITER.fr.md) · [Analyseurs BPM](BPM_ANALYSERS.md)
