#include "GameObjects.h"
#include "GameBullet.h"

#ifndef CG_GAME_ENEMY_H
#define CG_GAME_ENEMY_H

/* ENEMY DATA */

extern char* EnemyCharBuffer;
extern Color EnemyColorBuffer[];
extern int EnemyColorBufferSize;

extern const Vector2 SLOW_ENEMY_BULLET_SPEED;
extern const Vector2 NORM_ENEMY_BULLET_SPEED;
extern const Vector2 FAST_ENEMY_BULLET_SPEED;

extern const Vector2 SLOW_ENEMY_SPEED;
extern const Vector2 NORM_ENEMY_SPEED;
extern const Vector2 FAST_ENEMY_SPEED;

extern const float SMALL_ENEMY_WAIT_TIME;
extern const float MEDIUM_ENEMY_WAIT_TIME;
extern const float LARGE_ENEMY_WAIT_TIME;

extern const float SMALL_ENEMY_BULLET_COOLDOWN_TIME;
extern const float NORM_ENEMY_BULLET_COOLDOWN_TIME;
extern const float LARGE_ENEMY_BULLET_COOLDOWN_TIME;

extern const Vector2 SMALL_COOLDOWN_DISTANCE;
extern const Vector2 NORM_COOLDOWN_DISTANCE;
extern const Vector2 LARGE_COOLDOWN_DISTANCE;

/* ENEMY FUNCTIONS */

// Enemy
GOID CreateEnemy(void);
int DestroyEnemy(GOID ID);
int SetEnemyBullet(GOID ID);

// Path Makers

// Timed Shooter Component Setters
void SetEnemyTimedShooter(GOID ID, float WaitTime, float ActiveTime);

#endif //CG_GAME_ENEMY_H
