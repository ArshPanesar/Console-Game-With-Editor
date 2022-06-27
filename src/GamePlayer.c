#include "GamePlayer.h"

char* PlayerCharBuffer =    "[**^^^**]]\n"\
                            "[**===**]]\n"\
                            "[**===**]]\n"\
                            "[**...**]]";

Color PlayerColorBuffer[] = {
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_g, C_g, C_g, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db
                                    };

// Color Frames
static Color ColorBufferFrame_1_[] = {
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db
                                    };
static Color ColorBufferFrame_2_[] = {
                                    C_dg, C_g, C_g, C_g, C_g, C_g, C_g, C_g, C_dg, C_dg,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db
                                    };
static Color ColorBufferFrame_3_[] = {
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_dg, C_g, C_g, C_g, C_g, C_g, C_g, C_g, C_dg, C_dg,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db
                                    };
static Color ColorBufferFrame_4_[] = {
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_dg, C_g, C_g, C_g, C_g, C_g, C_g, C_g, C_dg, C_dg,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db
                                    };
static Color ColorBufferFrame_5_[] = {
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_db, C_b, C_b, C_dg, C_dg, C_dg, C_b, C_b, C_db, C_db,
                                    C_dg, C_g, C_g, C_g, C_g, C_g, C_g, C_g, C_dg, C_dg
                                    };

const float PLAYER_SHOOT_COOLDOWN = 0.15f;
const Vector2 PLAYER_MOVE_SPEED   = (Vector2){ 56, 22 };
const Vector2 PLAYER_BULLET_SPEED = (Vector2){ 56, 31 };
const int PLAYER_NUM_OF_BULLETS   = 1;
const Vector2 PLAYER_SHOOT_COOLDOWN_DISTANCE = (Vector2){ 4, 8 };

