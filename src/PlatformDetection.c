#include "PlatformDetection.h"

#ifdef CG_PLATFORM_WINDOWS
const char* GetWindowsErrorMessage(DWORD ErrorCode)
{
	const char* Message = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPSTR)&Message, 0, NULL);
	return Message;
}
#endif