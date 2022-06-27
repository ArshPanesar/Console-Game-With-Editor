#include "DebugEditor.h"

// Allocation
static int DebugDataAllocatedFlag = 0;

// Editing Mode
static Vector2 OG_Size;
static Vector2 TerminalSizeOffset;
static GOID DebugMenuID;

static SharedGameWorldData SharedData;

enum eMenu
{
    MENU_MAIN = 0,
    MENU_SPAWN,
    MENU_SHOOT_EDIT,
    MENU_SHOOT_VAR_EDIT,
    MENU_PATH_EDIT,
    MENU_PATH_VAR_EDIT,
    MENU_HORDE
};

static enum eMenu CurrentMenu;

static const float KEY_ACCEPT_TIME = 0.2f;
static float KeyTimer = 0.0f;

/**** FUNCTION DECLARATIONS ****/
int ValidateSharedData(SharedGameWorldData* pSharedData);

// Menu Switcher : Takes Care of Initializing and Quiting New and Old Menus Respectively
int SwitchMenu(enum eMenu NewMenu);

// Main Menu
int InitMainMenu(void);
int UpdateMainMenu(void);
int QuitMainMenu(void);

// Spawn Menu
int InitSpawnMenu(void);
int UpdateSpawnMenu(void);
int QuitSpawnMenu(void);

// Shoot Edit Menu
int InitShootEditMenu(void);
int UpdateShootEditMenu(void);
int QuitShootEditMenu(void);

// Shoot Variable Edit Menu
int InitShootVarEditMenu(void);
int UpdateShootVarEditMenu(void);
int QuitShootVarEditMenu(void);

// Path Edit Menu
int InitPathEditMenu(void);
int UpdatePathEditMenu(void);
int QuitPathEditMenu(void);

// Path Variable Edit Menu
int InitPathVarEditMenu(void);
int UpdatePathVarEditMenu(void);
int QuitPathVarEditMenu(void);

// Path Edit Menu
int InitHordeMenu(void);
int UpdateHordeMenu(void);
int QuitHordeMenu(void);

// Variable Sub-Menu
#define MAX_VAR_SUB_MENUS 2
static int NumOfVarSubMenus = 0;

typedef struct VarSubMenuStruct
{
    FileManager FM;
    VarSection VS;
    char SectionList[VF_MAX_SECTIONS][VF_MAX_SECTION_LEN];

    char* VarFile;
    char* VarName;
    char* Section;
    int NumOfEntriesFilled;

    GOID ID;
    int MenuIndex;

    short Initialized;
    short SectionSelectionMenu;

    union Data
    {
        Vector2* pVec2;
        float* pfloat;
    };

    // Input For VarMenu will Only Be Accepted When this Key is Held
#ifdef CG_PLATFORM_WINDOWS
    SHORT HoldKey;
#endif
} VarSubMenu;

static VarSubMenu VarSubMenuList[MAX_VAR_SUB_MENUS];

int InitVarSubMenu(VarSubMenu* VM, char* VarName, enum eVarSectionValueType ValType);
int SetVarSubMenu(VarSubMenu* VM, char* Section);
int UpdateVarSubMenu(VarSubMenu* VM);
int QuitVarSubMenu(VarSubMenu* VM);

int AllocateEditingData(SpawnRecord* pSpawnRecord)
{
    if (DebugDataAllocatedFlag)
    {
        Log_Warn("AllocateEditingData(): Function Called But Data Already Allocated.")
        return 0;
    }

    AllocateDebugCursor(pSpawnRecord);

    DebugDataAllocatedFlag = 1;
    return 1;
}

int DeallocateEditingData(SpawnRecord* pSpawnRecord)
{
    if (!DebugDataAllocatedFlag)
    {
        Log_Warn("DeallocateEditingData(): Function Called But Data NOT Allocated.")
        return 0;
    }

    DeallocateDebugCursor(pSpawnRecord);
    
    DebugDataAllocatedFlag = 0;
    return 1;
}

int InitEditingMode(SharedGameWorldData* pSD)
{
    if (!ValidateSharedData(pSD))
    {
        Log_Warn("InitEditingMode(): SharedData Invalid. Debug Menu Not Started.");
        return 0;
    }

    SharedData = *pSD;
    
    Log_Info("InitEditingMode(): Debug Session Started");
    
    TerminalSizeOffset = (Vector2) {0, 8};  // Debug Menu Size

    OG_Size = GetTerminalSize();
    Vector2 NewSize = Vec2_Add(OG_Size, TerminalSizeOffset);

    SetTerminalSize(NewSize);


    DebugMenuID = CreateGameObj();
    AttachComponent(COMP_DRAWING, DebugMenuID);

    InitMainMenu();
    CurrentMenu = MENU_MAIN;

    InitCursor(DebugMenuID);

    //SetCursorMoveList(g_SharedCompList.IDList_InfoComp.IDList, g_SharedCompList.IDList_InfoComp.CurrentSize);

    // Despawn All Bullets
    GOIDList BulletList;
    BulletList.CurrentSize = 0;
    for (int i = 0; i < g_SharedCompList.IDList_InfoComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_InfoComp.IDList[i];
        if (g_SharedCompList.InfoCompList[ID].Tag == TAG_BULLET)
            BulletList.IDList[BulletList.CurrentSize++] = ID;
    }
    if (BulletList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, BulletList.IDList, BulletList.CurrentSize);

    // Initialize Variable Editing Menus
    for (int i = 0; i < MAX_VAR_SUB_MENUS; ++i)
    {
        VarSubMenuList[i].Initialized = 0;
        VarSubMenuList[i].MenuIndex = i + 1;
        VarSubMenuList[i].VarFile = "varfile";
    }
    VarSubMenuList[0].HoldKey = CG_KEY_LEFT;
    VarSubMenuList[1].HoldKey = CG_KEY_RIGHT;

    //InitVarSubMenu(&VarSubMenuList[0], "WaitTime", VAR_SECTION_VALUE_TYPE_FLOAT);
    //InitVarSubMenu(&VarSubMenuList[1], "Speed", VAR_SECTION_VALUE_TYPE_VEC2);

    SharedData.pHordeMgr->DebugFlag = 1;
    return 1;
}

void UpdateEditingMode(void)
{
    switch(CurrentMenu)
    {
        case MENU_MAIN:
            UpdateMainMenu();
            break;

        case MENU_SPAWN:
            UpdateSpawnMenu();
            break;
        
        case MENU_SHOOT_EDIT:
            UpdateShootEditMenu();
            break;
        
        case MENU_SHOOT_VAR_EDIT:
            UpdateShootVarEditMenu();
            break;
    
        case MENU_PATH_EDIT:
            UpdatePathEditMenu();
            break;

        case MENU_PATH_VAR_EDIT:
            UpdatePathVarEditMenu();
            break;

        case MENU_HORDE:
            UpdateHordeMenu();
            break;
    }
    
    for (int i = 0; i < NumOfVarSubMenus; ++i)
        UpdateVarSubMenu(&VarSubMenuList[i]);

    UpdateCursor();
}

void QuitEditingMode(void)
{
    // Switch to Main Menu First
    SwitchMenu(MENU_MAIN);

    // Quit Variable Sub Menus
    for (int i = 0; i < MAX_VAR_SUB_MENUS; ++i)
    {
        if (VarSubMenuList[i].Initialized)
            QuitVarSubMenu(&VarSubMenuList[i]);
    }
    // Destroy Debug Menu
    DestroySprite(&g_SharedCompList.DrawingCompList[DebugMenuID].Spr);

    DetachComponent(COMP_DRAWING, DebugMenuID);
    DestroyGameObj(DebugMenuID);
    DebugMenuID = CG_INVALID_GOID;

    // Reset the Terminal Size
    SetTerminalSize(OG_Size);

    // Destroy Any Debug Game Objects
    GOIDList DebugGOIDList;
    DebugGOIDList.CurrentSize = 0;
    for (int i = 0; i < g_SharedCompList.IDList_InfoComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_InfoComp.IDList[i];
        if (ID != DebugMenuID && ID != DebugCursorID && g_SharedCompList.InfoCompList[ID].DebugFlag)
            DebugGOIDList.IDList[DebugGOIDList.CurrentSize++] = ID;
    }
    if (DebugGOIDList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, DebugGOIDList.IDList, DebugGOIDList.CurrentSize);

    QuitCursor();
    
    SharedData.pHordeMgr->DebugFlag = 0;
    Log_Info("QuitEditingMode(): Debug Session Ended");
}

/**** FUNCTION DEFINITIONS ****/

int ValidateSharedData(SharedGameWorldData* pSharedData)
{
    if (pSharedData == NULL)
    {
        Log_Critical("ValidateSharedData(): SharedData Pointer is NULL");
        return 0;
    }
    
    if (pSharedData->pSpawnRecord == NULL)
    {
        Log_Critical("ValidateSharedData(): pSpawnRecord Pointer is NULL");
        return 0;
    }

    if (pSharedData->pRunPathSystemFlag == NULL || pSharedData->pRunShootSystemFlag == NULL)
    {
        Log_Critical("ValidateSharedData(): pRunPathSystemFlag or pRunShootSystemFlag Pointer is NULL");
        return 0;
    }

    return 1;
}

