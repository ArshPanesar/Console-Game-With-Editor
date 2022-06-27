#include "PlatformDetection.h"

#ifndef CG_COLOR_H
#define CG_COLOR_H

/*
*	Colors (OS Dependent):
*	Windows -> Windows Attributes
*	Linux/Mac -> ANSI Escape Codes
*/

#ifdef CG_PLATFORM_WINDOWS

#define CG_COLOR_RED 			FOREGROUND_RED | FOREGROUND_INTENSITY
#define CG_COLOR_BLUE 			FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define CG_COLOR_GREEN			FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define CG_COLOR_DARK_RED 		FOREGROUND_RED
#define CG_COLOR_DARK_BLUE 		FOREGROUND_BLUE
#define CG_COLOR_DARK_GREEN		FOREGROUND_GREEN

#define CG_COLOR_BLACK			0
#define CG_COLOR_WHITE			FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY

#define CG_COLOR_GRAY			FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN
#define CG_COLOR_YELLOW			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define CG_COLOR_MAGENTA		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define CG_COLOR_CYAN			FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define CG_COLOR_DARK_GRAY		0 | FOREGROUND_INTENSITY
#define CG_COLOR_DARK_YELLOW	FOREGROUND_RED | FOREGROUND_GREEN
#define CG_COLOR_DARK_MAGENTA	FOREGROUND_RED | FOREGROUND_BLUE
#define CG_COLOR_DARK_CYAN		FOREGROUND_GREEN | FOREGROUND_BLUE

#define CG_BGCOLOR_RED 			BACKGROUND_RED | BACKGROUND_INTENSITY
#define CG_BGCOLOR_BLUE 		BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define CG_BGCOLOR_GREEN		BACKGROUND_GREEN | BACKGROUND_INTENSITY
#define CG_BGCOLOR_DARK_RED 	BACKGROUND_RED
#define CG_BGCOLOR_DARK_BLUE 	BACKGROUND_BLUE
#define CG_BGCOLOR_DARK_GREEN	BACKGROUND_GREEN

#define CG_BGCOLOR_BLACK		0
#define CG_BGCOLOR_WHITE		BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY

#define CG_BGCOLOR_GRAY			BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN
#define CG_BGCOLOR_YELLOW		BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY
#define CG_BGCOLOR_MAGENTA		BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define CG_BGCOLOR_CYAN			BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define CG_BGCOLOR_DARK_GRAY	0 | BACKGROUND_INTENSITY
#define CG_BGCOLOR_DARK_YELLOW	BACKGROUND_RED | BACKGROUND_GREEN
#define CG_BGCOLOR_DARK_MAGENTA	BACKGROUND_RED | BACKGROUND_BLUE
#define CG_BGCOLOR_DARK_CYAN	BACKGROUND_GREEN | BACKGROUND_BLUE

typedef WORD Color;

#endif

#endif //CG_COLOR_H
