#include "game/dvd.h"
#include "game/data.h"

#include "dolphin/dvd.h"
#include "dolphin/os.h"


static DVDDiskID correctDiskID = {
    { 'M', 'P', 'G', 'C' }, //gameName
    { 'H', 'U' }, //company
    1, //diskNumber 
    1, //gameVersion
    1, //streaming
    0, //streamingBufSize
};


static int CallBackStatus;
static s32 beforeDvdStatus;

static void HuDVDReadAsyncCallBack(s32 result, DVDFileInfo* fileInfo)
{
    CallBackStatus = 1;
}

static void *HuDvdDataReadWait(DVDFileInfo *file, int mode, int param, BOOL async)
{
    u32 len;
    void *buf;
    len = file->length;
    DirDataSize = len;
    switch(mode) {
        case 0:
            buf = HuMemDirectMalloc(HEAP_DVD, OSRoundUp32B(len));
            break;
            
        case 1:
            buf = HuMemDirectMallocNum(HEAP_DVD, OSRoundUp32B(len), param);
            break;
            
        case 2:
            buf = HuMemDirectMalloc(param, OSRoundUp32B(len));
            break;
         
        case 3:
            buf = HuMemDirectTailMalloc(param, OSRoundUp32B(len));
            break;
        
        default:
            OSPanic("dvd.c", 70, "dvd.c: HuDvdDataReadWait Mode Error");
            break;
    }
    if(!buf) {
        OSReport("dvd.c: Memory Allocation Error (Length %x) (mode %d)\n", len, mode);
        OSReport("Rest Memory %x\n", HuRestMemGet(HEAP_DVD));
        OSPanic("dvd.c", 75, "\n");
        return NULL;
    }
    OSReport("Rest Memory %x\n", HuRestMemGet(HEAP_DVD));
    if(async) {
        if(len > HU_DVD_BLOCKSIZE) {
            len = HU_DVD_BLOCKSIZE;
        }
        DVDReadAsyncPrio(file, buf, OSRoundUp32B(len), 0, HuDataDirReadAsyncCallBack, 3);
    } else {
        u32 readSize;
        u32 readOfs;
        for(readOfs=readSize=0; readOfs<len; readOfs += HU_DVD_BLOCKSIZE) {
            readSize = len-readOfs;
            if(readSize > HU_DVD_BLOCKSIZE) {
                readSize = HU_DVD_BLOCKSIZE;
            }
            CallBackStatus = 0;
            DVDReadAsyncPrio(file, ((u8 *)buf)+readOfs, OSRoundUp32B(readSize), readOfs, HuDVDReadAsyncCallBack, 2);
            if(!async) {
                while(CallBackStatus == 0) {
                    HuDvdErrorWatch();
                }
                HuDvdErrorWatch();
            }
        }
    }
    
    return buf;
}

void *HuDvdDataRead(char *path)
{
    DVDFileInfo file;
    void *data = NULL;
    if(!DVDOpen(path, &file)) {
        OSPanic("dvd.c", 123, "dvd.c: File Open Error");
    } else {
        data = HuDvdDataReadWait(&file, 0, 0, FALSE);
        DVDClose(&file);
    }
    return data;
}

void **HuDvdDataReadMulti(char **paths)
{
    DVDFileInfo file;
    int i;
    u32 count;
    void **file_ptrs;
    count = 0;
    while(paths[count]) {
        count++;
    }
    file_ptrs = HuMemDirectMalloc(0, count*sizeof(void *));
    for(i=0; i<count; i++) {
        if(!DVDOpen(paths[i], &file)) {
            OSPanic("dvd.c", 159, "dvd.c: File Open Error");
            return NULL;
        } else {
            file_ptrs[i] = HuDvdDataReadWait(&file, 0, 0, FALSE);
            DVDClose(&file);
        }
    }
    return file_ptrs;
}

void *HuDvdDataReadDirect(char *path, HEAPID heap)
{
    DVDFileInfo file;
    void *data = NULL;
    if(!DVDOpen(path, &file)) {
        OSPanic("dvd.c", 178, "dvd.c: File Open Error");
    } else {
        data = HuDvdDataReadWait(&file, 2, heap, FALSE);
        DVDClose(&file);
    }
    return data;
}

void *HuDvdDataFastRead(s32 entrynum)
{
    DVDFileInfo file;
    void *data = NULL;
    if(!DVDFastOpen(entrynum, &file)) {
        OSPanic("dvd.c", 219, "dvd.c: File Open Error");
    } else {
        data = HuDvdDataReadWait(&file, 0, 0, FALSE);
        DVDClose(&file);
    }
    return data;
}

void *HuDvdDataFastReadNum(s32 entrynum, s32 num)
{
    DVDFileInfo file;
    void *data = NULL;
    if(!DVDFastOpen(entrynum, &file)) {
        OSPanic("dvd.c", 234, "dvd.c: File Open Error");
    } else {
        data = HuDvdDataReadWait(&file, 1, num, FALSE);
        DVDClose(&file);
    }
    return data;
}

void *HuDvdDataFastReadAsync(s32 entrynum, HUDATASTAT *stat)
{
    DVDFileInfo file;
    void *data = NULL;
    if(!DVDFastOpen(entrynum, &stat->dvdFile)) {
        OSPanic("dvd.c", 250, "dvd.c: File Open Error");
    } else {
        stat->readOfs = 0;
        stat->readLen = stat->dvdFile.length;
        data = HuDvdDataReadWait(&stat->dvdFile, 0, 0, TRUE);
    }
    return data;
}

void HuDvdDataClose(void *ptr)
{
    if(ptr) {
        HuMemDirectFree(ptr);
    }
}

void HuDvdErrorWatch()
{
    int status = DVDGetDriveStatus();
    if(status == beforeDvdStatus) {
        return;
    }
    beforeDvdStatus = status;
    switch(status+1) {
        case 0:
            OSReport("DVD ERROR:Fatal error occurred\n***HALT***");
            while(1);
            break;
            
        case 5:
            OSReport("DVD ERROR:No disk\n");
            break;
            
        case 6:
            OSReport("DVD ERROR:Cover open\n");
            break;
            
        case 7:
            OSReport("DVD ERROR:Wrong disk\n");
            break;
            
        case 12:
            OSReport("DVD ERROR:Please retry\n");
            break;
            
        default:
            break;
    }
}