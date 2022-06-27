#include "FileManager.h"

/* CONFIG DATA */
// Horde File will be Filled with Blocks of Data where each Block has a #Title and Key=Value Pairs
// Blocks are written/read One After Another, Recognized as Blocks By their Title Token

// Data
static const int HF_MAX_LINE_LEN = 64;


// Horde Data Tokens
static const char* HF_NUM_OF_ENEMIES_TOKEN = "NumOfEnemies";

// Shooting Component
static const char* HF_NUM_OF_BULLETS_TOKEN = "NumOfBullets";
static const char* HF_BULLET_DIRS_TOKEN = "BulletDirections";
static const char* HF_BULLET_SPEEDS_TOKEN = "BulletSpeeds";
static const char* HF_BULLET_COOLDOWN_TIME_TOKEN = "CooldownTime";
static const char* HF_BULLET_TYPE_TOKEN = "BulletType";
static const char* HF_TIMED_SHOOTER_ACTIVE_TIME_TOKEN = "ActiveTime";
static const char* HF_TIMED_SHOOTER_WAIT_TIME_TOKEN = "WaitTime";
static const char* HF_CONDITIONAL_SHOOTER_FUNC_TOKEN = "CondFunc";

// Path Component
static const char* HF_NUM_OF_COORDS_TOKEN = "NumOfCoords";
static const char* HF_PATH_COORDS_TOKEN = "PathCoords";
static const char* HF_PATH_SPEEDS_TOKEN = "PathSpeeds";
static const char* HF_PATH_WAIT_TIMES_TOKEN = "PathWaitTimePerCoord";
static const char* HF_PATH_LOGIC_TOKEN = "PathLogic";


/* GLOBAL FUNCTION DEFINITIONS */
int WriteToHordeFile(FileManager* FM, EnemyHorde* Horde)
{
    if (FM == NULL)
    {
        Log_Critical("WriteToHordeFile(): File Manager is NULL");
        return 0;
    }
    else if (Horde == NULL)
    {
        Log_Critical("WriteToHordeFile(): Horde is NULL");
        return 0;
    }
    else if (!FM->FileOpenFlag)
    {
        Log_Critical("WriteToHordeFile(): No File is Open.");
        return 0;
    }
    else if (strcmp(FM->FileExt, HF_EXTENSION) != 0)
    {
        Log_Critical("WriteToHordeFile(): Not a Horde File. Name: %s, Extension: %s", FM->FileName, FM->FileExt);
        return 0;
    }
    // Writing a Horde
    // Horde Data
    fprintf(FM->File, "#HordeData");
    fprintf(FM->File, "\n%s = %d", HF_NUM_OF_ENEMIES_TOKEN, Horde->NumOfEnemies);
    
    // Enemies
    for (int i = 0; i < Horde->NumOfEnemies; ++i)
    {
        // Data
        GOID ID = Horde->IDList[i];
        ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
        TimedShooterComponent* TSC = &g_SharedCompList.TimedShooterCompList[ID];
        ConditionalShooterComponent* CondComp = &g_SharedCompList.CondShooterCompList[ID];
        PathComponent* PC = &g_SharedCompList.PathCompList[ID];

        // Enemy
        fprintf(FM->File, "\n#Enemy%d", (i + 1));
        
        // Shoot Component
        fprintf(FM->File, "\n%s = %d", HF_NUM_OF_BULLETS_TOKEN, SC->NumOfBullets);
        fprintf(FM->File, "\n%s = ", HF_BULLET_DIRS_TOKEN);
        for (int j = 0; j < SC->NumOfBullets; ++j)
            fprintf(FM->File, "\n(%d, %d), ", SC->pDirList[j].x, SC->pDirList[j].y);
        fprintf(FM->File, "\n%s = ", HF_BULLET_SPEEDS_TOKEN);
        for (int j = 0; j < SC->NumOfBullets; ++j)
            fprintf(FM->File, "\n(%d, %d), ", SC->pSpeedList[j].x, SC->pSpeedList[j].y);
        fprintf(FM->File, "\n%s = %f", HF_BULLET_COOLDOWN_TIME_TOKEN, SC->CooldownTime);
        fprintf(FM->File, "\n%s = %d", HF_BULLET_TYPE_TOKEN, SC->BulletType);
        fprintf(FM->File, "\n%s = %f", HF_TIMED_SHOOTER_ACTIVE_TIME_TOKEN, TSC->ActiveTime);
        fprintf(FM->File, "\n%s = %f", HF_TIMED_SHOOTER_WAIT_TIME_TOKEN, TSC->WaitTime);
        fprintf(FM->File, "\n%s = %d", HF_CONDITIONAL_SHOOTER_FUNC_TOKEN, CondComp->FuncEnum);

        // Path Component
        fprintf(FM->File, "\n%s = %d", HF_NUM_OF_COORDS_TOKEN, PC->Size);
        fprintf(FM->File, "\n%s = ", HF_PATH_COORDS_TOKEN);
        for (int j = 0; j < PC->Size; ++j)
            fprintf(FM->File, "\n(%d, %d), ", PC->pCoordsList[j].x, PC->pCoordsList[j].y);
        fprintf(FM->File, "\n%s = ", HF_PATH_SPEEDS_TOKEN);
        for (int j = 0; j < PC->Size; ++j)
            fprintf(FM->File, "\n(%d, %d), ", PC->pSpeedList[j].x, PC->pSpeedList[j].y);
        fprintf(FM->File, "\n%s = ", HF_PATH_WAIT_TIMES_TOKEN);
        for (int j = 0; j < PC->Size; ++j)
            fprintf(FM->File, "\n%f, ", PC->pWaitTimeList[j]);
        fprintf(FM->File, "\n%s = %d", HF_PATH_LOGIC_TOKEN, (int)PC->PathLogic);
    }

    // End of File
    fprintf(FM->File, "\n");
    

    return 1;
}

