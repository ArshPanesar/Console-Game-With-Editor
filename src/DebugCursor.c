#include "DebugCursor.h"

// DATA
static int DebugCursorAllocatedFlag = 0;
static int ActiveFlag = 0;

GOID DebugCursorID = CG_INVALID_GOID;

static GOID DebugMenuID = CG_INVALID_GOID;
static Vector2 LastCursorPosition = (Vector2){ 0, 0 };

enum eCursorState 
{
    CURSOR_EMPTY,
    CURSOR_SELECTED
};
static enum eCursorState CursorState = CURSOR_EMPTY;
static GOID SelectedID = CG_INVALID_GOID;

enum eCursorMoveState
{
    CURSOR_MOVE_FREE,
    CURSOR_MOVE_GOID_LIST
};
static enum eCursorMoveState CursorMoveState = CURSOR_MOVE_FREE;
static enum eCursorMoveState PrevCursorMoveState = CURSOR_MOVE_FREE;

static const float SPACE_KEY_ACCEPT_TIME = 0.3f; 

static GOID* CursorGOIDList = NULL;
static int   CursorGOIDListSize = 0;
static int   CurrentCursorGOIDListIndex = 0;

// Rulers for Alignment
enum eRulerGOIDIndex
{
    RULER_UP_INDEX = 0,
    RULER_DOWN_INDEX,
    RULER_LEFT_INDEX,
    RULER_RIGHT_INDEX,
    NUM_OF_RULERS
};

static GOID RulerGOIDList[NUM_OF_RULERS];

void CreateRulers(void);
void UpdateRulers(Vector2 TargetPosition, Vector2 TargetSize);
void DestroyRulers(void);

// FUNCTIONS
int AllocateDebugCursor(SpawnRecord* pSpawnRecord)
{
    if (DebugCursorAllocatedFlag)
    {
        Log_Warn("In DebugCursor.c -> AllocateDebugCursor(): Function Called But Data Already Allocated.")
        return 0;
    }

    DebugCursorID = CreateGameObj();
    AttachComponent(COMP_DRAWING, DebugCursorID);

    // Rulers
    for (int i = 0; i < NUM_OF_RULERS; ++i)
    {
        RulerGOIDList[i] = CreateGameObj();
        AttachComponent(COMP_DRAWING, RulerGOIDList[i]);

        Sprite* Spr = &g_SharedCompList.DrawingCompList[RulerGOIDList[i]].Spr;
        Spr->CharBuffer = NULL;
        Spr->ColorBuffer = NULL;
        Spr->Size = Spr->Position = CreateVec2(0, 0);
    }

    DebugCursorAllocatedFlag = 1;
    return 1;
}

int DeallocateDebugCursor(SpawnRecord* pSpawnRecord)
{
    if (!DebugCursorAllocatedFlag)
    {
        Log_Warn("In DebugCursor.c -> DeallocateDebugCursor(): Function Called But Data NOT Allocated.")
        return 0;
    }

    QuitCursor();

    DetachComponent(COMP_DRAWING, DebugCursorID);
    DebugCursorID = CreateGameObj();
    
    // Rulers
    for (int i = 0; i < NUM_OF_RULERS; ++i)
    {
        DestroySprite(&g_SharedCompList.DrawingCompList[RulerGOIDList[i]].Spr);
        DetachComponent(COMP_DRAWING, RulerGOIDList[i]);
        
        DestroyGameObj(RulerGOIDList[i]);
        RulerGOIDList[i] = CG_INVALID_GOID;
    }

    DebugCursorAllocatedFlag = 0;
    return 1;
}

// DEBUG CURSOR
void InitCursor(GOID MenuID)
{
    if (MenuID == CG_INVALID_GOID)
    {
        Log_Critical("In DebugCursor.c -> InitCursor(MenuID): MenuID is INVALID.");
        return;
    }

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    DestroySprite(pSpr);

    char* CursorSprite =    "CCC\n"\
                            "CCC";

    SetSprite(pSpr, CursorSprite, '\n');
    FillSpriteSingleColor(pSpr, C_g);

    pSpr->Position = LastCursorPosition;

    DebugMenuID = MenuID;

    SetCursorMoveFree();
    ActiveFlag = 1;
}

