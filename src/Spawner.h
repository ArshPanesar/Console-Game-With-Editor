#include "GamePlayer.h"
#include "GameEnemy.h"
#include "GameBullet.h"

#ifndef CG_SPAWNER_H
#define CG_SPAWNER_H

/* GAME FUNCTIONS */

// Spawning/Despawing Game Objects
typedef struct SpawnRecordStruct
{
    short IsSpawned[CG_MAX_NUM_OF_GAME_OBJ];
    enum eTag TagList[CG_MAX_NUM_OF_GAME_OBJ];
} SpawnRecord;

int Spawn(SpawnRecord* pRecord, enum eTag Name, int Num);
int SpawnExt(SpawnRecord* pRecord, enum eTag Name, int Num, GOID IDList[]);
int Despawn(SpawnRecord* pRecord, GOID ID);
int DespawnGroup(SpawnRecord* pRecord, GOID IDList[], int Num);

#endif //CG_SPAWNER_H
