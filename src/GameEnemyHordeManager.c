#include "GameEnemyHordeManager.h"

// Horde Helper Functions
int CreateEnemyHorde(EnemyHorde* pH, int NumOfEnemies, SpawnRecord* pSR);
int DestroyEnemyHorde(EnemyHorde* pH);

// Hordes
int MakeHorde(EnemyHorde* pH);
int SetHorde(EnemyHorde* pH);

//EnemyHordeData* FillEnemyData(EnemyHorde* pH);

int InitEnemyHordeManager(EnemyHordeManager* pHM)
{
    if (pHM == NULL)
    {
        Log_Warn("InitEnemyHordeManager() Warning: Enemy Horde Manager is NULL");
        return 0;
    }
    
    pHM->CurrentHorde = -1;
    pHM->NumOfHordes = 3;
    pHM->Horde.IDList = NULL;
    pHM->Horde.NumOfEnemies = 0;
    pHM->Horde.IsReady = 0;
    // pHM->Horde.EHD.Size = NULL;
    // pHM->Horde.EHD.HalfSize = NULL;
    // pHM->Horde.EHD.Pos = NULL;
    pHM->DebugFlag = 0;
    
    InitFileManager(&pHM->HFM);
    SetWorkingDirectory(&pHM->HFM, "resources/good hordes");
    ScanDirectory(&pHM->HFM, HF_EXTENSION, FM_MAX_FILES);

    //g_ScreenSize = GetTerminalSize();
    //g_ScreenSize.x = g_ScreenSize.x - (int)((float)g_ScreenSize.x / (float)4);

    return 1;
}

int DestroyEnemyHordeManager(EnemyHordeManager* pHM)
{
    if (pHM == NULL)
    {
        Log_Warn("DestroyEnemyHordeManager() Warning: Enemy Horde Manager is NULL");
        return 0;
    }

    DestroyEnemyHorde(&pHM->Horde);
    QuitFileManager(&pHM->HFM);
    pHM->CurrentHorde = 0;
    pHM->NumOfHordes = 0;

    return 1;
}

// Sending in the Next Horde
int SendNextEnemyHorde(EnemyHordeManager* pHM, SpawnRecord* pSpawnRecord)
{
    if (pHM == NULL)
    {
        Log_Warn("SendNextEnemyHorde() Warning: Enemy Horde Manager is NULL");
        return 0;
    }
    else if (pHM->CurrentHorde + 1 >= pHM->NumOfHordes)
    {
        //Log_Warn("SendNextEnemyHorde() Warning: No More Hordes Left");
        //return 0;
    }
    else if (pHM->HFM.NumOfFiles == 0)
    {
        //Log_Warn("SendNextEnemyHorde(): No Horde Files Found.");
        return 0;    
    }


    //Not In Editing Mode
    if (!pHM->DebugFlag)
    {
        DestroyEnemyHorde(&pHM->Horde);
    
        pHM->CurrentHorde = pHM->CurrentHorde + 1;

        char* File = pHM->HFM.ScannedFilesInDir[0];
        if (pHM->CurrentHorde < pHM->HFM.NumOfFiles)
            File = pHM->HFM.ScannedFilesInDir[pHM->CurrentHorde];

        ReadEnemyHordeFromFile(pHM, File, pSpawnRecord);
    }

    pHM->Horde.IsReady = 1;
    Log_Warn("SendNextEnemyHorde(): Keeping Horde.IsReady Always True");

    /*switch (pHM->CurrentHorde)
    {
        default:
            MakeTestHorde(&pHM->Horde, pSpawnRecord);
    }*/
    for (int i = 0; i < pHM->Horde.NumOfEnemies; ++i)
    {
        GOID ID = pHM->Horde.IDList[i];
        if (g_SharedCompList.PathCompList[ID].Size > 0)
            g_SharedCompList.DrawingCompList[ID].Spr.Position = g_SharedCompList.PathCompList[ID].pCoordsList[0];
    }

    Log_Info("SendNextEnemyHorde() -> Sent Out a New Horde!");
    return 1;
}