void UpdateCursor(void)
{
    static float SpaceKeyTimer = 0.0f;
    static float MoveTimer = 0.0f;
    
    if (!ActiveFlag)
    {
        Log_Warn("In DebugCursor.c -> UpdateCursor(): Function Called But Cursor Not Yet Active. Check Log For Critical Error(s).");
        return;
    }
    // Local Functions
    GOID TestCursorIntersection(void);

    // Input
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    if (CursorMoveState == CURSOR_MOVE_FREE)
    {
        if (IsKeyPressed(CG_KEY_W))
            pSpr->Position.y -= 1;
        else if (IsKeyPressed(CG_KEY_S))
            pSpr->Position.y += 1;

        if (IsKeyPressed(CG_KEY_A))
            pSpr->Position.x -= 1;
        else if (IsKeyPressed(CG_KEY_D))
            pSpr->Position.x += 1;
    }
    else if (CursorMoveState == CURSOR_MOVE_GOID_LIST && MoveTimer > 0.1f)
    {
        if (IsKeyPressed(CG_KEY_W) || IsKeyPressed(CG_KEY_S) || IsKeyPressed(CG_KEY_A) || IsKeyPressed(CG_KEY_D))
        {
            do
            {
                CurrentCursorGOIDListIndex = (CurrentCursorGOIDListIndex + 1 < CursorGOIDListSize) ? CurrentCursorGOIDListIndex + 1 : 0;
            } 
            while(CursorGOIDList[CurrentCursorGOIDListIndex] == DebugCursorID);

            Sprite* pSprGO = &g_SharedCompList.DrawingCompList[CursorGOIDList[CurrentCursorGOIDListIndex]].Spr;
            pSpr->Position = Vec2_Sub(pSprGO->Position, pSpr->Size);
        }

        MoveTimer = 0.0f;
    }
    else
        MoveTimer += g_dtAsSeconds;

    // State Selection
    switch (CursorState)
    {
        case CURSOR_EMPTY:
        {
            // Test For Intersection
            if (SpaceKeyTimer > SPACE_KEY_ACCEPT_TIME && IsKeyPressed(CG_KEY_SPACE))
            {
                GOID ID = TestCursorIntersection();
                if (ID != CG_INVALID_GOID)
                {
                    SetCursorSelectionMode(ID);

                    SpaceKeyTimer = 0.0f;
                }
            }
            else 
                SpaceKeyTimer += g_dtAsSeconds;
        }
        break;

        case CURSOR_SELECTED:
        {
            if (SelectedID != CG_INVALID_GOID)
            {
                // Move Selected Game Object with Cursor
                Sprite* pSelectedSpr = &g_SharedCompList.DrawingCompList[SelectedID].Spr;
                pSelectedSpr->Position = Vec2_Add(pSpr->Position, CreateVec2(1, 1));

                // Update Rulers
                UpdateRulers(pSelectedSpr->Position, pSelectedSpr->Size);

                if (SpaceKeyTimer > SPACE_KEY_ACCEPT_TIME && IsKeyPressed(CG_KEY_SPACE))
                {
                    DestroyRulers();
                    SetCursorEmptyMode();

                    SpaceKeyTimer = 0.0f;
                }
                else
                    SpaceKeyTimer += g_dtAsSeconds;
            }
        }
        break;
    }

    LastCursorPosition = pSpr->Position;
}

// UPDATECURSOR LOCAL FUNCTIONS
GOID TestCursorIntersection(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    Sprite* pSprGO = NULL;
    for (int i = 0; i < g_SharedCompList.IDList_DrawingComp.CurrentSize; ++i)
    {
        GOID TestID = g_SharedCompList.IDList_DrawingComp.IDList[i];
        if (TestID == DebugCursorID || TestID == DebugMenuID)
            continue;
        
        pSprGO = &g_SharedCompList.DrawingCompList[TestID].Spr;
        Vector2 Pos1 = pSpr->Position;
        Vector2 Size1 = pSpr->Size;
        Vector2 Pos2 = pSprGO->Position;
        Vector2 Size2 = pSprGO->Size;
        
        // Early-Out Intersection Test
        if (Pos1.x > Pos2.x + Size2.x || Pos1.x + Size1.x < Pos2.x || 
            Pos1.y > Pos2.y + Size2.y || Size1.y + Pos1.y < Pos2.y)
            continue;

        return TestID;
    }

    return CG_INVALID_GOID;
}

void QuitCursor(void)
{
    CursorState = CURSOR_EMPTY;
    ActiveFlag = 0;

    free(CursorGOIDList);
    CursorGOIDList = NULL;
    CurrentCursorGOIDListIndex = 0;

    // Destroy Rulers
    // Rulers
    for (int i = 0; i < NUM_OF_RULERS; ++i)
        DestroySprite(&g_SharedCompList.DrawingCompList[RulerGOIDList[i]].Spr);

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    DestroySprite(pSpr);
}

