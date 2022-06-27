#include "Spawner.h"

int Spawn(SpawnRecord* pRecord, enum eTag Name, int Num)
{
    GOID IDList[Num];
    return SpawnExt(pRecord, Name, Num, IDList);
}

int SpawnExt(SpawnRecord* pRecord, enum eTag Name, int Num, GOID IDList[])
{
    GOID (*FuncPtr)() = NULL;
    switch(Name)
    {
        case TAG_PLAYER:
        FuncPtr = CreatePlayer;
        break;

        case TAG_ENEMY:
        FuncPtr = CreateEnemy;
        break;

        case TAG_BULLET:
        FuncPtr = CreateBullet;
        break;
    }

    if (FuncPtr == NULL)
    {
        Log_Warn("Spawn() Error: No Creation Function for Tag %s Found!", TagInString(Name));
        return 0;
    }
    else if (pRecord == NULL)
    {
        Log_Warn("Spawn() Error: Record is NULL for Tag = %s", TagInString(Name));
        return 0;
    }

    //Log_Info("Spawn() Info: Spawning %d IDs for Tag = %s", Num, TagInString(Name));
    for (int i = 0; i < Num; ++i)
    {
        IDList[i] = FuncPtr();
        pRecord->IsSpawned[IDList[i]] = 1;
        pRecord->TagList[IDList[i]] = Name;
    }

    return 1;
}

int Despawn(SpawnRecord* pRecord, GOID ID)
{
    GOID IDList[] = {ID};
    return DespawnGroup(pRecord, IDList, 1);
}

int DespawnGroup(SpawnRecord* pRecord, GOID IDList[], int Num)
{
    
    int (*FuncPtr)(GOID) = NULL;
    enum eTag Name = pRecord->TagList[IDList[0]];
    switch(Name)
    {
        case TAG_PLAYER:
        FuncPtr = DestroyPlayer;
        break;

        case TAG_ENEMY:
        FuncPtr = DestroyEnemy;
        break;

        case TAG_BULLET:
        FuncPtr = DestroyBullet;
        break;
    }

    if (FuncPtr == NULL)
    {
        Log_Warn("Despawn() Warning: Tag %d Not Recognized!", TagInString(Name));
        return 0;
    }
    else if (pRecord == NULL)
    {
        Log_Warn("Despawn() Warning: Record is NULL for Tag = %s", TagInString(Name));
        return 0;
    }

    //Log_Info("Despawn() Info: Despawning %d IDs of Tag = %s", Num, TagInString(Name));
    for (int i = 0; i < Num; ++i)
    {
        GOID ID = IDList[i];
        if (pRecord->IsSpawned[ID] > 0)
        {
            pRecord->IsSpawned[ID] = 0;
            pRecord->TagList[ID] = TAG_NONE;
            FuncPtr(ID);
        }
    }

    return 1;
}
