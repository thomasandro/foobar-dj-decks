#include "stdafx.h"
#include "foo_dj_version.h"
DECLARE_COMPONENT_VERSION("DJ Decks", FOO_DJ_VERSION,
    "Experimental dual DJ decks for foobar2000 / Double platine DJ experimentale.\n"
    "Copyright (c) 2026 DJ Decks contributors.\n"
    "SoundTouch 2.3.3, Copyright (c) Olli Parviainen 2001-2024, LGPL-2.1-or-later.\n"
    "Includes foobar2000 SDK-derived code, WTL and picojson; see THIRD_PARTY_NOTICES.md.\n"
    "Complete matching source and build instructions accompany the source release.");
VALIDATE_COMPONENT_FILENAME("foo_dj.dll");
FOOBAR2000_IMPLEMENT_CFG_VAR_DOWNGRADE;
