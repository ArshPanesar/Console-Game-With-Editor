#include "GameWorld.h"

GOID Player = CG_INVALID_GOID;

SpawnRecord* g_SpawnRecord;
EnemyHordeManager g_EnemyHordeMngr;
GOIDList DespawnList;

enum WorldState g_WorldState;

static FileManager g_HordeFileManager;

// Debug Flags
static short RunShootSystemDebugModeFlag = 0;
static short RunPathSystemDebugModeFlag = 0;

void AddToDespawnList(GOID ID)
{
    if (ID == CG_INVALID_GOID)
        return;

    if (g_SpawnRecord->IsSpawned[ID])
    {
        ++DespawnList.CurrentSize;
        GOIDArrayInsert(DespawnList.IDList, DespawnList.CurrentSize, ID, DespawnList.CurrentSize - 1);
        //Log_Info("AddToDespawnList(): ID = %d, Tag: %s Added to Despawn List", ID, TagInString(g_SharedCompList.InfoCompList[ID].Tag));
    }
}

void FreeDespawnList(void)
{
    if (DespawnList.CurrentSize > 0)
    {
        for (int i = 0; i < DespawnList.CurrentSize; ++i)
        {
            Despawn(g_SpawnRecord, DespawnList.IDList[i]);
            DespawnList.IDList[i] = CG_INVALID_GOID;
        }

        DespawnList.CurrentSize = 0;
        //Log_Info("FreeDespawnList(): All IDs Despawned! Resetting List.");
    }
}

