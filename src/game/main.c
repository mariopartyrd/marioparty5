#include "game/main.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/process.h"
#include "game/printfunc.h"
#include "game/perf.h"
#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/sreset.h"
#include "game/saveload.h"
#include "game/frand.h"

extern OVLTBL _ovltbl[];

u32 GlobalCounter;
BOOL HuDvdErrWait;
BOOL HuSRDisableF;
BOOL NintendoDispF;

void main(void)
{
    s16 i;
    s32 retrace;
    s16 temp = 0;
    
    HuSRDisableF = FALSE;
    HuDvdErrWait = 0;
    NintendoDispF = 0;
    HuSysInit(&GXNtsc480IntDf);
    HuPrcInit();
    HuPadInit();
    GWInit();
    pfInit();
    GlobalCounter = 0;
    HuDataInit();
    HuSprInit();
    Hu3DInit();
    SLSaveFlagSet(FALSE);
    HuPerfInit();
    HuPerfCreate("USR0", 0xFF, 0xFF, 0xFF, 0xFF);
    HuPerfCreate("USR1", 0, 0xFF, 0xFF, 0xFF);
    WipeInit(RenderMode);
    
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayerConf[i].charNo = CHARNO_NONE;
    }
    
    omMasterInit(0, _ovltbl, DLL_MAX, DLL_bootdll);
    VIWaitForRetrace();
    
    if(VIGetNextField() == 0) {
        OSReport("VI_FIELD_BELOW\n");
        VIWaitForRetrace();
    }
    OSReport("%s USA Mode\n", "Sep 25 2003");
    SLSaveFlagSet(FALSE);
    while(1) {
        retrace = VIGetRetraceCount();
        if (HuSoftResetButtonCheck() || HuDvdErrWait) {
            continue;
        }
        HuPerfZero();
        HuPerfBegin(2);
        HuSysBeforeRender();
        HuPerfBegin(0);
        Hu3DPreProc();
        HuPadRead();
        pfClsScr();
        HuPrcCall(1);
        GameMesExec();
        HuPerfBegin(1);
        Hu3DExec();
        HuDvdErrorWatch();
        WipeExecAlways();
        HuPerfEnd(0);
        pfDrawFonts();
        HuPerfEnd(1);
        msmMusFdoutEnd();
        HuSysDoneRender(retrace);
        frand();
        rand8();
        HuPerfEnd(2);
        GlobalCounter++;
    }
}

void HuSysVWaitSet(s16 vWait)
{
    minimumVcount = vWait;
    minimumVcountf = vWait;
}

s16 HuSysVWaitGet(s16 prev)
{
    return minimumVcount;
}

s32 rnd_seed = 0x0000D9ED;

int rand8(void)
{
    rnd_seed = (rnd_seed * 0x41C64E6D) + 0x3039;
    return (u8)(((rnd_seed + 1) >> 16) & 0xFF);
}