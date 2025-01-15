#ifndef _GAME_DATA_H
#define _GAME_DATA_H

#include "game/memory.h"

#include "datadir_enum.h"

#define HU_DECODE_TYPE_NONE 0
#define HU_DECODE_TYPE_LZ 1
#define HU_DECODE_TYPE_SLIDE 2
#define HU_DECODE_TYPE_FSLIDE_ALT 3
#define HU_DECODE_TYPE_FSLIDE 4
#define HU_DECODE_TYPE_RLE 5
#define HU_DECODE_TYPE_ZLIB 7

#define HU_DATANUM_NONE -1
#define HU_DATA_STAT_NONE -1

#include "dolphin/types.h"
#include "dolphin/dvd.h"


typedef struct HuDataStat_s {
    s32 dirId;
    void *dirP;
    void *fileDataP;
    u32 rawLen;
    u32 readLen;
    u32 readOfs;
    u32 decodeType;
    BOOL used;
    s32 num;
    u32 status;
    DVDFileInfo dvdFile;
} HUDATASTAT;

void HuDataInit(void);
s32 HuDataReadChk(int dataNum);
HUDATASTAT *HuDataGetStatus(void *dirP);
void *HuDataGetDirPtr(int dirNum);
HUDATASTAT *HuDataDirRead(int dirNum);
HUDATASTAT *HuDataDirReadNum(int dirNum, s32 num);
HUDATASTAT *HuDataDirSet(void *dirP, int dataNum);
void HuDataDirReadAsyncCallBack(s32 result, DVDFileInfo* fileInfo);
s32 HuDataDirReadAsync(int dataNum);
s32 HuDataDirReadNumAsync(int dataNum, s32 num);
BOOL HuDataGetAsyncStat(s32 statId);
void *HuDataRead(int dataNum);
void *HuDataReadNum(int dataNum, s32 num);
void *HuDataSelHeapRead(int dataNum, HEAPID heap);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
void **HuDataReadMulti(int *dataNum);
s32 HuDataGetSize(int dataNum);
void HuDataClose(void *ptr);
void HuDataCloseMulti(void **ptrTbl);
void HuDataDirClose(int dataNum);
void HuDataDirCloseNum(s32 num);
void *HuDataReadNumHeapShortForce(int dataNum, s32 num, HEAPID heap);

void HuDecodeData(void *src, void *dst, u32 size, s32 decode_type);

extern u32 DirDataSize;

#endif