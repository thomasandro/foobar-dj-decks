# Half/double tempo matching — 0.22.0-dev

The BPM-match and SYNC buttons now compare three targets: the reference deck's
effective tagged BPM (tag × speed), half of it, and double it. Only candidates
inside the destination's selected ±8%, ±16% or ±50% range are eligible. Choose the
candidate needing the smallest absolute speed change from the destination's
current setting, preferring exact BPM if tied. Failure leaves speed unchanged.

Examples from neutral speed:

- Destination 86, reference 174: target 87, speed +1.16%, relation ½×reference.
- Destination 174, reference 86: target 172, speed −1.15%, relation 2×reference.
- Destination 100, reference 140, ±50%: 70 needs −30%, while 140 needs +40%, so
  half tempo wins. If the destination is already at +30%, exact 140 is closer.

The speed caption shows **½×A/B** or **2×A/B** when the effective BPMs match that
relationship. It updates from actual BPM/speed values, including manual adjustments.
Metadata remains unchanged. Other multiples, triplets and automatic BPM detection
are not part of this operation.

SYNC uses a virtual half/double reference period to keep phases compatible after
matching. It retains the reference grid's anchor; it does not infer the first beat
of a bar or guarantee the desired grouping of every other beat. Existing beat-SYNC
limitations (tag accuracy, variable tempo, onset confidence and buffering) still apply.
Manual grid/phase adjustment remains available; tags and stored grids are untouched.

Français : les boutons BPM et SYNC choisissent le BPM exact, sa moitié ou son double,
selon le plus petit déplacement depuis la vitesse actuelle et la plage autorisée.
Le rapport s'affiche dans le libellé de vitesse. Les tags ne sont jamais modifiés.
