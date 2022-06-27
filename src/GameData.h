#include "Sprite.h"
#include "Log.h"
#include "KeyInput.h"
#include "Terminal.h"

#ifndef CG_GAME_DATA_H
#define CG_GAME_DATA_H

/* 
* USEFUL PREFIXES:
* C_    -> Color
*/

/* COLOR CODES */
#define C_r     CG_COLOR_RED
#define C_b     CG_COLOR_BLUE
#define C_g     CG_COLOR_GREEN
#define C_dr    CG_COLOR_DARK_RED
#define C_db    CG_COLOR_DARK_BLUE
#define C_dg    CG_COLOR_DARK_GREEN

#define C_bl    CG_COLOR_BLACK
#define C_w     CG_COLOR_WHITE

#define C_gry   CG_COLOR_GRAY
#define C_y     CG_COLOR_YELLOW
#define C_m     CG_COLOR_MAGENTA
#define C_c     CG_COLOR_CYAN
#define C_dgry  CG_COLOR_DARK_GRAY
#define C_dy    CG_COLOR_DARK_YELLOW
#define C_dm    CG_COLOR_DARK_MAGENTA
#define C_dc    CG_COLOR_DARK_CYAN

#define C_ur    CG_BGCOLOR_RED
#define C_ub    CG_BGCOLOR_BLUE
#define C_ug    CG_BGCOLOR_GREEN
#define C_udr   CG_BGCOLOR_DARK_RED
#define C_udb   CG_BGCOLOR_DARK_BLUE
#define C_udg   CG_BGCOLOR_DARK_GREEN
        
#define C_ubl   CG_BGCOLOR_BLACK
#define C_uw    CG_BGCOLOR_WHITE
        
#define C_ugry  CG_BGCOLOR_GRAY
#define C_uy    CG_BGCOLOR_YELLOW
#define C_um    CG_BGCOLOR_MAGENTA
#define C_uc    CG_BGCOLOR_CYAN
#define C_udgry CG_BGCOLOR_DARK_GRAY
#define C_udy   CG_BGCOLOR_DARK_YELLOW
#define C_udm   CG_BGCOLOR_DARK_MAGENTA
#define C_udc   CG_BGCOLOR_DARK_CYAN

/* 
*   All Game Objects Are Just IDs and Their Data (in Components)
*   Can Only Be Accessed By these IDs.
*/
#define CG_MAX_NUM_OF_GAME_OBJ  128
#define CG_INVALID_GOID         CG_MAX_NUM_OF_GAME_OBJ + 1

extern Vector2 g_TerminalSize;
extern double g_dtAsSeconds;
extern Vector2 g_ScreenSize;

typedef unsigned int GOID;

void InitGameData(void);
int GOIDArrayInsert(GOID* Array, int Size, GOID Key, int Index);
int GOIDArrayDelete(GOID* Array, int* Size, GOID Key, int Start);

/* Game Object Functions */
GOID CreateGameObj(void);
void DestroyGameObj(GOID ID);

/* Tags for All Game Objects*/
enum eTag
{
    TAG_NONE = 0,
    TAG_PLAYER,
    TAG_ENEMY,
    TAG_BULLET,
    NUM_OF_TAGS
};

/* Components */
enum eComponent
{
    COMP_INFO = 0,
    COMP_DRAWING,
    COMP_MOVEMENT,
    COMP_PATH,
    COMP_SHOOT,
    COMP_COLLIDER,
    COMP_TIMED_SHOOTER,
    COMP_CONDITIONAL_SHOOTER,
    COMP_ANIMATION
};

typedef struct InfoComponentStruct
{
    enum eTag Tag;
    int Health;
    int Damage;
    int DebugFlag;
} InfoComponent;

typedef struct DrawingComponentStruct
{
    Sprite Spr;
    float PositionXBuildUp;
    float PositionYBuildUp;
} DrawingComponent;

typedef struct MovementComponentStruct
{
    Vector2 Direction;
    Vector2 Speed;
} MovementComponent;

