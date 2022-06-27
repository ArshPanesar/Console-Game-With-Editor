#include "GameObjects.h"

const int PLAYER_COLLIDER_MASK  = 0x0001;
const int ENEMY_COLLIDER_MASK   = 0x0010;
const int PLAYER_BULLET_COLLIDER_MASK  = 0x0100;
const int ENEMY_BULLET_COLLIDER_MASK  = 0x1000;

const int PLAYER_BULLET_DAMAGE = 10;
const int ENEMY_BULLET_DAMAGE = 50;

const int MAX_NUM_OF_BULLETS = 12;

const Vector2 BULLET_DIR_UP    = (Vector2){ 0, -1 };
const Vector2 BULLET_DIR_DOWN  = (Vector2){ 0, 1 };
const Vector2 BULLET_DIR_LEFT  = (Vector2){ -1, 0 };
const Vector2 BULLET_DIR_RIGHT = (Vector2){ 1, 0 };

// Diagonal
const Vector2 BULLET_DIR_UP_LEFT    = (Vector2){ -1, -1 };
const Vector2 BULLET_DIR_UP_RIGHT    = (Vector2){ 1, -1 };
const Vector2 BULLET_DIR_DOWN_LEFT    = (Vector2){ -1, 1 };
const Vector2 BULLET_DIR_DOWN_RIGHT    = (Vector2){ 1, 1 };

const Vector2 BULLET_DIRS_TABLE[] = {
    BULLET_DIR_UP,
    BULLET_DIR_UP_LEFT, 
    BULLET_DIR_LEFT,
    BULLET_DIR_DOWN_LEFT,
    BULLET_DIR_DOWN,
    BULLET_DIR_DOWN_RIGHT,
    BULLET_DIR_RIGHT,   
    BULLET_DIR_UP_RIGHT
};

const int BULLET_DIRS_TABLE_SIZE = ARR_LEN(BULLET_DIRS_TABLE);

const int BULLET_POSITION_OFFSET = 2;

const int BULLET_TYPE_DEFAULT = 0;
const int BULLET_TYPE_PLAYER = 1;
// FUNCTIONS 
int CopyShootPattern(ShootComponent* Dst, ShootComponent* Src);
int SwapBulletData(ShootComponent* SC, int i, int j);
void GenerateBulletPositionOffsets(Vector2 Offsets[], Vector2 SourceSize, Vector2 BulletSize, Vector2 Directions[], int Size);

// PATH LOCAL FUNCTIONS
int DeletePathCoord(PathComponent* PC, int i);
int CopyPath(PathComponent* Dst, PathComponent* Src);

// Path Generators
int CreatePath(PathComponent* PC, int NumOfCoords)
{
    if (PC == NULL)
    {
        Log_Warn("CreatePath() Warning: PathComponent is NULL");
        return 0;
    }

    PC->Size = NumOfCoords;
    PC->CurrentDestCoordIndex = 0;
    PC->CurrentTimer = 0.0f;
    PC->PathLogic = PATH_LOOP;
    PC->CurrentPass = 0;

    // Empty Component
    if (NumOfCoords < 1)
    {
        PC->pCoordsList = NULL;
        PC->pSpeedList = NULL;
        PC->pWaitTimeList = NULL;
        return 1;
    }

    PC->pCoordsList = (Vector2*)malloc(sizeof(Vector2) * NumOfCoords);
    PC->pSpeedList = (Vector2*)malloc(sizeof(Vector2) * NumOfCoords);
    PC->pWaitTimeList = (float*)malloc(sizeof(float) * NumOfCoords);

    for(int i = 0; i < NumOfCoords; ++i)
    {
        PC->pCoordsList[i] = CreateVec2(0, 0);
        PC->pSpeedList[i] = CreateVec2(15, 15);
        PC->pWaitTimeList[i] = 0.5f;
    }

    return 1;
}

