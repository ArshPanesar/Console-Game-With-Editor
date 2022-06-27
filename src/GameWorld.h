#include "DebugEditor.h"
#include "Spawner.h"
#include "GameEnemyHordeManager.h"

#ifndef CG_GAME_WORLD_H
#define CG_GAME_WORLD_H

enum WorldState
{
    WORLD_STATE_RUN = 0,
    WORLD_STATE_EDIT
};

//extern enum WorldState g_WorldState;

/* Game World Functions */
void InitGameWorld(void);
void UpdateGameWorld(void);
void DrawGameWorld(void);
void DestroyGameWorld(void);

/* System Functions for Components */
// Movement
void UpdateMovementSystem(void);

// Path
void UpdatePathSystem(void);

// Shoot
void UpdateShootSystem(void);

// Collision
void UpdateCollisionSystem(void);

// Info
void UpdateInfoSystem(void);

// Timed Shooting
void UpdateTimedShooterSystem(void);

// Conditional Shooting
void UpdateConditionalShooterSystem(void);

// Animation
void UpdateAnimationSystem(void);

#endif //CG_GAME_WORLD_H
