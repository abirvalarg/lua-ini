#pragma once

#ifndef NULL
#define NULL 0
#endif

#define INI_SECTION_NAME_START ('[')
#define INI_SECTION_NAME_STOP (']')
#define INI_SEPORATE ('=')
#define INI_COMMENT ('#')

enum INI_ERRORS { OK=0, NO_SECTION, UNEXP_EOF, STR_TOO_BIG };
const char *ini_errors_text[] = {
    NULL,
    "No section defined",
    "Unexpected EOF",
    "String too big",
};
