#include "GameObjects.h"

#ifndef CG_GAME_BULLET_H
#define CG_GAME_BULLET_H

// Bullet
GOID CreateBullet(void);
int DestroyBullet(GOID ID);

// Create a Sprite from a Type of Bullet
int CreateBulletSpriteFromType(Sprite* pSpr, int Type);
int SetBulletCollisionMask(GOID ID, enum eTag Tag);

#endif //CG_GAME_BULLET_H
