#include "DebugCursor.h"
#include "GameEnemyHordeManager.h"

#ifndef CG_DEBUG_EDITOR_H
#define CG_DEBUG_EDITOR_H

typedef struct SharedGameWorldDataStruct
{
    SpawnRecord* pSpawnRecord;
    GOID* pPlayerID;

    EnemyHordeManager* pHordeMgr;

    short* pRunShootSystemFlag;
    short* pRunPathSystemFlag;
    //char* VarFileNameList[FM_MAX_FILES];
    //int NumOfVarFiles;

} SharedGameWorldData;

/* Editing Data Must Be Allocated Before Game Starts and Deallocated Before Game Ends */
int AllocateEditingData(SpawnRecord* pSpawnRecord);
int DeallocateEditingData(SpawnRecord* pSpawnRecord);

/* Debug: Editing Functions */
int InitEditingMode(SharedGameWorldData* pSharedData);
void UpdateEditingMode(void);
void QuitEditingMode(void);

#endif //CG_DEBUG_EDITOR_H
