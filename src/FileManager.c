#include "FileManager.h"

// Writing to File Stream
int WriteEntry(FILE* File, const char* Token, char* Data, int Length);

// Reading From File Stream
int ReadEntry(FILE* File, const char* Token, char* Data, int Length);

int ClearScannedFilesList(FileManager* FM);

/* GLOBAL FUNCTION DEFINITIONS */
int InitFileManager(FileManager* FM)
{
    if (FM == NULL)
    {
        Log_Critical("InitFileManager(): File Manager is NULL");
        return 0;
    }

    // Initializing File Manager
    FM->File = NULL;
    FM->FileName = NULL;
    FM->FileExt = NULL;
    FM->NumOfFiles = 0;
    FM->WorkingDirectory = NULL;
    FM->WorkingDirectoryLength = 0;
    
    int BufferSize = FM_MAX_CHARS_IN_FILE_NAME + FM_MAX_CHARS_IN_FILE_EXT + 1;
    for (int i = 0; i < FM_MAX_FILES; ++i)
        FM->ScannedFilesInDir[i] = (char*)malloc(sizeof(char) * BufferSize);
    ClearScannedFilesList(FM);

    FM->FileOpenFlag = 0;

    // Scan Directory
    SetWorkingDirectory(FM, FM_DEFAULT_WORKING_DIR);
}

int QuitFileManager(FileManager* FM)
{
    if (FM == NULL)
    {
        Log_Critical("QuitFileManager(): File Manager is NULL");
        return 0;
    }

    // Close any Open File
    if (FM->FileOpenFlag)
        CloseFile(FM);
    
    FM->NumOfFiles = 0;
    for (int i = 0; i < FM_MAX_FILES; ++i)
    {
        free(FM->ScannedFilesInDir[i]);
        FM->ScannedFilesInDir[i] = NULL;
    }
}

int SetWorkingDirectory(FileManager* FM, char* Directory)
{
    if (FM == NULL)
    {
        Log_Critical("SetWorkingDirectory(): File Manager is NULL");
        return 0;
    }

    int Len = strnlen(Directory, FM_MAX_CHARS_IN_FILE_NAME);
    if (FM->WorkingDirectoryLength == FM_MAX_CHARS_IN_FILE_NAME)
    {
        Log_Warn("SetWorkingDirectory(): Directory Name Length Exceeded.\nDirectory: %s, Length: %d", Directory, Len);
        return 0;
    }

    FM->WorkingDirectory = Directory;
    FM->WorkingDirectoryLength = Len;

    ScanDirectory(FM, NULL, FM_MAX_FILES);

    return 1;
}

int CheckFileExists(FileManager* FM, char* FileName)
{
    if (FM == NULL)
    {
        Log_Critical("CheckFileExists(): File Manager is NULL");
        return 0;
    }
    else if (FileName == NULL)
    {
        Log_Warn("CheckFileExists(): FileName is NULL");
        return 0;
    }

    for (int i = 0; i < FM->NumOfFiles; ++i)
    {
        if (FM->ScannedFilesInDir[i] == FileName)
            return 1;
    }

    return 0;
}

int ScanDirectory(FileManager* FM, char* Extension, int Limit)
{
    char* Directory = FM->WorkingDirectory;
    if (Directory == NULL)
    {
        Log_Warn("ScanDirectory(): Directory is NULL");
        return -1;
    }
    else if (Limit < 1 && Limit > FM_MAX_FILES)
    {
        Log_Warn("ScanDirectory(): Limit is Invalid. Limit=%d", Limit);
        return -1;
    }

    int NumOfFiles = 0;
    ClearScannedFilesList(FM);

#ifdef CG_PLATFORM_WINDOWS
    char DirBuffer[FM_MAX_CHARS_IN_FILE_NAME + FM_MAX_CHARS_IN_FILE_EXT];
    int BufferSize = FM_MAX_CHARS_IN_FILE_NAME + FM_MAX_CHARS_IN_FILE_EXT;
    //for (int i = 0; i < BufferSize; ++i)
    //    DirBuffer[i] = '\0';

    strncpy(DirBuffer, Directory, BufferSize);
    strcat(DirBuffer, "\\*");
    /*if (Extension != NULL && Extension != "")
    {
        //strcat(DirBuffer, ".");
        strcat(DirBuffer, Extension);
    }*/

    Log_Info("Scanning Directory (Platform -> Windows): %s", DirBuffer);    
    // Scanning Windows Directory
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    hFind = FindFirstFileA(DirBuffer, &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        Log_Warn("ScanDirectory() (Platform -> Windows): Windows Error: %s", GetWindowsErrorMessage(GetLastError()));
        return -1;
    }
    
    //strcpy(FM->ScannedFilesInDir[NumOfFiles++], ffd.cFileName);
    while (FindNextFileA(hFind, &ffd) != 0 && NumOfFiles < Limit)
    {   
        if (hFind != INVALID_HANDLE_VALUE)
        {
            if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0)
            {
                // If Extension is Given, Only Look For Those Files!
                if (Extension == NULL || (Extension != NULL && strstr(ffd.cFileName, Extension) != NULL))
                    strcpy(FM->ScannedFilesInDir[NumOfFiles++], ffd.cFileName);
            }
        }
        else
        {
            Log_Warn("ScanDirectory() (Platform -> Windows): Could Not Get File Name. Windows Error: %s. Last File Scanned: %s", 
                    GetWindowsErrorMessage(GetLastError()), FM->ScannedFilesInDir[NumOfFiles - 1]);
        }
    } 
    
