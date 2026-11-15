# Automatic playback — 0.20.0-alpha.1

Experimental public alpha; installation is manual. Use the new black
**AUTO** button on each deck to open the settings menu (French/English).

## Positionnement pendant AUTO / Seeking during AUTO (0.34.1-beta.1)

Cliquer dans un morceau ou revenir à son début conserve désormais le mode AUTO
choisi, les états lecture/pause et l’ordre des playlists. L’alternance A/B n’est
plus désactivée par ce positionnement. Si un fondu est en cours, il garde sa
direction et la position actuelle du crossfader ; sa durée restante est recalculée
selon les nouvelles positions, les fins utiles et les vitesses des deux decks.
Un saut près de la fin peut donc raccourcir le fondu. Un deck en pause reste en pause.

Seeking within a deck, including return to start, preserves the selected AUTO
mode, playback states and queue positions. During a live crossfade, keep its
direction/current fader gain and replan its remaining duration from the updated
source positions, useful ends and speeds. A paused deck is not started by seeking.

## Independent playlists

Choose **Enchaîner cette liste / Auto-next on this deck**, then start that deck.
At its natural end, the next entry loads and starts on the same deck. Both decks
can independently use this mode. Pause does not advance the list. At the end of
the list playback stops; there is no automatic loop. An unreadable next entry
stops progression rather than repeatedly retrying or silently skipping tracks.
This mode does not overlap tracks on the same deck and is not guaranteed gapless.

## Alternating A / B

Load a playable current track on both decks. From the starting deck's AUTO menu,
choose **Alterner A / B depuis cette platine / Alternate A / B from this deck**.
This disables independent auto-next on both decks, pauses the other deck and moves
the crossfader to the chosen deck. If it is paused, press Play to start.

Near the end of A's current track, B starts at its loaded position and the automatic
crossfader moves to B. Once the fade completes, A stops and loads A's next entry,
paused, ready for the return transition. Playback proceeds A1 → B1 → A2 → B2 etc.
If one list is exhausted, the other finishes its remaining entries on its own,
without reusing the last exhausted track. There is no automatic playlist repeat.

The common transport or either deck's Play/Pause pauses/resumes alternation,
including both decks during a fade. Files explicitly released for tagging are
never reopened by this mode: resume them explicitly before arming alternation.

## Crossfader settings

The same menu on either deck provides shared A/B fade settings:

- Duration: **0, 1, 2, 3, 5, 8, 10, 15, 20 or 30 seconds**. Default 5 seconds.
  Zero means a direct handover with the mixer's normal block-level gain smoothing.
- Curve: **Puissance constante / Equal power** (default) or **Linéaire / Linear**.
  The curve also applies to the manual crossfader. Per-deck volumes remain active.

The trigger uses remaining source time divided by deck speed. The actual fade
clock counts produced output audio frames; pauses and output stalls cannot advance
it. Fade duration is shortened when a track has less time remaining. Unknown
duration falls back to handover at EOF. This is an experimental automatic transition,
not beat synchronization or guaranteed sample-accurate/gapless sequencing. Decoder,
buffering, duration accuracy, tempo changes and UI loading delays can affect timing.
There is still no limiter: equal-power fades can increase correlated signal peaks.

Moving/resetting the crossfader, seeking, manually loading a track, releasing a file
or choosing Manual cancels alternating automation. After taking manual control,
both decks can remain playing so the user can finish the mix. An incoming decoder
failure or premature EOF during a fade cancels alternation and returns the fader
to the outgoing deck. Invalid files are not automatically reopened or retried.

Independent auto-next flags, duration and curve are saved with the existing session.
Alternation requires explicit activation each session; restart never starts playback.
No new background worker, external monitoring or screenshot operation was added.
The controller uses the panel's existing UI timer and the existing audio thread.

## Validation

The x64 build and native tests cover the actual controller methods extracted from
the component: independent queues, manual pause, A/B/A order, list exhaustion,
unequal list lengths, failures, released readers, unknown duration and early EOF.
Fade tests cover both directions, pause and 44.1/48/96 kHz output rates; existing
layout, playlist-window and 30 audio duration/frequency tests also run independently
of foobar. Real-host transition quality, output backends, long sessions and restart
behavior still require manual testing after installation. No foobar launch, capture
or monitoring is part of this development verification.
