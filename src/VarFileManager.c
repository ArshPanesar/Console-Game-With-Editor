#include "FileManager.h"

static const int MAX_LINE_LEN = 64;

// Local Functions
enum eVarSectionValueType GetValType(char* Section);

int CreateVarSection(VarSection* pVS, int Size, enum eVarSectionValueType ValType)
{
    if (pVS == NULL)
    {
        Log_Critical("CreateVarSection(): VarSection is NULL");
        return -1;
    }

    // Clearing Key=Value List
    for (int i = 0; i < VF_MAX_KV_PAIRS; ++i)
    {
        for (int j = 0; j < VF_MAX_KV_PAIRS_LEN; ++j)
            pVS->KeyValList[i][j] = '\0';
    }

    for (int i = 0; i < VF_MAX_SECTION_LEN; ++i)
        pVS->Section[i] = '\0';

    if (Size < 1 || ValType == VAR_SECTION_VALUE_TYPE_NONE)
    {
        pVS->pFloatValueList = NULL;
        pVS->pVecValueList = NULL;
    }
    else
    {
        if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
            pVS->pVecValueList = (Vector2*)malloc(sizeof(Vector2) * Size);
        else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
            pVS->pFloatValueList = (float*)malloc(sizeof(float) * Size);
    }

    pVS->ValueType = ValType;
    pVS->Size = Size;
    return Size;
}

int DestroyVarSection(VarSection* pVS)
{
    if (pVS == NULL)
    {
        Log_Critical("DestroyVarSection(): VarSection is NULL");
        return 0;
    }

    // Clearing Key=Value List
    for (int i = 0; i < VF_MAX_KV_PAIRS; ++i)
    {
        for (int j = 0; j < VF_MAX_KV_PAIRS_LEN; ++j)
            pVS->KeyValList[i][j] = '\0';
    }
    for (int i = 0; i < VF_MAX_SECTION_LEN; ++i)
        pVS->Section[i] = '\0';


    if (pVS->ValueType == VAR_SECTION_VALUE_TYPE_VEC2)
        free(pVS->pVecValueList);
    else if (pVS->ValueType == VAR_SECTION_VALUE_TYPE_FLOAT)
        free(pVS->pFloatValueList);

    pVS->pFloatValueList = NULL;
    pVS->pVecValueList = NULL;

    pVS->ValueType = VAR_SECTION_VALUE_TYPE_NONE;
    pVS->Size = 0;

    return 1;
}

int ReadAllSectionsFromVarFile(FileManager* FM, char SectionList[VF_MAX_SECTIONS][VF_MAX_SECTION_LEN], enum eVarSectionValueType ValType)
{
    if (FM == NULL)
    {
        Log_Critical("ReadAllSectionsFromVarFile(): File Manager is NULL");
        return -1;
    }
    else if (!FM->FileOpenFlag)
    {
        Log_Critical("ReadAllSectionsFromVarFile(): No File is Open.");
        return -1;
    }

    // Clear Section List
    for (int i = 0; i < VF_MAX_SECTIONS; ++i)
    {
        for (int j = 0; j < VF_MAX_SECTION_LEN; ++j)
            SectionList[i][j] = '\0';
    }

    // Working Buffer
    char Buffer[MAX_LINE_LEN];
    int NumOfSections = 0;

    // Getting Sections
    while (fgets(Buffer, MAX_LINE_LEN, FM->File) != NULL)
    {
        // Sections Start from #
        if (Buffer[0] == '#')
        {
            if (ValType == VAR_SECTION_VALUE_TYPE_NONE || ValType == GetValType(Buffer))
            {
                // Copying Section Name
                strncpy(SectionList[NumOfSections], Buffer, VF_MAX_SECTION_LEN);
                
                // Removing New Line Character
                char* NewLineChar = strchr(SectionList[NumOfSections], '\n');
                if (NewLineChar != NULL)
                    NewLineChar[0] = '\0';
                
                // Increment the Number of Sections Found
                ++NumOfSections;
            }
        }
    }

    fseek(FM->File, 0, SEEK_SET);

    return NumOfSections;
}

