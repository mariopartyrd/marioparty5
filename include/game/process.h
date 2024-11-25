#ifndef _GAME_PROCESS_H
#define _GAME_PROCESS_H

#include "game/jmp.h"
#include "dolphin/types.h"

#define HU_PRC_STAT_PAUSE 0x1
#define HU_PRC_STAT_UPAUSE 0x2
#define HU_PRC_STAT_PAUSE_ON 0x4
#define HU_PRC_STAT_UPAUSE_ON 0x8


#define HUPRC_EXEC_NORMAL 0
#define HUPRC_EXEC_SLEEP 1
#define HUPRC_EXEC_CHILDWATCH 2
#define HUPRC_EXEC_KILLED 3

typedef struct Process_s HUPROCESS;

struct Process_s {
    HUPROCESS *next;
    HUPROCESS *prev;
    HUPROCESS *child;
    HUPROCESS *parent;
    HUPROCESS *nextChild;
    HUPROCESS *firstChild;
    void *heap;
    u16 exec;
    u16 stat;
    u16 prio;
    s32 sleep;
    u32 spBase;
    jmp_buf jump;
    void (*destructor)(void);
    void *property;
};

void HuPrcInit(void);
void HuPrcEnd(void);
HUPROCESS *HuPrcCreate(void (*func)(void), u16 prio, u32 stackSize, s32 extraSize);
void HuPrcChildLink(HUPROCESS *parent, HUPROCESS *child);
void HuPrcChildUnlink(HUPROCESS *process);
HUPROCESS *HuPrcChildCreate(void (*func)(void), u16 prio, u32 stackSize, s32 extraSize, HUPROCESS *parent);
void HuPrcChildWatch(void);
HUPROCESS *HuPrcCurrentGet(void);
s32 HuPrcKill(HUPROCESS *process);
void HuPrcChildKill(HUPROCESS *process);
void HuPrcSleep(s32 time);
void HuPrcVSleep();
void HuPrcWakeup(HUPROCESS *process);
void HuPrcDestructorSet2(HUPROCESS *process, void (*func)(void));
void HuPrcDestructorSet(void (*func)(void));
void HuPrcCall(s32 tick);
void *HuPrcMemAlloc(s32 size);
void HuPrcMemFree(void *ptr);
void HuPrcSetStat(HUPROCESS *process, u16 value);
void HuPrcResetStat(HUPROCESS *process, u16 value);
void HuPrcAllPause(s32 flag);
void HuPrcAllUPause(s32 flag);

#endif