void InitGameWorld(void)
{
    int ScreenX = GetTerminalSize().x;
    int ScreenY = (int)((float)GetTerminalSize().y * 1.25f);

    // Trial
    ScreenX = (int)((float)GetTerminalSize().x / 1.15f);

    SetTerminalSize(CreateVec2(ScreenX, ScreenY));
    Log_Info("Terminal Size: R, C -> %d, %d", GetTerminalSize().x, GetTerminalSize().y);
    
    g_ScreenSize = GetTerminalSize();

    InitGameData();

    g_SpawnRecord = (SpawnRecord*)malloc(sizeof(SpawnRecord));
    DespawnList.CurrentSize = 0;
    for (int i = 0; i < CG_MAX_NUM_OF_GAME_OBJ; ++i)
    {
        g_SpawnRecord->IsSpawned[i] = 0;
        g_SpawnRecord->TagList[i] = TAG_NONE;

        DespawnList.IDList[i] = CG_INVALID_GOID;
    }
    
    // Allocating Debug Data
    AllocateEditingData(g_SpawnRecord);

    g_WorldState = WORLD_STATE_RUN;

    if (g_WorldState == WORLD_STATE_RUN)
    {
        GOID IDList[1];
        SpawnExt(g_SpawnRecord, TAG_PLAYER, 1, IDList);
        Player = IDList[0];

        InitEnemyHordeManager(&g_EnemyHordeMngr);
        SendNextEnemyHorde(&g_EnemyHordeMngr, g_SpawnRecord);
    }

    //TEST_ShootFunctions();
    //TEST_PathFunctions();
    //TEST_VarFileOperations();
    //InitFileManager(&g_HordeFileManager);

    //EnemyHorde Horde;
    //Horde.IDList = NULL;
    /*Horde.NumOfEnemies = 2;
    Horde.IDList = (GOID*)malloc(sizeof(GOID) * Horde.NumOfEnemies);

    SpawnExt(g_SpawnRecord, TAG_ENEMY, Horde.NumOfEnemies, Horde.IDList);

    Vector2 Coords1[] = {
        CreateVec2(20, 20),
        CreateVec2(GetTerminalSize().x - 50, 20)
    };

    Vector2 Coords2[] = {
        CreateVec2(GetTerminalSize().x - 50, 30),
        CreateVec2(20, 30)
    };

    for (int i = 0; i < Horde.NumOfEnemies; ++i)
    {
        GOID ID = Horde.IDList[i];
        ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
        PathComponent* PC = &g_SharedCompList.PathCompList[ID];

        
        CreateShootPattern(SC, 2);
        
        SC->CooldownTime = 0.5f;
        SC->pDirList[0] = BULLET_DIR_DOWN;
        SC->pDirList[1] = BULLET_DIR_UP;
        SC->pSpeedList[0] = SLOW_ENEMY_BULLET_SPEED;
        SC->pSpeedList[1] = SLOW_ENEMY_BULLET_SPEED;

        CreatePath(PC, 2);
        if (i == 0)
            SetPathCoords(PC, Coords1, ARR_LEN(Coords1));
        else
            SetPathCoords(PC, Coords2, ARR_LEN(Coords2));
        Vector2 Speed = FAST_ENEMY_SPEED;
        SetPathSpeeds(PC, &Speed, 1);
        float Wait = 0.2f;
        SetPathWaitingTime(PC, &Wait, 1);
        PC->PathLogic = PATH_STOP_AT_END;
    }

    OpenFileFromName(&g_HordeFileManager, "hordetest", HF_EXTENSION, FM_WRITE_MODE);
    WriteToHordeFile(&g_HordeFileManager, &Horde);
    CloseFile(&g_HordeFileManager);

    DespawnGroup(g_SpawnRecord, Horde.IDList, Horde.NumOfEnemies);

    OpenFileFromName(&g_HordeFileManager, "hordetest", HF_EXTENSION, FM_READ_MODE);
    ReadFromHordeFile(&g_HordeFileManager, &Horde, g_SpawnRecord);
    CloseFile(&g_HordeFileManager);
    

    for (int i = 0; i < Horde.NumOfEnemies; ++i)
    {
        GOID ID = Horde.IDList[i];
        Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
        PathComponent* PC = &g_SharedCompList.PathCompList[ID];

        pSpr->Position = PC->pCoordsList[0];
    }
    */

    // free(Horde.IDList);
    //SetWorkingDirectory(&g_HordeFileManager, HF_DIRECTORY);

    /*char* Name = "hordelol";
    
    OpenFileFromName(&g_HordeFileManager, Name, HF_EXTENSION);
    CloseFile(&g_HordeFileManager);

    OpenFileFromName(&g_HordeFileManager, Name, HF_EXTENSION);
    CloseFile(&g_HordeFileManager);
*/
    
    //Log_Info("File Found: %s", SearchFileInDirectory(&g_FileManager, "horde1"));

/*    char* FilesInDir[FM_MAX_FILES];
    for (int i = 0; i < FM_MAX_FILES; ++i)
        FilesInDir[i] = NULL;

    int NumFiles = ScanDirectory(FM_DEFAULT_WORKING_DIR, NULL, FilesInDir, FM_MAX_FILES - 1);*/
    
    /*InitText();

    TextObj = CreateGameObj();
    AttachComponent(COMP_DRAWING, TextObj);

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[TextObj].Spr;

    pSpr->CharBuffer = NULL;
    pSpr->ColorBuffer = NULL;
    pSpr->Position = CreateVec2(GetTerminalSize().x / 2, GetTerminalSize().y / 4);
    pSpr->Size = CreateVec2(0, 0);

    ConvertTextToSprite(pSpr, "30", 0);*/
}

