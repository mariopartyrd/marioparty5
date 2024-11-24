#ifndef _BOARD_PAUSE_H
#define _BOARD_PAUSE_H

#include "game/object.h"
#include "datanum/bpause.h"

typedef void (*MBPAUSEHOOK)(BOOL dispF);

//Translate data numbers in bpause.bin
#define MBPAUSEDATANUM(dataNum) dataNum

void MBPauseInit(void);
void MBPauseProcCreate(int playerNo);
int MBPauseStartCheck(void);
BOOL MBPauseProcCheck(void);
void MBPauseDisableSet(BOOL disableF);
BOOL MBPauseDisableGet(void);
void MBPauseHookPush(MBPAUSEHOOK hook);
void MBPauseHookPop(MBPAUSEHOOK hook);
void MBPauseSet(BOOL pauseF);

BOOL MBPauseOptionExec(int playerNo);
void MBPauseDispCopyCreate(void);
void MBPauseDispCopyKill(void);

#endif