typedef struct PathComponentStruct
{
    Vector2* pCoordsList;
    Vector2* pSpeedList;
    float* pWaitTimeList;

    float CurrentTimer;

    int Size;
    int CurrentDestCoordIndex;

    int CurrentPass;    // Starts From 0, Increments After All Coords Have Been Reached Once

    enum ePathLogic
    {
        PATH_STOP_AT_END = 0,
        PATH_LOOP,
        PATH_REVERSE
    } PathLogic;
    
} PathComponent;

typedef struct ShootComponentStruct
{
    Vector2* pDirList;
    Vector2* pPositionOffsetList;
    Vector2* pSpeedList;
    GOID* pBulletIDList;

    // Bullet Type: Determines Which Sprite To Use
    int BulletType;
    
    Vector2 DistanceBetweenBullets;

    float CurrentTimer;
    float CooldownTime;

    int NumOfBullets;   // Per Shot
    int ShouldShoot;
} ShootComponent;

typedef struct ColliderComponentStruct
{
    Vector2 PositionOffset;
    Vector2 SizeOffset;
    unsigned int MaskBit;
    unsigned int FilterBit;
    int Mass;
} ColliderComponent;

typedef struct TimedShooterComponentStruct
{
    float WaitTime; // How Long to Wait before Shooting in Seconds
    float ActiveTime;   // How Long to Shoot in Seconds
    float CurrentTime;  // Current Time in Seconds
} TimedShooterComponent;

typedef struct ConditionalShooterComponentStruct
{
    enum eConditionalShooterFunc
    {
        COND_SHOOTER_NO_FUNC = 0,   // No Function
        COND_SHOOTER_AT_PATH_POSITIONS_FUNC, // Shoot whenever at any Path Position
        NUM_OF_COND_SHOOTER_FUNCS
    } FuncEnum;
} ConditionalShooterComponent;

// Animation
#define MAX_ANIMATION_FRAMES 8

typedef struct AnimationComponentStruct
{
    // Color Animation
    Color* pColorAnimFrames[MAX_ANIMATION_FRAMES];
    int ColorAnimFrameSizeList[MAX_ANIMATION_FRAMES];
    int CurrentColorAnimFrameIndex;
    int NumOfColorFrames;

    float CurrentTimer;
    float NextFrameTime;
} AnimationComponent;

/* ID List */
typedef struct GOIDListStruct
{
    GOID IDList[CG_MAX_NUM_OF_GAME_OBJ];
    int CurrentSize;
} GOIDList;

/* Shared Component List */
struct ComponentList
{
    /* Components */
    InfoComponent                   InfoCompList[CG_MAX_NUM_OF_GAME_OBJ];
    DrawingComponent                DrawingCompList[CG_MAX_NUM_OF_GAME_OBJ];
    MovementComponent               MovementCompList[CG_MAX_NUM_OF_GAME_OBJ];
    PathComponent                   PathCompList[CG_MAX_NUM_OF_GAME_OBJ];
    ShootComponent                  ShootCompList[CG_MAX_NUM_OF_GAME_OBJ];
    ColliderComponent               ColliderCompList[CG_MAX_NUM_OF_GAME_OBJ];
    TimedShooterComponent           TimedShooterCompList[CG_MAX_NUM_OF_GAME_OBJ];
    ConditionalShooterComponent     CondShooterCompList[CG_MAX_NUM_OF_GAME_OBJ];
    AnimationComponent              AnimationCompList[CG_MAX_NUM_OF_GAME_OBJ];
    
    /* ID Lists */
    GOIDList IDList_InfoComp;
    GOIDList IDList_DrawingComp;
    GOIDList IDList_MovementComp;
    GOIDList IDList_PathComp;
    GOIDList IDList_ShootComp;
    GOIDList IDList_ColliderComp;
    GOIDList IDList_TimedShooterComp;
    GOIDList IDList_CondShooterComp;
    GOIDList IDList_AnimationComp;
};

int AttachComponent(const enum eComponent CompEnum, const GOID ID);
int DetachComponent(const enum eComponent CompEnum, const GOID ID);
int HasComponent(const enum eComponent CompEnum, const GOID ID);

extern struct ComponentList g_SharedCompList;

// Debug
const char* TagInString(enum eTag Name);

#endif //CG_GAME_DATA_H
