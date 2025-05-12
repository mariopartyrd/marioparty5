#ifndef _BOARD_PAD_H
#define _BOARD_PAD_H

#include "game/pad.h"

u16 MBPadDStkRepGetAll();
u16 MBPadBtnDownGetAll();
s8 MBPadStkXGet(int padNo);
s8 MBPadStkYGet(int padNo);

#define MBPadBtn(padNo) (HuPadBtn[padNo])
#define MBPadBtnDown(padNo) (HuPadBtnDown[padNo])
#define MBPadBtnRep(padNo) (HuPadBtnRep[padNo])
#define MBPadTrigL(padNo) (HuPadTrigL[padNo])
#define MBPadTrigR(padNo) (HuPadTrigR[padNo])

#endif