// MENU SWITCHER
int SwitchMenu(enum eMenu NewMenu)
{
    switch (CurrentMenu)
    {
        case MENU_MAIN:
        {
            QuitMainMenu();

            if (NewMenu == MENU_SPAWN)
                InitSpawnMenu();
            else if (NewMenu == MENU_SHOOT_EDIT)
                InitShootEditMenu();
            else if (NewMenu == MENU_PATH_EDIT)
                InitPathEditMenu();
            else if (NewMenu == MENU_HORDE)
                InitHordeMenu();
        }
        break;

        case MENU_SPAWN:
        {
            QuitSpawnMenu();
            InitMainMenu();
        }
        break;
        
        case MENU_SHOOT_EDIT:
        {
            QuitShootEditMenu();
            if (NewMenu == MENU_MAIN)
                InitMainMenu();
            else if (NewMenu == MENU_SHOOT_VAR_EDIT)
                InitShootVarEditMenu();
        }
        break;

        case MENU_SHOOT_VAR_EDIT:
        {
            QuitShootVarEditMenu();
            InitShootEditMenu();
        }
        break;

        case MENU_PATH_EDIT:
        {
            QuitPathEditMenu();
            if (NewMenu == MENU_MAIN)
                InitMainMenu();
            else if (NewMenu == MENU_PATH_VAR_EDIT)
                InitPathVarEditMenu();
        }
        break;

        case MENU_PATH_VAR_EDIT:
        {
            QuitPathVarEditMenu();
            InitPathEditMenu();
        }
        break;

        case MENU_HORDE:
        {
            QuitHordeMenu();
            InitMainMenu();
        }
        break;
    }

    CurrentMenu = NewMenu;

    return 1;
}

// MAIN MENU FUNCTIONS
int InitMainMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "OPTIONS:\n"\
                            "1      Spawn/Despawn Game Objects\n"\
                            "2      Edit Shoot Patterns\n"\
                            "3      Edit Paths\n"\
                            "4      Edit Enemy Horde Files\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 1);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);
    
    SetCursorMoveFree();
    SetCursorEmptyMode();

    Log_Info("InitMainMenu(): Initializing Main Menu");
}

int UpdateMainMenu(void)
{
    // Input
    if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_1)) // Switch to Spawn Menu 
        {
            SwitchMenu(MENU_SPAWN);
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2)) // Switch to Shoot Edit Menu 
        {
            SwitchMenu(MENU_SHOOT_EDIT);
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_3)) // Switch to Path Edit Menu 
        {
            SwitchMenu(MENU_PATH_EDIT);
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_4)) // Switch to Horde Menu 
        {
            SwitchMenu(MENU_HORDE);
            KeyTimer = 0.0f;
        }
    }
    else
        KeyTimer += g_dtAsSeconds;
}

int QuitMainMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    Log_Info("QuitMainMenu(): Quiting Main Menu");
}

int InitSpawnMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "SPAWN OPTIONS:\n"\
                            "1      Spawn Player\n"\
                            "2      Spawn Enemy\n"\
                            "3      Despawn\n"\
                            "0      Back to Main Menu\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 1);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    Log_Info("InitSpawnMenu(): Initializing Spawn Menu");
}

static enum eSpawnMenuOption
{
    OPTION_SPAWN_MENU_NONE,
    OPTION_SPAWN_MENU_DESPAWN,
    OPTION_SPAWN_MENU_QUIT
} SpawnMenuOption = OPTION_SPAWN_MENU_NONE;

static enum eSpawnMenuState
{
    STATE_SPAWN_MENU_NONE,
    STATE_SPAWN_MENU_DESPAWN_INIT,
    STATE_SPAWN_MENU_DESPAWN
} SpawnMenuState = STATE_SPAWN_MENU_NONE;

int UpdateSpawnMenu(void)
{
    // Input
    SpawnMenuOption = OPTION_SPAWN_MENU_NONE;
    if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_0))
        {
            SpawnMenuOption = OPTION_SPAWN_MENU_QUIT;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_1))    // Spawn Player
        {
            GOID ID = *SharedData.pPlayerID;
            if (!SharedData.pSpawnRecord->IsSpawned[ID])
                SpawnExt(SharedData.pSpawnRecord, TAG_PLAYER, 1, &ID);
            
            *SharedData.pPlayerID = ID;
            SpawnMenuState = STATE_SPAWN_MENU_NONE;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2))    // Despawn
        {
            GOID ID;
            SpawnExt(SharedData.pSpawnRecord, TAG_ENEMY, 1, &ID);
            
            SpawnMenuState = STATE_SPAWN_MENU_NONE;
            KeyTimer = 0.0f;
            
        }
        else if (IsKeyPressed(CG_KEY_3))
        {  
            SpawnMenuOption = OPTION_SPAWN_MENU_DESPAWN;
            KeyTimer = 0.0f;
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    // State Change
    if (SpawnMenuOption == OPTION_SPAWN_MENU_DESPAWN && SpawnMenuState == STATE_SPAWN_MENU_NONE)
        SpawnMenuState = STATE_SPAWN_MENU_DESPAWN_INIT;
    else if (SpawnMenuOption == OPTION_SPAWN_MENU_QUIT && SpawnMenuState == STATE_SPAWN_MENU_NONE)
        SwitchMenu(MENU_MAIN);
    
    // State Behaviour
    if (SpawnMenuState == STATE_SPAWN_MENU_DESPAWN_INIT)
    {
        // Getting IDs List
        GOIDList List;
        List.CurrentSize = 0;
        for (int i = 0; i < g_SharedCompList.IDList_InfoComp.CurrentSize; ++i)
        {
            GOID ID = g_SharedCompList.IDList_InfoComp.IDList[i];
            if (ID != DebugCursorID && ID != DebugMenuID)
                List.IDList[List.CurrentSize++] = ID;
        }

        // Nothing to Despawn
        if (List.CurrentSize <= 0)
        {
            SpawnMenuState = STATE_SPAWN_MENU_NONE;
            return 0;
        }

        // Setting Cursor Mode
        SetCursorEmptyMode();
        SetCursorMoveList(List.IDList, List.CurrentSize);

        SpawnMenuState = STATE_SPAWN_MENU_DESPAWN;
    }
    else if (SpawnMenuState == STATE_SPAWN_MENU_DESPAWN)
    {
        GOID ID = GetCursorMoveListCurrentID();
        if (SpawnMenuOption == OPTION_SPAWN_MENU_DESPAWN && ID != CG_INVALID_GOID)
        {
            // Despawn
            Despawn(SharedData.pSpawnRecord, ID);
            SpawnMenuState = STATE_SPAWN_MENU_DESPAWN_INIT;
        }
        else if (SpawnMenuOption == OPTION_SPAWN_MENU_QUIT)
        {
            SetCursorEmptyMode();
            SetCursorMoveFree();
            SpawnMenuState = STATE_SPAWN_MENU_NONE;
        }
    }
}

int QuitSpawnMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);

    SetCursorEmptyMode();
    SetCursorMoveFree();
    SpawnMenuState = STATE_SPAWN_MENU_NONE;
    Log_Info("QuitSpawnMenu(): Quiting Spawn Menu");
}

/* SHOOT EDIT MENU */
struct ShootEditDataStruct
{
    enum eShootEditOptions
    {
        OPTION_SHOOT_EDIT_NONE,
        OPTION_SHOOT_EDIT_ADD,
        OPTION_SHOOT_EDIT_REMOVE,
        OPTION_SHOOT_EDIT_SET_VARS,
        OPTION_SHOOT_EDIT_QUIT
    } Option;

    enum eShootEditState
    {
        STATE_SHOOT_EDIT_NONE,
        STATE_SHOOT_EDIT_REINIT,
        STATE_SHOOT_EDIT_ADDING_BULLET_INIT,
        STATE_SHOOT_EDIT_ADDING_BULLET,
        STATE_SHOOT_EDIT_REMOVING_BULLET_INIT,
        STATE_SHOOT_EDIT_REMOVING_BULLET
    } State;

    GOIDList DebugGOIDList;

    GOID CurrentID;
};

static struct ShootEditDataStruct ShootEditData;

int InitShootEditMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "SHOOT EDIT OPTIONS:\n"\
                            "1  Add    Bullet\n"\
                            "2  Remove Bullet\n"\
                            "3  Set Variables\n"\
                            "0  Back to Main Menu\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 2);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    // Set Data
    ShootEditData.State = STATE_SHOOT_EDIT_REINIT;
    ShootEditData.Option = OPTION_SHOOT_EDIT_NONE;
    ShootEditData.DebugGOIDList.CurrentSize = 0;
    
    // List of GOIDs Containing Shoot Component
    SetCursorMoveList(g_SharedCompList.IDList_ShootComp.IDList, g_SharedCompList.IDList_ShootComp.CurrentSize);
    ShootEditData.CurrentID = GetCursorMoveListCurrentID();

    Log_Info("InitShootEditMenu(): Initializing Shoot Editing Menu");
    return 1;
}