int SetCursorEmptyMode(void)
{
    // Deselect the Game Object
    InitCursor(DebugMenuID);

    // Position the Cursor
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    Sprite* pSelectedSpr = &g_SharedCompList.DrawingCompList[SelectedID].Spr;
    pSpr->Position = Vec2_Sub(pSelectedSpr->Position, pSpr->Size);
    
    // Change States
    CursorState = CURSOR_EMPTY;
    CursorMoveState = PrevCursorMoveState;

    return 1;
}

int SetCursorSelectionMode(GOID TargetID)
{
    if (TargetID != CG_INVALID_GOID)
    {
        // Size: X -> +3 For Spacing and Newline/NULL Character
        // Size: Y -> +2 For Spacing
        GOID ID = TargetID;
        Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
        Sprite* pSprGO = &g_SharedCompList.DrawingCompList[ID].Spr;
        Vector2 Size = CreateVec2(pSprGO->Size.x + 3, pSprGO->Size.y + 2);
        char* CharBuffer = (char*)malloc(sizeof(char) * Size.x * Size.y);
        for (int i = 0; i < Size.y; ++i)
        {
            for (int j = 0; j < Size.x; ++j)
            {
                int Index = (i * Size.x) + j;
                if (j != Size.x - 1)
                    CharBuffer[Index] = 'C';
                else
                    CharBuffer[Index] = '\n';
            }
        }
        CharBuffer[(Size.x * Size.y) - 1] = '\0';

        DestroySprite(pSpr);
        SetSprite(pSpr, CharBuffer, '\n');
        FillSpriteSingleColor(pSpr, C_g);
        
        free(CharBuffer);
        CharBuffer = NULL;
        
        pSpr->Position = Vec2_Sub(pSprGO->Position, CreateVec2(1, 1));
        SelectedID = ID;
        CursorState = CURSOR_SELECTED;
        
        // Cursor Should Move Freely in Selected Mode
        PrevCursorMoveState = CursorMoveState;
        SetCursorMoveFree();

        // Rulers should be there
        CreateRulers();
        
        return 1;
    }

    return 0;
}

int SetCursorMoveList(GOID* List, int Size)
{
    return SetCursorMoveListWithStartingID(List, Size, CG_INVALID_GOID);
}

int SetCursorMoveListWithStartingID(GOID* List, int Size, GOID StartID)
{
    if (CursorState == CURSOR_SELECTED)
    {
        Log_Warn("SetCursorMoveList(): Cannot Change Cursor Move State when Something is Selected.");
        return 0;
    }
    else if (List == NULL)
    {
        Log_Warn("SetCursorMoveList(): List is NULL.");
        return 0;    
    }

    free(CursorGOIDList);
    CursorGOIDList = NULL;

    CursorGOIDList = (GOID*)malloc(sizeof(GOID) * Size);
    CursorGOIDListSize = Size;

    for (int i = 0; i < Size; ++i)
        CursorGOIDList[i] = List[i];

    if (StartID == CG_INVALID_GOID)
        CurrentCursorGOIDListIndex = 0;
    else
    {
        for (int i = 0; i < Size; ++i)
        {
            if (List[i] == StartID)
            {
                CurrentCursorGOIDListIndex = i;
                break;
            }
        }
    }
    CursorMoveState = CURSOR_MOVE_GOID_LIST;
    
    Sprite* pSprGO = &g_SharedCompList.DrawingCompList[CursorGOIDList[CurrentCursorGOIDListIndex]].Spr;
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugCursorID].Spr;
    pSpr->Position = Vec2_Sub(pSprGO->Position, pSpr->Size);
    
    return 1;
}

GOID GetCursorMoveListCurrentID(void)
{
    if (CursorGOIDList == NULL)
        return CG_INVALID_GOID;
    
    return CursorGOIDList[CurrentCursorGOIDListIndex];
}

int SetCursorMoveFree()
{
    CursorMoveState = CURSOR_MOVE_FREE;
}

