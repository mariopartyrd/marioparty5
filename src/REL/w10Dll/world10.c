#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/scroll.h"
#include "game/board/masu.h"
#include "game/board/capsule.h"

#include "datanum/w10.h"

#define MB10_MASU_DONKEY MASU_FLAG_BIT(15)
#define MB10_MASU_SAI MASU_FLAG_BIT(14)

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}


static MBMODELID mb10_MainMdlId;

void MB10_Create(void);
void MB10_Kill(void);
void MB10_LightSet(void);
void MB10_LightReset(void);

void ObjectSetup(void)
{
    GWPartyFSet(TRUE);
    
    _SetFlag(FLAG_BOARD_TUTORIAL);
    _ClearFlag(FLAG_MG_CIRCUIT);
    GwSystem.turnMax = 20;
    GWTeamFSet(FALSE);
    GWBonusStarFSet(FALSE);
    GWMgInstFSet(TRUE);
    GWMgComFSet(TRUE);
    GWMgPackSet(GW_MGPACK_ALL);
    GWRumbleFSet(GwCommon.rumbleF);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    MBObjectSetup(MBNO_TUTORIAL, MB10_Create, MB10_Kill);
}

static void MB10_MainObjUpdate(OMOBJ *obj);

void MB10_Create(void)
{
    int boardNo = MBBoardNoGet();
    MBMasuCreate(W10_BIN_masuData);
    mb10_MainMdlId = MBModelCreate(W10_HSF_map, NULL, FALSE);
    MBModelPosSet(mb10_MainMdlId, 0, 0, 0);
    MBModelAttrSet(mb10_MainMdlId, HU3D_MOTATTR_LOOP);
    MBModelUnkSpeedSet(mb10_MainMdlId, -1);
    MBScrollInit(0, 0, 0);
    MBCapsuleColCreate(HU_DATANUM_NONE);
    MBLightHookSet(MB10_LightSet, MB10_LightReset);
    HuDataDirClose(DATA_w10);
    MBCapsuleDonkeyAdd(MBMasuFlagMatchFind(MASU_LAYER_DEFAULT, MB10_MASU_DONKEY));
    GwSystem.saiMasuId = MBMasuFlagMatchFind(MASU_LAYER_DEFAULT, MB10_MASU_SAI);
    MBAddObj(32768, 0, 0, MB10_MainObjUpdate);
}

void MB10_Kill(void)
{
    if(mb10_MainMdlId >= 0) {
        MBModelKill(mb10_MainMdlId);
    }
}

static void MB10_MainObjUpdate(OMOBJ *obj)
{
    
}

void MB10_LightSet(void)
{
    Hu3DModelLightInfoSet(MBModelIdGet(mb10_MainMdlId), TRUE);
}

void MB10_LightReset(void)
{
    
}