int UpdateShootEditMenu(void)
{
    // Local Functions
    int ShootEditFunc_Reinit(void);

    // Reinitialize Debug Bullets
    if (ShootEditData.State == STATE_SHOOT_EDIT_REINIT)
    {
        ShootEditFunc_Reinit();
        ShootEditData.State = STATE_SHOOT_EDIT_NONE;
    }

    // Input
    ShootEditData.Option = OPTION_SHOOT_EDIT_NONE;
    if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_0)) // Switch to Main Menu
        {
            ShootEditData.Option = OPTION_SHOOT_EDIT_QUIT;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_1))
        {
            ShootEditData.Option = OPTION_SHOOT_EDIT_ADD;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2))
        {
            ShootEditData.Option = OPTION_SHOOT_EDIT_REMOVE;
            KeyTimer = 0.0f;    
        }
        else if (IsKeyPressed(CG_KEY_3))
        {
            ShootEditData.Option = OPTION_SHOOT_EDIT_SET_VARS;
            KeyTimer = 0.0f;    
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    // Change States
    if (ShootEditData.Option == OPTION_SHOOT_EDIT_ADD && ShootEditData.State == STATE_SHOOT_EDIT_NONE)
        ShootEditData.State = STATE_SHOOT_EDIT_ADDING_BULLET_INIT;
    else if (ShootEditData.Option == OPTION_SHOOT_EDIT_REMOVE && ShootEditData.State == STATE_SHOOT_EDIT_NONE)
        ShootEditData.State = STATE_SHOOT_EDIT_REMOVING_BULLET_INIT; 
    else if (ShootEditData.Option == OPTION_SHOOT_EDIT_QUIT && ShootEditData.State == STATE_SHOOT_EDIT_NONE)
        SwitchMenu(MENU_MAIN);
    else if (ShootEditData.Option == OPTION_SHOOT_EDIT_SET_VARS && ShootEditData.State == STATE_SHOOT_EDIT_NONE)
    {
        GOID ID = GetCursorMoveListCurrentID();
        if (ID != CG_INVALID_GOID)
        {
            ShootEditData.CurrentID = ID;
            SwitchMenu(MENU_SHOOT_VAR_EDIT);
        }
        else
        {
            Log_Warn("UpdateShootEditMenu(): When Switching to Variable Editing -> GOID is Invalid. ID=%d, State=%d", ID, ShootEditData.State);
        }
    }
    // State Behaviour
    if (ShootEditData.State == STATE_SHOOT_EDIT_ADDING_BULLET_INIT)
    {
        GOID ID = GetCursorMoveListCurrentID();
        if (ID != CG_INVALID_GOID)
        {
            ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
            if (AddBulletToShootPattern(SC) < 0)
            {
                ShootEditData.State = STATE_SHOOT_EDIT_NONE;
                return 0;
            }

            // Getting Available Directions
            Vector2 Dirs[BULLET_DIRS_TABLE_SIZE];
            int Size = 0;
            for (int i = 0; i < BULLET_DIRS_TABLE_SIZE; ++i)
                Dirs[Size++] = BULLET_DIRS_TABLE[i];
            /*for (int i = 0; i < MAX_NUM_OF_BULLETS; ++i)
            {
                int IgnoreFlag = 0;
                for (int j = 0; j < SC->NumOfBullets; ++j)
                {
                    if (BULLET_DIRS_TABLE[i].x == SC->pDirList[j].x && BULLET_DIRS_TABLE[i].y == SC->pDirList[j].y)
                    {
                        IgnoreFlag = 1;
                        break;
                    }
                }

                if (!IgnoreFlag)
                    Dirs[Size++] = BULLET_DIRS_TABLE[i];
            }*/
            
            // Spawn Dummy Bullets (No Sprite Just Need For Positions)
            GOID IDList[Size + 1];
            SpawnExt(SharedData.pSpawnRecord, TAG_BULLET, Size + 1, IDList);
            for (int i = 0; i < Size + 1; ++i) 
                ShootEditData.DebugGOIDList.IDList[ShootEditData.DebugGOIDList.CurrentSize++] = IDList[i];

            // Set Position Offsets
            Vector2 PositionOffsets[BULLET_DIRS_TABLE_SIZE];
            Sprite BulletSpr;
            BulletSpr.CharBuffer = NULL;
            BulletSpr.ColorBuffer = NULL;
            CreateBulletSpriteFromType(&BulletSpr, 0);
            g_SharedCompList.DrawingCompList[IDList[Size]].Spr = BulletSpr;

            Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
            GenerateBulletPositionOffsets(PositionOffsets, pSpr->Size, BulletSpr.Size, Dirs, Size);

            // Set Sprite
            for (int i = 0; i < Size; ++i) 
            {
                Sprite* pBSpr = &g_SharedCompList.DrawingCompList[IDList[i]].Spr;
                pBSpr->Position = Vec2_Add(pSpr->Position, PositionOffsets[i]);
                pBSpr->Size = CreateVec2(0, 0);

                // Set Direction
                g_SharedCompList.MovementCompList[IDList[i]].Direction = Dirs[i];

                // Set as Debug
                g_SharedCompList.InfoCompList[IDList[i]].DebugFlag = 1;
            }

            // Set Cursor to Move Along Available Directions
            SetCursorMoveList(IDList, Size);
            ShootEditData.CurrentID = ID;
            g_SharedCompList.DrawingCompList[IDList[Size]].Spr.Position = g_SharedCompList.DrawingCompList[IDList[0]].Spr.Position;
            
            ShootEditData.State = STATE_SHOOT_EDIT_ADDING_BULLET;
        }
        else
        {
            Log_Warn("UpdateShootEditMenu(): Cursor Current ID is Invalid. ID=%d, State=%d", ID, ShootEditData.State);
        }

    }
    else if (ShootEditData.State == STATE_SHOOT_EDIT_ADDING_BULLET)
    {
        GOID ID = GetCursorMoveListCurrentID();
        if (ID != CG_INVALID_GOID)
        {
            ShootComponent* SC = &g_SharedCompList.ShootCompList[ShootEditData.CurrentID];
            
            GOID BulletID = ShootEditData.DebugGOIDList.IDList[ShootEditData.DebugGOIDList.CurrentSize - 1];
            g_SharedCompList.DrawingCompList[BulletID].Spr.Position = g_SharedCompList.DrawingCompList[ID].Spr.Position;
            SetBulletDirection(SC, SC->NumOfBullets - 1, g_SharedCompList.MovementCompList[ID].Direction);
            SetBulletSpeed(SC, SC->NumOfBullets - 1, NORM_ENEMY_BULLET_SPEED);

            if (ShootEditData.Option == OPTION_SHOOT_EDIT_QUIT || ShootEditData.Option == OPTION_SHOOT_EDIT_ADD)
            {
                // Dont Add a Bullet
                if (ShootEditData.Option == OPTION_SHOOT_EDIT_QUIT)
                    RemoveBulletFromShootPattern(SC, SC->NumOfBullets - 1);
                   
                SetCursorMoveListWithStartingID(g_SharedCompList.IDList_ShootComp.IDList, g_SharedCompList.IDList_ShootComp.CurrentSize, ShootEditData.CurrentID);
                ShootEditData.State = STATE_SHOOT_EDIT_REINIT;
            }
        }
    }
    else if (ShootEditData.State == STATE_SHOOT_EDIT_REMOVING_BULLET_INIT)
    {
        // Copy List of Bullets of Current Entity
        GOID ID = GetCursorMoveListCurrentID();
        if (ID != CG_INVALID_GOID)
        {
            ShootComponent* SC = &g_SharedCompList.ShootCompList[ID];
            if (SC->NumOfBullets > 0)
            {
                SetCursorMoveList(SC->pBulletIDList, SC->NumOfBullets);
                ShootEditData.CurrentID = ID;
                ShootEditData.State = STATE_SHOOT_EDIT_REMOVING_BULLET;
            }
            else
                ShootEditData.State = STATE_SHOOT_EDIT_NONE;
        }
        else
        {
            Log_Warn("UpdateShootEditMenu(): Cursor Current ID is Invalid. ID=%d, State=%d", ID, ShootEditData.State);
        }
    }
    else if (ShootEditData.State == STATE_SHOOT_EDIT_REMOVING_BULLET)
    {
        // Dont Remove
        if (ShootEditData.Option == OPTION_SHOOT_EDIT_QUIT && ShootEditData.CurrentID != CG_INVALID_GOID)
        {
            // Reset Cursor List
            SetCursorMoveListWithStartingID(g_SharedCompList.IDList_ShootComp.IDList, g_SharedCompList.IDList_ShootComp.CurrentSize, ShootEditData.CurrentID);
            ShootEditData.State = STATE_SHOOT_EDIT_REINIT;
        }
        // Check if Remove Button was Pressed
        else if (ShootEditData.Option == OPTION_SHOOT_EDIT_REMOVE && ShootEditData.CurrentID != CG_INVALID_GOID)
        {
            GOID RemoveBulletID = GetCursorMoveListCurrentID();
            ShootComponent* SC = &g_SharedCompList.ShootCompList[ShootEditData.CurrentID];
            int Index = -1;
            for (int i = 0; i < SC->NumOfBullets; ++i)
            {
                if (RemoveBulletID == SC->pBulletIDList[i])
                {
                    Index = i;
                    break;
                }
            }

            RemoveBulletFromShootPattern(SC, Index);

            // Reset Cursor List
            SetCursorMoveListWithStartingID(g_SharedCompList.IDList_ShootComp.IDList, g_SharedCompList.IDList_ShootComp.CurrentSize, ShootEditData.CurrentID);
            ShootEditData.State = STATE_SHOOT_EDIT_REINIT;
        }
    }

    return 1;
}


