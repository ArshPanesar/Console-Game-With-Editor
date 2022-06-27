#include "KeyInput.h"

#ifdef CG_PLATFORM_WINDOWS
SHORT IsKeyPressed(int KeyCode)
{
	return GetKeyState(KeyCode) & 0x8000;
}
#endif
