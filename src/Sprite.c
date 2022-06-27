#include "Sprite.h"

Vector2 CalculateCharBufferSize(char* CharBuffer, char Delim)
{
    if (CharBuffer == NULL)
    {
        Log_Warn("CalcCharBufferSize() Warning: Char Buffer Was NULL");
        return CreateVec2(0, 0);
    }

    int Len = strlen(CharBuffer);
    int MaxLenBetweenDelim = 0, LenBetweenDelim = 0;
    int NumOfDelim = 0;
    for (int i = 0; i < Len; ++i)
    {
        if (CharBuffer[i] == Delim || i == Len - 1)
        {
            ++NumOfDelim;
            if (LenBetweenDelim > MaxLenBetweenDelim)
                MaxLenBetweenDelim = LenBetweenDelim;
            LenBetweenDelim = 0;
        }
        else
            ++LenBetweenDelim;
    }

    return CreateVec2(MaxLenBetweenDelim, NumOfDelim);
}

int SetSprite(Sprite* pSpr, char* CharBuffer, char Delim)
{
    if(pSpr == NULL)
    {
        Log_Warn("SetSprite() Warning: Sprite Was NULL");
        return 0;
    }

    Vector2 Size = CalculateCharBufferSize(CharBuffer, Delim);
    int Len = strlen(CharBuffer);
    int NumOfRows = Size.y;
    int NumOfCols = Size.x;
    
    pSpr->CharBuffer = (char*)malloc(sizeof(char) * NumOfRows * NumOfCols);
    pSpr->ColorBuffer = (Color*)malloc(sizeof(Color) * NumOfRows * NumOfCols);
    pSpr->Size = (Vector2){ NumOfCols, NumOfRows };

    //Log_Info("Number of Rows: %d\nNumber of Columns: %d", NumOfRows, NumOfCols);
    
    int Index = 0, StrIndex = 0;
    for (int i = 0; i < NumOfRows; ++i)
    {
        for (int j = 0; j < NumOfCols; ++j)
        {
            Index = (i * NumOfCols) + j;

            if (CharBuffer[StrIndex] == Delim || StrIndex >= Len) //Fill Empty Spaces
                pSpr->CharBuffer[Index] = ' ';
            else
            {
                pSpr->CharBuffer[Index] = CharBuffer[StrIndex++];
                pSpr->ColorBuffer[Index] = CG_COLOR_DARK_RED;
            }
            //Log_Info("StrIndex = %d, Index = %d, Character = %c", StrIndex, Index, pSpr->CharBuffer[Index]);
            
        }
        ++StrIndex;
    }

    return 1;
}

int DestroySprite(Sprite* pSpr)
{
    /*if(pSpr == NULL || pSpr->CharBuffer == NULL || pSpr->ColorBuffer == NULL)
    {
        Log_Warn("DestroySprite() Warning: Sprite or its Contents Were NULL");
        return 0;
    }*/

    free(pSpr->CharBuffer);
    free(pSpr->ColorBuffer);

    pSpr->Size = CreateVec2(0, 0);
    pSpr->Position = CreateVec2(0, 0);
    pSpr->CharBuffer = NULL;
    pSpr->ColorBuffer = NULL;

    return 1;
}

int FillSpriteColor(Sprite* pSpr, Color* ColorBuffer, int Size)
{
    if(pSpr == NULL || pSpr->CharBuffer == NULL || pSpr->ColorBuffer == NULL)
    {
        Log_Warn("FillSpriteColor() Warning: Sprite or its Contents Were NULL");
        return 0;
    }
    
    int Index = 0;
    for (int i = 0; i < pSpr->Size.x * pSpr->Size.y; ++i)
    {
        pSpr->ColorBuffer[i] = ColorBuffer[Index];
        Index = (Index + 1 < Size) ? Index + 1 : 0;
    }
}

int FillSpriteSingleColor(Sprite* pSpr, Color SingleColor)
{
    if(pSpr == NULL || pSpr->CharBuffer == NULL || pSpr->ColorBuffer == NULL)
    {
        Log_Warn("FillSpriteColor() Warning: Sprite or its Contents Were NULL");
        return 0;
    }

    for (int i = 0; i < pSpr->Size.x * pSpr->Size.y; ++i)
        pSpr->ColorBuffer[i] = SingleColor;

    return 1;
}
