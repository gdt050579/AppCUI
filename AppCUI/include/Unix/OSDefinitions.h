#ifndef _OS_DEFINITIONS_AND_INCLUDES_
#define _OS_DEFINITIONS_AND_INCLUDES_

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>


// dummy replacements for other systems
typedef struct { int characterCode, characterColor; } CHAR_INFO, CHARACTER_INFORMATION;
#define SET_CHARACTER_VALUE(ptrCharInfo,value)	    { (ptrCharInfo)->characterCode = (value); }
#define SET_CHARACTER_COLOR(ptrCharInfo,color)	    { (ptrCharInfo)->characterColor = (color); }
#define SET_CHARACTER(ptrCharInfo,value,color)      { (ptrCharInfo)->characterCode = (value);(ptrCharInfo)->characterColor = (color); }
#define GET_CHARACTER_COLOR(ptrCharInfo)            ((ptrCharInfo)->characterColor)
#define GET_CHARACTER_VALUE(ptrCharInfo)            ((ptrCharInfo)->characterCode)

#endif