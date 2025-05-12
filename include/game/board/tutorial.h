#ifndef _BOARD_TUTORIAL_H
#define _BOARD_TUTORIAL_H

#include "game/board/main.h"
#include "datanum/w10.h"

#define TUTORIAL_INST_TURN 0
#define TUTORIAL_INST_TURNEND 1
#define TUTORIAL_INST_SAI 2
#define TUTORIAL_INST_WALK_START 3
#define TUTORIAL_INST_CAPSULE 4
#define TUTORIAL_INST_MASU_CAPSULE 5
#define TUTORIAL_INST_WALKEND 6
#define TUTORIAL_INST_WALKEND_EVENT 7
#define TUTORIAL_INST_PATH_CHOICE 8
#define TUTORIAL_INST_SAI_TURN_END 9
#define TUTORIAL_INST_SAI_HIDDEN 10
#define TUTORIAL_INST_SAI_CAPSULE_SET 11
#define TUTORIAL_INST_SAI_CAPSULE_USE 12
#define TUTORIAL_INST_MAX 13

void MBTutorialWatchProcCreate(void);
BOOL MBTutorialExec(int instNo);
BOOL MBTutorialExitFlagGet(void);
s16 MBTutorialSprDispOn(unsigned int dataNum);
void MBTutorialSprDispOff(s16 sprId);

#endif