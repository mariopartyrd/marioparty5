#ifndef MSMFIO_H
#define MSMFIO_H

#include "dolphin.h"
#include "dolphin/types.h"
#include "msm.h"

struct msmFio {
    MSM_OPEN open;
    MSM_READ read;
    MSM_CLOSE close;
};

BOOL msmFioClose(DVDFileInfo *fileInfo);
BOOL msmFioRead(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset);
BOOL msmFioOpen(s32 entrynum, DVDFileInfo *fileInfo);
void msmFioInit(MSM_OPEN open, MSM_READ read, MSM_CLOSE close);

#endif