int ReadSectionFromVarFile(FileManager* FM, char* Section, VarSection* pVS)
{
    if (FM == NULL)
    {
        Log_Critical("ReadSectionFromVarFile(): File Manager is NULL");
        return -1;
    }
    else if (!FM->FileOpenFlag)
    {
        Log_Critical("ReadSectionFromVarFile(): No File is Open.");
        return -1;
    }
    else if (Section == NULL || pVS == NULL)
    {
        Log_Critical("ReadSectionFromVarFile(): Section or VarSection Passed is NULL.");
        return -1;
    }

    // Buffer
    char Buffer[MAX_LINE_LEN];
    int Found = 0;

    // Finding Section
    while (fgets(Buffer, MAX_LINE_LEN, FM->File) != NULL)
    {
        // Removing New Line Character
        char* NewLineChar = strchr(Buffer, '\n');
        if (NewLineChar != NULL)
            NewLineChar[0] = '\0';

        if (Buffer[0] == '#' && strcmp(Buffer, Section) == 0)
        {
            Found = 1;
            break;
        }
    }

    if (!Found)
    {
        Log_Warn("ReadSectionFromVarFile(): Section Not Found in File. Section=%s, File=%s", Section, FM->FileName);
        return -1;
    }

    // Finding Number of Key/Value Pairs
    int NumOfPairs = 0;
    long int Offset = ftell(FM->File);
    while (fgets(Buffer, MAX_LINE_LEN, FM->File) != NULL)
    {
        if (Buffer[0] != '#')
        {
            if (strchr(Buffer, '=') != NULL)
                ++NumOfPairs;
        }
        else
            break;
    }
    
    // Set VarSection Data Holder
    DestroyVarSection(pVS);
    int i = 0;
    enum eVarSectionValueType ValType = GetValType(Section);
    CreateVarSection(pVS, NumOfPairs, ValType);
    strncpy(pVS->Section, Section, VF_MAX_SECTION_LEN);

    // Fill Data
    fseek(FM->File, Offset, SEEK_SET);
    while (i < NumOfPairs)
    {
        fgets(Buffer, MAX_LINE_LEN, FM->File);

        // Copying KV Pair, Ignoring New Line Character
        strncpy(pVS->KeyValList[i], Buffer, strnlen(Buffer, MAX_LINE_LEN) - 1);

        // Finding the Value
        char* ResStr = strchr(Buffer, '=');
        if (ResStr == NULL)
            continue;

        if (ValType == VAR_SECTION_VALUE_TYPE_VEC2)
        {
            ResStr = strchr(++ResStr, '(');
            pVS->pVecValueList[i].x = (int)strtol(++ResStr, &ResStr, 10);
            ResStr = strchr(ResStr, ',');
            pVS->pVecValueList[i].y = (int)strtol(++ResStr, NULL, 10);
        }
        else if (ValType == VAR_SECTION_VALUE_TYPE_FLOAT)
        {
            pVS->pFloatValueList[i] = (float)strtof(++ResStr, NULL);
        }

        ++i;
    }

    fseek(FM->File, 0, SEEK_SET);
    return NumOfPairs;
}

int TEST_VarFileOperations(void)
{
    FileManager FM;
    InitFileManager(&FM);
    OpenFileFromName(&FM, "testvarfile", VF_EXTENSION, FM_READ_MODE);
    
    char SectionList[VF_MAX_SECTIONS][VF_MAX_SECTION_LEN];
    int n = ReadAllSectionsFromVarFile(&FM, SectionList, 0);

    VarSection VS;
    CreateVarSection(&VS, 0, 0);

    Log_Info("Sections Found: %d", n);
    for (int i = 0; i < n; ++i)
    {
        Log_Info("%s", SectionList[i]);

        ReadSectionFromVarFile(&FM, SectionList[i], &VS);
        for (int j = 0; j < VS.Size; ++j)
        {
            Log_Info("%s", VS.KeyValList[j]);
            /*if (VS.ValueType == VAR_SECTION_VALUE_TYPE_VEC2)
            {
                Log_Info("%d, %d", VS.pVecValueList[j].x, VS.pVecValueList[j].y);
            }
            else if (VS.ValueType == VAR_SECTION_VALUE_TYPE_FLOAT)
            {
                Log_Info("%f", VS.pFloatValueList[j]);
            }*/
        }
    }

    DestroyVarSection(&VS);
    CloseFile(&FM);  
    QuitFileManager(&FM); 
}

enum eVarSectionValueType GetValType(char* Section)
{
    if (strstr(Section, "(Vector2)") != NULL)
        return VAR_SECTION_VALUE_TYPE_VEC2;
    else if (strstr(Section, "(float)") != NULL)
        return VAR_SECTION_VALUE_TYPE_FLOAT;

    return VAR_SECTION_VALUE_TYPE_NONE;
}