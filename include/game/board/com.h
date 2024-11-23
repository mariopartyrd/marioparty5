#ifndef _BOARD_COM_H
#define _BOARD_COM_H

#include "game/gamework.h"

void MBComChoiceSetLeft(void);
void MBComChoiceSetRight(void);
void MBComChoiceSetUp(void);
void MBComChoiceSetDown(void);
int MBComMasuPathNoGet(int playerNo, int linkNum, s16 *linkTbl);

#endif