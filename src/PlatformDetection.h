#include <stdio.h>
#include <stdlib.h>

#ifndef CG_PLATFORM_DETECTION_H
#define CG_PLATFORM_DETECTION_H

// Windows 32-Bit and 64-Bit
#ifdef _WIN32

// Only Windows Console Functions Needed
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

const char* GetWindowsErrorMessage(DWORD ErrorCode);

#define CG_PLATFORM_WINDOWS

#else

// Other Platforms Will Be Ported to Later
#error OS Not Yet Supported

#endif


#endif //CG_PLATFORM_DETECTION_H