void UpdateGameWorld(void)
{
    static double Timer = 0.0f;
    static int EditModeRunning = 0;
    static int DespawnIndex = 0;

    if (Timer > 0.2f)
    {
        if (IsKeyPressed(CG_KEY_UP))
        {
            if (g_WorldState == WORLD_STATE_EDIT)
                g_WorldState = WORLD_STATE_RUN;
            else if (g_WorldState == WORLD_STATE_RUN)
                g_WorldState = WORLD_STATE_EDIT;
            
            Timer = 0.0f;
        }
    }
    else
        Timer += g_dtAsSeconds;


    // Game World Will Run in Either Editing Mode (Debug Menu)
    // Or Running Mode (Game)



    if (g_WorldState == WORLD_STATE_EDIT)
    {
        if (!EditModeRunning)
        {
            Log_Info("Game World: Editing Mode");

            SharedGameWorldData SD;
            SD.pSpawnRecord = g_SpawnRecord;
            SD.pRunShootSystemFlag = &RunShootSystemDebugModeFlag;
            SD.pRunPathSystemFlag = &RunPathSystemDebugModeFlag;
            SD.pHordeMgr = &g_EnemyHordeMngr;
            SD.pPlayerID = &Player;

            if (InitEditingMode(&SD))
                EditModeRunning = 1;
            else
                g_WorldState = WORLD_STATE_RUN;
        }
        else
        {
            UpdateEditingMode();

            // Run Some Systems in Debug Mode if Necessary
            if (RunShootSystemDebugModeFlag || RunPathSystemDebugModeFlag)
            {
                UpdateMovementSystem();
                UpdateInfoSystem();
                if (RunShootSystemDebugModeFlag)
                {
                    UpdateTimedShooterSystem();
                    UpdateConditionalShooterSystem();
                    UpdateShootSystem();
                }
                else
                    UpdatePathSystem();
                
                
                FreeDespawnList();
            }
        }
    }
    else if (g_WorldState == WORLD_STATE_RUN)
    {
        if (EditModeRunning)
        {
            QuitEditingMode();
            Log_Info("Game World: Running");
            EditModeRunning = 0;
        }
    
        if (WaitForEnemyHorde(&g_EnemyHordeMngr, g_SpawnRecord))
            SendNextEnemyHorde(&g_EnemyHordeMngr, g_SpawnRecord);
        
        UpdateMovementSystem();
        UpdateInfoSystem();
        UpdatePathSystem();
        UpdateCollisionSystem();
        UpdateTimedShooterSystem();
        UpdateConditionalShooterSystem();
        UpdateShootSystem();
        UpdateAnimationSystem();

        FreeDespawnList();
    }
}

void DrawGameWorld(void)
{
    ClearTerminal();
    
    GOIDList* pList = &g_SharedCompList.IDList_DrawingComp;
    for (int i = 0; i < pList->CurrentSize; ++i)
	    DrawSpriteToTerminal(&g_SharedCompList.DrawingCompList[pList->IDList[i]].Spr);
    
    DisplayTerminal();
}

void DestroyGameWorld(void)
{
    DestroyEnemyHordeManager(&g_EnemyHordeMngr);

    // Allocating Debug Data
    DeallocateEditingData(g_SpawnRecord);

    for (int i = 0; i < CG_MAX_NUM_OF_GAME_OBJ; ++i)
    {
        if (g_SpawnRecord->IsSpawned[i])
            Despawn(g_SpawnRecord, i);
    }

    free(g_SpawnRecord);
    g_SpawnRecord = NULL;

    //QuitFileManager(&g_HordeFileManager);
}

/* System Functions */
void UpdateMovementSystem(void)
{
    Vector2 Dir;
    Vector2 Speed;
    GOID ID;
    Sprite* Spr;

    static int FrameCount = 0;

    FrameCount += 1;
    for (int i = 0; i < g_SharedCompList.IDList_MovementComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_MovementComp.IDList[i];
        Dir = g_SharedCompList.MovementCompList[ID].Direction;
        Speed = g_SharedCompList.MovementCompList[ID].Speed;
        Spr = &g_SharedCompList.DrawingCompList[ID].Spr;
        
        float XBuildUp = g_SharedCompList.DrawingCompList[ID].PositionXBuildUp;
        float YBuildUp = g_SharedCompList.DrawingCompList[ID].PositionYBuildUp;

        float PosX = (float)Spr->Position.x + XBuildUp + ((float)(Dir.x * Speed.x) * (float)g_dtAsSeconds);
        float PosY = (float)Spr->Position.y + YBuildUp + ((float)(Dir.y * Speed.y) * (float)g_dtAsSeconds);
        
        XBuildUp += (float)(PosX - (float)Spr->Position.x);
        YBuildUp += (float)(PosY - (float)Spr->Position.y);
        
        const float BuildUpLimit = 0.5f;
        if (XBuildUp < -BuildUpLimit || XBuildUp > BuildUpLimit)
        {
            Spr->Position.x = (Dir.x < 0) ? (int)floorf(PosX) : (int)ceilf(PosX);
            XBuildUp = 0.0f;
        }

        if (YBuildUp < -BuildUpLimit || YBuildUp > BuildUpLimit)
        {
            Spr->Position.y = (Dir.y < 0) ? (int)floorf(PosY) : (int)ceilf(PosY);
            YBuildUp = 0.0f;
        }

        g_SharedCompList.DrawingCompList[ID].PositionXBuildUp = XBuildUp;
        g_SharedCompList.DrawingCompList[ID].PositionYBuildUp = YBuildUp;
    }
}

