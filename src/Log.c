#include "Log.h"

// Logging to a File
FILE* LogFile = NULL;

/* FUNCTION DEFINITIONS */
int OpenLogFile(const char* Path)
{
	LogFile = fopen(Path, "w");
	if(LogFile != NULL)
		return 1;
	
	return 0;
}

int CloseLogFile()
{
	if(fclose(LogFile) != EOF)
		return 1;
	
	return 0;
}
