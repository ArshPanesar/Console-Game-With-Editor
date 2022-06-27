#include "Sprite.h"
#include "Log.h"

#ifndef CG_TERMINAL_H
#define CG_TERMINAL_H

/* FUNCTION DECLARATIONS */
// Configuring the Terminal For the Game
int InitTerminal(void);
int QuitTerminal(void);

// Setting Terminal Size
int SetTerminalSize(Vector2 Size);
Vector2 GetTerminalSize(void);

// Terminal Rendering
void DrawSpriteToTerminal(Sprite* pSprite);
void DisplayTerminal(void);
void ClearTerminal(void);

#endif //CG_TERMINAL_H
