#include "GameEnemy.h"

char* EnemyCharBuffer = "&??666??&]\n"\
                        "&??666??&]\n"\
                        "&??666??&]\n"\
                        "&??666??&]";

Color EnemyColorBuffer[] =   {
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_dc,
                                    C_db, C_b, C_b, C_r, C_r, C_r, C_b, C_b, C_db, C_dc,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_dc,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_dc
                                    }; 

// Frames
static Color ColorBufferFrame_1_[] =   {
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db
                                    }; 
static Color ColorBufferFrame_2_[] =   {
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_dr, C_r, C_r, C_r, C_r, C_r, C_r, C_r, C_dr, C_dr
                                    }; 
static Color ColorBufferFrame_3_[] =   {
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_dr, C_r, C_r, C_r, C_r, C_r, C_r, C_r, C_dr, C_dr,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db
                                    }; 
static Color ColorBufferFrame_4_[] =   {
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_dr, C_r, C_r, C_r, C_r, C_r, C_r, C_r, C_dr, C_dr,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db
                                    }; 


static Color ColorBufferFrame_5_[] =   {
                                    C_dr, C_r, C_r, C_r, C_r, C_r, C_r, C_r, C_dr, C_dr,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dr, C_dr, C_dr, C_b, C_b, C_db, C_db
                                    }; 


int EnemyColorBufferSize = ARR_LEN(EnemyColorBuffer);

const Vector2 ENEMY_MOVE_SPEED = (Vector2){2, 1};

const Vector2 SLOW_ENEMY_BULLET_SPEED = (Vector2){20, 9};
const Vector2 NORM_ENEMY_BULLET_SPEED = (Vector2){40, 15};
const Vector2 FAST_ENEMY_BULLET_SPEED = (Vector2){50, 20};

const Vector2 SLOW_ENEMY_SPEED = (Vector2){2, 5};
const Vector2 NORM_ENEMY_SPEED = (Vector2){6, 7};
const Vector2 FAST_ENEMY_SPEED = (Vector2){14, 10};

const float SMALL_ENEMY_WAIT_TIME  = 0.5f;
const float MEDIUM_ENEMY_WAIT_TIME = 1.0f;
const float LARGE_ENEMY_WAIT_TIME  = 2.0f;

const float SMALL_ENEMY_BULLET_COOLDOWN_TIME = 0.1f;
const float NORM_ENEMY_BULLET_COOLDOWN_TIME  = 0.3f;
const float LARGE_ENEMY_BULLET_COOLDOWN_TIME = 0.5f;

const Vector2 SMALL_COOLDOWN_DISTANCE = (Vector2){ 22, 12 };
const Vector2 NORM_COOLDOWN_DISTANCE  = (Vector2){ 30, 17 };
const Vector2 LARGE_COOLDOWN_DISTANCE = (Vector2){ 40, 21 };

//void GenerateBulletPositionOffsets(Vector2 Offsets[], Vector2 ObjSize, Vector2 BulletSize, Vector2 Directions[], int Size);

