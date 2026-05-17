#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/scroll.h"
#include "game/board/masu.h"
#include "game/board/capsule.h"
#include "game/audio.h"

#include "msm_grp.h"

#include "datanum/w20.h"

typedef struct MB20_Obj_s {
    int no;
    HuVecF pos;
} MB20_OBJ;

typedef struct MB20_ObjFile_s {
    int mdlFile;
    int motFile[4];
    int motMax;
} MB20_OBJFILE;

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

static MBMODELID mb20_MainMdlId;

static const MB20_OBJ mb20_ObjTbl[10] = {
    { 0, { -2635, 195, -3535 } },
    { 0, { -2470, 195, -3535 } },
    { 0, { 6830, 335, -7320 } },
    { 0, { 6995, 335, -7320 } },
    { 1, { 905, 508, -8155 } },
    { 1, { 1080, 508, -8155 } },
    { 1, { 6345, 260, -2400 } },
    { 1, { 6480, 260, -2290 } },
    { 2, { 4760, 266, -55 } },
    { 2, { 4930, 266, -55 } },
};

static const HuVecF camViewPos = { 1643, 629, -2962 };

static const MB20_OBJFILE mb20_ObjFileTbl[3] = {
    {
        W20_HSF_heiho,
        {
            W20_HSF_heihoIdle,
            W20_HSF_heihoWin,
            HU_DATANUM_NONE,
            HU_DATANUM_NONE
        },
        2
    },
    {
        W20_HSF_noko2,
        {
            W20_HSF_noko2Jump,
            HU_DATANUM_NONE,
            HU_DATANUM_NONE,
            HU_DATANUM_NONE
        },
        1
    },
    {
        W20_HSF_pakkun,
        {
            W20_HSF_pakkunIdle,
            W20_HSF_pakkunAngry,
            HU_DATANUM_NONE,
            HU_DATANUM_NONE
        },
        2
    },
};

void MB20_Create(void);
void MB20_Kill(void);
void MB20_LightSet(void);
void MB20_LightReset(void);

void ObjectSetup(void)
{
    GWPartyFSet(TRUE);
    
    _SetFlag(FLAG_MG_CIRCUIT);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    GwSystem.turnMax = 99;
    GWTeamFSet(FALSE);
    GWBonusStarFSet(FALSE);
    GWMgInstFSet(TRUE);
    GWMgComFSet(TRUE);
    GWMgPackSet(GW_MGPACK_ALL);
    GWRumbleFSet(GwCommon.rumbleF);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    MBObjectSetup(MBNO_CIRCUIT, MB20_Create, MB20_Kill);
}

static void MB20_MainObjUpdate(OMOBJ *obj);

void MB20_Create(void)
{
    int boardNo = MBBoardNoGet();
    int i;
    MBMasuCreate(W20_BIN_masuData);
    mb20_MainMdlId = MBModelCreate(W20_HSF_map, NULL, FALSE);
    MBModelPosSet(mb20_MainMdlId, 0, 0, 0);
    MBModelAttrSet(mb20_MainMdlId, HU3D_MOTATTR_LOOP);
    MBScrollInit(W20_HSF_scrollCol, 0, 0);
    MBMapViewCameraSet(NULL, &camViewPos, 25312);
    MBLightHookSet(MB20_LightSet, MB20_LightReset);
    MBAddObj(32768, 0, 0, MB20_MainObjUpdate);
    for(i=0; i<10; i++) {
        int no = mb20_ObjTbl[i].no;
        MBMODELID mdlId = MBModelCreate(mb20_ObjFileTbl[no].mdlFile, mb20_ObjFileTbl[no].motFile, FALSE);
        int motNo;
        float maxTime;
        float time;
        MBModelPosSetV(mdlId, &mb20_ObjTbl[i].pos);
        motNo = frandmod(mb20_ObjFileTbl[no].motMax)+1;
        MBMotionNoSet(mdlId, motNo, HU3D_MOTATTR_LOOP);
        maxTime = MBMotionMaxTimeGet(mdlId);
        time = maxTime*frandf();
        MBMotionTimeSet(mdlId, time);
    }
    HuAudSndGrpSetSet(MSM_GRP_MGCIRCUIT);
}

void MB20_Kill(void)
{
    if(mb20_MainMdlId >= 0) {
        MBModelKill(mb20_MainMdlId);
    }
    HuDataDirClose(DATA_w20);
    
}

static void MB20_MainObjUpdate(OMOBJ *obj)
{
    
}

void MB20_LightSet(void)
{
    Hu3DModelLightInfoSet(MBModelIdGet(mb20_MainMdlId), TRUE);
    Hu3DBGColorSet(255, 255, 255);
}

void MB20_LightReset(void)
{
    
}
