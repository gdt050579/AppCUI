#pragma once

#include <windows.h>
#include <Shlobj.h>

#define CHARACTER_INFORMATION CHAR_INFO
#define SET_CHARACTER_VALUE(ptrCharInfo, value)                                                                        \
    {                                                                                                                  \
        (ptrCharInfo)->Char.UnicodeChar = (value);                                                                     \
    }
#define SET_CHARACTER_COLOR(ptrCharInfo, color)                                                                        \
    {                                                                                                                  \
        (ptrCharInfo)->Attributes = (color);                                                                           \
    }
#define SET_CHARACTER(ptrCharInfo, value, color)                                                                       \
    {                                                                                                                  \
        (ptrCharInfo)->Char.UnicodeChar = (value);                                                                     \
        (ptrCharInfo)->Attributes       = (color);                                                                     \
    }
#define GET_CHARACTER_COLOR(ptrCharInfo) ((ptrCharInfo)->Attributes)
#define GET_CHARACTER_VALUE(ptrCharInfo) ((ptrCharInfo)->Char.UnicodeChar)
#define KEYTRANSLATION_MATRIX_SIZE       256