GOID CreatePlayer(void)
{
    GOID Player = CreateGameObj();

    AttachComponent(COMP_INFO, Player);
    AttachComponent(COMP_DRAWING, Player);
    AttachComponent(COMP_MOVEMENT, Player);
    AttachComponent(COMP_SHOOT, Player);
    AttachComponent(COMP_COLLIDER, Player);
    AttachComponent(COMP_ANIMATION, Player);
    
    //Tag
    g_SharedCompList.InfoCompList[Player].Tag = TAG_PLAYER;
    g_SharedCompList.InfoCompList[Player].Health = 100;
    g_SharedCompList.InfoCompList[Player].Damage = 0;
    g_SharedCompList.InfoCompList[Player].DebugFlag = 0;
    
    //Drawing
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[Player].Spr;
    SetSprite(pSpr, PlayerCharBuffer, '\n');
    FillSpriteColor(pSpr, ColorBufferFrame_1_, ARR_LEN(ColorBufferFrame_1_));
    pSpr->Position = CreateVec2(GetTerminalSize().x / 2, GetTerminalSize().y / 2);
    
    g_SharedCompList.DrawingCompList[Player].PositionXBuildUp = 0.0f;
    g_SharedCompList.DrawingCompList[Player].PositionYBuildUp = 0.0f;


    //Movement
    g_SharedCompList.MovementCompList[Player].Direction = CreateVec2(0, 0);
    g_SharedCompList.MovementCompList[Player].Speed = PLAYER_MOVE_SPEED;
    
    //Shoot
    ShootComponent* SC = &g_SharedCompList.ShootCompList[Player];
    SC->CooldownTime = PLAYER_SHOOT_COOLDOWN;
    //Log_Info("Player Cooldown: %f", SC->CooldownTime);
    //SC->CooldownTime = PLAYER_SHOOT_COOLDOWN;
    SC->CurrentTimer = 1.0f;
    SC->ShouldShoot = 0;
    SC->NumOfBullets = PLAYER_NUM_OF_BULLETS;
    SC->pDirList = (Vector2*)malloc(sizeof(Vector2) * SC->NumOfBullets);
    SC->pPositionOffsetList = (Vector2*)malloc(sizeof(Vector2) * SC->NumOfBullets);
    SC->pSpeedList = (Vector2*)malloc(sizeof(Vector2) * SC->NumOfBullets);
    SC->pBulletIDList = (GOID*)malloc(sizeof(GOID) * SC->NumOfBullets);

    for (int i = 0; i < SC->NumOfBullets; ++i)
        SC->pBulletIDList[i] = CG_INVALID_GOID;

    SC->DistanceBetweenBullets = PLAYER_SHOOT_COOLDOWN_DISTANCE;
    SC->BulletType = BULLET_TYPE_PLAYER;

    //SetShootBulletData(SC, BulletCharBuffer, BulletColorBuffer, BulletColorBufferSize);
    SC->pDirList[0] = CreateVec2(0, -1);
    //SC->BulletType = BULLET_TYPE_DEFAULT;
    //Vector2 BSize = CalculateCharBufferSize(SC->BulletCharBuffer, '\n');
    //SC->pPositionOffsetList[0] = CreateVec2( (pSpr->Size.x / 2) - BSize.x + 1, -4 );
    SC->pSpeedList[0] = PLAYER_BULLET_SPEED;
    //SC->SourcePosition = pSpr->Position;
    Sprite BulletSpr;
    BulletSpr.CharBuffer = NULL;
    BulletSpr.ColorBuffer = NULL;
    CreateBulletSpriteFromType(&BulletSpr, SC->BulletType);
    SetShootPositionOffsets(SC, pSpr->Size, BulletSpr.Size);

    // Collider
    ColliderComponent* CollComp = &g_SharedCompList.ColliderCompList[Player];
    CollComp->PositionOffset = (Vector2) {0, 0};
    CollComp->SizeOffset = (Vector2) {-1, -1};
    CollComp->MaskBit = PLAYER_COLLIDER_MASK;
    CollComp->FilterBit = ENEMY_COLLIDER_MASK | ENEMY_BULLET_COLLIDER_MASK;
    CollComp->Mass = 0;

    // Animation
    AnimationComponent* AC = &g_SharedCompList.AnimationCompList[Player];
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
    

    return Player;
}

void InputPlayer(GOID Player)
{
    Vector2* Dir = &g_SharedCompList.MovementCompList[Player].Direction;
    ShootComponent* SC = &g_SharedCompList.ShootCompList[Player];

    if (IsKeyPressed(CG_KEY_S))
        *Dir = CreateVec2(Dir->x, 1);
    else if (IsKeyPressed(CG_KEY_W))
        *Dir = CreateVec2(Dir->x, -1);
    else
        *Dir = CreateVec2(Dir->x, 0);

    if (IsKeyPressed(CG_KEY_D))
        *Dir= CreateVec2(1, Dir->y);
    else if (IsKeyPressed(CG_KEY_A))
        *Dir = CreateVec2(-1, Dir->y);
    else
        *Dir = CreateVec2(0, Dir->y);

    if (IsKeyPressed(CG_KEY_SPACE))
        SC->ShouldShoot = 1;
    else
        SC->ShouldShoot = 0;
}

int DestroyPlayer(GOID Player)
{
    DestroySprite(&g_SharedCompList.DrawingCompList[Player].Spr);

    ShootComponent* SC = &g_SharedCompList.ShootCompList[Player];
    free(SC->pDirList);
    free(SC->pPositionOffsetList);
    free(SC->pSpeedList);
    free(SC->pBulletIDList);

    DetachComponent(COMP_INFO, Player);
    DetachComponent(COMP_DRAWING, Player);
    DetachComponent(COMP_MOVEMENT, Player);
    DetachComponent(COMP_SHOOT, Player);
    DetachComponent(COMP_COLLIDER, Player);
    DetachComponent(COMP_ANIMATION, Player);

    DestroyGameObj(Player);
    return 1;
}

