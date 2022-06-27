#include "Terminal.h"

/* VARIABLE DECLARATIONS */
#ifdef CG_PLATFORM_WINDOWS
HANDLE Win_OutputHandle = NULL;
CHAR_INFO* Win_CharInfoBuffer = NULL;
COORD Win_MaxSize;
#endif

Vector2 Terminal_Size;


/* FUNCTION DECLARATIONS */
int IsSpriteInsideScreen(Sprite* pSprite);
int ClipSprite(Sprite* pSpr, Vector2* pNewPosition, Vector2* pNewSize);

/* FUNCTION DEFINITIONS */
int InitTerminal(void)
{
#ifdef CG_PLATFORM_WINDOWS
	Win_OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	
	CONSOLE_SCREEN_BUFFER_INFO CSBI;
	int Result = GetConsoleScreenBufferInfo(Win_OutputHandle, &CSBI);
	if(!Result)
	{
		Log_Critical("InitTerminal() on Windows: GetConsoleScreenBufferInfo() Failed. \
						Error Code:");
		return 0;
	}
	Win_MaxSize = (COORD){CSBI.dwMaximumWindowSize.X, CSBI.dwMaximumWindowSize.Y};
	COORD MaxSize = GetLargestConsoleWindowSize(Win_OutputHandle);
	//Log_Info("GetLargestConsoleWindowSize() -> (R, C) = (%d, %d)", MaxSize.Y, MaxSize.X);
	//Log_Info("Max Size Using CSBI -> (R, C) = (%d, %d)", Win_MaxSize.Y, Win_MaxSize.X);

	Terminal_Size = (Vector2){ CSBI.dwSize.X, CSBI.dwSize.Y };
	Vector2 Size;
	Size.x = MaxSize.X / 2;
	Size.y = MaxSize.Y - 20;
	SetTerminalSize(Size);

	Log_Info("InitTerminal(): Windows Console Initialized. Size: (R, C) = (%d, %d)", Terminal_Size.x, Terminal_Size.y);
#endif

	return 1;
}

int QuitTerminal(void)
{
#ifdef CG_PLATFORM_WINDOWS
	Win_OutputHandle = NULL;
	Terminal_Size =  (Vector2){0, 0};
	free(Win_CharInfoBuffer);
	Win_CharInfoBuffer = NULL;
	Log_Info("QuitTerminal(): Windows Console Closed.");
#endif	
}

