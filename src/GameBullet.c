#include "GameBullet.h"

GOID CreateBullet(void)
{
    GOID ID = CreateGameObj();
    AttachComponent(COMP_INFO, ID);
    AttachComponent(COMP_DRAWING, ID);
    AttachComponent(COMP_MOVEMENT, ID);
    AttachComponent(COMP_COLLIDER, ID);

    //Tag
    g_SharedCompList.InfoCompList[ID].Tag = TAG_BULLET;
    g_SharedCompList.InfoCompList[ID].Health = 0;
    g_SharedCompList.InfoCompList[ID].Damage = 10;
    g_SharedCompList.InfoCompList[ID].DebugFlag = 0;
    
    //Drawing
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
    pSpr->CharBuffer = NULL;
    pSpr->ColorBuffer = NULL;
    pSpr->Position = CreateVec2(0, 0);
    
    g_SharedCompList.DrawingCompList[ID].PositionXBuildUp = 0.0f;
    g_SharedCompList.DrawingCompList[ID].PositionYBuildUp = 0.0f;


    //Movement
    g_SharedCompList.MovementCompList[ID].Direction = CreateVec2(0, 0);
    g_SharedCompList.MovementCompList[ID].Speed = CreateVec2(0, 0);
    
    // Collider
    ColliderComponent* CollComp = &g_SharedCompList.ColliderCompList[ID];
    CollComp->PositionOffset = (Vector2) {0, 0};
    CollComp->SizeOffset = (Vector2) {-1, -1};
    CollComp->MaskBit = PLAYER_BULLET_COLLIDER_MASK;
    CollComp->FilterBit = ENEMY_COLLIDER_MASK;
    CollComp->Mass = 0;

    return ID;
}

int DestroyBullet(GOID ID)
{
    DestroySprite(&g_SharedCompList.DrawingCompList[ID].Spr);

    DetachComponent(COMP_INFO, ID);
    DetachComponent(COMP_DRAWING, ID);
    DetachComponent(COMP_MOVEMENT, ID);
    DetachComponent(COMP_COLLIDER, ID);

    DestroyGameObj(ID);
    return 1;
}

int CreateBulletSpriteFromType(Sprite* pSpr, int Type)
{
    DestroySprite(pSpr);

    if (Type == BULLET_TYPE_PLAYER) // Player
    {
        SetSprite(pSpr, BulletCharBuffer, '\n');
        FillSpriteColor(pSpr, PlayerBulletColorBuffer, BulletColorBufferSize);
    }
    else // Default
    {
        SetSprite(pSpr, BulletCharBuffer, '\n');
        FillSpriteColor(pSpr, BulletColorBuffer, BulletColorBufferSize);
    }
    return 1;
}

int SetBulletCollisionMask(GOID ID, enum eTag Tag)
{
    unsigned int MaskBit = 0, FilterBit = 0;
    switch(Tag)
    {
        case TAG_PLAYER:
            MaskBit = PLAYER_BULLET_COLLIDER_MASK, FilterBit = ENEMY_COLLIDER_MASK;
            break;

        case TAG_ENEMY:
            MaskBit = ENEMY_BULLET_COLLIDER_MASK, FilterBit = PLAYER_COLLIDER_MASK;
            break;

        default:
        {
            Log_Warn("SetBulletCollisionMask(): Unknown Tag. GOID=%d, Tag=%d", ID, Tag);
            return 0;
        }
    }

    ColliderComponent* pCollComp = &g_SharedCompList.ColliderCompList[ID];
    pCollComp->MaskBit = MaskBit;
    pCollComp->FilterBit = FilterBit;

    return 1;
}