int ReadFromHordeFile(FileManager* FM, EnemyHorde* Horde, SpawnRecord* pSR)
{
    if (FM == NULL)
    {
        Log_Critical("ReadFromHordeFile(): File Manager is NULL");
        return 0;
    }
    else if (Horde == NULL)
    {
        Log_Critical("ReadFromHordeFile(): Horde is NULL");
        return 0;
    }
    else if (!FM->FileOpenFlag)
    {
        Log_Critical("ReadFromHordeFile(): No File is Open.");
        return 0;
    }
    else if (FM->FileExt != NULL && strcmp(FM->FileExt, HF_EXTENSION) != 0)
    {
        Log_Critical("ReadFromHordeFile(): Not a Horde File. Name: %s, Extension: %s", FM->FileName, FM->FileExt);
        return 0;
    }

    // Reading a Horde
    char Buffer[HF_MAX_LINE_LEN];
    char* ResStr = NULL;

    // Horde Data
    // Discarding Title
    fgets(Buffer, HF_MAX_LINE_LEN, FM->File); 
    // NumOfEnemies
    fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
    ResStr = strchr(Buffer, '=');
    Horde->NumOfEnemies = (int)strtol(++ResStr, NULL, 10);
    
    // Allocating IDs
    if (Horde->IDList != NULL)
        free(Horde->IDList);
    Horde->IDList = (GOID*)malloc(sizeof(GOID) * Horde->NumOfEnemies);

    SpawnExt(pSR, TAG_ENEMY, Horde->NumOfEnemies, Horde->IDList);

    for (int i = 0; i < Horde->NumOfEnemies; ++i)
    {
        GOID ID = Horde->IDList[i];
        ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
        TimedShooterComponent* TSC = &g_SharedCompList.TimedShooterCompList[ID];
        ConditionalShooterComponent* CondComp = &g_SharedCompList.CondShooterCompList[ID];
        PathComponent* PC = &g_SharedCompList.PathCompList[ID];

        // Discarding Enemy Title
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);

        // Shoot Component
        // Number of Bullets
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        int NumOfBullets = (int)strtol(++ResStr, NULL, 10);
        CreateShootPattern(SC, NumOfBullets);
        // Directions
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        for (int j = 0; j < NumOfBullets; ++j)
        {
            fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
            ResStr = strchr(Buffer, '(');
            SC->pDirList[j].x = (int)strtol(++ResStr, &ResStr, 10);
            SC->pDirList[j].y = (int)strtol(ResStr + 2, NULL, 10);
        }
        // Speeds
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        for (int j = 0; j < NumOfBullets; ++j)
        {
            fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
            ResStr = strchr(Buffer, '(');
            SC->pSpeedList[j].x = (int)strtol(++ResStr, &ResStr, 10);
            SC->pSpeedList[j].y = (int)strtol(ResStr + 2, NULL, 10);
        }
        // Cooldown Time
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        SC->CooldownTime = (float)strtof(++ResStr, NULL);
        // Bullet Type
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        SC->BulletType = (int)strtol(++ResStr, NULL, 10);
        
        // Timed Shooter
        // Active Time
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        TSC->ActiveTime = (float)strtof(++ResStr, NULL);
        // Wait Time
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        TSC->WaitTime = (float)strtof(++ResStr, NULL);
        
        // Conditional Shooter
        // FuncEnum
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        CondComp->FuncEnum = (int)strtol(++ResStr, NULL, 10);
        

        // PATH COMPONENT
        // Num Of Coords
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        int NumOfCoords = (int)strtol(++ResStr, NULL, 10);
        CreatePath(PC, NumOfCoords);
        // Coords
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        for (int j = 0; j < NumOfCoords; ++j)
        {
            fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
            ResStr = strchr(Buffer, '(');
            PC->pCoordsList[j].x = (int)strtol(++ResStr, &ResStr, 10);
            PC->pCoordsList[j].y = (int)strtol(ResStr + 2, NULL, 10);
        }
        // Speeds
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        for (int j = 0; j < NumOfCoords; ++j)
        {
            fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
            ResStr = strchr(Buffer, '(');
            PC->pSpeedList[j].x = (int)strtol(++ResStr, &ResStr, 10);
            PC->pSpeedList[j].y = (int)strtol(ResStr + 2, NULL, 10);
        }
        // Wait Times
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        for (int j = 0; j < NumOfCoords; ++j)
        {
            fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
            PC->pWaitTimeList[j] = (float)strtof(Buffer, NULL);
        }
        // Path Logic
        fgets(Buffer, HF_MAX_LINE_LEN, FM->File);
        ResStr = strchr(Buffer, '=');
        PC->PathLogic = (enum ePathLogic)((int)strtol(++ResStr, NULL, 10));
    }

    fseek(FM->File, 0, SEEK_SET);
    return 1;
}
