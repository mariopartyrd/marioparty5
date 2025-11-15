#include "game/object.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hu3d.h"

void omGameSysInit(OMOBJMAN *objman)
{
    int i;
    OMOBJ *obj;

    omSystemKeyCheckSetup(objman);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    omSysPauseEnable(0);

    for(i=0; i<GW_PLAYER_MAX; i++) {
        GWMgCoinBonusSet(i, 0);
        GWMgCoinSet(i, 0);
        GWMgScoreSet(i, 0);
    }

    if(mgPauseExitF) {
        obj = omAddObjEx(objman, 0x7FDC, 0, 0, -1, GameMesExitCheck);
        omSetStatBit(obj, OM_STAT_NOPAUSE|OM_STAT_SPRPAUSE);
    }
    HuWinInit(0);
}

void omVibrate(s16 playerNo, s16 duration, s16 off, s16 on)
{
    if (GWRumbleFGet() != FALSE && GwPlayerConf[playerNo].type == GW_TYPE_MAN) {
        HuPadRumbleSet(GwPlayerConf[playerNo].padNo, duration, off, on);
    }
}