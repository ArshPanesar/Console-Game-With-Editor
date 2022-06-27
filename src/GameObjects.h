#include "GameData.h"
#include "GameBulletData.h"
#include "Text.h"

#ifndef CG_GAME_OBJECTS_H
#define CG_GAME_OBJECTS_H

/* GAME DATA */
extern const int PLAYER_COLLIDER_MASK;
extern const int ENEMY_COLLIDER_MASK;
extern const int PLAYER_BULLET_COLLIDER_MASK;
extern const int ENEMY_BULLET_COLLIDER_MASK;

extern const int PLAYER_BULLET_DAMAGE;
extern const int ENEMY_BULLET_DAMAGE;

extern const int MAX_NUM_OF_BULLETS;

extern const Vector2 BULLET_DIR_UP;
extern const Vector2 BULLET_DIR_DOWN;
extern const Vector2 BULLET_DIR_LEFT; 
extern const Vector2 BULLET_DIR_RIGHT;
extern const Vector2 BULLET_DIR_UP_LEFT;
extern const Vector2 BULLET_DIR_UP_RIGHT;
extern const Vector2 BULLET_DIR_DOWN_LEFT;
extern const Vector2 BULLET_DIR_DOWN_RIGHT;

extern const Vector2 BULLET_DIRS_TABLE[];
extern const int BULLET_DIRS_TABLE_SIZE;

extern const int BULLET_POSITION_OFFSET;

extern const int BULLET_TYPE_DEFAULT;
extern const int BULLET_TYPE_PLAYER;
/* GAME FUNCTIONS */

// Path Generators
int CreatePath(PathComponent* PC, int NumOfCoords);
int DestroyPath(PathComponent* PC);
int AddCoordToPath(PathComponent* PC, Vector2 Coord);
int RemoveCoordFromPath(PathComponent* PC, int Index);
// Set a Coord in Path
int SetPathCoord(PathComponent* PC, Vector2 Coord, int Index);
// Set Speed Between Two Coords in Path
int SetPathSpeed(PathComponent* PC, Vector2 Speed, int Index);
// Set Waiting Time At a Coord
int SetPathWaitingTime(PathComponent* PC, float WaitTime, int Index);
// Swap Two Coords in Path
int SwapPathData(PathComponent* PC, int i, int j);

int LogPath(PathComponent* PC);

//int SetPathCoords(PathComponent* PC, Vector2 CoordsList[], int Size);
//int SetPathSpeeds(PathComponent* PC, Vector2 SpeedList[], int Size);
//int SetPathWaitingTime(PathComponent* PC, float WaitTimeList[], int Size);

// Shooting Pattern Generators
int CreateShootPattern(ShootComponent* SC, int NumOfBullets);
int DestroyShootPattern(ShootComponent* SC);
// Returns Index of the Bullet Created, Otherwise -1
int AddBulletToShootPattern(ShootComponent* SC);
int RemoveBulletFromShootPattern(ShootComponent* SC, int Index);
int SetBulletDirection(ShootComponent* SC, int Index, Vector2 Dir);
int SetBulletSpeed(ShootComponent* SC, int Index, Vector2 Speed);
void GenerateBulletPositionOffsets(Vector2 Offsets[], Vector2 SourceSize, Vector2 BulletSize, Vector2 Directions[], int Size);
int SetShootPositionOffsets(ShootComponent* SC, Vector2 SourceSize, Vector2 BulletSize);

void LogShootPattern(ShootComponent* SC);
//int SetShootDirections(ShootComponent* SC, Vector2 DirsList[], int Size);
//int SetShootSpeeds(ShootComponent* SC, Vector2 SpeedList[], int Size);
//int SetShootBulletData(ShootComponent* SC, char* CharBuffer, Color* ColorBuffer, int Size);


void TEST_ShootFunctions();
void TEST_PathFunctions();

#endif //CG_GAME_OBJECTS_H