int DestroyPath(PathComponent* PC)
{
    if (PC == NULL)
    {
        Log_Warn("DestroyPath(): PathComponent is NULL");
        return 0;
    }

    free(PC->pCoordsList);
    free(PC->pSpeedList);
    free(PC->pWaitTimeList);

    PC->pCoordsList = NULL;
    PC->pSpeedList = NULL;
    PC->pWaitTimeList = NULL;
    PC->CurrentTimer = 0.0f;
    PC->CurrentDestCoordIndex = 0;
    PC->Size = 0;

    return 1;
}

int AddCoordToPath(PathComponent* PC, Vector2 Coord)
{
    if (PC == NULL)
    {
        Log_Warn("DestroyPath(): PathComponent is NULL");
        return 0;
    }

    PathComponent Temp;
    CreatePath(&Temp, PC->Size + 1);
    CopyPath(&Temp, PC);

    DestroyPath(PC);
    CreatePath(PC, Temp.Size);
    CopyPath(PC, &Temp);

    DestroyPath(&Temp);

    SetPathCoord(PC, Coord, PC->Size - 1);

    return 0;
}

int RemoveCoordFromPath(PathComponent* PC, int Index)
{
    if (PC == NULL || Index < 0 || Index >= PC->Size)
    {
        Log_Warn("DestroyPath(): PathComponent is NULL or Index is Invalid. Index=%d, NumOfCoords=%d", Index, PC->Size);
        return 0;
    }

    // Pushing Removed Coord to Last
    for (int i = Index; i < PC->Size - 1; ++i)
    {
        SwapPathData(PC, i, i + 1);
    }
    
    PathComponent Temp;
    CreatePath(&Temp, PC->Size - 1);

    CopyPath(&Temp, PC);

    DestroyPath(PC);
    CreatePath(PC, Temp.Size);
    CopyPath(PC, &Temp);

    DestroyPath(&Temp);

    PC->CurrentDestCoordIndex = 0;
    PC->CurrentPass = 0;

    return 0;
}

int SetPathCoord(PathComponent* PC, Vector2 Coord, int Index)
{
    if (PC == NULL || Index < 0 || Index >= PC->Size)
    {
        Log_Warn("SetPathCoord(): PathComponent is NULL or Index is Invalid. Index=%d, NumOfCoords=%d", Index, PC->Size);
        return 0;
    }

    PC->pCoordsList[Index] = Coord;
}

int SetPathSpeed(PathComponent* PC, Vector2 Speed, int Index)
{
    if (PC == NULL || Index < 0 || Index >= PC->Size)
    {
        Log_Warn("SetPathSpeed(): PathComponent is NULL or Index is Invalid. Index=%d, NumOfCoords=%d", Index, PC->Size);
        return 0;
    }

    PC->pSpeedList[Index] = Speed;
}

int SetPathWaitingTime(PathComponent* PC, float WaitTime, int Index)
{
    if (PC == NULL || Index < 0 || Index >= PC->Size)
    {
        Log_Warn("SetPathWaitingTime(): PathComponent is NULL or Index is Invalid. Index=%d, NumOfCoords=%d", Index, PC->Size);
        return 0;
    }

    PC->pWaitTimeList[Index] = WaitTime;
}

int LogPath(PathComponent* PC)
{
    Log_Info("[ PATH COMPONENT ]");
    Log_Info("NumOfCoords = %d", PC->Size);
    Log_Info("Coords = ");
    for (int i = 0; i < PC->Size; ++i)
    {
        Log_Info("%d, %d", PC->pCoordsList[i].x, PC->pCoordsList[i].y);
    }
    Log_Info("Speeds = ");
    for (int i = 0; i < PC->Size; ++i)
    {
        Log_Info("%d, %d", PC->pSpeedList[i].x, PC->pSpeedList[i].y);
    }
    Log_Info("WaitingTime = ");
    for (int i = 0; i < PC->Size; ++i)
    {
        Log_Info("%f", PC->pWaitTimeList[i]);
    }
}