int ShootEditFunc_Reinit(void)
{
    // Despawn Previously Spawned Bullets
    if (ShootEditData.DebugGOIDList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, ShootEditData.DebugGOIDList.IDList, ShootEditData.DebugGOIDList.CurrentSize);
    
    ShootEditData.DebugGOIDList.CurrentSize = 0;
    for (int i = 0; i < g_SharedCompList.IDList_ShootComp.CurrentSize; ++i)
    {
        ShootComponent* SC = &g_SharedCompList.ShootCompList[g_SharedCompList.IDList_ShootComp.IDList[i]];
        if (SC->NumOfBullets > 0)
        {
            //Log_Info("Number of Bullets: %d", SC->NumOfBullets);
            // Local List
            GOIDList BulletList;
            BulletList.CurrentSize = SC->NumOfBullets;

            // Preparing Position Offsets
            Sprite BulletSpr;
            BulletSpr.CharBuffer = NULL;
            BulletSpr.ColorBuffer = NULL;
            CreateBulletSpriteFromType(&BulletSpr, 0);
            Sprite* pSpr = &g_SharedCompList.DrawingCompList[g_SharedCompList.IDList_ShootComp.IDList[i]].Spr;
            SetShootPositionOffsets(SC, pSpr->Size, BulletSpr.Size);

            // Spawn Bullets
            SpawnExt(SharedData.pSpawnRecord, TAG_BULLET, SC->NumOfBullets, BulletList.IDList);
            for (int j = 0; j < SC->NumOfBullets; ++j)
            {
                // Set Debug Flag for All Bullets
                g_SharedCompList.InfoCompList[BulletList.IDList[j]].DebugFlag = 1;
                // Setting Positions
                Sprite* pBSpr = &g_SharedCompList.DrawingCompList[BulletList.IDList[j]].Spr;
                pBSpr->CharBuffer = NULL;
                pBSpr->ColorBuffer = NULL;
                CreateBulletSpriteFromType(pBSpr, 0);
                pBSpr->Position = Vec2_Add(pSpr->Position, SC->pPositionOffsetList[j]);
                // Add the Bullets to Lists
                ShootEditData.DebugGOIDList.IDList[ShootEditData.DebugGOIDList.CurrentSize++] = BulletList.IDList[j];
                SC->pBulletIDList[j] = BulletList.IDList[j];

                //Log_Info("Bullet %d Position: %d, %d", (j + 1), pBSpr->Position.x, pBSpr->Position.y);
                //Log_Info("Bullet %d Size: %d, %d", (j + 1), pBSpr->Size.x, pBSpr->Size.y);
            }
        }
    }

    return 1;
}

int QuitShootEditMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    // Despawn any debug stuff
    if (ShootEditData.DebugGOIDList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, ShootEditData.DebugGOIDList.IDList, ShootEditData.DebugGOIDList.CurrentSize);

    Log_Info("QuitShootEditMenu(): Quiting Shoot Editing Menu");
    return 1;
}

int InitShootVarEditMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "SHOOT VARIABLE EDIT OPTIONS:\n"\
                            "1 Set Speed and Cooldown Time\n"\
                            "2 Set Patterned Shooting\n"\
                            "3 Set Conditional Shooting\n"\
                            "0 Back to Shoot Edit Menu\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 2);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    SetCursorMoveFree();

    // Run Shoot System
    *SharedData.pRunShootSystemFlag = 1;
    for (int i = 0; i < g_SharedCompList.IDList_ShootComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_ShootComp.IDList[i];
        g_SharedCompList.ShootCompList[ID].ShouldShoot = 0;
    }
    g_SharedCompList.ShootCompList[ShootEditData.CurrentID].ShouldShoot = 1;

    // Resetting Movement
    for (int i = 0; i < g_SharedCompList.IDList_MovementComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_MovementComp.IDList[i];    
        g_SharedCompList.MovementCompList[ID].Direction = CreateVec2(0, 0);
    }

    Log_Info("InitShootVarEditMenu(): Initializing Shoot Variables Editing Menu");
    return 1;
}

int UpdateShootVarEditMenu(void)
{
    static float KeyTimer = 0.0f;
    static Vector2 CondShooterFuncEnumVec = (Vector2){ 0, 0 };
    static int HasConditionalShooterComp = 0;

    // Copy Edited Data
    ShootComponent* SC = &g_SharedCompList.ShootCompList[ShootEditData.CurrentID];
    for (int i = 1; i < SC->NumOfBullets; ++i)
        SC->pSpeedList[i] = SC->pSpeedList[0];
    
    if (HasConditionalShooterComp && CondShooterFuncEnumVec.x >= 0 && CondShooterFuncEnumVec.x < (int)NUM_OF_COND_SHOOTER_FUNCS)
    {
        g_SharedCompList.CondShooterCompList[ShootEditData.CurrentID].FuncEnum = (CondShooterFuncEnumVec.x);
    }

    if (IsKeyPressed(VarSubMenuList[0].HoldKey) || IsKeyPressed(VarSubMenuList[1].HoldKey))
        return 0;
    
    if (IsKeyPressed(CG_KEY_0) && KeyTimer > KEY_ACCEPT_TIME)
    {
        if (VarSubMenuList[0].Initialized)
            QuitVarSubMenu(&VarSubMenuList[0]);
        if (VarSubMenuList[1].Initialized)
            QuitVarSubMenu(&VarSubMenuList[1]);
        SwitchMenu(MENU_SHOOT_EDIT);
        
        KeyTimer = 0.0f;
    }
    else if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_1) && !VarSubMenuList[0].Initialized && !VarSubMenuList[1].Initialized)
        {
            InitVarSubMenu(&VarSubMenuList[0], "Speed", VAR_SECTION_VALUE_TYPE_VEC2);
            InitVarSubMenu(&VarSubMenuList[1], "Cooldown Time", VAR_SECTION_VALUE_TYPE_FLOAT);
            
            VarSubMenuList[0].pVec2 = &g_SharedCompList.ShootCompList[ShootEditData.CurrentID].pSpeedList[0];
            VarSubMenuList[1].pfloat = &g_SharedCompList.ShootCompList[ShootEditData.CurrentID].CooldownTime;
            
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2) && !VarSubMenuList[0].Initialized && !VarSubMenuList[1].Initialized)
        {
            InitVarSubMenu(&VarSubMenuList[0], "Active Time", VAR_SECTION_VALUE_TYPE_FLOAT);
            InitVarSubMenu(&VarSubMenuList[1], "Wait Time", VAR_SECTION_VALUE_TYPE_FLOAT);
            
            VarSubMenuList[0].pfloat = &g_SharedCompList.TimedShooterCompList[ShootEditData.CurrentID].ActiveTime;
            VarSubMenuList[1].pfloat = &g_SharedCompList.TimedShooterCompList[ShootEditData.CurrentID].WaitTime;
            
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_3) && !VarSubMenuList[0].Initialized)
        {
            if (HasComponent(COMP_CONDITIONAL_SHOOTER, ShootEditData.CurrentID) && HasComponent(COMP_PATH, ShootEditData.CurrentID))
            {
                HasConditionalShooterComp = 1;
                
                InitVarSubMenu(&VarSubMenuList[0], "Condition Function", VAR_SECTION_VALUE_TYPE_VEC2);
                
                VarSubMenuList[0].pVec2 = &CondShooterFuncEnumVec;
            }
            KeyTimer = 0.0f;
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    return 1;
}

int QuitShootVarEditMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    // Stop Shoot System
    *SharedData.pRunShootSystemFlag = 0;
    g_SharedCompList.ShootCompList[ShootEditData.CurrentID].ShouldShoot = 0;

    // Reset ShouldShoot Flags
    for (int i = 0; i < g_SharedCompList.IDList_ShootComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_ShootComp.IDList[i];
        g_SharedCompList.ShootCompList[ID].ShouldShoot = 1;
    }
    g_SharedCompList.ShootCompList[*SharedData.pPlayerID].ShouldShoot = 0;

    // Despawn All Bullets
    GOIDList BulletList;
    BulletList.CurrentSize = 0;
    for (int i = 0; i < g_SharedCompList.IDList_InfoComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_InfoComp.IDList[i];
        if (g_SharedCompList.InfoCompList[ID].Tag == TAG_BULLET)
            BulletList.IDList[BulletList.CurrentSize++] = ID;
    }
    if (BulletList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, BulletList.IDList, BulletList.CurrentSize);
    
    Log_Info("QuitShootVarEditMenu(): Quiting Shoot Editing Menu");
    return 1;
}

/***** PATH EDITING *****/
struct PathEditDataStruct
{
    enum ePathEditOption 
    {
        OPTION_PATH_EDIT_NONE,
        OPTION_PATH_EDIT_ADD,
        OPTION_PATH_EDIT_REMOVE,
        OPTION_PATH_EDIT_EDIT_VARS,
        OPTION_PATH_EDIT_CHANGE,
        OPTION_PATH_EDIT_QUIT
    } Option;

    enum ePathEditState 
    {
        STATE_PATH_EDIT_NONE,
        STATE_PATH_EDIT_REINIT,
        STATE_PATH_EDIT_ADDING_INIT,
        STATE_PATH_EDIT_ADDING,
        STATE_PATH_EDIT_REMOVING_INIT,
        STATE_PATH_EDIT_REMOVING,
        STATE_PATH_EDIT_CHANGING_INIT,
        STATE_PATH_EDIT_CHANGING
    } State;

    GOID CurrentID;
    GOID PrevID;

    GOID AddingID;
    GOID RemovingID;
    
    int ChangingCoordIndex;
    short ChangingCoordFlag;

    GOIDList DebugGOIDList;

    // VARIABLE EDITING
    Vector2 PathLogicVar;
};
static struct PathEditDataStruct PathEditData;

int InitPathEditMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "PATH EDIT OPTIONS:\n"\
                            "1  Add a Coordinate\n"\
                            "2  Remove a Coordinate\n"\
                            "3  Set Variables\n"\
                            "4  Change a Coordinate"
                            "0  Back to Main Menu\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 2);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    // Initializing Editing Data
    PathEditData.PrevID = PathEditData.CurrentID = CG_INVALID_GOID;
    PathEditData.Option = OPTION_PATH_EDIT_NONE;
    PathEditData.State = STATE_PATH_EDIT_REINIT;
    PathEditData.DebugGOIDList.CurrentSize = 0;
    PathEditData.ChangingCoordFlag = 0;
    PathEditData.ChangingCoordIndex = -1;
    
    // List of GOIDs Containing Path Component
    SetCursorMoveList(g_SharedCompList.IDList_PathComp.IDList, g_SharedCompList.IDList_PathComp.CurrentSize);
    PathEditData.CurrentID = GetCursorMoveListCurrentID();

    // Initialize Everyone into Starting Positions
    for (int i = 0; i < g_SharedCompList.IDList_PathComp.CurrentSize; ++i)
    {
        GOID ID = g_SharedCompList.IDList_PathComp.IDList[i];
        PathComponent* PC = &g_SharedCompList.PathCompList[ID];

        if (PC->Size > 0)
        {
            Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
            pSpr->Position = PC->pCoordsList[0];
        }
    }

    Log_Info("InitPathEditMenu(): Initializing Path Editing Menu");
    return 1;
}

int UpdatePathEditMenu(void)
{
    // Check if Debug Path Needs to Be Drawn
    if (PathEditData.State == STATE_PATH_EDIT_REINIT)
    {
        // NO PATH LINES FOR NOW
        //
        // Despawn any previously Spawned Stuff
        if (PathEditData.DebugGOIDList.CurrentSize > 0)
            DespawnGroup(SharedData.pSpawnRecord, PathEditData.DebugGOIDList.IDList, PathEditData.DebugGOIDList.CurrentSize);
        PathEditData.DebugGOIDList.CurrentSize = 0;

        // Get Current ID's Path Data
        PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];
        if (PC->Size > 0)
        {
            // Resetting Path Data
            PC->CurrentDestCoordIndex = 0;
            PC->CurrentPass = 0;
            PC->CurrentTimer = 0.0f;

            // Spawn Dummy Enemies
            int NumOfDummies = PC->Size - 1;
            SpawnExt(SharedData.pSpawnRecord, TAG_ENEMY, NumOfDummies, PathEditData.DebugGOIDList.IDList);
            PathEditData.DebugGOIDList.CurrentSize = NumOfDummies;
            
            for (int i = 1; i < PC->Size; ++i)
            {
                // Make Debug
                GOID ID = PathEditData.DebugGOIDList.IDList[i - 1];
                g_SharedCompList.InfoCompList[ID].DebugFlag = 1;

                // Replace All Dummy Enemy Sprite Chars with Numbers
                char CoordNum = i + '0';
                Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
                int Limit = (pSpr->Size.x * pSpr->Size.y);
                for (int j = 0; j < Limit; ++j)
                {
                    pSpr->CharBuffer[j] = CoordNum;
                    pSpr->ColorBuffer[j] = C_db;
                }
            
                // Set Positions
                pSpr->Position = PC->pCoordsList[i];
            }
        }

        PathEditData.State = STATE_PATH_EDIT_NONE;
    }
    

    // Input
    PathEditData.Option = OPTION_PATH_EDIT_NONE;
    if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_0)) // Switch to Main Menu 
        {
            PathEditData.Option = OPTION_PATH_EDIT_QUIT;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_1))
        {
            PathEditData.Option = OPTION_PATH_EDIT_ADD;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2))
        {
            PathEditData.Option = OPTION_PATH_EDIT_REMOVE;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_3))
        {
            PathEditData.Option = OPTION_PATH_EDIT_EDIT_VARS;
            KeyTimer = 0.0f;    
        }
        else if (IsKeyPressed(CG_KEY_4))
        {
            PathEditData.Option = OPTION_PATH_EDIT_CHANGE;
            KeyTimer = 0.0f;    
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    // Changing State
    if (PathEditData.Option == OPTION_PATH_EDIT_ADD && PathEditData.State == STATE_PATH_EDIT_NONE)
        PathEditData.State = STATE_PATH_EDIT_ADDING_INIT;
    else if (PathEditData.Option == OPTION_PATH_EDIT_REMOVE && PathEditData.State == STATE_PATH_EDIT_NONE)
        PathEditData.State = STATE_PATH_EDIT_REMOVING_INIT;
    else if (PathEditData.Option == OPTION_PATH_EDIT_CHANGE && PathEditData.State == STATE_PATH_EDIT_NONE)
        PathEditData.State = STATE_PATH_EDIT_CHANGING;
    else if (PathEditData.Option == OPTION_PATH_EDIT_EDIT_VARS && PathEditData.State == STATE_PATH_EDIT_NONE)
    {
        SwitchMenu(MENU_PATH_VAR_EDIT);
    }
    else if (PathEditData.Option == OPTION_PATH_EDIT_QUIT && PathEditData.State == STATE_PATH_EDIT_NONE)
    {
        SwitchMenu(MENU_MAIN);
        return 0;
    }

    // State Behaviour
    if (PathEditData.State == STATE_PATH_EDIT_NONE)
    {
        PathEditData.PrevID = PathEditData.CurrentID;
        PathEditData.CurrentID = GetCursorMoveListCurrentID();
        //Log_Info("PrevId=%d, NewID=%d", PathEditData.PrevID, PathEditData.CurrentID);
        if (PathEditData.CurrentID != PathEditData.PrevID)
            PathEditData.State = STATE_PATH_EDIT_REINIT;
    }
    else if (PathEditData.State == STATE_PATH_EDIT_ADDING_INIT)
    {
        // Spawn New Dummy
        GOID ID = CG_INVALID_GOID;
        SpawnExt(SharedData.pSpawnRecord, TAG_ENEMY, 1, &ID);
        if (ID != CG_INVALID_GOID)
        {
            // Make Debug
            g_SharedCompList.InfoCompList[ID].DebugFlag = 1;

            // No Path. Creating One.
            PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];
            //if (PC->Size == 0)
            //    AddCoordToPath(PC, g_SharedCompList.DrawingCompList[PathEditData.CurrentID].Spr.Position);

            // Replace All Dummy Enemy Sprite Chars with Numbers
            int Num = PC->Size;
            if (PathEditData.ChangingCoordFlag)
                Num = PathEditData.ChangingCoordIndex;

            char CoordNum = Num + '0';
            Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
            int Limit = (pSpr->Size.x * pSpr->Size.y);
            for (int j = 0; j < Limit; ++j)
            {
                pSpr->CharBuffer[j] = CoordNum;
                pSpr->ColorBuffer[j] = C_b;
            }

            SetCursorMoveFree();
            SetCursorSelectionMode(ID);

            PathEditData.AddingID = ID;
            PathEditData.State = STATE_PATH_EDIT_ADDING;
        }
        else
        {
            Log_Warn("UpdatePathEditMenu(): ID is Invalid. State=%d", PathEditData.State);
        }
    }
    else if (PathEditData.State == STATE_PATH_EDIT_ADDING)
    {
        if (PathEditData.Option == OPTION_PATH_EDIT_ADD || PathEditData.Option == OPTION_PATH_EDIT_QUIT ||
            PathEditData.Option == OPTION_PATH_EDIT_CHANGE)
        {
            PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];

            // Add Coordinate
            if (PathEditData.Option == OPTION_PATH_EDIT_ADD && !PathEditData.ChangingCoordFlag)
            {
                Vector2 NewCoord = g_SharedCompList.DrawingCompList[PathEditData.AddingID].Spr.Position;
                AddCoordToPath(PC, NewCoord);
                SetPathSpeed(PC, PC->pSpeedList[0], PC->Size - 1);
                SetPathWaitingTime(PC, PC->pWaitTimeList[0], PC->Size - 1);
            }
            else if (PathEditData.Option == OPTION_PATH_EDIT_CHANGE && PathEditData.ChangingCoordFlag)
            {
                int Index = PathEditData.ChangingCoordIndex;
                Vector2 NewCoord = g_SharedCompList.DrawingCompList[PathEditData.AddingID].Spr.Position;
                AddCoordToPath(PC, NewCoord);
                
                // Shift All Coords
                Log_Info("Index: %d", Index);
                for (int i = Index; i < PC->Size - 1; ++i)
                {
                    SwapPathData(PC, i + 1, Index);
                }
                
                PC->pCoordsList[Index] = NewCoord;
                SetPathSpeed(PC, PC->pSpeedList[0], Index);
                SetPathWaitingTime(PC, PC->pWaitTimeList[0], Index);
                
                PathEditData.ChangingCoordFlag = 0;
                PathEditData.ChangingCoordIndex = -1;
            }


            // Despawn Adding Dummy
            Despawn(SharedData.pSpawnRecord, PathEditData.AddingID);

            // Set Cursor Back to Move List
            GOIDList IDList;
            IDList.CurrentSize = 0;
            for (int i = 0; i < g_SharedCompList.IDList_PathComp.CurrentSize; ++i)
            {
                GOID ID = g_SharedCompList.IDList_PathComp.IDList[i];
                if (!g_SharedCompList.InfoCompList[ID].DebugFlag)
                    IDList.IDList[IDList.CurrentSize++] = ID;
            }
            SetCursorEmptyMode();
            SetCursorMoveListWithStartingID(IDList.IDList, IDList.CurrentSize, PathEditData.CurrentID);
            
            PathEditData.State = STATE_PATH_EDIT_REINIT;
        }
    }
    else if (PathEditData.State == STATE_PATH_EDIT_REMOVING_INIT)
    {
        // No Path
        PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];
        if (PC->Size == 0)
        {
            PathEditData.ChangingCoordFlag = 0;
            PathEditData.State = STATE_PATH_EDIT_REINIT;
            return 0;
        }

        // Create Dummy First Coordinate in Place of Game Object
        GOID ID = CG_INVALID_GOID;
        SpawnExt(SharedData.pSpawnRecord, TAG_ENEMY, 1, &ID);
        if (ID != CG_INVALID_GOID)
        {
            // Make Debug
            g_SharedCompList.InfoCompList[ID].DebugFlag = 1;

            // Replace Dummy Sprite Chars with 0
            char CoordNum = '0';
            Sprite* pSpr = &g_SharedCompList.DrawingCompList[ID].Spr;
            int Limit = (pSpr->Size.x * pSpr->Size.y);
            for (int j = 0; j < Limit; ++j)
            {
                pSpr->CharBuffer[j] = CoordNum;
                pSpr->ColorBuffer[j] = C_b;
            }
            pSpr->Position = g_SharedCompList.DrawingCompList[PathEditData.CurrentID].Spr.Position;
            g_SharedCompList.DrawingCompList[PathEditData.CurrentID].Spr.Position = CreateVec2(-100, -100);

            // Prepare List of Coordinates
            GOIDList DummyList;
            DummyList.IDList[0] = ID;
            for (int i = 0; i < PathEditData.DebugGOIDList.CurrentSize; ++i)
                DummyList.IDList[i + 1] = PathEditData.DebugGOIDList.IDList[i];
            DummyList.CurrentSize = PathEditData.DebugGOIDList.CurrentSize + 1;

            // Set Cursor to List
            SetCursorMoveListWithStartingID(DummyList.IDList, DummyList.CurrentSize, ID);

            PathEditData.RemovingID = ID;
            //Log_Info("DebugFlag=%d", g_SharedCompList.InfoCompList[PathEditData.CurrentID].DebugFlag);
            PathEditData.State = STATE_PATH_EDIT_REMOVING;
        }
    }
    else if (PathEditData.State == STATE_PATH_EDIT_REMOVING)
    {
        if (PathEditData.Option == OPTION_PATH_EDIT_REMOVE || PathEditData.Option == OPTION_PATH_EDIT_QUIT ||
            PathEditData.Option == OPTION_PATH_EDIT_CHANGE)
        {
            GOID ID = GetCursorMoveListCurrentID();
            PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];

            // Place GO Back to First Position
            Vector2 PrevPosition = PC->pCoordsList[0];
            g_SharedCompList.DrawingCompList[PathEditData.CurrentID].Spr.Position = PrevPosition;

            // Remove Coordinate
            int Index = g_SharedCompList.DrawingCompList[ID].Spr.CharBuffer[0] - '0';
            if ((PathEditData.Option == OPTION_PATH_EDIT_REMOVE && !PathEditData.ChangingCoordFlag) ||
                (PathEditData.Option == OPTION_PATH_EDIT_CHANGE && PathEditData.ChangingCoordFlag))
            {
                RemoveCoordFromPath(PC, Index);
            }

            // Despawn First Dummy
            Despawn(SharedData.pSpawnRecord, PathEditData.RemovingID);

            // Set Cursor Back to Move List
            GOIDList IDList;
            IDList.CurrentSize = 0;
            for (int i = 0; i < g_SharedCompList.IDList_PathComp.CurrentSize; ++i)
            {
                GOID ID = g_SharedCompList.IDList_PathComp.IDList[i];
                if (!g_SharedCompList.InfoCompList[ID].DebugFlag)
                    IDList.IDList[IDList.CurrentSize++] = ID;
            }

            SetCursorEmptyMode();
            SetCursorMoveListWithStartingID(IDList.IDList, IDList.CurrentSize, PathEditData.CurrentID);
            
            if (PathEditData.Option == OPTION_PATH_EDIT_CHANGE && PathEditData.ChangingCoordFlag)
            {
                PathEditData.ChangingCoordIndex = Index;
                PathEditData.State = STATE_PATH_EDIT_ADDING_INIT;
            }
            else
                PathEditData.State = STATE_PATH_EDIT_REINIT;
        }
    }
    else if (PathEditData.State == STATE_PATH_EDIT_CHANGING)
    {
        // First Remove, then Add
        PathEditData.ChangingCoordFlag = 1;
        PathEditData.State = STATE_PATH_EDIT_REMOVING_INIT;
    }

    return 0;
}