void UpdatePathSystem(void)
{
    GOID ID;
    PathComponent* PC;
    MovementComponent* MC;
    for (int i = 0; i < g_SharedCompList.IDList_PathComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_PathComp.IDList[i];
        PC = &g_SharedCompList.PathCompList[ID];
        MC = &g_SharedCompList.MovementCompList[ID];
        
        if (PC->PathLogic == PATH_STOP_AT_END && PC->CurrentPass > 0)
            continue;

        // Validate Data
        if (PC->pCoordsList == NULL || PC->pSpeedList == NULL || PC->pWaitTimeList == NULL)
            continue;

        Vector2 Pos = g_SharedCompList.DrawingCompList[ID].Spr.Position;

        int Index = PC->CurrentDestCoordIndex;
        int ShouldMove = 1;
        
        int AtDestination = Pos.x == PC->pCoordsList[Index].x && Pos.y == PC->pCoordsList[Index].y;
        if (AtDestination)
        {
            if (PC->CurrentTimer > PC->pWaitTimeList[PC->CurrentDestCoordIndex])
            {
                if (PC->CurrentDestCoordIndex + 1 == PC->Size || (PC->PathLogic == PATH_REVERSE && PC->CurrentDestCoordIndex == 0))
                {
                    PC->CurrentPass = PC->CurrentPass + 1;
                    if (PC->PathLogic == PATH_STOP_AT_END)
                        continue;
                }

                if (PC->PathLogic == PATH_REVERSE && PC->CurrentPass % 2 != 0)
                    Index = (Index - 1 >= 0) ? Index - 1 : 0;
                else
                    Index = (Index + 1 < PC->Size) ? Index + 1 : 0;

                PC->CurrentDestCoordIndex = Index;
                PC->CurrentTimer = 0.0f;
            }
            else
            {
                MC->Direction = CreateVec2(0, 0);
                PC->CurrentTimer += g_dtAsSeconds;
                ShouldMove = 0;
            }
        }

        if (!ShouldMove)
            continue; 
        
        MC->Direction = Vec2_Sub(PC->pCoordsList[PC->CurrentDestCoordIndex], Pos);
        MC->Direction = Vec2_Div(MC->Direction, Vec2_Mag(MC->Direction));
        MC->Speed = PC->pSpeedList[PC->CurrentDestCoordIndex];
    }
}