/*int SetPathCoords(PathComponent* PC, Vector2 CoordsList[], int Size)
{
    if (PC == NULL || PC->pCoordsList == NULL)
    {
        Log_Warn("SetPathCoords() Warning: PathComponent or CoordsList is NULL");
        return 0;
    }
    
    int Index = 0;
    for (int i = 0; i < PC->Size; ++i)
    {
        PC->pCoordsList[i] = CoordsList[Index];
        Index = (Index + 1 < Size) ? Index + 1 : 0;
    }

    return 1;
}

int SetPathSpeeds(PathComponent* PC, Vector2 SpeedList[], int Size)
{
    if (PC == NULL || PC->pSpeedList == NULL)
    {
        Log_Warn("SetPathSpeeds() Warning: PathComponent or SpeedList is NULL");
        return 0;
    }

    int Index = 0;
    for (int i = 0; i < PC->Size; ++i)
    {
        PC->pSpeedList[i] = SpeedList[Index];
        Index = (Index + 1 < Size) ? Index + 1 : 0;
    }

    return 1;
}

int SetPathWaitingTime(PathComponent* PC, float WaitTimeList[], int Size)
{
    if (PC == NULL || PC->pWaitTimeList == NULL)
    {
        Log_Warn("SetPathWaitingTime() Warning: PathComponent or WaitingList is NULL");
        return 0;
    }

    int Index = 0;
    for (int i = 0; i < PC->Size; ++i)
    {
        PC->pWaitTimeList[i] = WaitTimeList[Index];
        Index = (Index + 1 < Size) ? Index + 1 : 0;
    }

    return 1;
}*/

// Shooting Pattern
int CreateShootPattern(ShootComponent* SC, int NumOfBullets)
{
    if (SC == NULL)
    {
        Log_Warn("CreateShootPattern() Warning: SC was NULL");
        return 0;
    }

    SC->CooldownTime = 0.3f;
    SC->CurrentTimer = 1.0f;
    SC->NumOfBullets = NumOfBullets;
    SC->ShouldShoot = 1;
    SC->DistanceBetweenBullets = CreateVec2(15, 15);
    SC->BulletType = 0;

    /*SC->BulletCharBuffer = BulletCharBuffer;
    SC->BulletColorBuffer = BulletColorBuffer;
    SC->BulletColorBufferSize = BulletColorBufferSize;
    SC->SourcePosition = CreateVec2(0, 0);*/

    if (NumOfBullets < 1)
    {
        SC->pBulletIDList = NULL;
        SC->pDirList = NULL;
        SC->pPositionOffsetList = NULL;
        SC->pSpeedList = NULL;
        return 1;
    }
    
    
    SC->pDirList = (Vector2*)malloc(sizeof(Vector2) * NumOfBullets);
    SC->pPositionOffsetList = (Vector2*)malloc(sizeof(Vector2) * NumOfBullets);
    SC->pSpeedList = (Vector2*)malloc(sizeof(Vector2) * NumOfBullets);
    SC->pBulletIDList = (GOID*)malloc(sizeof(GOID) * NumOfBullets);
    
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        SC->pDirList[i] = CreateVec2(0, 0);
        SC->pPositionOffsetList[i] = CreateVec2(0, 0);
        SC->pSpeedList[i] = CreateVec2(0, 0);
        SC->pBulletIDList[i] = CG_INVALID_GOID;
    }
    return 1;
}

int DestroyShootPattern(ShootComponent* SC)
{
    if (SC == NULL)
    {
        Log_Warn("DestroyShootPattern() Warning: SC was NULL");
        return 0;
    }

    free(SC->pDirList);
    free(SC->pPositionOffsetList);
    free(SC->pSpeedList);
    free(SC->pBulletIDList);

    SC->pDirList = NULL;
    SC->pPositionOffsetList = NULL;
    SC->pSpeedList = NULL;
    SC->pBulletIDList = NULL;

    //SC->SourcePosition = CreateVec2(0, 0);
    SC->CooldownTime = 0.0f;
    SC->CurrentTimer = 0.0f;
    SC->NumOfBullets = 0;
    SC->ShouldShoot = 0;
    SC->BulletType = 0;

    return 1;
}


