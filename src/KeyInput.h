#include <stdio.h>
#include <stdlib.h>
#include "PlatformDetection.h"

#ifndef CG_KEY_INPUT_H
#define CG_KEY_INPUT_H

#define ARR_LEN(a) (sizeof(a) / sizeof(a[0]))

#ifdef CG_PLATFORM_WINDOWS

#define CG_KEY_ESC 		0x1B
#define CG_KEY_SPACE 	0x20

#define CG_KEY_LEFT		0x25
#define CG_KEY_RIGHT	0x27
#define CG_KEY_UP		0x26
#define CG_KEY_DOWN		0x28

#define CG_KEY_W 		0x57
#define CG_KEY_A 		0x41
#define CG_KEY_S 		0x53
#define CG_KEY_D 		0x44
#define CG_KEY_Z        0x5A

#define CG_KEY_0        0x30
#define CG_KEY_1        0x31
#define CG_KEY_2        0x32
#define CG_KEY_3        0x33
#define CG_KEY_4        0x34
#define CG_KEY_5        0x35
#define CG_KEY_6        0x36
#define CG_KEY_7        0x37
#define CG_KEY_8        0x38
#define CG_KEY_9        0x39


SHORT IsKeyPressed(int KeyCode);
#endif

#endif //CG_KEY_INPUT_H