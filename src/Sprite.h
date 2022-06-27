#include "Color.h"
#include "Vector2.h"
#include "Log.h"

#ifndef CG_SPRITE_H
#define CG_SPRITE_H

// A Sprite will be represented by Colored ASCII Characters.
typedef struct SpriteStruct
{
	Vector2 Position;
	Vector2 Size;

	// Size.y -> Rows
	// Size.x -> Columns
	char* CharBuffer;
	Color* ColorBuffer;
} Sprite;

// Functions

/* 
*	CharBuffer Can Contain a Delimiter 
*	to Make a New Row and Must Contain
*	the Terminating '\0' Character 
*/
Vector2 CalculateCharBufferSize(char* CharBuffer, char Delim);
int SetSprite(Sprite* pSpr, char* CharBuffer, char Delim);
int FillSpriteColor(Sprite* pSpr, Color* ColorBuffer, int Size);
int FillSpriteSingleColor(Sprite* pSpr, Color SingleColor);
int DestroySprite(Sprite* pSpr);
#endif //CG_SPRITE_H