int QuitPathEditMenu(void)
{
    // Despawn any previously Spawned Stuff
    if (PathEditData.DebugGOIDList.CurrentSize > 0)
        DespawnGroup(SharedData.pSpawnRecord, PathEditData.DebugGOIDList.IDList, PathEditData.DebugGOIDList.CurrentSize);
    PathEditData.DebugGOIDList.CurrentSize = 0;

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    Log_Info("QuitPathEditMenu(): Quiting Path Editing Menu");
    return 1;
}

int InitPathVarEditMenu(void)
{
    char* MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "PATH VARIABLE EDIT OPTIONS:\n"\
                            "1 Set Speed and Wait Time\n"\
                            "2 Set Path Logic\n"\
                            "0 Back to Path Edit Menu\n"\
                            "... Press UP to Close Menu";


    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 2);
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    SetCursorMoveFree();

    // Run Path System
    *SharedData.pRunPathSystemFlag = 1;

    Log_Info("InitPathVarEditMenu(): Initializing Path Variables Editing Menu");
    return 1;
}

int UpdatePathVarEditMenu(void)
{
    // Copy Edited Data
    PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];
    for (int i = 1; i < PC->Size; ++i)
    {
        // Speed and Wait Time
        PC->pSpeedList[i] = PC->pSpeedList[0];
        PC->pWaitTimeList[i] = PC->pWaitTimeList[0];

        // Path Logic
        int PL = min(2, max(0, PathEditData.PathLogicVar.x));
        PC->PathLogic = PL;
    }

    if (IsKeyPressed(VarSubMenuList[0].HoldKey) || IsKeyPressed(VarSubMenuList[1].HoldKey))
        return 0;
    
    if (IsKeyPressed(CG_KEY_0) && KeyTimer > KEY_ACCEPT_TIME)
    {
        if (VarSubMenuList[0].Initialized)
            QuitVarSubMenu(&VarSubMenuList[0]);
        if (VarSubMenuList[1].Initialized)
            QuitVarSubMenu(&VarSubMenuList[1]);
        SwitchMenu(MENU_PATH_EDIT);
        
        KeyTimer = 0.0f;
    }
    else if (KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_1) && !VarSubMenuList[0].Initialized && !VarSubMenuList[1].Initialized)
        {
            InitVarSubMenu(&VarSubMenuList[0], "Speed", VAR_SECTION_VALUE_TYPE_VEC2);
            InitVarSubMenu(&VarSubMenuList[1], "Waiting Time", VAR_SECTION_VALUE_TYPE_FLOAT);
            
            VarSubMenuList[0].pVec2 = &PC->pSpeedList[0];
            VarSubMenuList[1].pfloat = &PC->pWaitTimeList[0];
            
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2) && !VarSubMenuList[0].Initialized)
        {
            InitVarSubMenu(&VarSubMenuList[0], "Path Logic", VAR_SECTION_VALUE_TYPE_VEC2);

            PathEditData.PathLogicVar = CreateVec2(0, 0);
            VarSubMenuList[0].pVec2 = &PathEditData.PathLogicVar;
            
            KeyTimer = 0.0f;
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    return 1;
}

int QuitPathVarEditMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    // Stop Path System
    PathComponent* PC = &g_SharedCompList.PathCompList[PathEditData.CurrentID];
    PC->CurrentPass = 0;
    *SharedData.pRunPathSystemFlag = 0;

    Log_Info("QuitPathVarEditMenu(): Quiting Path Variables Editing Menu");
}

#define HORDE_MENU_MAX_BUFFER_LEN   1024
#define HORDE_MENU_MAX_FILES        5
struct HordeMenuStruct
{
    char* MainMenuStr;
    
//    char FilesMenuStr[HORDE_MENU_MAX_BUFFER_LEN];

    int StartingFileIndex;

    enum eHordeMenuOption
    {
        OPTION_HORDE_MENU_NONE,
        OPTION_HORDE_MENU_LOAD,
        OPTION_HORDE_MENU_SAVE
    } HordeMenuOption;
    
    
    enum eHordeMenuState
    {
        STATE_HORDE_MENU_NONE,
        STATE_HORDE_MENU_LOADING_INIT,
        STATE_HORDE_MENU_LOADING,
        STATE_HORDE_MENU_SAVING_INIT,
        STATE_HORDE_MENU_SAVING,
        STATE_HORDE_MENU_READ_HORDE,
        STATE_HORDE_MENU_WRITE_HORDE
    } HordeMenuState, NextStateAfterLoading;

    int SelectedFileIndex;
};
static struct HordeMenuStruct HordeMenuData;

int InitHordeMenu(void)
{
    HordeMenuData.MainMenuStr =     "********** DEBUG MENU **********\n"\
                            "HORDE MENU:\n"\
                            "1      Load Horde from a File\n"\
                            "2      Save Horde in a File\n"\
                            "0      Back to Main Menu\n"\
                            "... Press UP to Close Menu";

    //strncpy(HordeMenuData.FilesMenuStr, "\0", HORDE_MENU_MAX_BUFFER_LEN);

    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    
    pSpr->Position = CreateVec2(0, OG_Size.y - 1);
    SetSprite(pSpr, HordeMenuData.MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);
    
    SetCursorMoveFree();

    HordeMenuData.HordeMenuOption = OPTION_HORDE_MENU_NONE;
    HordeMenuData.HordeMenuState = STATE_HORDE_MENU_NONE;
    HordeMenuData.NextStateAfterLoading = STATE_HORDE_MENU_NONE;
    HordeMenuData.StartingFileIndex = 0;
    HordeMenuData.SelectedFileIndex = -1;

    Log_Info("InitHordeMenu(): Initializing Horde Menu");
}