GOID CreateEnemy(void)
{
    GOID ID = CreateGameObj();
    AttachComponent(COMP_INFO, ID);
    AttachComponent(COMP_DRAWING, ID);
    AttachComponent(COMP_MOVEMENT, ID);
    AttachComponent(COMP_PATH, ID);
    AttachComponent(COMP_COLLIDER, ID);
    AttachComponent(COMP_SHOOT, ID);
    AttachComponent(COMP_TIMED_SHOOTER, ID);
    AttachComponent(COMP_CONDITIONAL_SHOOTER, ID);
    AttachComponent(COMP_ANIMATION, ID);

    //Tag
    g_SharedCompList.InfoCompList[ID].Tag = TAG_ENEMY;
    g_SharedCompList.InfoCompList[ID].Health = 100;
    g_SharedCompList.InfoCompList[ID].Damage = 0;
    g_SharedCompList.InfoCompList[ID].DebugFlag = 0;
    
    //Drawing
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
    SetSprite(pSpr, EnemyCharBuffer, '\n');
    FillSpriteColor(pSpr, EnemyColorBuffer, ARR_LEN(EnemyColorBuffer));

    //Ranged Rand: rand() % (max_number + 1 - minimum_number) + minimum_number
    int PosX = rand() % (g_TerminalSize.x - pSpr->Size.x + 1 - pSpr->Size.x) + pSpr->Size.x;
    int PosY = rand() % (g_TerminalSize.y - pSpr->Size.y + 1 - pSpr->Size.y) + pSpr->Size.y;
    pSpr->Position = CreateVec2(PosX, PosY);
    
    //pSpr->Position = CreateVec2(g_TerminalSize.x / 2, g_TerminalSize.y / 2);
    g_SharedCompList.DrawingCompList[ID].PositionXBuildUp = 0.0f;
    g_SharedCompList.DrawingCompList[ID].PositionYBuildUp = 0.0f;

    //Movement
    g_SharedCompList.MovementCompList[ID].Direction = CreateVec2(0, 0);
    g_SharedCompList.MovementCompList[ID].Speed = ENEMY_MOVE_SPEED;
    
    //Path
    //Vector2 EndCoord = CreateVec2(pSpr->Position.x, pSpr->Position.y);
    //Vector2 CoordList[] = { pSpr->Position, EndCoord };
    //GeneratePath(ID, CoordList, ARR_LEN(CoordList));
    PathComponent* PC = &g_SharedCompList.PathCompList[ID];
    PC->CurrentDestCoordIndex = 0;
    PC->pCoordsList = NULL;
    PC->pSpeedList = NULL;
    PC->pWaitTimeList = NULL;
    PC->CurrentTimer = 0.0f;
    PC->Size = 0;
    PC->PathLogic = PATH_LOOP;

    // Collider
    ColliderComponent* CollComp = &g_SharedCompList.ColliderCompList[ID];
    CollComp->PositionOffset = (Vector2) {0, 0};
    CollComp->SizeOffset = (Vector2) {-1, -1};
    CollComp->MaskBit = ENEMY_COLLIDER_MASK;
    CollComp->FilterBit = PLAYER_COLLIDER_MASK | PLAYER_BULLET_COLLIDER_MASK;
    CollComp->Mass = 1;

    //Shooting
    ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
    SC->CooldownTime = 0.1f;
    SC->CurrentTimer = 0.0f;
    SC->NumOfBullets = 0;
    SC->pDirList = NULL;
    SC->pPositionOffsetList = NULL;
    SC->pSpeedList = NULL;
    SC->ShouldShoot = 0;
    SC->BulletType = BULLET_TYPE_DEFAULT;
    //SC->SourcePosition = CreateVec2(0, 0);

    // Timed Shooter
    TimedShooterComponent* TSC = &g_SharedCompList.TimedShooterCompList[ID];
    TSC->WaitTime = 0.0f;
    TSC->CurrentTime = 0.0f;
    TSC->ActiveTime = 0.0f;

    // Conditional Shooter
    g_SharedCompList.CondShooterCompList[ID].FuncEnum = COND_SHOOTER_NO_FUNC;

    // Animation
    AnimationComponent* AC = &g_SharedCompList.AnimationCompList[ID];
    AC->CurrentColorAnimFrameIndex = 0;
    AC->CurrentTimer = 0.0f;
    AC->NextFrameTime = (1.0f / 10.0f);
    
    AC->pColorAnimFrames[0] = ColorBufferFrame_1_;
    AC->pColorAnimFrames[1] = ColorBufferFrame_2_;
    AC->pColorAnimFrames[2] = ColorBufferFrame_3_;
    AC->pColorAnimFrames[3] = ColorBufferFrame_4_;
    AC->pColorAnimFrames[4] = ColorBufferFrame_5_;
    AC->ColorAnimFrameSizeList[0] = ARR_LEN(ColorBufferFrame_1_);
    AC->ColorAnimFrameSizeList[1] = ARR_LEN(ColorBufferFrame_2_);
    AC->ColorAnimFrameSizeList[2] = ARR_LEN(ColorBufferFrame_3_);
    AC->ColorAnimFrameSizeList[3] = ARR_LEN(ColorBufferFrame_4_);
    AC->ColorAnimFrameSizeList[4] = ARR_LEN(ColorBufferFrame_5_);

    
    AC->NumOfColorFrames = 5;
    return ID;
}

int DestroyEnemy(GOID ValidID)
{
    GOID ID = ValidID;

    DestroySprite(&g_SharedCompList.DrawingCompList[ID].Spr);
    DestroyPath(&g_SharedCompList.PathCompList[ID]);
    DestroyShootPattern(&g_SharedCompList.ShootCompList[ID]);

    DetachComponent(COMP_INFO, ID);
    DetachComponent(COMP_DRAWING, ID);
    DetachComponent(COMP_MOVEMENT, ID);
    DetachComponent(COMP_PATH, ID);
    DetachComponent(COMP_COLLIDER, ID);
    DetachComponent(COMP_SHOOT, ID);
    DetachComponent(COMP_TIMED_SHOOTER, ID);
    DetachComponent(COMP_CONDITIONAL_SHOOTER, ID);
    DetachComponent(COMP_ANIMATION, ID);

    DestroyGameObj(ID);
    return 1;
}

void SetEnemyTimedShooter(GOID ID, float WaitTime, float ActiveTime)
{
    TimedShooterComponent* TC = &g_SharedCompList.TimedShooterCompList[ID];
    TC->WaitTime = WaitTime;
    TC->ActiveTime = ActiveTime;
    TC->CurrentTime = 0.0f;
}