int AddBulletToShootPattern(ShootComponent* SC)
{
    if (SC == NULL)
    {
        Log_Warn("AddBulletToShootPattern(): SC was NULL");
        return -1;
    }
    else if (MAX_NUM_OF_BULLETS == SC->NumOfBullets)
    {
        Log_Warn("AddBulletToShootPattern(): Maximum Number of Bullets Reached! NumOfBullets=%d", SC->NumOfBullets);
        return -1;
    }

    ShootComponent Temp;
    
    CreateShootPattern(&Temp, SC->NumOfBullets + 1);
    CopyShootPattern(&Temp, SC);

    DestroyShootPattern(SC);
    CreateShootPattern(SC, Temp.NumOfBullets);
    CopyShootPattern(SC, &Temp);
    
    DestroyShootPattern(&Temp);

    SC->pDirList[SC->NumOfBullets - 1] = CreateVec2(0, 0);
    SetBulletSpeed(SC, SC->NumOfBullets - 1, CreateVec2(15, 15));

    return (SC->NumOfBullets - 1);
}

int RemoveBulletFromShootPattern(ShootComponent* SC, int Index)
{
    if (SC == NULL)
    {
        Log_Warn("RemoveBulletFromShootPattern() Warning: SC was NULL");
        return 0;
    }
    else if (Index < 0 || Index >= SC->NumOfBullets)
    {
        Log_Warn("RemoveBulletFromShootPattern() Warning: Index was Invalid. Index=%d", Index);
        return 0;
    }

    // Moving Index
    SwapBulletData(SC, Index, SC->NumOfBullets - 1);

    // Re-allocating
    ShootComponent Temp;

    CreateShootPattern(&Temp, SC->NumOfBullets - 1);
    CopyShootPattern(&Temp, SC);

    DestroyShootPattern(SC);
    CreateShootPattern(SC, Temp.NumOfBullets);
    CopyShootPattern(SC, &Temp);
    
    DestroyShootPattern(&Temp);

    return 1;
}

