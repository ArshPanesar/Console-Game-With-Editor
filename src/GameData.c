#include "GameData.h"

Vector2 g_TerminalSize;
double g_dtAsSeconds;
Vector2 g_ScreenSize;

struct ComponentList g_SharedCompList;

// GOID Stack Impl
int Top = -1;
int Size = CG_MAX_NUM_OF_GAME_OBJ;
GOID ID_Stack[CG_MAX_NUM_OF_GAME_OBJ];

int AddToIDList(GOIDList* pList, const GOID ID);
int RemoveIDFromList(GOIDList* pList, const GOID ID);

void InitGameData(void)
{
    // Initializing ID Stack
    int FillID = Size - 1;
    for (int i = 0; i < Size; ++i)
        ID_Stack[i] = FillID--;
    Top = Size - 1;

    // Initializing ID Lists in Shared Component List
    g_SharedCompList.IDList_InfoComp.CurrentSize = 0;
    g_SharedCompList.IDList_MovementComp.CurrentSize = 0;
    g_SharedCompList.IDList_DrawingComp.CurrentSize = 0;
    g_SharedCompList.IDList_PathComp.CurrentSize = 0;
    g_SharedCompList.IDList_ShootComp.CurrentSize = 0;
    g_SharedCompList.IDList_ColliderComp.CurrentSize = 0;
    g_SharedCompList.IDList_TimedShooterComp.CurrentSize = 0;
    g_SharedCompList.IDList_CondShooterComp.CurrentSize = 0;
    g_SharedCompList.IDList_AnimationComp.CurrentSize = 0;
}

int GOIDArrayInsert(GOID* Array, int Size, GOID Key, int Index)
{
    if (Index < Size)
    {
        Array[Index] = Key;
        return 1;
    }

    Log_Warn("GOIDArrayInsert() Warning: Index Exceeds Size!");
    return 0;
}

int GOIDArrayDelete(GOID* Array, int* Size, GOID Key, int Start)
{
    // Linear Search
    int i, Found = 0;
    for (i = Start; i < *Size; ++i)
    {
        if (Array[i] == Key)
        {
            Found = 1;
            break;
        }
    }

    if (!Found)
    {
        Log_Warn("GOIDArrayDelete() Warning: Key Not Found!");
        return 0;
    }
    
    for (i = i; (i + 1) < *Size; ++i)
        Array[i] = Array[i + 1];
    
    *Size = *Size - 1;
}

GOID CreateGameObj(void)
{
    // Popping from the Stack
    if (Top < 0)
    {
        Log_Critical("CreateGameObj() Error: No More IDs Left! Maximum Size Exceeded = %d", Size);
        return CG_INVALID_GOID;
    }

    GOID ID = ID_Stack[Top--];
    return ID;
}

void DestroyGameObj(GOID ID)
{
    if (Top < Size)
    {
        ID_Stack[++Top] = ID;
        return;
    }

    Log_Critical("DestroyGameObj() Error: Too Many IDs! Maximum Size Exceeded = %d", Size);
}

int AddToIDList(GOIDList* pList, const GOID ID)
{
    if (pList->CurrentSize + 1 >= CG_MAX_NUM_OF_GAME_OBJ)
    {
        Log_Critical("AddToIDList() Error: ID %u Cannot Be Added As Maximum Number of Game Obj Exceeded!", ID);
        return 0;
    }

    ++(pList->CurrentSize);
    return GOIDArrayInsert(pList->IDList, pList->CurrentSize, ID, pList->CurrentSize - 1);
}

int RemoveIDFromList(GOIDList* pList, const GOID ID)
{
    if (pList->CurrentSize - 1 < 0)
    {
        Log_Warn("RemoveIDFromList() Error: No IDs Present in List. Called for ID %u", ID);
        return 0;
    }

    return GOIDArrayDelete(pList->IDList, &pList->CurrentSize, ID, 0);
}