void UpdateShootSystem(void)
{
    GOID ID;
    ShootComponent* SC;
    MovementComponent* MC;

    for (int i = 0; i < g_SharedCompList.IDList_ShootComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_ShootComp.IDList[i];
        SC = &g_SharedCompList.ShootCompList[ID];
        MC = &g_SharedCompList.MovementCompList[ID];

        if (!SC->ShouldShoot)
            continue;

        SC->CurrentTimer += (float)g_dtAsSeconds;
        if (SC->CurrentTimer < SC->CooldownTime)
            continue;

        // Validate Data
        if (SC->pBulletIDList == NULL || SC->pDirList == NULL || SC->pPositionOffsetList == NULL || SC->pSpeedList == NULL)
            continue;

        /*Vector2 ShouldCheckDistance = CreateVec2(0, 0);
        int BIndex = -1;
        for (int j = 0; j < SC->NumOfBullets; ++j)
        {
            if (MC->Direction.x < 0 && SC->pDirList[j].x < 0)
            {
                ShouldCheckDistance.x = 1;
                BIndex = j;
                break;
            }
            else if (MC->Direction.x > 0 && SC->pDirList[j].x > 0)
            {
                ShouldCheckDistance.x = 1;
                BIndex = j;
                break;
            }
            else if (MC->Direction.y < 0 && SC->pDirList[j].y < 0)
            {
                ShouldCheckDistance.y = 1;
                BIndex = j;
                break;
            }
            else if (MC->Direction.y > 0 && SC->pDirList[j].y > 0)
            {
                ShouldCheckDistance.y = 1;
                BIndex = j;
                break;
            }
            else
            {
                ShouldCheckDistance = SC->pDirList[j];
                BIndex = j;
            }
        }

        int IsDistanceMaintained = 1;
        Vector2 BulletPos = CreateVec2(0, 0);
        if (BIndex > -1 && SC->pBulletIDList[BIndex] != CG_INVALID_GOID && g_SpawnRecord->IsSpawned[SC->pBulletIDList[BIndex]])
        {
            BulletPos = g_SharedCompList.DrawingCompList[SC->pBulletIDList[BIndex]].Spr.Position;
            int CheckX = ShouldCheckDistance.x != 0 && SC->DistanceBetweenBullets.x > abs(abs(BulletPos.x) - abs(SC->SourcePosition.x + SC->pPositionOffsetList[BIndex].x));
            int CheckY = ShouldCheckDistance.y != 0 && SC->DistanceBetweenBullets.y > abs(abs(BulletPos.y) - abs(SC->SourcePosition.y + SC->pPositionOffsetList[BIndex].y));
            if (CheckX || CheckY)
                IsDistanceMaintained = 0;
        }
        
        if (!IsDistanceMaintained)
            continue;
        
        */
        int Size = SC->NumOfBullets;
        GOID BulletList[Size];

        SpawnExt(g_SpawnRecord, TAG_BULLET, Size, BulletList);

        /*if (g_SharedCompList.InfoCompList[ID].Tag == TAG_PLAYER)
        {
            Log_Info("Bullet Speed: %d, %d", SC->pSpeedList[0].x, SC->pSpeedList[0].y)
        }*/
        for (int j = 0; j < Size; ++j)
        {
            GOID BulletID = BulletList[j];

            Sprite* pSpr = &g_SharedCompList.DrawingCompList[BulletID].Spr;
            CreateBulletSpriteFromType(pSpr, SC->BulletType);
            
            Sprite* pSourceSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
            SetShootPositionOffsets(SC, pSourceSpr->Size, pSpr->Size);
            pSpr->Position = Vec2_Add(pSourceSpr->Position, SC->pPositionOffsetList[j]);

            g_SharedCompList.MovementCompList[BulletID].Direction = SC->pDirList[j];
            g_SharedCompList.MovementCompList[BulletID].Speed = SC->pSpeedList[j];

            SetBulletCollisionMask(BulletID, g_SharedCompList.InfoCompList[ID].Tag);

            SC->pBulletIDList[j] = BulletID;
        }

        SC->CurrentTimer = 0.0f;
    }
}