int UpdateHordeMenu(void)
{
// Keys Corresponding to Variables
#ifdef CG_PLATFORM_WINDOWS
    static SHORT Keys[] = 
    {
        CG_KEY_1,
        CG_KEY_2,
        CG_KEY_3,
        CG_KEY_4,
        CG_KEY_5
    };
#endif    

    HordeMenuData.HordeMenuOption = OPTION_HORDE_MENU_NONE;
    if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_NONE && KeyTimer > KEY_ACCEPT_TIME)
    {
        if (IsKeyPressed(CG_KEY_0))
        {
            SwitchMenu(MENU_MAIN);
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_1))
        {
            HordeMenuData.HordeMenuOption = OPTION_HORDE_MENU_LOAD;
            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_2))
        {
            HordeMenuData.HordeMenuOption = OPTION_HORDE_MENU_SAVE;
            KeyTimer = 0.0f;
        }
    }
    else
        KeyTimer += g_dtAsSeconds;

    // Changing State
    if (HordeMenuData.HordeMenuOption == OPTION_HORDE_MENU_LOAD)
        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_LOADING_INIT;
    else if (HordeMenuData.HordeMenuOption == OPTION_HORDE_MENU_SAVE)
        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_SAVING_INIT;

    // State Behaviour
    if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_READ_HORDE)
    {
        if (HordeMenuData.SelectedFileIndex != -1)
        {
            ReadEnemyHordeFromFile( SharedData.pHordeMgr, 
                                    SharedData.pHordeMgr->HFM.ScannedFilesInDir[HordeMenuData.SelectedFileIndex],
                                    SharedData.pSpawnRecord);
            SendNextEnemyHorde(SharedData.pHordeMgr, SharedData.pSpawnRecord);
        }

        HordeMenuData.StartingFileIndex = 0;
        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_NONE;
    }
    else if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_WRITE_HORDE)
    {
        Log_Info("Writing Outside, HordeMenuData.SelectedFileIndex: %d", HordeMenuData.SelectedFileIndex);
        if (HordeMenuData.SelectedFileIndex != -1)
        {
            OpenFileFromName(   &SharedData.pHordeMgr->HFM, 
                                SharedData.pHordeMgr->HFM.ScannedFilesInDir[HordeMenuData.SelectedFileIndex], 
                                NULL, 
                                FM_WRITE_MODE);

            // Creating a Horde
            GOIDList IDList;
            IDList.CurrentSize = 0;
            for (int i = 0; i < g_SharedCompList.IDList_InfoComp.CurrentSize; ++i)
            {
                GOID ID = g_SharedCompList.IDList_InfoComp.IDList[i];
                if (g_SharedCompList.InfoCompList[ID].Tag == TAG_ENEMY && !g_SharedCompList.InfoCompList[ID].DebugFlag)
                    IDList.IDList[IDList.CurrentSize++] = ID;
            }
            EnemyHorde Horde;
            Horde.NumOfEnemies = IDList.CurrentSize;
            Horde.IDList = (GOID*)malloc(sizeof(GOID) * Horde.NumOfEnemies);
            for (int i = 0; i < IDList.CurrentSize; ++i)
                Horde.IDList[i] = IDList.IDList[i];

            // Writing to File
            Log_Info("Wrirint");
            WriteToHordeFile(&SharedData.pHordeMgr->HFM, &Horde);
            CloseFile(&SharedData.pHordeMgr->HFM);

            DespawnGroup(SharedData.pSpawnRecord, IDList.IDList, IDList.CurrentSize);
            free(Horde.IDList);
        }
        
        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_READ_HORDE;
    }
    else if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_LOADING_INIT || HordeMenuData.HordeMenuState == STATE_HORDE_MENU_SAVING_INIT)
    {
        // Scanning Directory for New Files
        // int PrevNumOfFiles = SharedData.pHordeMgr->HFM.NumOfFiles;
        
        int NumOfFiles = ScanDirectory(&SharedData.pHordeMgr->HFM, HF_EXTENSION, FM_MAX_FILES);
        
        // if (PrevNumOfFiles > NumOfFiles && HordeMenuData.StartingFileIndex == NumOfFiles) // Files were Deleted
        //     HordeMenuData.StartingFileIndex = max(0, HordeMenuData.StartingFileIndex - HORDE_MENU_MAX_FILES);
        
        // Preparing Menu
        char FilesMenuStr[HORDE_MENU_MAX_BUFFER_LEN + 1];
        strncpy(FilesMenuStr, "Select a File, 0 to Back:\n", HORDE_MENU_MAX_BUFFER_LEN);
        strncat(FilesMenuStr, "Left to Back, Right to Next:", HORDE_MENU_MAX_BUFFER_LEN);
        int Size = min(NumOfFiles, HORDE_MENU_MAX_FILES);
        int Numbering = 0;
        for (int i = HordeMenuData.StartingFileIndex; i < (HordeMenuData.StartingFileIndex + Size); ++i)
        {
            if (i >= NumOfFiles)
                break;
            
            strncat(FilesMenuStr, "\n", HORDE_MENU_MAX_BUFFER_LEN);

            int Num = ++Numbering;
            int Len = snprintf(NULL, 0, "%d ", Num) + 1;
            char NumBuffer[Len];
            snprintf(NumBuffer, Len, "%d ", Num);

            strncat(FilesMenuStr, NumBuffer, HORDE_MENU_MAX_BUFFER_LEN);
            strncat(FilesMenuStr, SharedData.pHordeMgr->HFM.ScannedFilesInDir[i], HORDE_MENU_MAX_BUFFER_LEN);
        }

        // Setting Sprite
        Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
        Vector2 Position = pSpr->Position;
        DestroySprite(pSpr);
        SetSprite(pSpr, FilesMenuStr, '\n');
        FillSpriteSingleColor(pSpr, C_c);
        pSpr->Position = Position;
        if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_LOADING_INIT)
            HordeMenuData.NextStateAfterLoading = STATE_HORDE_MENU_READ_HORDE;
        else
            HordeMenuData.NextStateAfterLoading = STATE_HORDE_MENU_WRITE_HORDE;

        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_LOADING;
    }
    else if (HordeMenuData.HordeMenuState == STATE_HORDE_MENU_LOADING)
    {
        // New Input
        if (KeyTimer > KEY_ACCEPT_TIME)
        {
            if (IsKeyPressed(CG_KEY_0))
            {
                Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
                DestroySprite(pSpr);

                InitHordeMenu();

                KeyTimer = 0.0f;
            }
            else if (IsKeyPressed(CG_KEY_LEFT)) // back
            {
                if (HordeMenuData.StartingFileIndex > 0)
                {
                    HordeMenuData.StartingFileIndex = max(0, HordeMenuData.StartingFileIndex - HORDE_MENU_MAX_FILES);
                    if (HordeMenuData.NextStateAfterLoading == STATE_HORDE_MENU_READ_HORDE)
                        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_LOADING_INIT;
                    else
                        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_SAVING_INIT;
                }
                KeyTimer = 0.0f;
            }
            else if (IsKeyPressed(CG_KEY_RIGHT)) // next
            {
                if (HordeMenuData.StartingFileIndex + HORDE_MENU_MAX_FILES < SharedData.pHordeMgr->HFM.NumOfFiles)
                {
                    HordeMenuData.StartingFileIndex = HordeMenuData.StartingFileIndex + HORDE_MENU_MAX_FILES;
                    if (HordeMenuData.NextStateAfterLoading == STATE_HORDE_MENU_READ_HORDE)
                        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_LOADING_INIT;
                    else
                        HordeMenuData.HordeMenuState = STATE_HORDE_MENU_SAVING_INIT;
                }
                KeyTimer = 0.0f;
            }
            else
            {
                for (int i = 0; i < ARR_LEN(Keys); ++i)
                {
                    if (IsKeyPressed(Keys[i]))
                    {
                        int StartingFileIndex = HordeMenuData.StartingFileIndex;
                        enum eHordeMenuState NextState = HordeMenuData.NextStateAfterLoading;
                        DestroySprite(&g_SharedCompList.DrawingCompList[DebugMenuID].Spr);
                        InitHordeMenu();                    
                    
                        HordeMenuData.StartingFileIndex = StartingFileIndex;
                        HordeMenuData.SelectedFileIndex = min(HordeMenuData.StartingFileIndex + i, SharedData.pHordeMgr->HFM.NumOfFiles - 1);
                        
                        HordeMenuData.HordeMenuState = NextState;
                        
                        KeyTimer = 0.0f;
                        break;
                    }  
                }
            }
        }
    }
}

int QuitHordeMenu(void)
{
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    DestroySprite(pSpr);
    
    Log_Info("QuitHordeMenu(): Quiting Horde Menu");
}

