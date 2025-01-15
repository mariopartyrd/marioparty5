#include "game/process.h"
#include "game/memory.h"
#include "dolphin/os.h"

#define FAKE_RETADDR 0xA5A5A5A5


static jmp_buf processjmpbuf;
static HUPROCESS *processtop;
static HUPROCESS *processcur;
static u16 processcnt;
u32 procfunc;

void HuPrcInit(void)
{
    processcnt = 0;
    processtop = NULL;
}

static void LinkProcess(HUPROCESS **root, HUPROCESS *process) {
    HUPROCESS *src_process = *root;

    if (src_process && (src_process->prio >= process->prio)) {
        while (src_process->next && src_process->next->prio >= process->prio) {
            src_process = src_process->next;
        }

        process->next = src_process->next;
        process->prev = src_process;
        src_process->next = process;
        if (process->next) {
            process->next->prev = process;
        }
    } else {
        process->next = (*root);
        process->prev = NULL;
        *root = process;
        if (src_process) {
            src_process->prev = process;
        }
    }
}
static void UnlinkProcess(HUPROCESS **root, HUPROCESS *process) {
    if (process->next) {
        process->next->prev = process->prev;
    }
    if (process->prev) {
        process->prev->next = process->next;
    } else {
        *root = process->next;
    }
}

HUPROCESS *HuPrcCreate(void (*func)(void), u16 prio, u32 stackSize, s32 heapSize)
{
    HUPROCESS *process;
    s32 allocSize;
    void *heap;
    if(stackSize == 0) {
        stackSize = 2048;
    }
    allocSize = HuMemMemoryAllocSizeGet(sizeof(HUPROCESS))
                    +HuMemMemoryAllocSizeGet(stackSize)
                    +HuMemMemoryAllocSizeGet(heapSize);
    if(!(heap = HuMemDirectMalloc(HEAP_HEAP, allocSize))) {
        OSReport("process> malloc error size %d\n", allocSize);
        return NULL;
    }
    HuMemHeapInit(heap, allocSize);
    process = HuMemMemoryAlloc(heap, sizeof(HUPROCESS), FAKE_RETADDR);
    process->heap = heap;
    process->exec = HUPRC_EXEC_NORMAL;
    process->stat = 0;
    process->prio = prio;
    process->sleep = 0;
    process->spBase = ((u32)HuMemMemoryAlloc(heap, stackSize, FAKE_RETADDR))+stackSize-8;
    gcsetjmp(&process->jump);
    process->jump.lr = (u32)func;
    process->jump.sp = process->spBase;
    process->destructor = NULL;
    process->property = NULL;
    LinkProcess(&processtop, process);
    process->child = NULL;
    process->parent = NULL;
    processcnt++;
    return process;
}

void HuPrcChildLink(HUPROCESS *parent, HUPROCESS *child)
{
    HuPrcChildUnlink(child);
    if(parent->child) {
        parent->child->firstChild = child;
    }
    child->nextChild = parent->child;
    child->firstChild = NULL;
    parent->child = child;
    child->parent = parent;
}

void HuPrcChildUnlink(HUPROCESS *process)
{
    if(process->parent) {
        if(process->nextChild) {
            process->nextChild->firstChild = process->firstChild;
        }
        if(process->firstChild) {
            process->firstChild->nextChild = process->nextChild;
        } else {
            process->parent->child = process->nextChild;
        }
        process->parent = NULL;
    }
}

HUPROCESS *HuPrcChildCreate(void (*func)(void), u16 prio, u32 stackSize, s32 heapSize, HUPROCESS *parent)
{
    HUPROCESS *child = HuPrcCreate(func, prio, stackSize, heapSize);
    HuPrcChildLink(parent, child);
    return child;
}

void HuPrcChildWatch()
{
    HUPROCESS *curr = HuPrcCurrentGet();
    if(curr->child) {
        curr->exec = HUPRC_EXEC_CHILDWATCH;
        if(!gcsetjmp(&curr->jump)) {
            gclongjmp(&processjmpbuf, 1);
        }
    }
}

HUPROCESS *HuPrcCurrentGet()
{
    return processcur;
}

static s32 SetKillStatusProcess(HUPROCESS *process)
{
    if(process->exec != HUPRC_EXEC_KILLED) {
        HuPrcWakeup(process);
        process->exec = HUPRC_EXEC_KILLED;
        return 0;
    } else {
        return -1;
    }
}

s32 HuPrcKill(HUPROCESS *process)
{
    if(process == NULL) {
        process = HuPrcCurrentGet();
    }
    HuPrcChildKill(process);
    HuPrcChildUnlink(process);
    return SetKillStatusProcess(process);
}