int SetBulletDirection(ShootComponent* SC, int Index, Vector2 Dir)
{
    if (SC == NULL || Index < 0 || Index >= SC->NumOfBullets)
    {
        Log_Warn("SetBulletDirection(): SC was NULL or Index was Invalid. Index=%d, NumOfBullets=%d", Index, SC->NumOfBullets);
        return 0;
    }

    // Find out Number of Bullets in Each Direction
    enum eAxisDirs
    {
        AXIS_DIRS_UP = 0,
        AXIS_DIRS_DOWN,
        AXIS_DIRS_LEFT,
        AXIS_DIRS_RIGHT,
        AXIS_DIRS_UP_LEFT,
        AXIS_DIRS_UP_RIGHT,
        AXIS_DIRS_DOWN_LEFT,
        AXIS_DIRS_DOWN_RIGHT,
        NUM_OF_AXIS_DIRS
    };
    int NumBulletAlreadyPlaced[NUM_OF_AXIS_DIRS];
    for (int i = 0; i < NUM_OF_AXIS_DIRS; ++i)
        NumBulletAlreadyPlaced[i] = 0;

    enum eAxisDirs DirIndex = NUM_OF_AXIS_DIRS, CurrentDirIndex = NUM_OF_AXIS_DIRS;
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        Vector2 Direction = (i != Index) ? SC->pDirList[i] : Dir;

        if (Direction.x == BULLET_DIR_UP.x && Direction.y == BULLET_DIR_UP.y)
            DirIndex = AXIS_DIRS_UP;
        else if (Direction.x == BULLET_DIR_DOWN.x && Direction.y == BULLET_DIR_DOWN.y)
            DirIndex = AXIS_DIRS_DOWN;
        else if (Direction.x == BULLET_DIR_LEFT.x && Direction.y == BULLET_DIR_LEFT.y)
            DirIndex = AXIS_DIRS_LEFT;
        else if (Direction.x == BULLET_DIR_RIGHT.x && Direction.y == BULLET_DIR_RIGHT.y)
            DirIndex = AXIS_DIRS_RIGHT;
        else if (Direction.x == BULLET_DIR_UP_LEFT.x && Direction.y == BULLET_DIR_UP_LEFT.y)
            DirIndex = AXIS_DIRS_UP_LEFT;
        else if (Direction.x == BULLET_DIR_DOWN_LEFT.x && Direction.y == BULLET_DIR_DOWN_LEFT.y)
            DirIndex = AXIS_DIRS_DOWN_LEFT;
        else if (Direction.x == BULLET_DIR_UP_RIGHT.x && Direction.y == BULLET_DIR_UP_RIGHT.y)
            DirIndex = AXIS_DIRS_UP_RIGHT;
        else if (Direction.x == BULLET_DIR_DOWN_RIGHT.x && Direction.y == BULLET_DIR_DOWN_RIGHT.y)
            DirIndex = AXIS_DIRS_DOWN_RIGHT;
        
        if (i != Index)
            ++NumBulletAlreadyPlaced[DirIndex];
        else
            CurrentDirIndex = DirIndex;
    }

    int IgnoreFlag = 0;
    if (CurrentDirIndex == AXIS_DIRS_UP_RIGHT || CurrentDirIndex == AXIS_DIRS_UP_LEFT || 
        CurrentDirIndex == AXIS_DIRS_DOWN_RIGHT || CurrentDirIndex == AXIS_DIRS_DOWN_LEFT)
    {
        if (NumBulletAlreadyPlaced[CurrentDirIndex] >= 1)
            IgnoreFlag = 1;
    }
    else if (CurrentDirIndex == AXIS_DIRS_UP || CurrentDirIndex == AXIS_DIRS_LEFT || 
             CurrentDirIndex == AXIS_DIRS_DOWN || CurrentDirIndex == AXIS_DIRS_RIGHT)
    {
        if (NumBulletAlreadyPlaced[CurrentDirIndex] >= 2)
            IgnoreFlag = 1;
    }

    if (!IgnoreFlag)
        SC->pDirList[Index] = Dir;
}

int SetBulletSpeed(ShootComponent* SC, int Index, Vector2 Speed)
{
    if (SC == NULL || Index < 0 || Index >= SC->NumOfBullets)
    {
        Log_Warn("SetBulletSpeed(): SC was NULL or Index was Invalid. Index=%d, NumOfBullets=%d", Index, SC->NumOfBullets);
        return 0;
    }

    SC->pSpeedList[Index] = Speed;
}

void LogShootPattern(ShootComponent* SC)
{
    if (SC == NULL)
    {
        Log_Warn("RemoveBulletFromShootPattern() Warning: SC was NULL");
        return;
    }

    Log_Info("[ SHOOT COMPONENT ]");
    Log_Info("NumOfBullets: %d", SC->NumOfBullets);
    Log_Info("IDs (Unintialized): ");
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        Log_Info("%d", SC->pBulletIDList[i]);
    }
    Log_Info("Directions: ");
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        Log_Info("%d, %d", SC->pDirList[i].x, SC->pDirList[i].y);
    }
    Log_Info("Speeds: ");
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        Log_Info("%d, %d", SC->pSpeedList[i].x, SC->pSpeedList[i].y);
    }
}