#endif

    /*Log_Info("Scan Complete. Files Found: %d", NumOfFiles);
    for (int i = 0; i < NumOfFiles; ++i)
    {
        Log_Info("%s", FM->ScannedFilesInDir[i]);
    }*/

    FM->NumOfFiles = NumOfFiles;
    return NumOfFiles;
}

char* SearchFileInDirectory(FileManager* FM, char* FileName)
{
    if (FM == NULL)
    {
        Log_Critical("SearchFileInDirectory(): File Manager is NULL. FileName: %s", FileName);
        return 0;
    }

    for (int i = 0; i < FM->NumOfFiles; ++i)
    {
        if (strstr(FM->ScannedFilesInDir[i], FileName) != NULL)
            return FM->ScannedFilesInDir[i];
    }

    return NULL;
}

int OpenFileFromName(FileManager* FM, char* FileName, char* Extension, char* Mode)
{
    if (FM == NULL)
    {
        Log_Critical("OpenFileFromName(): File Manager is NULL. FileName: %s, Extension: %s", FileName, Extension);
        return 0;
    }
    else if (FM->NumOfFiles > FM_MAX_FILES)
    {
        Log_Critical("OpenFileFromName(): File Limit Exceeded. Limit = %d, FileName: %s, Extension: %s", FM_MAX_FILES, FileName, Extension);
        return 0;    
    }

    if (FM->FileOpenFlag)
    {
        Log_Warn("OpenFileFromName(): A File is Already Open. Closing That File => Name: %s, Extension: %s", FM->FileName, FM->FileExt);
        CloseFile(FM);
    }

    int NameLen = strnlen(FileName, FM_MAX_CHARS_IN_FILE_NAME);
    //int ExtLen = strnlen(Extension, FM_MAX_CHARS_IN_FILE_EXT);
    int ExtLen = 0;
    if (Extension != NULL)
        ExtLen = strnlen(Extension, FM_MAX_CHARS_IN_FILE_EXT);
    if (NameLen == FM_MAX_CHARS_IN_FILE_NAME || ExtLen == FM_MAX_CHARS_IN_FILE_EXT)
    {
        Log_Critical("OpenFileFromName(): File Not Created/Opened Since Name or Extension Exceeds Maximum Values. \n"\
                     "Name: %s, Extension: %s\nName Length: %d, Ext Length: %d\nMax Name Length: %d, Max Ext Length: %d",
                     FileName, Extension, NameLen, ExtLen, FM_MAX_CHARS_IN_FILE_NAME, FM_MAX_CHARS_IN_FILE_EXT);
        return 0;
    }

    char NameBuffer[FM->WorkingDirectoryLength + NameLen + ExtLen + 2];
    
    int Index = 0;
    for (int i = 0; i < FM->WorkingDirectoryLength; ++i)
        NameBuffer[Index++] = FM->WorkingDirectory[i];
    NameBuffer[Index++] = '/';
    for (int i = 0; i < NameLen; ++i)
        NameBuffer[Index++] = FileName[i];
    
    if (Extension != NULL)
    {
        for (int i = 0; i < ExtLen; ++i)
            NameBuffer[Index++] = Extension[i];
    }
    NameBuffer[Index] = '\0';
    
    FM->File = fopen(NameBuffer, Mode);
    if(FM->File == NULL)
    {
        Log_Warn("OpenFileFromName(): File Cannot Be Opened. FileName: %s, Mode: %s", NameBuffer, Mode);
        return 0;
    }


    Log_Info("OpenFileFromName(): File Opened. Path: %s", NameBuffer);
    
    // File Successfully Opened
    FM->FileName = FileName;
    FM->FileExt = Extension;
    if (Extension == NULL)
        FM->FileExt = strchr(FileName, '.');
    FM->FileOpenFlag = 1;

    return 1;
}

int CloseFile(FileManager* FM)
{
    if (FM == NULL)
    {
        Log_Critical("CloseFile(): File Manager is NULL");
        return 0;
    }
    else if (!FM->FileOpenFlag)
    {
        Log_Warn("CloseFile(): No File is Open.");
        return 0;
    }
    
    fflush(FM->File);
    fclose(FM->File);

    // File Successfully Closed
    FM->FileOpenFlag = 0;

    Log_Info("CloseFile(): File Closed. Name: %s, Extension: %s", FM->FileName, FM->FileExt);
    FM->File = NULL;
    FM->FileName = FM->FileExt = NULL;

    return 1;
}

/* LOCAL FUNCTION DEFINITIONS */

int ClearScannedFilesList(FileManager* FM)
{
    int BufferSize = FM_MAX_CHARS_IN_FILE_NAME + FM_MAX_CHARS_IN_FILE_EXT + 1;
    for (int i = 0; i < FM_MAX_FILES; ++i)
    {
        for (int j = 0; j < BufferSize; ++j)
            FM->ScannedFilesInDir[i][j] = '\0'; 
    }
}