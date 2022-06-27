#include "GameEnemyHorde.h"

#ifndef CG_FILE_MGR_H
#define CG_FILE_MGR_H

/* DATA */

// File Limit
#define FM_MAX_FILES                50
#define FM_DEFAULT_WORKING_DIR      "resources"

#define FM_MAX_CHARS_IN_FILE_NAME   64
#define FM_MAX_CHARS_IN_FILE_EXT    16

#define FM_WRITE_MODE   "w"
#define FM_READ_MODE    "r"

// Horde File
#define HF_DIRECTORY FM_DEFAULT_WORKING_DIR
#define HF_EXTENSION ".hordefile"

// Var File
#define VF_DIRECTORY        FM_DEFAULT_WORKING_DIR
#define VF_EXTENSION        ".varfile"
#define VF_MAX_SECTIONS     16
#define VF_MAX_KV_PAIRS     16
#define VF_MAX_SECTION_LEN  64
#define VF_MAX_KV_PAIRS_LEN 64

typedef struct FileManagerStruct
{
    // File Stuff
    FILE* File;
    char* FileName;
    char* FileExt;

    // Directory Stuff
    char* WorkingDirectory;
    int WorkingDirectoryLength;
    char* ScannedFilesInDir[FM_MAX_FILES];

    // Gives Number of Scanned Files in Working Directory
    int NumOfFiles; 
    
    // Flags
    short FileOpenFlag;
} FileManager;

/* FUNCTIONS */

// File Manager Operations
int InitFileManager(FileManager* FM);
int QuitFileManager(FileManager* FM);

// File Operations
//
// Set the Working Directory
int SetWorkingDirectory(FileManager* FM, char* Directory);

// Checks If a File Exists in Working Directory
int CheckFileExists(FileManager* FM, char* FileName);

// Scans Working Directory For Files. If Extension is NULL, It Searches All Files Within Limit and Fills Output.
// Returns Number of Files Found, if Error then -1.
int ScanDirectory(FileManager* FM, char* Extension, int Limit);

// Searches for the Given File in the Scanned File List.
// Returns the File if Found, Otherwise NULL
char* SearchFileInDirectory(FileManager* FM, char* FileName);

// File Operations
int OpenFileFromName(FileManager* FM, char* FileName, char* Extension, char* Mode);
int CloseFile(FileManager* FM);

// Horde File Function Declarations
//
// Opens or Creates a Horde File
int WriteToHordeFile(FileManager* FM, EnemyHorde* Horde);
int ReadFromHordeFile(FileManager* FM, EnemyHorde* Horde, SpawnRecord* pSR);

// Variable File
//
// Section Struct
typedef struct VarSectionStruct
{
    char KeyValList[VF_MAX_KV_PAIRS][VF_MAX_KV_PAIRS_LEN];
    char Section[VF_MAX_SECTION_LEN];
    
    union Value
    {
        Vector2* pVecValueList;
        float* pFloatValueList;
    };

    enum eVarSectionValueType
    {
        VAR_SECTION_VALUE_TYPE_NONE,
        VAR_SECTION_VALUE_TYPE_VEC2,
        VAR_SECTION_VALUE_TYPE_FLOAT
    } ValueType;

    int Size;
} VarSection;

// VarSection Functions
int CreateVarSection(VarSection* pVS, int Size, enum eVarSectionValueType ValType);
int DestroyVarSection(VarSection* pVS);

// Variables File Functions
//
// Returns Number of Sections Found, Otherwise -1 on Failure
int ReadAllSectionsFromVarFile(FileManager* FM, char SectionList[VF_MAX_SECTIONS][VF_MAX_SECTION_LEN], enum eVarSectionValueType ValType);
// Fills pVS with Key/Vals Found in Section. Section should Start with #. Returns -1 on Failure.
int ReadSectionFromVarFile(FileManager* FM, char* Section, VarSection* pVS);
int TEST_VarFileOperations(void);

#endif //CG_FILE_MGR_H
