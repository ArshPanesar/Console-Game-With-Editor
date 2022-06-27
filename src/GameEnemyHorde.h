#include "Spawner.h"

#ifndef CG_GAME_ENEMY_HORDE_H
#define CG_GAME_ENEMY_HORDE_H

typedef struct EnemyHordeDataStruct
{
    Vector2* Pos;
    Vector2* Size;
    Vector2* HalfSize;
} EnemyHordeData;

typedef struct EnemyHordeStruct
{
    //EnemyHordeData EHD;
    GOID* IDList;
    int IsReady;
    int NumOfEnemies;
} EnemyHorde;


#endif //CG_GAME_ENEMY_HORDE_MGR_H
