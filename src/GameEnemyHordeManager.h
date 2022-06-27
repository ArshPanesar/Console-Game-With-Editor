#include "FileManager.h"

#ifndef CG_GAME_ENEMY_HORDE_MGR_H
#define CG_GAME_ENEMY_HORDE_MGR_H

typedef struct HordeManagerStruct
{
    EnemyHorde Horde;
    FileManager HFM;
    int NumOfHordes;
    int CurrentHorde;

    short DebugFlag;
} EnemyHordeManager;

/* FUNCTIONS */


// Creating / Destroying
int InitEnemyHordeManager(EnemyHordeManager* pHM);
int DestroyEnemyHordeManager(EnemyHordeManager* pHM);

// Reading Enemy Horde Data from File (Destroys Previous Horde)
int ReadEnemyHordeFromFile(EnemyHordeManager* pHM, char* File, SpawnRecord* pSR);

// Sending in the Next Horde
int SendNextEnemyHorde(EnemyHordeManager* pHM, SpawnRecord* pSpawnRecord);

// Waiting for Current Horde to Die Out
int WaitForEnemyHorde(EnemyHordeManager* pHM, SpawnRecord* pSpawnRecord);

#endif //CG_GAME_ENEMY_HORDE_MGR_H
