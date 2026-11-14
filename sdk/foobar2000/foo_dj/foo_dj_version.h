#pragma once
// Release candidate; see CHANGELOG.md at repository root.
#define FOO_DJ_VERSION "0.18.0-alpha.1"
#define FOO_DJ_WIDE_(x) L##x
#define FOO_DJ_WIDE(x) FOO_DJ_WIDE_(x)
#define FOO_DJ_VERSION_W FOO_DJ_WIDE(FOO_DJ_VERSION)