void UpdateCollisionSystem(void)
{
    void ResolveCollision(GOID ID1, GOID ID2);


    GOID ID1, ID2;
    ColliderComponent* CollComp1, *CollComp2;
    Vector2 Pos1, Pos2, Size1, Size2;
    Sprite* pSpr1, *pSpr2;
    //Log_Info("Collision Time");


    int NumOfObjects = g_SharedCompList.IDList_ColliderComp.CurrentSize;
    for (int i = 0; i < NumOfObjects - 1; ++i)
    {
        for (int j = i + 1; j < NumOfObjects; ++j)
        {
            ID1 = g_SharedCompList.IDList_ColliderComp.IDList[i];
            ID2 = g_SharedCompList.IDList_ColliderComp.IDList[j];

            CollComp1 = &g_SharedCompList.ColliderCompList[ID1];
            CollComp2 = &g_SharedCompList.ColliderCompList[ID2];
            
            // Should Their Collision Be Detected?
            if (!(CollComp1->MaskBit & CollComp2->FilterBit))
                continue;
            
            pSpr1 = &g_SharedCompList.DrawingCompList[ID1].Spr;
            pSpr2 = &g_SharedCompList.DrawingCompList[ID2].Spr;
            
            // Collision Mask
            Pos1 =  Vec2_Add(pSpr1->Position, CollComp1->PositionOffset);
            Pos2 =  Vec2_Add(pSpr2->Position, CollComp2->PositionOffset);
            Size1 = Vec2_Add(pSpr1->Size, CollComp1->SizeOffset);
            Size2 = Vec2_Add(pSpr2->Size, CollComp2->SizeOffset);
            
            // Early-Out AABB
            if (Pos1.x > Pos2.x + Size2.x || Pos1.x + Size1.x < Pos2.x || 
                Pos1.y > Pos2.y + Size2.y || Size1.y + Pos1.y < Pos2.y)
                continue;
           
            // Solid Collision Resolution
            Vector2 HalfSize1 = Vec2_Div(Size1, 2);
            Vector2 HalfSize2 = Vec2_Div(Size2, 2);
            Vector2 CenterPos1 = Vec2_Add(Pos1, HalfSize1);
            Vector2 CenterPos2 = Vec2_Add(Pos2, HalfSize2);
            
            Vector2 Delta = Vec2_Sub(CenterPos1, CenterPos2);
            Vector2 Intersect = CreateVec2(  abs(Delta.x) - (HalfSize1.x + HalfSize2.x),
                                            abs(Delta.y) - (HalfSize1.y + HalfSize2.y)  );

            //Log_Info("DeltaX: %d, DeltaY: %d", Delta.x, Delta.y);
            //Log_Info("IntersectX: %d, IntersectY: %d", Intersect.x, Intersect.y);
            
            int Mass1 = CollComp1->Mass;
            int Mass2 = CollComp2->Mass;

            Sprite* pSpr = pSpr1;
            if (Mass1 > Mass2)
                pSpr = pSpr2;

            if (Intersect.x > Intersect.y)
            {
                if (Delta.x > 0)
                {
                    pSpr->Position = Vec2_Add(pSpr->Position, CreateVec2( -Intersect.x, 0 ));
                    //pSpr2->Position = Vec2_Add(pSpr2->Position, CreateVec2( Intersect.x * PO1, 0 ));
                }
                else
                {
                    pSpr->Position = Vec2_Add(pSpr->Position, CreateVec2( Intersect.x, 0 ));
                    //pSpr2->Position = Vec2_Add(pSpr2->Position, CreateVec2( -Intersect.x * PO1, 0 ));
                }
            }
            else
            {
                if (Delta.y > 0)
                {
                    pSpr->Position = Vec2_Add(pSpr->Position, CreateVec2( 0, -Intersect.y ));
                    //pSpr2->Position = Vec2_Add(pSpr2->Position, CreateVec2( 0, Intersect.y * PO1 ));
                }
                else
                {
                    pSpr->Position = Vec2_Add(pSpr->Position, CreateVec2( 0, Intersect.y ));
                    //pSpr2->Position = Vec2_Add(pSpr2->Position, CreateVec2( 0, -Intersect.y * PO1 ));
                }
            }

            //g_SharedCompList.MovementCompList[ID1].Direction = CreateVec2( 0, 0 );
            //g_SharedCompList.MovementCompList[ID2].Direction = CreateVec2( 0, 0 );
            

            ResolveCollision(ID1, ID2);
            //Log_Info("%s Collides with %s", TagInString(g_SharedCompList.InfoCompList[ID1].Tag), TagInString(g_SharedCompList.InfoCompList[ID2].Tag));
        }
    }
}

