# Stem cache manager — 0.25.0-dev

Open the gear **Settings → Stem cache** (French: **Réglages → Cache des stems**).
The window shows the size in MiB and number of recognized prepared-stem entries.
Use **Refresh size** or **Clear unused cache**. Scanning and clearing run on a
below-normal background thread, outside the audio mutex. Closing the dialog waits
for the current operation to finish. The result shows removed/retained entries,
freed space, the updated size and any access/deletion errors.

Active stems and the current preparation key are protected. Open mapped readers
and worker byte locks also prevent removal, including readers/workers from another
process. To clear active stems as well, select **Original audio** or **Stop/Release**
on both decks and reopen the manager. This does not stop playback automatically.
Inactive loaded tracks may lose their generated cache; prepare them again to use stems.

Only the five expected direct files in recognized 64-lowercase-hex cache folders
are eligible. File handles are preflighted before deleting an entry; locked,
read-only, linked or unknown contents are retained. No recursive deletion is used.
Root/parent reparse points are refused. Source music, the model/Python engine,
session settings, lock files, temporary job directories and unknown/quarantined
folders are not removed. Temporary export/model sizes are not part of the display.

There is still no automatic size limit or eviction. Clearing only affects generated
audio; its preparation can be repeated. Per-file operating-system errors are
reported; a disk failure can leave part of an entry, which must be prepared again.

[French beta test sheet](BETA_TEST_PLAN.fr.md) includes the manual cache scenarios.