int AttachComponent(const enum eComponent CompEnum, const GOID ID)
{
    switch(CompEnum)
    {
        case COMP_INFO:
            return AddToIDList(&g_SharedCompList.IDList_InfoComp, ID);
        case COMP_DRAWING:
            return AddToIDList(&g_SharedCompList.IDList_DrawingComp, ID);
        case COMP_MOVEMENT:
            return AddToIDList(&g_SharedCompList.IDList_MovementComp, ID);
        case COMP_PATH:
            return AddToIDList(&g_SharedCompList.IDList_PathComp, ID);
        case COMP_SHOOT:
            return AddToIDList(&g_SharedCompList.IDList_ShootComp, ID);
        case COMP_COLLIDER:
            return AddToIDList(&g_SharedCompList.IDList_ColliderComp, ID);
        case COMP_TIMED_SHOOTER:
            return AddToIDList(&g_SharedCompList.IDList_TimedShooterComp, ID);
        case COMP_CONDITIONAL_SHOOTER:
            return AddToIDList(&g_SharedCompList.IDList_CondShooterComp, ID);
        case COMP_ANIMATION:
            return AddToIDList(&g_SharedCompList.IDList_AnimationComp, ID);
    }
    
    Log_Warn("AttachComponent() Error: Component %d Not Recognized for GOID %u", CompEnum, ID);
    return 0;
}

int DetachComponent(const enum eComponent CompEnum, const GOID ID)
{
    switch(CompEnum)
    {
        case COMP_INFO:
            return RemoveIDFromList(&g_SharedCompList.IDList_InfoComp, ID);
        case COMP_DRAWING:
            return RemoveIDFromList(&g_SharedCompList.IDList_DrawingComp, ID);
        case COMP_MOVEMENT:
            return RemoveIDFromList(&g_SharedCompList.IDList_MovementComp, ID);
        case COMP_PATH:
            return RemoveIDFromList(&g_SharedCompList.IDList_PathComp, ID);
        case COMP_SHOOT:
            return RemoveIDFromList(&g_SharedCompList.IDList_ShootComp, ID);
        case COMP_COLLIDER:
            return RemoveIDFromList(&g_SharedCompList.IDList_ColliderComp, ID);
        case COMP_TIMED_SHOOTER:
            return RemoveIDFromList(&g_SharedCompList.IDList_TimedShooterComp, ID);
        case COMP_CONDITIONAL_SHOOTER:
            return RemoveIDFromList(&g_SharedCompList.IDList_CondShooterComp, ID);
        case COMP_ANIMATION:
            return RemoveIDFromList(&g_SharedCompList.IDList_AnimationComp, ID);
    }
    
    Log_Warn("DetachComponent() Error: Component %d Not Recognized for GOID %u", CompEnum, ID);
    return 0;
}

int HasComponent(const enum eComponent CompEnum, const GOID ID)
{
    GOIDList* List = NULL;
    switch(CompEnum)
    {
        case COMP_INFO:
            List = &g_SharedCompList.IDList_InfoComp;
            break;
        case COMP_DRAWING:
            List = &g_SharedCompList.IDList_DrawingComp;
            break;
        case COMP_MOVEMENT:
            List = &g_SharedCompList.IDList_MovementComp;
            break;
        case COMP_PATH:
            List = &g_SharedCompList.IDList_PathComp;
            break;
        case COMP_SHOOT:
            List = &g_SharedCompList.IDList_ShootComp;
            break;
        case COMP_COLLIDER:
            List = &g_SharedCompList.IDList_ColliderComp;
            break;
        case COMP_TIMED_SHOOTER:
            List = &g_SharedCompList.IDList_TimedShooterComp;
            break;
        case COMP_CONDITIONAL_SHOOTER:
            List = &g_SharedCompList.IDList_CondShooterComp;
            break;
        case COMP_ANIMATION:
            List = &g_SharedCompList.IDList_AnimationComp;
            break;
    }

    if (List == NULL)
    {
        Log_Warn("HasComponent() Error: Component %d Does Not Exist! Called For GOID %u", CompEnum, ID);
        return 0;
    }
    
    // Linear Search
    for (int i = 0; i < List->CurrentSize; ++i)
    {
        if (List->IDList[i] == ID)
            return 1;
    }

    return 0;
}

const char* TagInString(enum eTag Name)
{
    switch (Name)
    {
        case TAG_PLAYER:
            return "PLAYER";
        
        case TAG_ENEMY:
            return "ENEMY";
        
        case TAG_BULLET:
            return "BULLET";
    }

    return "";
}
