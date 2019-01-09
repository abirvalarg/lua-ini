#pragma once

#ifndef NULL
#define NULL 0
#endif

#define INI_SECTION_NAME_START ('[')
#define INI_SECTION_NAME_STOP (']')
#define INI_SEPORATE ('=')
#define INI_COMMENT ('#')
#define INI_SCREEN ('\\')

enum INI_ERRORS { OK=0, INI_NO_SECTION, INI_UNEXP_EOF, INI_STR_TOO_BIG, INI_COMM_AFTER_KEY, INI_UNALLOW_NAME };
extern const char *ini_errors_text[];