int SetTerminalSize(Vector2 Size)
{
#ifdef CG_PLATFORM_WINDOWS
	COORD MaxSize = GetLargestConsoleWindowSize(Win_OutputHandle);
	//Log_Info("Requested Size: (%d, %d)", Size.x, Size.y);
	Size.x = (Size.x < MaxSize.X) ? Size.x : MaxSize.X;
	Size.y = (Size.y < MaxSize.Y) ? Size.y : MaxSize.Y;
	//Log_Info("Changed Size: (%d, %d)", Size.x, Size.y);

	//Log_Info("Max Dim: (X, Y) = (%d, %d)", MaxSize.X, MaxSize.Y);
	//Log_Info("Minimum Dim: (X, Y) = (%d, %d)", GetSystemMetrics(SM_CYMIN), GetSystemMetrics(SM_CXMIN));
	//Log_Info("PrevSize: ( %d, %d ), NewSize: ( %d, %d )", Terminal_Size.x, Terminal_Size.y, Size.x, Size.y);
	if (Terminal_Size.x < Size.x || Terminal_Size.y < Size.y)
	{
		//Log_Info("Terminal_Size.x < Size.x || Terminal_Size.y < Size.y");
		if (Terminal_Size.x < Size.x)
			Terminal_Size.x = Size.x;
		if (Terminal_Size.y < Size.y)	
			Terminal_Size.y = Size.y;
	
		COORD Win_Size = {Terminal_Size.x, Terminal_Size.y};
		int Result = SetConsoleScreenBufferSize(Win_OutputHandle, Win_Size);
		if(!Result)
		{
			Log_Warn("SetTerminalSize(): SetConsoleScreenBufferSize() Failed For Requested Size = (%d, %d).\nError Code: %d, %s", Size.x, Size.y, 
						Result, GetWindowsErrorMessage(GetLastError()));
			//return 0;
		}

		SMALL_RECT Win_Rect = {0, 0, Win_Size.X - 1, Win_Size.Y - 1};
		Result = SetConsoleWindowInfo(Win_OutputHandle, TRUE, &Win_Rect);
		if(!Result)
		{
			Log_Warn("SetTerminalSize(): SetConsoleWindowInfo() Failed.\nError Code: %d, %s", Result, GetWindowsErrorMessage(GetLastError()));
			//return 0;
		}
	}
	
	if (Terminal_Size.x > Size.x || Terminal_Size.y > Size.y)
	{
		//Log_Info("Terminal_Size.x >= Size.x || Terminal_Size.y >= Size.y");
		Terminal_Size.x = Size.x;
		Terminal_Size.y = Size.y;
	
		COORD Win_Size = {Terminal_Size.x, Terminal_Size.y};
		SMALL_RECT Win_Rect = {0, 0, Win_Size.X - 1, Win_Size.Y - 1};
		int Result = SetConsoleWindowInfo(Win_OutputHandle, TRUE, &Win_Rect);
		if(!Result)
		{
			Log_Warn("SetTerminalSize(): SetConsoleWindowInfo() Failed.\nError Code: %d, %s", Result, GetWindowsErrorMessage(GetLastError()));
			//return 0;
		}

		Result = SetConsoleScreenBufferSize(Win_OutputHandle, Win_Size);
		if(!Result)
		{
			Log_Warn("SetTerminalSize(): SetConsoleScreenBufferSize() Failed For Requested Size = (%d, %d).\nError Code: %d, %s", Size.x, Size.y, 
						Result, GetWindowsErrorMessage(GetLastError()));
			//return 0;
		}
	}

	free(Win_CharInfoBuffer);
	Win_CharInfoBuffer = (CHAR_INFO*)malloc(sizeof(CHAR_INFO) * Terminal_Size.x * Terminal_Size.y);
	for(int i = 0; i < (Terminal_Size.x * Terminal_Size.y); ++i)
		Win_CharInfoBuffer[i].Char.AsciiChar = ' ';
	
	Log_Info("SetTerminalSize(): Windows Console Resized. Size: (R, C) = ( %d, %d )", Terminal_Size.x, Terminal_Size.y);
	//Log_Info("SetTerminalSize(): MAX SIZE: (X, Y) = ( %d, %d )", MaxSize.Y, MaxSize.X);
	//printf("%d %d", Terminal_Size.x, Terminal_Size.y);
	
#endif

	return 1;
}

Vector2 GetTerminalSize(void)
{
	return Terminal_Size;
}

int IsSpriteInsideScreen(Sprite* pSprite)
{
	if((pSprite->Position.x + pSprite->Size.x) <= 0 || pSprite->Position.x > Terminal_Size.x
	|| (pSprite->Position.y + pSprite->Size.y) <= 0 || pSprite->Position.y > Terminal_Size.y)
	{
		//Log_Info("Position: (R, C) = (%d, %d)", pSprite->Position.x, pSprite->Position.y);
		//Log_Info("Size: (R, C) = (%d, %d)", pSprite->Size.x, pSprite->Size.y);
		//Log_Info("Terminal Size: (R, C) = (%d, %d)", Terminal_Size.x, Terminal_Size.y);
		return 0;
	}
	return 1;
}

int ClipSprite(Sprite* pSpr, Vector2* pNewPosition, Vector2* pNewSize)
{
	//Log_Info("Old Position: (R, C) = (%d, %d)", pSpr->Position.x, pSpr->Position.y);
	//Log_Info("OldSize: (R, C) = (%d, %d)", pSpr->Size.x, pSpr->Size.y);
	
	pNewPosition->x = (pNewPosition->x >= 0) ? pSpr->Position.x : 0;
	pNewPosition->y = (pNewPosition->y >= 0) ? pSpr->Position.y : 0;
	
	//int ClippedSizeX = pSpr->Size.x + (pNewPosition->x - abs(pSpr->Position.x));
	//int ClippedSizeY = pSpr->Size.y + (pNewPosition->y - abs(pSpr->Position.y));

	pNewSize->x = ((pNewPosition->x + pSpr->Size.x) <= Terminal_Size.x) ? pSpr->Size.x : (Terminal_Size.x - pNewPosition->x);
	pNewSize->y = ((pNewPosition->y + pSpr->Size.y) <= Terminal_Size.y) ? pSpr->Size.y : (Terminal_Size.y - pNewPosition->y);


	//Log_Info("New Position: (R, C) = (%d, %d)", pNewPosition->x, pNewPosition->y);
	//Log_Info("New Size: (R, C) = (%d, %d)", pNewSize->x, pNewSize->y);
	
	return 1;
}

