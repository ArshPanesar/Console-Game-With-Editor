#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>
#include "GameWorld.h"

double Counter = 0.0;

double StartTimer(void)
{
#ifdef CG_PLATFORM_WINDOWS
	LARGE_INTEGER Ticks;
    QueryPerformanceCounter(&Ticks);
	Counter = (double)Ticks.QuadPart;
	return Counter;
#endif
}

//In Seconds
double GetElapsedTime(void)
{
#ifdef CG_PLATFORM_WINDOWS
    LARGE_INTEGER Ticks;
	QueryPerformanceFrequency(&Ticks);
	double CounterFreq = (double)Ticks.QuadPart;
    
    QueryPerformanceCounter(&Ticks);
	return ((double)Ticks.QuadPart - Counter) / CounterFreq;
#endif
}

int main()
{
	srand(time(0));

	OpenLogFile("Log.txt");
	
	Log_Info("Logging Session Started!");
	InitTerminal();
	
	g_TerminalSize = GetTerminalSize();

	const double dt_limit = 1.0 / 60.0;
	g_dtAsSeconds = dt_limit;
	unsigned long Start = 0, End = 0;

	InitGameWorld();

	//g_dtAsSeconds = dt_limit;
	while(!IsKeyPressed(CG_KEY_ESC))
	{
		Start = StartTimer();
		UpdateGameWorld();
		DrawGameWorld();

		// Timing	
		g_dtAsSeconds = GetElapsedTime();
		while(g_dtAsSeconds < dt_limit)
			g_dtAsSeconds = GetElapsedTime();
		
		//Log_Info("FPS: %lf\tDeltaTime: %lf", 1.0 / g_dtAsSeconds, g_dtAsSeconds);
	}
	
	DestroyGameWorld();
	QuitTerminal();
	
	Log_Info("Logging Session Ended!");
	CloseLogFile();
	
	return 0;
}