# Scrobbling and ReplayGain — 0.27.1-dev

The detector accepts module-only names and DLL paths. If activation is refused, the message now specifies whether the component, supported version or exact service is missing. Settings → foo_scrobble diagnostic shows the detected module/version without submitting any track.


Both options are in the gear Settings menu, saved independently of foobar's main
playback preferences, and disabled by default. They do not change the main transport.

## ReplayGain

Choose Off, Track (gain + peak) or Album (gain + peak). DJ Decks uses the SDK's
ReplayGain calculation on each source track's metadata, before mixing. Album/track
fallback and peak clipping prevention follow the SDK. No gain tags means unity
gain unless a peak tag requires attenuation. There is no extra preamp, and foobar's
global ReplayGain preamp/order-dependent mode is not inherited or modified.

The factor applies once to the deck output, including its mixed stems, with the
existing block gain smoothing. Scanning, waveforms, cached stems and source tags
are unchanged. Refresh tags updates the factors; loading a track reads its metadata.
Peak prevention is based on the tagged original-track peak: stem isolation and
two-deck summation can still exceed full scale. This is not a mixer limiter.

## Optional foo_scrobble adapter

Enable **Scrobble audible decks via foo_scrobble**. Install/configure foo_scrobble
for Last.fm first and leave its Playback → Scrobble tracks switch enabled. No API
key, password or session token is read by DJ Decks. Its adapter queues entries in
foo_scrobble's existing asynchronous service; authentication, offline queue and
network retries remain foo_scrobble's responsibility.

The adapter is limited to the foo_scrobble 1.6.x family and checks its component
version and service GUID before use. The unversioned service/Track declarations
were taken from gix/foo_scrobble commit bc9e53db898f3293240a2477732ef909cb9477eb,
with MIT attribution. The three declarations also match the installed 1.6.0 build source at commit 657ddd74eb239a82156d5512741c4221fdccf7e9. A different component, absent service or unsupported version
leaves this feature unavailable. Live host/API compatibility is still experimental.

- Count output seconds on each deck, only while its mixed gain exceeds 0.001
  (−60 dB), scrobbling is enabled and it has an audible original/stem selection.
- No time credit for pauses, output stalls, muted decks, silent stem selections,
  seeking or faster source-position advance. Gain changes follow the same mixer.
- Tracks must exceed 30 seconds and have artist/title metadata. Queue after half
  the nominal track duration or 240 seconds, whichever is shorter, of audible
  output time. A 1-second UI poll means qualification may be delivered slightly later.
- One submission attempt per loaded playback occurrence; restarting after EOF
  creates a new occurrence. Seeking to the beginning alone does not reset it.
- If normal foobar playback holds the same item, discard this deck's unsubmitted
  time and leave scrobbling to normal playback, avoiding a second submission path.
- Both decks otherwise qualify independently. Two audible copies of the same item
  on separate decks are separate occurrences; avoid this if it would create unwanted
  duplicates. Successful queueing is logged, but is not proof of Last.fm acceptance.
- No DJ Decks Now Playing updates are sent, avoiding two decks competing for one
  Last.fm Now Playing slot. No changes are made to playback statistics callbacks.

The adapter uses ordinary artist/title/album/album artist/tracknumber/MusicBrainz
tags. **foo_scrobble's custom title-format mappings, skip-expression and
library-only filtering belong to its normal playback collector and are not applied
by this direct adapter.** Use the DJ Decks opt-in accordingly. Its global Scrobble
tracks switch is honored; disabling either switch discards unsubmitted time.

## Verification and manual checks

Automated checks exercise output-time thresholds, inactive/inaudible decks,
missing metadata/service, independent decks, global opt-out, normal-player conflict
and submission failure. The actual ScrobbleTick method runs against a mock backend;
no real scrobbles are sent. SDK ReplayGain scale code is tested with neutral,
track/album, missing-gain fallback and peak-limit cases, plus the existing audio suite.

Manual checks remain: service interoperability inside foobar, a real Last.fm
submission/offline retry with an opted-in test track, repeated playback and audible
ReplayGain switching with original audio and stems. No foobar capture or monitoring
is required. See the [beta test sheet](BETA_TEST_PLAN.fr.md).