// Waiting for Current Horde to Die Out
int WaitForEnemyHorde(EnemyHordeManager* pHM, SpawnRecord* pSpawnRecord)
{
    if (pHM == NULL)
    {
        Log_Warn("WaitForEnemyHorde() Warning: Enemy Horde Manager is NULL");
        return 0;
    }
    else if (pHM->CurrentHorde + 1 > pHM->NumOfHordes)
    {
        //Log_Warn("WaitForEnemyHorde() Warning: No More Hordes Left");
        return 0;
    }

    // Are Enemies at Starting Positions?
    if (!pHM->Horde.IsReady)
    {
        int NumReady = 0;
        int NumDead = 0;
        for (int i = 0; i < pHM->Horde.NumOfEnemies; ++i)
        {
            GOID ID = pHM->Horde.IDList[i];

            // In-case Enemies Die Before Setting
            if (!pSpawnRecord->IsSpawned[ID])
            {
                ++NumDead;
                continue;
            }

            PathComponent* PC = &g_SharedCompList.PathCompList[ID];            
            if (PC->CurrentPass > 0 && PC->PathLogic == PATH_STOP_AT_END)
                ++NumReady;
        }

        if (NumReady >= pHM->Horde.NumOfEnemies - NumDead)
        {
            /*switch(pHM->CurrentHorde)
            {
                default:
                    SetTestHorde(&pHM->Horde);
            }*/
            
            pHM->Horde.IsReady = 1;
        }
        return 0;
    }

    for (int i = 0; i < pHM->Horde.NumOfEnemies; ++i)
    {
        if (pSpawnRecord->IsSpawned[pHM->Horde.IDList[i]])
            return 0;
    }

    //Log_Info("WaitForEnemyHorde() -> Current Horde Died!");
    return 1;
}

// Horde Functions
int CreateEnemyHorde(EnemyHorde* pH, int NumOfEnemies, SpawnRecord* pSR)
{
    if (pH == NULL || NumOfEnemies < 1 || NumOfEnemies > CG_MAX_NUM_OF_GAME_OBJ)
    {
        Log_Warn("CreateEnemyHorde() Warning: Horde is null OR Number of Enemies is Invalid = %d", NumOfEnemies);
        return 0;
    }

    pH->NumOfEnemies = NumOfEnemies;
    pH->IDList = (GOID*)malloc(sizeof(GOID) * NumOfEnemies);
    pH->IsReady = 0;

    // pH->EHD.Pos = (Vector2*)malloc(sizeof(Vector2) * NumOfEnemies);
    // pH->EHD.Size = (Vector2*)malloc(sizeof(Vector2) * NumOfEnemies);
    // pH->EHD.HalfSize = (Vector2*)malloc(sizeof(Vector2) * NumOfEnemies);

    SpawnExt(pSR, TAG_ENEMY, pH->NumOfEnemies, pH->IDList);
}

int DestroyEnemyHorde(EnemyHorde* pH)
{
    if (pH == NULL)
    {
        Log_Warn("DestroyEnemyHorde() Warning: Horde is NULL");
        return 0;
    }

    free(pH->IDList);
    // free(pH->EHD.Size);
    // free(pH->EHD.HalfSize);
    // free(pH->EHD.Pos);
    
    pH->IDList = NULL;
    pH->IsReady = 0;
    pH->NumOfEnemies = 0;
}


int ReadEnemyHordeFromFile(EnemyHordeManager* pHM, char* File, SpawnRecord* pSR)
{
    if (File == NULL)
    {
        Log_Warn("ReadEnemyHordeFromFile(): File is NULL");
        return 0;
    }

    //Log_Info("ReadEnemyHordeFromFile(): Destroying Current Horde. NumOfEnemies: %d", pHM->Horde.NumOfEnemies);
    
    // Destroy Previous Horde
    for (int i = 0; i < pHM->Horde.NumOfEnemies; ++i)
    {
        if (pSR->IsSpawned[pHM->Horde.IDList[i]])
            Despawn(pSR, pHM->Horde.IDList[i]);
    }
    DestroyEnemyHorde(&pHM->Horde);

    // Read and Load New Horde
    OpenFileFromName(&pHM->HFM, File, NULL, FM_READ_MODE);
    ReadFromHordeFile(&pHM->HFM, &pHM->Horde, pSR);
    CloseFile(&pHM->HFM);

    return 1;
}

int MakeHorde(EnemyHorde* pH)
{
    return 1;
}

int SetHorde(EnemyHorde* pH)
{
    return 1;
}


