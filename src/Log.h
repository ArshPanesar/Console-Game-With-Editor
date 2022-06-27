#include <stdio.h>
#include <stdarg.h>

#ifndef CG_LOG_H
#define CG_LOG_H

extern FILE* LogFile;

/* FUNCTION DECLARATIONS */
// Log File Operations
int OpenLogFile(const char* Path);
int CloseLogFile();

/*
*	Logging Levels: 
*	Information -> Information on a Function/System State Change.
*	Warning 	-> Something Went Wrong but probably won't Break the System.
*	Critical 	-> Something Went Terribly Wrong and will Break the System.
*/

#define LOG_LEVEL_INFO

#ifdef LOG_LEVEL_INFO

#define Log_Info(Message, ...)	fprintf(LogFile, "[INFO] ");\
								fprintf(LogFile, Message, ##__VA_ARGS__);\
								fprintf(LogFile, "\n");\
								fflush(LogFile);

#define Log_Warn(Message, ...)	fprintf(LogFile, "[WARN] ");\
								fprintf(LogFile, Message, ##__VA_ARGS__);\
								fprintf(LogFile, "\n");\
								fflush(LogFile);


#define Log_Critical(Message, ...)	fprintf(LogFile, "[CRITICAL] ");\
									fprintf(LogFile, Message, ##__VA_ARGS__);\
									fprintf(LogFile, "\n");\
									fflush(LogFile);

#endif

#ifdef LOG_LEVEL_WARN

#define Log_Info(Message, ...)

#define Log_Warn(Message, ...)	fprintf(LogFile, "[WARN] ");\
								fprintf(LogFile, Message, ##__VA_ARGS__);\
								fprintf(LogFile, "\n");\
								fflush(LogFile);


#define Log_Critical(Message, ...)	fprintf(LogFile, "[CRITICAL] ");\
									fprintf(LogFile, Message, ##__VA_ARGS__);\
									fprintf(LogFile, "\n");\
									fflush(LogFile);

#endif

#ifdef LOG_LEVEL_CRITICAL

#define Log_Info(Message, ...)

#define Log_Warn(Message, ...)

#define Log_Critical(Message, ...)	fprintf(LogFile, "[CRITICAL] ");\
									fprintf(LogFile, Message, ##__VA_ARGS__);\
									fprintf(LogFile, "\n");\
									fflush(LogFile);

#endif

#endif //CG_LOG_H