void HuPrcChildKill(HUPROCESS *process)
{
    HUPROCESS *child = process->child;
    while(child) {
        if(child->child) {
            HuPrcChildKill(child);
        }
        SetKillStatusProcess(child);
        child = child->nextChild;
    }
    process->child = NULL;
}

static void gcTerminateProcess(HUPROCESS *process)
{
    if(process->destructor) {
        process->destructor();
    }
    UnlinkProcess(&processtop, process);
    processcnt--;
    gclongjmp(&processjmpbuf, 2);
}

void HuPrcEnd()
{
    HUPROCESS *process = HuPrcCurrentGet();
    HuPrcChildKill(process);
    HuPrcChildUnlink(process);
    gcTerminateProcess(process);
}

void HuPrcSleep(s32 time)
{
    HUPROCESS *process = HuPrcCurrentGet();
    if(time != 0 && process->exec != HUPRC_EXEC_KILLED) {
        process->exec = HUPRC_EXEC_SLEEP;
        process->sleep = time;
    }
    if(!gcsetjmp(&process->jump)) {
        gclongjmp(&processjmpbuf, 1);
    }
}

void HuPrcVSleep()
{
    HuPrcSleep(0);
}

void HuPrcWakeup(HUPROCESS *process)
{
    process->sleep = 0;
}

void HuPrcDestructorSet2(HUPROCESS *process, void (*func)(void))
{
    process->destructor = func;
}

void HuPrcDestructorSet(void (*func)(void))
{
    HUPROCESS *process = HuPrcCurrentGet();
    process->destructor = func;
}

void HuPrcCall(s32 tick)
{
    HUPROCESS *process;
    s32 ret;
    processcur = processtop;
    ret = gcsetjmp(&processjmpbuf);
    while(1) {
        switch(ret) {
            case 2:
                HuMemDirectFree(processcur->heap);
            case 1:
                if(((u8 *)(processcur->heap))[4] != 165) {
                    printf("stack overlap error.(process pointer %x)\n", processcur);
                    while(1);
                } else {
                    processcur = processcur->next;
                }
                break;
        }
        process = processcur;
        if(!process) {
            return;
        }
        procfunc = process->jump.lr;
        if((process->stat & (HU_PRC_STAT_PAUSE|HU_PRC_STAT_UPAUSE)) && process->exec != HUPRC_EXEC_KILLED) {
            ret = 1;
            continue;
        }
        switch(process->exec) {
            case HUPRC_EXEC_SLEEP:
                if(process->sleep > 0) {
                    process->sleep -= tick;
                    if(process->sleep <= 0) {
                        process->sleep = 0;
                        process->exec = HUPRC_EXEC_NORMAL;
                    }
                }
                ret = 1;
                break;
                
            case HUPRC_EXEC_CHILDWATCH:
                if(process->child) {
                    ret = 1;
                } else {
                    process->exec = HUPRC_EXEC_NORMAL;
                    ret = 0;
                }
                break;
                
            case HUPRC_EXEC_KILLED:
                process->jump.lr = (u32)HuPrcEnd;
            case HUPRC_EXEC_NORMAL:
                gclongjmp(&process->jump, 1);
                break;
        }
    }
}

void *HuPrcMemAlloc(s32 size)
{
    HUPROCESS *process = HuPrcCurrentGet();
    return HuMemMemoryAlloc(process->heap, size, FAKE_RETADDR);
}

void HuPrcMemFree(void *ptr)
{
    HuMemMemoryFree(ptr, FAKE_RETADDR);
}

void HuPrcSetStat(HUPROCESS *process, u16 value)
{
    process->stat |= value;
}

void HuPrcResetStat(HUPROCESS *process, u16 value)
{
    process->stat &= ~value;
}

void HuPrcAllPause(s32 flag)
{
    HUPROCESS *process = processtop;
    if(flag) {
        while(process != NULL) {
            if(!(process->stat & HU_PRC_STAT_PAUSE_ON)) {
                HuPrcSetStat(process, HU_PRC_STAT_PAUSE);
            }
            
            process = process->next;
        }
    } else {
        while(process != NULL) {
            if(process->stat & HU_PRC_STAT_PAUSE) {
                HuPrcResetStat(process, HU_PRC_STAT_PAUSE);
            }
            
            process = process->next;
        }
    }
}

void HuPrcAllUPause(s32 flag)
{
    HUPROCESS *process = processtop;
    if(flag) {
        while(process != NULL) {
            if(!(process->stat & HU_PRC_STAT_UPAUSE_ON)) {
                HuPrcSetStat(process, HU_PRC_STAT_UPAUSE);
            }
            
            process = process->next;
        }
    } else {
        while(process != NULL) {
            if(process->stat & HU_PRC_STAT_UPAUSE) {
                HuPrcResetStat(process, HU_PRC_STAT_UPAUSE);
            }
            
            process = process->next;
        }
    }
}