/* 
****************************** HORDES ******************************

EnemyHordeData* FillEnemyData(EnemyHorde* pH)
{
    if (pH == NULL)
    {
        Log_Warn("FillEnemyData(): Horde is NULL");
        return NULL;
    }

    EnemyHordeData* pEHD = &pH->EHD; 
    for (int i = 0; i < pH->NumOfEnemies; ++i)
    {
        pEHD->Pos[i] = g_SharedCompList.DrawingCompList[pH->IDList[i]].Spr.Position;
        pEHD->Size[i] = g_SharedCompList.DrawingCompList[pH->IDList[i]].Spr.Size;
        pEHD->HalfSize[i] = Vec2_Div(pEHD->Size[i], 2);
    }

    return pEHD;
}

int MakeTestHorde(EnemyHorde* pH, SpawnRecord* pSR)
{
    MakeHorde2(pH, pSR);                         
}

int SetTestHorde(EnemyHorde* pH)
{
    SetHorde2(pH);
}

int MakeHorde1(EnemyHorde* pH, SpawnRecord* pSR)
{
    int NumOfEnemies = 3;
    CreateEnemyHorde(pH, NumOfEnemies, pSR);

    EnemyHordeData* pEHD = FillEnemyData(pH);

    MakeEnemySingleLinePath(pH->IDList[0], 
                            CreateVec2(pEHD->Size[0].x, -g_ScreenSize.y / 2), 
                            CreateVec2(pEHD->Size[0].x, (g_ScreenSize.y / 2) - (pEHD->Size[0].y * 3)),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);  

    MakeEnemySingleLinePath(pH->IDList[1], 
                            CreateVec2(pEHD->Pos[0].x + pEHD->Size[0].x + pEHD->HalfSize[1].x, -g_ScreenSize.y / 2), 
                            CreateVec2(pEHD->Pos[0].x + pEHD->Size[0].x + pEHD->HalfSize[1].x, pEHD->Size[1].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);

    MakeEnemySingleLinePath(pH->IDList[2], 
                            CreateVec2(g_ScreenSize.x / 2, -g_ScreenSize.y / 2), 
                            CreateVec2(g_ScreenSize.x / 2, (g_ScreenSize.y / 2) - (pEHD->Size[2].y * 3)),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);
}

int SetHorde1(EnemyHorde* pH)
{
    EnemyHordeData* pEHD = FillEnemyData(pH);

    MakeEnemySingleDirShooting( pH->IDList[0],
                                BULLET_DIR_DOWN,
                                LARGE_ENEMY_BULLET_COOLDOWN_TIME,
                                SLOW_ENEMY_BULLET_SPEED);

    MakeEnemySingleDirShooting( pH->IDList[1],
                                BULLET_DIR_DOWN,
                                LARGE_ENEMY_BULLET_COOLDOWN_TIME,
                                SLOW_ENEMY_BULLET_SPEED);

    MakeEnemySingleDirShooting( pH->IDList[2],
                                BULLET_DIR_DOWN,
                                LARGE_ENEMY_BULLET_COOLDOWN_TIME,
                                SLOW_ENEMY_BULLET_SPEED);
}

int MakeHorde2(EnemyHorde* pH, SpawnRecord* pSR)
{
    int NumOfEnemies = 3;
    CreateEnemyHorde(pH, NumOfEnemies, pSR);

    EnemyHordeData* pEHD = FillEnemyData(pH);

    MakeEnemySingleLinePath(pH->IDList[0], 
                            CreateVec2(pEHD->Size[0].x, -g_ScreenSize.y / 2), 
                            CreateVec2(pEHD->Size[0].x, (g_ScreenSize.y / 2) - (pEHD->Size[0].y * 3)),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);  

    MakeEnemySingleLinePath(pH->IDList[1], 
                            CreateVec2((g_ScreenSize.x / 2) - pEHD->HalfSize[1].x, -g_ScreenSize.y / 2), 
                            CreateVec2((g_ScreenSize.x / 2) - pEHD->HalfSize[1].x, pEHD->Size[1].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);

    MakeEnemySingleLinePath(pH->IDList[2], 
                            CreateVec2(g_ScreenSize.x - pEHD->Size[2].x * 2, -g_ScreenSize.y / 2), 
                            CreateVec2(g_ScreenSize.x - (pEHD->Size[2].x * 2), (g_ScreenSize.y / 2) - (pEHD->Size[2].y * 3)),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);
}

int SetHorde2(EnemyHorde* pH)
{
    EnemyHordeData* pEHD = FillEnemyData(pH);

    MakeEnemySingleLinePath(pH->IDList[0], 
                            CreateVec2(pEHD->Pos[0].x, pEHD->Pos[0].y), 
                            CreateVec2(pEHD->Pos[0].x + (int)((float)pEHD->Size[0].x * 1.5f), pEHD->Pos[0].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_LOOP);  

    MakeEnemySingleLinePath(pH->IDList[2], 
                            CreateVec2(pEHD->Pos[2].x, pEHD->Pos[2].y), 
                            CreateVec2(pEHD->Pos[2].x - (int)((float)pEHD->Size[2].x * 1.5f), pEHD->Pos[2].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_LOOP);


    float CooldownTime = 0.5f;
    MakeEnemySingleDirShooting( pH->IDList[0],
                                BULLET_DIR_DOWN,
                                CooldownTime,
                                SLOW_ENEMY_BULLET_SPEED);

    MakeEnemySingleDirShooting( pH->IDList[1],
                                BULLET_DIR_DOWN,
                                CooldownTime,
                                SLOW_ENEMY_BULLET_SPEED);

    MakeEnemySingleDirShooting( pH->IDList[2],
                                BULLET_DIR_DOWN,
                                CooldownTime,
                                SLOW_ENEMY_BULLET_SPEED);

    ShootComponent* SC = &g_SharedCompList.ShootCompList[pH->IDList[1]];
    SC->CooldownTime = 0.2f;
    SetEnemyTimedShooter(pH->IDList[1], 0.5f, 0.4f);
}

int MakeHordeT1(EnemyHorde* pH, SpawnRecord* pSR)
{
    int NumOfEnemies = 2;
    CreateEnemyHorde(pH, NumOfEnemies, pSR);

    EnemyHordeData* pEHD = FillEnemyData(pH);

    MakeEnemySingleLinePath(pH->IDList[0], 
                            CreateVec2(pEHD->Size[0].x, -g_ScreenSize.y / 2), 
                            CreateVec2(pEHD->Size[0].x, pEHD->Size[0].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);  

    MakeEnemySingleLinePath(pH->IDList[1], 
                            CreateVec2((g_ScreenSize.x / 2) + pEHD->Size[1].x, -g_ScreenSize.y / 2), 
                            CreateVec2((g_ScreenSize.x / 2) + pEHD->Size[1].x, pEHD->Size[0].y),
                            //CreateVec2((g_ScreenSize.x / 2) + pEHD->Size[1].x, (g_ScreenSize.y / 2) - pEHD->Size[1].y * 2),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_STOP_AT_END);
}

int SetHordeT1(EnemyHorde* pH)
{
    EnemyHordeData* pEHD = FillEnemyData(pH);

    Vector2 MidPoint = Vec2_Sub(pEHD->Pos[1], pEHD->Pos[0]);
    MakeEnemySingleLinePath(pH->IDList[0], 
                            CreateVec2(pEHD->Pos[0].x, pEHD->Pos[0].y), 
                            Vec2_Add(MidPoint, pEHD->Pos[0]),
                            //CreateVec2((g_ScreenSize.x / 2), pEHD->Pos[0].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_REVERSE);  

    MakeEnemySingleLinePath(pH->IDList[1], 
                            CreateVec2(pEHD->Pos[1].x, pEHD->Pos[1].y), 
                            Vec2_Add(MidPoint, pEHD->Pos[1]),
                            //CreateVec2((g_ScreenSize.x / 2), pEHD->Pos[1].y),
                            NORM_ENEMY_SPEED, 
                            SMALL_ENEMY_WAIT_TIME, 
                            PATH_REVERSE);

    //Vector2 CooldownDist = CreateVec2(12, 7);
    float CooldownTime = 0.45f;

    MakeEnemySingleDirShooting( pH->IDList[0],
                                BULLET_DIR_DOWN,
                                CooldownTime,
                                SLOW_ENEMY_BULLET_SPEED);
    
    MakeEnemySingleDirShooting( pH->IDList[1],
                                BULLET_DIR_DOWN,
                                CooldownTime,
                                SLOW_ENEMY_BULLET_SPEED);

    SetEnemyTimedShooter(pH->IDList[0], 0.0f, 0.0f);
    SetEnemyTimedShooter(pH->IDList[1], 0.0f, 0.0f);
}
*/