void ResolveCollision(GOID ID1, GOID ID2)
{
    void CollideBulletWithEnemy(GOID Bullet, GOID Enemy);
    void CollideBulletWithPlayer(GOID Bullet, GOID Player);

    enum eCollisionType
    {
        COLL_NONE = 0,
        COLL_BULLET_WITH_ENEMY,
        COLL_BULLET_WITH_PLAYER
    };

    static enum eCollisionType CollChecker[NUM_OF_TAGS][NUM_OF_TAGS];

    static int IsInitialized = 0;
    if (!IsInitialized)
    {
        for (int i = 0; i < NUM_OF_TAGS; ++i)
        {
            for (int j = 0; j < NUM_OF_TAGS; ++j)
                CollChecker[i][j] = COLL_NONE;
        }

        CollChecker[TAG_BULLET][TAG_ENEMY] = COLL_BULLET_WITH_ENEMY;
        CollChecker[TAG_ENEMY][TAG_BULLET] = COLL_BULLET_WITH_ENEMY;

        CollChecker[TAG_BULLET][TAG_PLAYER] = COLL_BULLET_WITH_PLAYER;
        CollChecker[TAG_PLAYER][TAG_BULLET] = COLL_BULLET_WITH_PLAYER;
        
        IsInitialized = 1;
    }

    enum eTag Tag1 = g_SharedCompList.InfoCompList[ID1].Tag;
    enum eTag Tag2 = g_SharedCompList.InfoCompList[ID2].Tag;
    
    switch(CollChecker[Tag1][Tag2])
    {
        case COLL_BULLET_WITH_ENEMY:
            {
                if (Tag1 == TAG_BULLET)
                    CollideBulletWithEnemy(ID1, ID2);
                else
                    CollideBulletWithEnemy(ID2, ID1);
            }
            break;

        case COLL_BULLET_WITH_PLAYER:
            {
                if (Tag1 == TAG_BULLET)
                    CollideBulletWithPlayer(ID1, ID2);
                else
                    CollideBulletWithPlayer(ID2, ID1);
            }
            break;
    }
}

void CollideBulletWithEnemy(GOID Bullet, GOID Enemy)
{
    AddToDespawnList(Bullet);
    
    g_SharedCompList.InfoCompList[Enemy].Health -= g_SharedCompList.InfoCompList[Bullet].Damage;

    if (g_SharedCompList.InfoCompList[Enemy].Health <= 0)
        AddToDespawnList(Enemy);
}

void CollideBulletWithPlayer(GOID Bullet, GOID Player)
{
    AddToDespawnList(Bullet);
    
    g_SharedCompList.InfoCompList[Player].Health -= g_SharedCompList.InfoCompList[Bullet].Damage;

    if (g_SharedCompList.InfoCompList[Player].Health <= 0)
        AddToDespawnList(Player);
}

void UpdateInfoSystem(void)
{
    GOID ID;

    void UpdatePlayer(GOID ID);
    void UpdateEnemy(GOID ID);
    void UpdateBullet(GOID ID);

    int Size = g_SharedCompList.IDList_InfoComp.CurrentSize;
    for (int i = 0; i < Size; ++i)
    {
        ID = g_SharedCompList.IDList_InfoComp.IDList[i];

        switch(g_SharedCompList.InfoCompList[ID].Tag)
        {
            case TAG_PLAYER:
                UpdatePlayer(ID);
                break;

            case TAG_ENEMY:
                UpdateEnemy(ID);
                break;

            case TAG_BULLET:
                UpdateBullet(ID);
                break;
        }
    }
}