int InitVarSubMenu(VarSubMenu* VM, char* VarName, enum eVarSectionValueType ValType)
{
    static const int MAX_SIZE = 256;

    if (NumOfVarSubMenus >= MAX_VAR_SUB_MENUS)
    {
        Log_Warn("InitVarSubMenu(): Max Number of Sub-Menus Exceeded");
        return 0;
    }

    char MainMenuStr[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; ++i)
        MainMenuStr[i] = '\0';
    
    // Initializing the File Manager and VarSection Data Holder
    InitFileManager(&VM->FM);
    OpenFileFromName(&VM->FM, VM->VarFile, VF_EXTENSION, FM_READ_MODE);
    CreateVarSection(&VM->VS, 0, ValType);

    // Reading All Sections
    int NumOfSections = ReadAllSectionsFromVarFile(&VM->FM, VM->SectionList, ValType);
    // Creating a Sub Menu
    VM->ID = CreateGameObj();
    AttachComponent(COMP_DRAWING, VM->ID);
    
    NumOfVarSubMenus++;
    VM->Section = NULL;
    VM->VarName = VarName;

    // Setting up the Sub-Menu
    Sprite* pDebugMenuSpr = &g_SharedCompList.DrawingCompList[DebugMenuID].Spr;
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[VM->ID].Spr;
    pSpr->CharBuffer = NULL;
    pSpr->ColorBuffer = NULL;
    pSpr->Position = CreateVec2(pDebugMenuSpr->Position.x + (pDebugMenuSpr->Size.x * VM->MenuIndex) + 1, 
                                    pDebugMenuSpr->Position.y - 1);
    pSpr->Size = CreateVec2(pDebugMenuSpr->Size.x, pDebugMenuSpr->Size.y);
    

    // Setting up the Initial Main Menu
    strncpy(MainMenuStr, "VAR: ", MAX_SIZE);
    strncat(MainMenuStr, VarName, MAX_SIZE);
    strncat(MainMenuStr, ", Z to Back\n", MAX_SIZE);
    int MaxRowsForSections = TerminalSizeOffset.y - 2;
    int Size = (NumOfSections < MaxRowsForSections) ? NumOfSections : MaxRowsForSections;
    for (int i = 0; i < Size; ++i)
    {
        int length = snprintf(NULL, 0, "%d", i + 1);
        char* NumStr = (char*)malloc(length + 1);
        snprintf( NumStr, length + 1, "%d", i + 1 );

        strncat(MainMenuStr, NumStr, MAX_SIZE);
        strncat(MainMenuStr, " ", MAX_SIZE);
        strncat(MainMenuStr, VM->SectionList[i], MAX_SIZE);
        strncat(MainMenuStr, "\n", MAX_SIZE);

        free(NumStr);
        NumStr = NULL;
    }
    VM->NumOfEntriesFilled = Size;
    
    char* LastLine = "Hold LeftKey + Num, 0-Read File Again";
    if (VM->MenuIndex == 2)
        LastLine = "Hold RightKey + Num, 0-Read File Again";
    strncat(MainMenuStr, LastLine, MAX_SIZE);
    
    // Adding the Initial Main Menu
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);

    VM->Initialized = 1;
    VM->SectionSelectionMenu = 1;
    Log_Info("InitVarSubMenu(): Initialized Variable Sub Menu. VMIndex=%d, VarName=%s", VM->MenuIndex, VM->VarName);
    return 1;
}

int SetVarSubMenu(VarSubMenu* VM, char* Section)
{
    static const int MAX_SIZE = 256;

    // Read KV Pairs from Section
    VM->Section = Section;
    ReadSectionFromVarFile(&VM->FM, Section, &VM->VS);

    // Initialize Menu Text Buffer
    char MainMenuStr[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; ++i)
        MainMenuStr[i] = '\0';
    
    // Set Menu Text Buffer
    strncpy(MainMenuStr, "VAR: ", MAX_SIZE);
    strncat(MainMenuStr, VM->VarName, MAX_SIZE);
    strncat(MainMenuStr, ", Z to Back\n", MAX_SIZE);
    int MaxRowsForSections = TerminalSizeOffset.y - 2;
    int Size = (VM->VS.Size < MaxRowsForSections) ? VM->VS.Size : MaxRowsForSections;
    for (int i = 0; i < Size; ++i)
    {
        int length = snprintf(NULL, 0, "%d", i + 1);
        char* NumStr = (char*)malloc(length + 1);
        snprintf( NumStr, length + 1, "%d", i + 1 );

        strncat(MainMenuStr, NumStr, MAX_SIZE);
        strncat(MainMenuStr, " ", MAX_SIZE);
        strncat(MainMenuStr, VM->VS.KeyValList[i], MAX_SIZE);
        strncat(MainMenuStr, "\n", MAX_SIZE);

        free(NumStr);
        NumStr = NULL;
    }
    VM->NumOfEntriesFilled = Size;
    
    char* LastLine = "Hold LeftKey + Num, 0-Read File Again";
    if (VM->MenuIndex == 2)
        LastLine = "Hold RightKey + Num, 0-Read File Again";
    strncat(MainMenuStr, LastLine, MAX_SIZE);
    
    
    // Adding the Initial Main Menu
    Sprite* pSpr = &g_SharedCompList.DrawingCompList[VM->ID].Spr;
    Vector2 Position = pSpr->Position;
    DestroySprite(pSpr);
    pSpr->Position = Position;
    SetSprite(pSpr, MainMenuStr, '\n');
    FillSpriteSingleColor(pSpr, C_c);
    
    VM->SectionSelectionMenu = 0;
    return 1;
}

int UpdateVarSubMenu(VarSubMenu* VM)
{   
    static float KeyTimer = 0.0f;
    static int MAX_KEYS = 10;
    //static int Init = 0;
    
    // Keys Corresponding to Variables
#ifdef CG_PLATFORM_WINDOWS
    static SHORT Keys[] = 
    {
        CG_KEY_1,
        CG_KEY_2,
        CG_KEY_3,
        CG_KEY_4,
        CG_KEY_5,
        CG_KEY_6,
        CG_KEY_7,
        CG_KEY_8,
        CG_KEY_9
    };
#endif
        
    if (VM->Initialized && IsKeyPressed(VM->HoldKey) && KeyTimer > KEY_ACCEPT_TIME)
    {
        // Go Back to Section Selection
        if (IsKeyPressed(CG_KEY_Z) && !VM->SectionSelectionMenu)
        {
            // Store metadata before deletion
            enum eVarSectionValueType ValType = VM->VS.ValueType;
            char* VarName = VM->VarName;
            
            // Store the Working Pointer
            Vector2* pVec2 = NULL;
            float* pFloat = NULL;
            if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
                pVec2 = VM->pVec2;
            else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
                pFloat = VM->pfloat;

            // Reset the Var Sub Menu
            QuitVarSubMenu(VM);
            InitVarSubMenu(VM, VarName, ValType);

            // Reset the Working Pointer
            if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
                VM->pVec2 = pVec2;
            else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
                VM->pfloat = pFloat;

            KeyTimer = 0.0f;
        }
        else if (IsKeyPressed(CG_KEY_0))
        {
            // Reread the File
            enum eVarSectionValueType ValType = VM->VS.ValueType;
            char* VarName = VM->VarName;
            char* Section = VM->Section;            

            // Store the Working Pointer
            Vector2* pVec2 = NULL;
            float* pFloat = NULL;
            if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
                pVec2 = VM->pVec2;
            else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
                pFloat = VM->pfloat;

            QuitVarSubMenu(VM);
            InitVarSubMenu(VM, VarName, ValType);
            SetVarSubMenu(VM, Section);

            // Reset the Working Pointer
            if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
                VM->pVec2 = pVec2;
            else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
                VM->pfloat = pFloat;

            KeyTimer = 0.0f;
        }
        else 
        {
            for (int i = 0; i < MAX_KEYS; ++i)
            {
                if (IsKeyPressed(Keys[i]))
                {
                    //Log_Warn("UpdateVarSubMenu(): RUNNING");    
                        
                    if (!VM->SectionSelectionMenu && i < VM->VS.Size)
                    {
                        if (VM->VS.ValueType == VAR_SECTION_VALUE_TYPE_VEC2)
                        {
                            if (VM->pVec2 != NULL)
                                *VM->pVec2 = CreateVec2(VM->VS.pVecValueList[i].x, VM->VS.pVecValueList[i].y);
                            else
                            {
                                Log_Warn("UpdateVarSubMenu(): VM->pVec2 is NULL. VMIndex=%d, VarName=%s", VM->MenuIndex, VM->VarName);
                            }
                        }
                        else if (VM->VS.ValueType == VAR_SECTION_VALUE_TYPE_FLOAT)
                        {
                            if (VM->pfloat != NULL)
                                *VM->pfloat = VM->VS.pFloatValueList[i];
                            else
                            {
                                Log_Warn("UpdateVarSubMenu(): VM->pfloat is NULL. VMIndex=%d, VarName=%s", VM->MenuIndex, VM->VarName);
                            }
                        }
                    }
                    else if (i < VM->NumOfEntriesFilled)
                    {
                        SetVarSubMenu(VM, VM->SectionList[i]);
                    }

                    KeyTimer = 0.0f;
                    break;
                }
            }
        }
    }
    else
        KeyTimer += g_dtAsSeconds;
}

int QuitVarSubMenu(VarSubMenu* VM)
{
    DestroySprite(&g_SharedCompList.DrawingCompList[VM->ID].Spr);

    DetachComponent(COMP_DRAWING, VM->ID);
    DestroyGameObj(VM->ID);
    VM->ID = CG_INVALID_GOID;

    DestroyVarSection(&VM->VS);
    CloseFile(&VM->FM);
    QuitFileManager(&VM->FM);
    
    VM->pVec2 = NULL;
    VM->Initialized = 0;
    VM->SectionSelectionMenu = 1;
    VM->VarName = NULL;

    --NumOfVarSubMenus;

    Log_Info("QuitVarSubMenu(): Quiting Variable Sub Menu. VMIndex=%d, VarName=%s", VM->MenuIndex, VM->VarName);
    return 1;
}