/*int SetShootDirections(ShootComponent* SC, Vector2 DirsList[], int Size)
{
    if (SC == NULL || SC->pDirList == NULL)
    {
        Log_Warn("SetShootDirections() Warning: SC or its Direction List was NULL");
        return 0;
    }

    for (int i = 0; i < SC->NumOfBullets; ++i)
        SC->pDirList[i] = (i < Size) ? DirsList[i] : CreateVec2(0, 0);

    return 1;
}


int SetShootSpeeds(ShootComponent* SC, Vector2 SpeedList[], int Size)
{
    if (SC == NULL || SC->pSpeedList == NULL)
    {
        Log_Warn("SetShootPositions() Warning: SC or its Position List was NULL");
        return 0;
    }

    int Index = 0;
    for (int i = 0; i < SC->NumOfBullets; ++i)
    {
        SC->pSpeedList[i] = SpeedList[Index];
        Index = (Index + 1 < Size) ? Index + 1 : 0;
    }

    return 1;
}*/

int SetShootPositionOffsets(ShootComponent* SC, Vector2 SourceSize, Vector2 BulletSize)
{
    if (SC == NULL || SC->pPositionOffsetList == NULL)
    {
        Log_Warn("SetShootPositionOffsets() Warning: SC or its Position List was NULL");
        return 0;
    }

    GenerateBulletPositionOffsets(SC->pPositionOffsetList, SourceSize, BulletSize, SC->pDirList, SC->NumOfBullets);

    return 1;
}

/*int SetShootBulletData(ShootComponent* SC, char* CharBuffer, Color* ColorBuffer, int Size)
{
    if (SC == NULL)
    {
        Log_Warn("SetShootBulletData() Warning: SC or its Position List was NULL");
        return 0;
    }

    SC->BulletCharBuffer = CharBuffer;
    SC->BulletColorBuffer = ColorBuffer;
    SC->BulletColorBufferSize = Size;
}*/

void TEST_ShootFunctions()
{
    ShootComponent SC;
    CreateShootPattern(&SC, 0);
    //Log_Info("Created 0 Bullets");
    AddBulletToShootPattern(&SC);
    LogShootPattern(&SC);
    
    //Log_Info("Added First Bullet");
    int i = AddBulletToShootPattern(&SC);
    LogShootPattern(&SC);
    
    AddBulletToShootPattern(&SC);
    LogShootPattern(&SC);
    
    int j = AddBulletToShootPattern(&SC);
    SC.pDirList[j] = CreateVec2(69, 420);
    SC.pSpeedList[j] = CreateVec2(96, 24);
    LogShootPattern(&SC);
    
    RemoveBulletFromShootPattern(&SC, SC.NumOfBullets - 1);
    RemoveBulletFromShootPattern(&SC, SC.NumOfBullets - 1);
    RemoveBulletFromShootPattern(&SC, SC.NumOfBullets - 1);
    RemoveBulletFromShootPattern(&SC, SC.NumOfBullets - 1);
    
    LogShootPattern(&SC);

    DestroyShootPattern(&SC);
}

void TEST_PathFunctions()
{
    PathComponent PC;
    CreatePath(&PC, 0);

    AddCoordToPath(&PC, CreateVec2(10, 10));
    LogPath(&PC);

    AddCoordToPath(&PC, CreateVec2(20, 20));
    LogPath(&PC);

    AddCoordToPath(&PC, CreateVec2(30, 30));
    LogPath(&PC);

    AddCoordToPath(&PC, CreateVec2(40, 40));
    LogPath(&PC);
    
    RemoveCoordFromPath(&PC, 0);
    LogPath(&PC);

    RemoveCoordFromPath(&PC, 0);
    LogPath(&PC);

    RemoveCoordFromPath(&PC, 0);
    LogPath(&PC);

    RemoveCoordFromPath(&PC, 0);
    LogPath(&PC);
    
    DestroyPath(&PC);
}

