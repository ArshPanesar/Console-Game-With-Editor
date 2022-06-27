#include "Text.h"

/* TEXT DATA */

// Digits
char* txt_0 =   "   0000   \n"\
                "000    000\n"\
                "000    000\n"\
                "000    000\n"\
                "000    000\n"\
                "000    000\n"\
                "   0000   \n";

char* txt_1 =   "   111 \n"\
                "  1111 \n"\
                "11 111 \n"\
                "   111 \n"\
                "   111 \n"\
                "   111 \n"\
                "1111111\n";

char* txt_2 =   "   2222  \n"\
                "2222  222\n"\
                "      222\n"\
                "   2222  \n"\
                "2222     \n"\
                "222222222\n";

char* txt_3 =   "3333   \n"\
                "   333 \n"\
                "     33\n"\
                "  3333 \n"\
                "     33\n"\
                "   333 \n"\
                "3333   \n";


// Text Variables

#define TOTAL_NUM_OF_CHARS 4

int Ready = 0;

char* TextList[TOTAL_NUM_OF_CHARS];

int TextNumOfColsList[TOTAL_NUM_OF_CHARS]; 
int TextNumOfRowsList[TOTAL_NUM_OF_CHARS]; 

int TextLargestNumOfCols = 0;
int TextLargestNumOfRows = 0;

/* TEXT FUNCTIONS */

int AddTextToBuffer(char Buffer[], int BufferRows, int BufferCols, int TextIndex, int Spacing, int* pStartIndex, int IsLastTxt);

int InitText(void)
{
    if (Ready > 0)
    {
        Log_Warn("InitText() Warning: Text Already Initialized");
        return 0;
    }
    else
    {
        Log_Info("InitText(): Initializing Text -> Digits and Letters");
    }

    // Adding all Text to Lists
    //    
    // Digits
    TextList[0] = txt_0;
    TextList[1] = txt_1;
    TextList[2] = txt_2;
    TextList[3] = txt_3;

    // Calculating Largest Num of Rows and Cols
    for (int i = 0; i < TOTAL_NUM_OF_CHARS; ++i)
    {
        Vector2 Size = CalculateCharBufferSize(TextList[i], '\n');
        Size.x += 1;

        TextNumOfColsList[i] = Size.x;
        TextNumOfRowsList[i] = Size.y;
        TextLargestNumOfCols = (TextLargestNumOfCols < Size.x) ? Size.x : TextLargestNumOfCols;
        TextLargestNumOfRows = (TextLargestNumOfRows < Size.y) ? Size.y : TextLargestNumOfRows;
    }

    Ready = 1;
}

int ConvertTextToSprite(Sprite* pSpr, const char* Str, int Spacing)
{
    if (Ready < 1)
    {
        Log_Warn("ConvertTextToSprite(): Text Not Yet Been Initialized!");
        return 0;
    }
    else if (pSpr == NULL)
    {
        Log_Warn("ConvertTextToSprite() Warning: Sprite is NULL");
        return 0;
    }

    if (Spacing < 0)
    {
        Log_Warn("ConvertTextToSprite() Warning: Spacing=%d Cannot be Negative. Resetting to 0.", Spacing);
        Spacing = 0;
    }


    int Len = strlen(Str);

    // Temp Buffer
    int BufferSize = (TextLargestNumOfCols * TextLargestNumOfRows * Len) + Spacing + 1;
    char TempBuffer[BufferSize];

    for (int i = 0; i < BufferSize; ++i)
        TempBuffer[i] = ' ';
    TempBuffer[BufferSize - 1] = '\0';

    //Log_Info("Buffer Size: %d", BufferSize);

    // Adding Text to Buffer
    int StartIndex = 0;
    for (int i = 0; i < Len; ++i)
    {
        int c = (int)Str[i];

        //Log_Info("%c", Str[i]);
        //Log_Info(" %d", c);
        
        // Is c a Digit?
        if (c >= 48 && c <= 57)
            AddTextToBuffer(TempBuffer, TextLargestNumOfRows, (TextLargestNumOfCols * Len), c - 48, Spacing, &StartIndex, (i == Len - 1));
    }

    //Log_Info("%s", TempBuffer);

    DestroySprite(pSpr);

    int NewBufferSize = (StartIndex * TextLargestNumOfRows * Len) + Spacing + 1;
    char NewBuffer[NewBufferSize];
    for (int i = 0; i < NewBufferSize; ++i)
        NewBuffer[i] = ' ';
    NewBuffer[NewBufferSize - 1] = '\0';
    
    int j = 0, row = 1;
    for (int i = 0; i < BufferSize; ++i)
    {
        if (i != 0 && TempBuffer[i - 1] == '\n')
        {
            i = i + (TextLargestNumOfCols * Len * row - i);
            ++row;
        }

        NewBuffer[j++] = TempBuffer[i];
    }

    NewBuffer[j] = '\0';

    SetSprite(pSpr, NewBuffer, '\n');
}

int AddTextToBuffer(char Buffer[], int BufferRows, int BufferCols, int TextIndex, int Spacing, int* pStartIndex, int IsLastTxt)
{
    int StartIndex = *pStartIndex;
    if (StartIndex >= BufferCols)
    {
        Log_Warn("AddTextToBuffer() Warning: StartIndex=%d is >= BufferCols=%d", StartIndex, BufferCols);
        return 0;
    }
    else if (TextIndex >= TOTAL_NUM_OF_CHARS)
    {
        Log_Warn("AddTextToBuffer() Warning: TextIndex=%d >= TOTAL_NUM_OF_CHARS=%d", TextIndex, TOTAL_NUM_OF_CHARS);
        return 0;
    }


    int rows = TextNumOfRowsList[TextIndex];
    int cols = TextNumOfColsList[TextIndex];

    //Log_Info("Rows: %d", rows);
    //Log_Info("Cols: %d", cols);
    //Log_Info("TextIndex: %d", TextIndex);
    //Log_Info("StartIndex: %d", StartIndex);

    char* txt = TextList[TextIndex];
    int BufferIndex = 0;
    //Log_Info("Buffer Cols: %d", BufferCols);
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            //BufferIndex = ((StartIndex + i) * BufferCols) + j;
            BufferIndex = (i * BufferCols) + j + StartIndex;
            int Index = (i * cols) + j;

            //Log_Info("Txt Index: %d", Index);
            if (txt[Index] == '\n' && !IsLastTxt)
                continue;

            //Log_Info("At Row=%d, BufferIndex: %d, Char: %c", i, BufferIndex, txt[Index]);
            
            //Log_Info("Buffer Index: %d", BufferIndex);
            Buffer[BufferIndex] = txt[Index];
        }
    }

    if (IsLastTxt)
        Buffer[BufferIndex] = '\0';

    *pStartIndex = StartIndex + cols;
    return 1;
}
