#ifndef _BOARD_MGCALL_H
#define _BOARD_MGCALL_H

#include "game/board/main.h"

void MBMgCallInit(void);
void MBMgCallExec(void);
void MBMgCallDispSet(BOOL killF);

void MBMgCallKettouExec(void);
void MBMgCallDonkeyExec(void);
void MBMgCallKupaExec(void);
void MBMgCallVsExec(void);

void MBMgCallDataClose(void);
void MBMgCallTutorialExec(void);

#endif