void DrawSpriteToTerminal(Sprite* pSprite)
{
	//Log_Info("Terminal Size: (%d, %d)", Terminal_Size.x, Terminal_Size.y);
	//Log_Info("Position: (%d, %d), Size: (%d, %d)", pSprite->Position.x, pSprite->Position.y, pSprite->Size.x, pSprite->Size.y);
	if(!IsSpriteInsideScreen(pSprite))
		return;
	
	Vector2 Position = pSprite->Position;
	Vector2 Size = pSprite->Size;
	ClipSprite(pSprite, &Position, &Size);

	//if (Position.x < 0 || Position.y < 0)
	//	ClipSprite(pSprite, &Position, &Size);

#ifdef CG_PLATFORM_WINDOWS

	int BufferIndex, SpriteIndex;

	int StartX = (Position.x - pSprite->Position.x);
	int StartY = (Position.y - pSprite->Position.y);
	
	//Log_Info("Old Position: (%d, %d)", pSprite->Position.x, pSprite->Position.y);
	//Log_Info("New Position: (%d, %d)", Position.x, Position.y);
	//Log_Info("(x, y) = (%d, %d)", StartX, StartY);
	//Log_Info("Old Size: (%d, %d)", pSprite->Size.x, pSprite->Size.y);
	//Log_Info("New Size: (%d, %d)", Size.x, Size.y);
	
	for(int h = StartY; h < Size.y; ++h)
	{
		for(int w = StartX; w < Size.x; ++w)
		{
			BufferIndex = (Position.y + h - StartY) * Terminal_Size.x + Position.x + (w - StartX);
			
			//BufferIndex = (Position.y + h) + Position.x + w;
			
			//Log_Info("%d, %d", Size.x, Size.y);
			
			SpriteIndex = h * pSprite->Size.x + w;
			//Log_Info("%d, %d, %d", Size.x, Size.y, SpriteIndex);
			//SpriteIndex = ((Position.y - pSprite->Position.y) + h) * Size.x + (Position.x - pSprite->Position.x) + w;
			Win_CharInfoBuffer[BufferIndex].Char.AsciiChar = pSprite->CharBuffer[SpriteIndex];
			Win_CharInfoBuffer[BufferIndex].Attributes = pSprite->ColorBuffer[SpriteIndex];
		}
	}

#endif
}

void DisplayTerminal(void)
{
#ifdef CG_PLATFORM_WINDOWS
	COORD StartingCoord, BufferSize;
	StartingCoord.X = 0;
	StartingCoord.Y = 0;
	BufferSize.X = Terminal_Size.x;
	BufferSize.Y = Terminal_Size.y;
	SMALL_RECT ScreenRect;
	ScreenRect.Left = 0;
	ScreenRect.Top = 0;
	ScreenRect.Right = BufferSize.X;
	ScreenRect.Bottom = BufferSize.Y;
	
	DWORD Result = WriteConsoleOutputA(Win_OutputHandle, Win_CharInfoBuffer, BufferSize, StartingCoord, &ScreenRect);
	//if(!Result)
		//Log_Warn("Error at DisplayTerminal(): WriteConsoleOutput Failed! \n%s", GetWindowsErrorMessage(Result));
#endif
}

void ClearTerminal(void)
{
#ifdef CG_PLATFORM_WINDOWS
	for(int i = 0; i < (Terminal_Size.x * Terminal_Size.y); ++i)
	{
		Win_CharInfoBuffer[i].Char.AsciiChar = ' ';
		Win_CharInfoBuffer[i].Attributes = 0;
	}
#endif
}
