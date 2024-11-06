#ifndef _GAME_ARMEM_H
#define _GAME_ARMEM_H

#include "dolphin.h"
#include "game/memory.h"
#include "game/data.h"

#define HU_AMEM_BASE 0x808000

typedef u32 AMEM_PTR;

void HuARInit(void);
AMEM_PTR HuARMalloc(u32 size);
void HuARFree(AMEM_PTR aMemP);
void HuAMemDump(void);
AMEM_PTR HuAR_DVDtoARAM(unsigned int dir);
AMEM_PTR HuAR_MRAMtoARAM(int dir);
AMEM_PTR HuAR_MRAMtoARAM2(void *dirPtr);
void HuAR_ARAMtoMRAM(AMEM_PTR aMemP);
void *HuAR_ARAMtoMRAMNum(AMEM_PTR aMemP, s32 num);
s32 HuARDMACheck(void);
AMEM_PTR HuARDirCheck(unsigned int dir);
void HuARDirFree(unsigned int dir);
void *HuAR_ARAMtoMRAMFileRead(unsigned int dataNum, u32 num, HUHEAPTYPE heap);


#endif