// RULER FUNCTIONS
void CreateRulers(void)
{
    static char RulerCharacters[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', '\0'
    };
    static int HorzRulerLen = ARR_LEN(RulerCharacters) * 2; // Add - Before Every Character For Increased Scale
    static int VertRulerLen = ARR_LEN(RulerCharacters) * 4; // Add - and NewLine Character Before Every Character For Increased Scale

    char RightRulerBuffer[HorzRulerLen];
    char LeftRulerBuffer[HorzRulerLen];
    char UpRulerBuffer[VertRulerLen];
    char DownRulerBuffer[VertRulerLen];

    // Generate Right Sprite
    int j = 0;
    for (int i = 0; i < HorzRulerLen; ++i)
    {
        if (i % 2 == 0)
            RightRulerBuffer[i] = RulerCharacters[j++];
        else
            RightRulerBuffer[i] = '-';
    }
    RightRulerBuffer[HorzRulerLen - 2] = '-';
    RightRulerBuffer[HorzRulerLen - 1] = '\0';
    // Generate Left Sprite
    j = 0;
    for (int i = HorzRulerLen - 2; i > 0; --i)
    {
        if (i % 2 != 0)
            LeftRulerBuffer[i] = RulerCharacters[j++];
        else
            LeftRulerBuffer[i] = '-';
    }
    LeftRulerBuffer[0] = '-';
    LeftRulerBuffer[HorzRulerLen - 1] = '\0';
    // Generate Down Sprite
    j = 0;
    int count = 0;
    for (int i = 0; i < VertRulerLen - 1; i += 2)
    {
        if (count % 2 == 0)
            DownRulerBuffer[i] = RulerCharacters[j++];
        else
            DownRulerBuffer[i] = '-';

        ++count;
        DownRulerBuffer[i + 1] = '\n';
    }
    DownRulerBuffer[VertRulerLen - 1] = '\0';
    // Generate Up Sprite
    j = 0;
    count = 0;
    for (int i = VertRulerLen - 3; i > 0; i -= 2)
    {
        if (count % 2 != 0)
            UpRulerBuffer[i] = RulerCharacters[j++];
        else
            UpRulerBuffer[i] = '-';

        ++count;
        UpRulerBuffer[i - 1] = '\n';
    }
    UpRulerBuffer[VertRulerLen - 1] = UpRulerBuffer[VertRulerLen - 2] = '\0';

    // Set Sprites
    Sprite* Spr_Up = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_UP_INDEX]].Spr;
    Sprite* Spr_Down = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_DOWN_INDEX]].Spr;
    Sprite* Spr_Left = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_LEFT_INDEX]].Spr;
    Sprite* Spr_Right = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_RIGHT_INDEX]].Spr;

    SetSprite(Spr_Up, UpRulerBuffer, '\n');
    SetSprite(Spr_Down, DownRulerBuffer, '\n');
    SetSprite(Spr_Left, LeftRulerBuffer, '\n');
    SetSprite(Spr_Right, RightRulerBuffer, '\n');

    FillSpriteSingleColor(Spr_Up, C_dy);
    FillSpriteSingleColor(Spr_Down, C_dy);
    FillSpriteSingleColor(Spr_Left, C_dy);
    FillSpriteSingleColor(Spr_Right, C_dy);

    Spr_Up->Position = Spr_Down->Position = Spr_Left->Position = Spr_Right->Position = CreateVec2(-1000, -1000);
}

void UpdateRulers(Vector2 TargetPosition, Vector2 TargetSize)
{
    Sprite* Spr_Up = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_UP_INDEX]].Spr;
    Sprite* Spr_Down = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_DOWN_INDEX]].Spr;
    Sprite* Spr_Left = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_LEFT_INDEX]].Spr;
    Sprite* Spr_Right = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_RIGHT_INDEX]].Spr;

    Spr_Up->Position = CreateVec2(      TargetPosition.x + (int)((float)TargetSize.x / 2.0f), 
                                        TargetPosition.y - Spr_Up->Size.y);
    
    Spr_Down->Position = CreateVec2(    TargetPosition.x + (int)((float)TargetSize.x / 2.0f), 
                                        TargetPosition.y + TargetSize.y);
    
    Spr_Left->Position = CreateVec2(    TargetPosition.x - Spr_Left->Size.x, 
                                        TargetPosition.y + (int)((float)TargetSize.y / 2.0f));
    
    Spr_Right->Position = CreateVec2(   TargetPosition.x + TargetSize.x, 
                                        TargetPosition.y + (int)((float)TargetSize.y / 2.0f));
}

void DestroyRulers(void)
{
    // Destroy Sprites
    Sprite* Spr_Up = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_UP_INDEX]].Spr;
    Sprite* Spr_Down = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_DOWN_INDEX]].Spr;
    Sprite* Spr_Left = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_LEFT_INDEX]].Spr;
    Sprite* Spr_Right = &g_SharedCompList.DrawingCompList[RulerGOIDList[RULER_RIGHT_INDEX]].Spr;

    DestroySprite(Spr_Up);
    DestroySprite(Spr_Down);
    DestroySprite(Spr_Left);
    DestroySprite(Spr_Right);
}