int CopyShootPattern(ShootComponent* Dst, ShootComponent* Src)
{
    if (Dst == NULL || Src == NULL)
    {
        Log_Warn("CopyShootPattern(): Dst or Src was NULL");
        return 0;
    }

    Dst->BulletType = Src->BulletType;
    Dst->CooldownTime = Src->CooldownTime;
    Dst->CurrentTimer = Src->CurrentTimer;
    Dst->DistanceBetweenBullets = Src->DistanceBetweenBullets;
    
    int Size = (Dst->NumOfBullets < Src->NumOfBullets) ? Dst->NumOfBullets : Src->NumOfBullets;
    for (int i = 0; i < Size; ++i)
    {
        Dst->pBulletIDList[i] = Src->pBulletIDList[i];
        Dst->pDirList[i] = Src->pDirList[i];
        Dst->pSpeedList[i] = Src->pSpeedList[i];
        Dst->pPositionOffsetList[i] = Src->pPositionOffsetList[i];
    }
    //Dst->NumOfBullets = Src->NumOfBullets;
    Dst->ShouldShoot = Src->ShouldShoot;

    return 1;
}

int SwapBulletData(ShootComponent* SC, int i, int j)
{
    if (i == j)
        return 0;

    GOID ID = SC->pBulletIDList[i];
    Vector2 Dir = SC->pDirList[i];
    Vector2 PosOff = SC->pPositionOffsetList[i];
    Vector2 Speed = SC->pSpeedList[i];
    
    SC->pBulletIDList[i] = SC->pBulletIDList[j];
    SC->pDirList[i] = SC->pDirList[j];
    SC->pPositionOffsetList[i] = SC->pPositionOffsetList[j];
    SC->pSpeedList[i] = SC->pSpeedList[j];

    SC->pBulletIDList[j] = ID;
    SC->pDirList[j] = Dir;
    SC->pPositionOffsetList[j] = PosOff;
    SC->pSpeedList[j] = Speed;

    return 1;
}

