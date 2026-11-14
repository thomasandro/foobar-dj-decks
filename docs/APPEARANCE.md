# Appearance — 0.23.2-dev

Open the **⚙ Settings / Réglages** menu in the lower strip. It now includes:

- **Theme → Glossy / Flat (subdued)** — **Thème → Brillant / Plat (sobre)**.
- **Deck A color / Deck B color** — **Couleur deck A / Couleur deck B**.

Eight presets: cyan, orange, blue, purple, green, rose/red, gold and grey.
Glossy remains the default, with cyan A and orange B. Choices are saved with
foobar configuration, independently of track/session files. The shared theme
and per-deck colors apply immediately to open floating playlists as well.

Flat mode keeps a dark neutral casing, subtle accent borders and solid button
fills. Glossy uses the existing plastic reflections and colored casings. Both
retain distinct hover, press and keyboard-focus states. Harmonic compatibility
colors keep their semantic green/amber/unknown meanings.

Header status labels (Playing/Ready/Free, Lecture/Prêt/Libre) reserve their
measured text width with extra inset from the rounded edge. Deck titles occupy
a separate rectangle and use ellipsis if space is insufficient. Track title and
artist scrolling is unchanged. Very narrow panels still have existing minimum
layout constraints; the tested minimum is 784 × 614 px.

Native tests render both modes offscreen and verify palette selection, button
state contrast and status fit at three font sizes across multiple panel widths.
Real-host appearance and saved preference restoration remain manual checks.
No host was launched, captured or monitored for these tests.

Français : dans **⚙ Settings / Réglages**, choisir le thème brillant ou plat et une couleur
pour chaque deck. Ces réglages sont conservés après fermeture normale de foobar.
