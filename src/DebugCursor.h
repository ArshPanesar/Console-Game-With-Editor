#include "Spawner.h"

#ifndef CG_DEBUG_CURSOR_H
#define CG_DEBUG_CURSOR_H

extern GOID DebugCursorID;

int AllocateDebugCursor(SpawnRecord* pSpawnRecord);
int DeallocateDebugCursor(SpawnRecord* pSpawnRecord);

void InitCursor(GOID MenuID);
void UpdateCursor(void);
void QuitCursor(void);

// Set Cursor Empty Mode
int SetCursorEmptyMode(void);
// Set Cursor to Selectable Mode
int SetCursorSelectionMode(GOID TargetID);

// Makes the Cursor Move Around Game Objects in the List.
// List is Copied so Can Be Passed Locally.
int SetCursorMoveList(GOID* List, int Size);
int SetCursorMoveListWithStartingID(GOID* List, int Size, GOID StartID);
// Get the Current ID the Cursor is On. If Cursor is in Free Mode, Returns CG_INVALID_GOID
GOID GetCursorMoveListCurrentID(void);

// Makes the Cursor Move Freely Around the Screen
int SetCursorMoveFree();

#endif //CG_DEBUG_CURSOR_H