void GenerateBulletPositionOffsets(Vector2 Offsets[], Vector2 SourceSize, Vector2 BulletSize, Vector2 Directions[], int Size)
{
    Vector2 HalfSourceSize = CreateVec2( (int)((float)SourceSize.x / 2.0f), (int)((float)SourceSize.y / 2.0f) );
    Vector2 HalfBulletSize = CreateVec2( (int)((float)BulletSize.x / 2.0f), (int)((float)BulletSize.y / 2.0f) );

    enum eAxisDirs
    {
        AXIS_DIRS_UP = 0,
        AXIS_DIRS_DOWN,
        AXIS_DIRS_LEFT,
        AXIS_DIRS_RIGHT,
        NUM_OF_AXIS_DIRS
    };
    int NumBulletAlreadyPlaced[NUM_OF_AXIS_DIRS];
    int AxisDirsBulletIndex[NUM_OF_AXIS_DIRS];
    for (int i = 0; i < NUM_OF_AXIS_DIRS; ++i)
    {
        NumBulletAlreadyPlaced[i] = 0;
        AxisDirsBulletIndex[i] = -1;
    }

    for (int i = 0; i < Size; ++i)
    {
        if (Directions[i].x < 0 && Directions[i].y < 0)         // Diagonal
            Offsets[i] = CreateVec2(-BULLET_POSITION_OFFSET, -BULLET_POSITION_OFFSET);
        else if (Directions[i].x > 0 && Directions[i].y < 0)
            Offsets[i] = CreateVec2(SourceSize.x + BULLET_POSITION_OFFSET, -BULLET_POSITION_OFFSET);
        else if (Directions[i].x > 0 && Directions[i].y > 0)
            Offsets[i] = CreateVec2(SourceSize.x + BULLET_POSITION_OFFSET, SourceSize.y + BULLET_POSITION_OFFSET);
        else if (Directions[i].x < 0 && Directions[i].y > 0)
            Offsets[i] = CreateVec2(-BULLET_POSITION_OFFSET, SourceSize.y + BULLET_POSITION_OFFSET);
        else if (Directions[i].x == 0 && Directions[i].y != 0)  // Up-Down
        {
            // Determine Direction
            enum eAxisDirs DirIndex = AXIS_DIRS_UP;
            if (Directions[i].y > 0)
                DirIndex = AXIS_DIRS_DOWN;
        
            // Not More than 2 Bullets
            if (NumBulletAlreadyPlaced[DirIndex] == 2)
                continue;
            
            // Double Bullets Allowed
            int XOffset = HalfSourceSize.x - BulletSize.x + 1;
            if (NumBulletAlreadyPlaced[DirIndex] == 1)
            {
                Offsets[AxisDirsBulletIndex[DirIndex]].x = HalfSourceSize.x - BulletSize.x - 1;
                XOffset = HalfSourceSize.x + BulletSize.x + 1;
                NumBulletAlreadyPlaced[DirIndex] = 2;
            }

            // One Bullet Added
            NumBulletAlreadyPlaced[DirIndex] = (NumBulletAlreadyPlaced[DirIndex] == 0) ? 1 : NumBulletAlreadyPlaced[DirIndex];

            AxisDirsBulletIndex[DirIndex] = i;
            Offsets[i] = CreateVec2(XOffset, -BULLET_POSITION_OFFSET);
            if (Directions[i].y > 0)
                Offsets[i].y = SourceSize.y + BULLET_POSITION_OFFSET;
        }
        else if (Directions[i].x != 0 && Directions[i].y == 0)  // Left-Right
        {
            // Determine Direction
            enum eAxisDirs DirIndex = AXIS_DIRS_LEFT;
            if (Directions[i].x > 0)
                DirIndex = AXIS_DIRS_RIGHT;
        
            // Not More than 2 Bullets
            if (NumBulletAlreadyPlaced[DirIndex] == 2)
                continue;
            
            // Double Bullets Allowed
            int YOffset = HalfSourceSize.y - HalfBulletSize.y + 1;
            if (NumBulletAlreadyPlaced[DirIndex] == 1)
            {
                Offsets[AxisDirsBulletIndex[DirIndex]].y = HalfSourceSize.y - HalfBulletSize.y - 1;
                YOffset = HalfSourceSize.y + HalfBulletSize.y + 1;
                NumBulletAlreadyPlaced[DirIndex] = 2;
            }

            // One Bullet Added
            NumBulletAlreadyPlaced[DirIndex] = (NumBulletAlreadyPlaced[DirIndex] == 0) ? 1 : NumBulletAlreadyPlaced[DirIndex];

            AxisDirsBulletIndex[DirIndex] = i;

            Offsets[i] = CreateVec2(-BULLET_POSITION_OFFSET, YOffset);
            if (Directions[i].x > 0)
                Offsets[i].x = SourceSize.x + BULLET_POSITION_OFFSET;
        }
    }
}

// PATH FUNCTIONS

int SwapPathData(PathComponent* PC, int i, int j)
{
    if (i == j)
        return 0;

    Vector2 Coord =  PC->pCoordsList[i];
    Vector2 Speed =  PC->pSpeedList[i];
    float WaitTime = PC->pWaitTimeList[i];
    
    PC->pCoordsList[i]   = PC->pCoordsList[j];
    PC->pSpeedList[i]    = PC->pSpeedList[j];
    PC->pWaitTimeList[i] = PC->pWaitTimeList[j];

    PC->pCoordsList[j]   = Coord;
    PC->pSpeedList[j]    = Speed;
    PC->pWaitTimeList[j] = WaitTime;    

    return 1;
}

int CopyPath(PathComponent* Dst, PathComponent* Src)
{
    Dst->CurrentDestCoordIndex = Src->CurrentDestCoordIndex;
    Dst->CurrentPass = Src->CurrentPass;
    Dst->PathLogic = Src->PathLogic;
    
    int Size = (Dst->Size < Src->Size) ? Dst->Size : Src->Size;
    for (int i = 0; i < Size; ++i)
    {
        Dst->pCoordsList[i] = Src->pCoordsList[i];
        Dst->pSpeedList[i] = Src->pSpeedList[i];
        Dst->pWaitTimeList[i] = Src->pWaitTimeList[i];
    }

    return 1;
}