void UpdateTimedShooterSystem(void)
{
    TimedShooterComponent* TC = NULL;
    ShootComponent* SC = NULL;
    GOID ID = CG_INVALID_GOID;

    for(int i = 0; i < g_SharedCompList.IDList_TimedShooterComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_TimedShooterComp.IDList[i];
        TC = &g_SharedCompList.TimedShooterCompList[ID];
        SC = &g_SharedCompList.ShootCompList[ID];
        
        if (SC->ShouldShoot && TC->CurrentTime >= TC->ActiveTime)
        {
            TC->CurrentTime = 0.0f;
            SC->ShouldShoot = 0;
        }
        else if (!SC->ShouldShoot && TC->CurrentTime >= TC->WaitTime)
        {
            TC->CurrentTime = 0.0f;
            SC->ShouldShoot = 1;
        }
        else
            TC->CurrentTime += g_dtAsSeconds;
    }
}

void UpdateConditionalShooterSystem(void)
{
    ConditionalShooterComponent* CondComp = NULL;
    ShootComponent* SC = NULL;
    GOID ID = CG_INVALID_GOID;

    for(int i = 0; i < g_SharedCompList.IDList_CondShooterComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_CondShooterComp.IDList[i];
        CondComp = &g_SharedCompList.CondShooterCompList[ID];
        SC = &g_SharedCompList.ShootCompList[ID];

        switch (CondComp->FuncEnum)
        {
            case COND_SHOOTER_NO_FUNC:
                break;
            
            case COND_SHOOTER_AT_PATH_POSITIONS_FUNC:
                {
                    PathComponent* PC = &g_SharedCompList.PathCompList[ID];
                    Vector2 CurrentPosition = g_SharedCompList.DrawingCompList[ID].Spr.Position;
                    Vector2 DstPosition = PC->pCoordsList[PC->CurrentDestCoordIndex];
                    if (CurrentPosition.x == DstPosition.x && CurrentPosition.y == DstPosition.y)
                        SC->ShouldShoot = 1;
                    else
                        SC->ShouldShoot = 0;
                }
                break;
            default:
                break;
        }
    }
}

void UpdateAnimationSystem(void)
{
    Sprite* Spr = NULL;
    AnimationComponent* AC = NULL;
    GOID ID = CG_INVALID_GOID;

    for(int i = 0; i < g_SharedCompList.IDList_AnimationComp.CurrentSize; ++i)
    {
        ID = g_SharedCompList.IDList_AnimationComp.IDList[i];
        AC = &g_SharedCompList.AnimationCompList[ID];

        if (AC->CurrentTimer < AC->NextFrameTime)
            AC->CurrentTimer += g_dtAsSeconds;
        else
        {
            Spr = &g_SharedCompList.DrawingCompList[ID].Spr;

            AC->CurrentColorAnimFrameIndex = ((AC->CurrentColorAnimFrameIndex + 1) < AC->NumOfColorFrames) ? (AC->CurrentColorAnimFrameIndex + 1) : 0;
            FillSpriteColor(Spr, AC->pColorAnimFrames[AC->CurrentColorAnimFrameIndex], AC->ColorAnimFrameSizeList[AC->CurrentColorAnimFrameIndex]);

            AC->CurrentTimer = 0.0f;
        }
    }
}

void UpdatePlayer(GOID ID)
{
    if (g_WorldState != WORLD_STATE_EDIT)
        InputPlayer(ID);

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;

    // Keep Player Within Walls
    if (pSpr->Position.x < 0)
        pSpr->Position.x = 0;
    else if (pSpr->Position.x + pSpr->Size.x > GetTerminalSize().x)
        pSpr->Position.x = GetTerminalSize().x - pSpr->Size.x;
}

void UpdateEnemy(GOID ID)
{
    
}

void UpdateBullet(GOID ID)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;

    if (pSpr->Position.x <= 0 || 
        pSpr->Position.y < -pSpr->Size.y || 
        pSpr->Position.x >= GetTerminalSize().x - 2 ||
        pSpr->Position.y > GetTerminalSize().y)
    {
        AddToDespawnList(ID);
    }
}
