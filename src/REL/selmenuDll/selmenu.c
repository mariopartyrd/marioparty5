#include "game/board/main.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/hu3d.h"
#include "game/gamemes.h"
#include "game/object.h"
#include "game/mgdata.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/wipe.h"
#include "game/gamework.h"
#include "msm.h"

#include "math.h"
#include "datanum/selmenu.h"

#define SM_PAGE_MAX 9
#define SM_PAGE_SIZE 10

#define SM_CHAR_MAX 10

#define SM_KEY_LEFT 0x0004
#define SM_KEY_RIGHT 0x0008
#define SM_KEY_UP 0x0001
#define SM_KEY_DOWN 0x0002

static char *smCharNameTbl[13] = { //Should be CHARNO_MAX
    "Mario",
    "Luigi",
    "Peach",
    "Yoshi",
    "Wario",
    "Daisy",
    "Waluigi",
    "Kinopio",
    "Teresa",
    "Minikoopa",
    "MinikoopaR",
    "MinikoopaG",
    "MinikoopaB"
};

typedef struct sm_entry {
    u16 on;
    char *name;
    OMOVL ovl;
} SMEntry;

static SMEntry smPageData[SM_PAGE_MAX * SM_PAGE_SIZE] = {
    { TRUE, "***:START", DLL_fileseldll },
    { TRUE, "***:MESS CHECK", DLL_meschkdll },
    { TRUE, "***:STAFF ROLL", DLL_staffdll },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { FALSE, "***:", DLL_selmenuDLL },
    { TRUE, "501:ICE CATCH", DLL_m501dll },
    { TRUE, "502:HIP DE GO GO GO!", DLL_m502dll },
    { TRUE, "503:UKIKI PANIC", DLL_m503dll },
    { TRUE, "504:OSANPO WAN2", DLL_m504dll },
    { TRUE, "505:PENGUIN DAIKOUSHIN", DLL_m505dll },
    { TRUE, "506:MAME NO KI JUMP", DLL_m506dll },
    { TRUE, "507:GIRI2 WAN2", DLL_m507dll },
    { TRUE, "508:PAKKUN DANCE", DLL_m508dll },
    { TRUE, "509:BIRI2 MEIRO", DLL_m509dll },
    { TRUE, "510:HUSTLE BATTING", DLL_m510dll },
    { TRUE, "511:MOTORBOAT RALLY", DLL_m511dll },
    { TRUE, "512:SKATE LACE", DLL_m512dll },
    { TRUE, "513:NENRIKI SPOON MAGE", DLL_m513dll },
    { TRUE, "514:TRIPLE JUMP", DLL_m514dll },
    { TRUE, "515:KURIBOH HOTEL", DLL_m515dll },
    { TRUE, "516:DOTABATA COIN BLOCK", DLL_m516dll },
    { TRUE, "517:TAMANORI ATTACK", DLL_m517dll },
    { TRUE, "518:CORO2 PETTAN", DLL_m518dll },
    { TRUE, "519:MACHINE BATTLE", DLL_m519dll },
    { TRUE, "520:MAWASHITE FIRE", DLL_m520dll },
    { TRUE, "521:KURU2 DOKEI", DLL_m521dll },
    { TRUE, "522:FUTTOBI TOWER", DLL_m522dll },
    { TRUE, "523:UFO DE TSUKAMAERO", DLL_m523dll },
    { TRUE, "524:LUCKY! COIN MACHINE", DLL_m524dll },
    { TRUE, "525:OTOSHITE KAWASHITE", DLL_m525dll },
    { TRUE, "526:BOMUHEI DOUKASEN", DLL_m526dll },
    { TRUE, "527:EAWSE UFO", DLL_m527dll },
    { TRUE, "528:KANKIRI SEESAW", DLL_m528dll },
    { TRUE, "529:HAND DE PYON", DLL_m529dll },
    { TRUE, "530:SHOT DE CATCH", DLL_m530dll },
    { TRUE, "531:PIKA2 CLEANER", DLL_m531dll },
    { TRUE, "532:CAPSULE CATCH", DLL_m532dll },
    { TRUE, "533:PUKU2 SENSUITEI", DLL_m533dll },
    { TRUE, "534:TATAITE HAMMER", DLL_m534dll },
    { TRUE, "535:MITSUKETE CARD", DLL_m535dll },
    { TRUE, "536:GURU2 TAIHOH", DLL_m536dll },
    { TRUE, "537:TSUNAHIKI SURVIVAL", DLL_m537dll },
    { TRUE, "538:HAMMER FIGHT", DLL_m538dll },
    { TRUE, "539:SOROTTE SLOT", DLL_m539dll },
    { TRUE, "540:UNMAKASE DERBY", DLL_m540dll },
    { TRUE, "541:BIRIKYUU LIGHTNING!", DLL_m541dll },
    { TRUE, "542:GIRI2! COUNTDOWN", DLL_m542dll },
    { TRUE, "543:TOBIDASE! BATTAN", DLL_m543dll },
    { TRUE, "544:HAYAUCHI GANMAN!", DLL_m544dll },
    { TRUE, "545:UEKARA BLOCK!", DLL_m545dll },
    { TRUE, "546:ERANDE ROPE!", DLL_m546dll },
    { TRUE, "547:TOBE2 HIJUMP!", DLL_m547dll },
    { TRUE, "548:TAIKETSU! KOOPA BAKUDAN", DLL_m548dll },
    { TRUE, "549:KUMO2 SHOT!", DLL_m549dll },
    { TRUE, "550:FUWA2 FIGHT!", DLL_m550dll },
    { TRUE, "551:HIP DE SWITCH!", DLL_m551dll },
    { TRUE, "552:PIECE WO UBAE!", DLL_m552dll },
    { TRUE, "553:SAGASE! KAKUSHI BLOCK", DLL_m553dll },
    { TRUE, "554:TSUMUZIKUN DANGEROUS", DLL_m554dll },
    { TRUE, "555:DOKOMADEMO SKY HIGH!", DLL_m555dll },
    { FALSE, "556:", DLL_selmenuDLL },
    { FALSE, "557:", DLL_selmenuDLL },
    { FALSE, "558:", DLL_selmenuDLL },
    { TRUE, "559:GANBATTEMO DAMENANODA", DLL_m559dll },
    { TRUE, "560:OCHITSUITEMO MURINANODA", DLL_m560dll },
    { TRUE, "561:KANNGAETEMO MUDANANODA", DLL_m561dll },
    { TRUE, "562:SAIGONO YUME", DLL_m562dll },
    { TRUE, "563:FLOWER GARDEN", DLL_m563dll },
    { TRUE, "564:BUTSUKERO BOMBHEI", DLL_m564dll },
    { TRUE, "565:PUKU2 ISLAND", DLL_m565dll },
    { TRUE, "566:BURUBUR! HEIHO SAGASHI", DLL_m566dll },
    { TRUE, "567:YAJIRUSHI PYON2", DLL_m567dll },
    { TRUE, "568:YUKIYAMA CHASE", DLL_m568dll },
    { TRUE, "569:ITADAKI! COIN SHOW", DLL_m569dll },
    { TRUE, "570:PANEL DE FIGHT", DLL_m570dll },
    { TRUE, "571:KOWASHITE BANANA", DLL_m571dll },
    { TRUE, "572:NOBOTTE BANANA", DLL_m572dll },
    { TRUE, "573:KAWASHITE BANANA", DLL_m573dll },
    { TRUE, "574:PANIC! PINBALL", DLL_m574dll },
    { TRUE, "575:GURA2! COIN GETs", DLL_m575dll },
    { TRUE, "576:YOKODORI XTAL", DLL_m576dll },
    { TRUE, "577:KUNE2 LACE", DLL_m577dll },
    { FALSE, "578:NEKKETSU! BEACH VOLLEY", DLL_selmenuDLL },
    { TRUE, "579:ISOIDE FISHING", DLL_m579dll },
    { TRUE, "580:GEKITOU! ICE HOCKEY", DLL_m580dll },
};

static u32 smCharSelCamBitTbl[] = {
    HU3D_CAM0,
    HU3D_CAM1,
    HU3D_CAM2,
    HU3D_CAM3
};

static u32 lbl_1_data_844[] = { 0, 1, 2, 3, 4, 5 };

static char *smDvdMusTbl[] = { "sound/mu_016a.dvd", "sound/mu_047a.dvd", "sound/mu_052a.dvd", "sound/mu_054a.dvd", "sound/mu_101a.dvd",
    "sound/mu_108a.dvd", "sound/mu002a.dvd", "" };

static s16 smPage;
static s16 smCursorNoPrev[SM_PAGE_MAX];
static OMOBJ *smMainObj;
static OMOBJ *smOutViewObj;
static GWPLAYERCONF smPlayerConf[GW_PLAYER_MAX];
static s16 smCursorNo;
static s16 smChar1Prev = -1;
static u16 smPadBtnDown;
static u16 smPadDStk;
static u16 smPadDStkDown;
static u16 smPadBtnDownAll[4];
static u16 smPadDStkAll[4];
static u16 smPadDStkDownAll[4];
static u16 smPadDStkAllPrev[4] = {};

static void SMInit(OMOBJ *obj);
static void SMRandMain(OMOBJ *obj);

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

void ObjectSetup(void)
{
    static char *funcId = "SMOBJECTSETUP\n";
    int i;
    OMOBJMAN *objman;
    HuVecF pos, dir;

    OSReport("ObjectSetup:%08x\n", ObjectSetup);
    OSReport("\n\n\n******* SMObjectSetup *********\n");
    OSReport(funcId);
    objman = omInitObjMan(50, 8192);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (GwPlayerConf[i].charNo >= SM_CHAR_MAX) {
            break;
        }
    }
    if (i != GW_PLAYER_MAX) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            GwPlayerConf[i].charNo = i;
        }
    }
    smMainObj = omAddObjEx(objman, 100, 0, 0, -1, SMInit);
    smOutViewObj = omAddObjEx(objman, 32730, 0, 0, -1, omOutViewMulti);
    omAddObjEx(objman, 100, 0, 0, -1, SMRandMain);
    smOutViewObj->work[0] = GW_PLAYER_MAX;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        CRotM[i].x = -20.0f;
        CRotM[i].y = 0.0f;
        CRotM[i].z = 0.0f;
        CenterM[i].x = 0.0f;
        CenterM[i].y = 50.0f;
        CenterM[i].z = 0.0f;
        CZoomM[i] = 500.0f;
        Hu3DCameraCreate(smCharSelCamBitTbl[i]);
        Hu3DCameraPerspectiveSet(smCharSelCamBitTbl[i], 45.0f, 20.0f, 25000.0f, 4.0f / 3.0f);
        Hu3DCameraViewportSet(smCharSelCamBitTbl[i], (i % 2) * 320.0f, (i / 2) * 240.0f, 320.0f, 240.0f, 0.0f, 1.0f);
    }
    Hu3DGLightCreate(0.0f, 0.0f, 100.0f, 0.0f, 0.0f, -1.0f, 255, 255, 255);
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 200.0f;
    dir.x = dir.y = 0.0f;
    dir.z = -1.0f;
    SetDefLight(&pos, &dir, 255, 255, 255, 64, 64, 64, 255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    GWSubGameNoSet(-1);
    _ClearFlag(FLAG_BOARD_SAVEINIT);
    GwSystem.turnNo = 1;
    GwSystem.turnMax = 20;
    HuMemHeapDump(HuMemHeapPtrGet(HEAP_MODEL), -1);
}

static void SMCopyConfig(GWPLAYERCONF *dst, GWPLAYERCONF *src)
{
    int i;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        *dst++ = *src++;
    }
}

static void SMBtnRead(void)
{
    int i;
    smPadDStk = smPadDStkDown = smPadBtnDown = 0;
    for (i = 0; i < 4; i++) {
        smPadDStkAll[i] = smPadDStkDownAll[i] = smPadBtnDownAll[i] = 0;
        if ((HuPadDStkRep[i] | HuPadBtn[i]) & PAD_BUTTON_LEFT) {
            smPadDStkAll[i] |= SM_KEY_LEFT;
        }
        if ((HuPadDStkRep[i] | HuPadBtn[i]) & PAD_BUTTON_RIGHT) {
            smPadDStkAll[i] |= SM_KEY_RIGHT;
        }
        if ((HuPadDStkRep[i] | HuPadBtn[i]) & PAD_BUTTON_UP) {
            smPadDStkAll[i] |= SM_KEY_UP;
        }
        if ((HuPadDStkRep[i] | HuPadBtn[i]) & PAD_BUTTON_DOWN) {
            smPadDStkAll[i] |= SM_KEY_DOWN;
        }
        smPadBtnDownAll[i] |= HuPadBtnDown[i];
        smPadDStkDownAll[i] = (smPadDStkAll[i] ^ smPadDStkAllPrev[i]) & smPadDStkAll[i];
        smPadDStkAllPrev[i] = smPadDStkAll[i];
        smPadDStk |= smPadDStkAll[i];
        smPadDStkDown |= smPadDStkDownAll[i];
        smPadBtnDown |= smPadBtnDownAll[i];
    }
}

static void SMPagePrint(void)
{
    int i;
    SMEntry *entry;
    fontcolor = FONT_COLOR_YELLOW;
    print8(200, 160, 2.5f, "PAGE:%d/%d", smPage + 1, SM_PAGE_MAX);
    for (i = 0; i < SM_PAGE_SIZE; i++) {
        entry = &smPageData[(smPage * SM_PAGE_SIZE) + i];
        if (entry->on == 1) {
            if (i == smCursorNo) {
                fontcolor = FONT_COLOR_CYAN;
            }
            else {
                fontcolor = FONT_COLOR_GREEN;
            }
        }
        else {
            fontcolor = FONT_COLOR_DARK_GREEN;
        }
        print8(100, (float)((i + 10) * 8) * 2.5f, 2.5f, "%s", entry->name);
    }
}

static void SMCursorNoAdd(s16 num);

static void SMPageNoAdd(s16 num)
{
    s16 page;
    int i;
    u16 on;
    s16 pagePrev;
    on = FALSE;
    page = smPage;
    pagePrev = page;
    do {
        page += num;
        if (page >= SM_PAGE_MAX) {
            page = 0;
        }
        else if (page < 0) {
            page = SM_PAGE_MAX - 1;
        }
        for (i = 0; i < SM_PAGE_SIZE; i++) {
            if (smPageData[(page * SM_PAGE_SIZE) + i].on == TRUE) {
                on = TRUE;
                break;
            }
        }
    } while (!on);
    smPage = page;
    if (pagePrev == -1) {
        smCursorNo = -1;
        SMCursorNoAdd(1);
        smCursorNoPrev[smPage] = smCursorNo;
    }
    else {
        smCursorNoPrev[pagePrev] = smCursorNo;
        smCursorNo = smCursorNoPrev[smPage];
    }
}

static void SMCursorNoAdd(s16 num)
{
    s16 pos;
    pos = smCursorNo;
    do {
        pos += num;
        if (pos >= SM_PAGE_SIZE) {
            pos = 0;
        }
        else if (pos < 0) {
            pos = SM_PAGE_SIZE - 1;
        }
    } while (!smPageData[(smPage * SM_PAGE_SIZE) + pos].on);
    smCursorNo = pos;
    (void)pos;
}

static s16 SMCharNoAdd(s16 playerNo, s16 num);
static void SMMain(OMOBJ *obj);

static void SMInit(OMOBJ *obj)
{
    int i, j;
    OSReport("SMinit:%08x\n", SMInit);
    omDLLDBGOut();
    Hu3DBGColorSet(0, 0, 0);
    SMCopyConfig(smPlayerConf, GwPlayerConf);
    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 4; j++) {
            if (smPlayerConf[i].charNo == smPlayerConf[j].charNo) {
                smPlayerConf[j].charNo = SMCharNoAdd(j, -1);
            }
        }
    }
    smPage = -1;
    for (i = 0; i < SM_PAGE_MAX; i++) {
        for (j = 0; j < SM_PAGE_SIZE; j++) {
            if (smPageData[(i * SM_PAGE_SIZE) + j].on == 1) {
                break;
            }
        }
        if (j == SM_PAGE_SIZE) {
            smCursorNoPrev[i] = -1;
        }
        else {
            smCursorNoPrev[i] = j;
        }
    }
    SMPageNoAdd(1);
    if (GameMesOvlPrev != DLL_NONE) {
        for (i = 0; i < SM_PAGE_MAX * SM_PAGE_SIZE; i++) {
            if (GameMesOvlPrev == smPageData[i].ovl && smPageData[i].on == 1) {
                smPage = i / SM_PAGE_SIZE;
                smCursorNo = i % SM_PAGE_SIZE;
                smCursorNoPrev[smPage] = smCursorNo;
                break;
            }
        }
    }
    obj->objFunc = SMMain;
}

static void SMGroupSet(int pos);
static void SMCharSelInit(OMOBJ *obj);
static void SMPlayerConfInit(OMOBJ *obj);
static void SMSound3DInit(OMOBJ *obj);

static void SMMain(OMOBJ *obj)
{
    SMBtnRead();
    SMPagePrint();
    if (smPadDStkDown & SM_KEY_UP) {
        SMCursorNoAdd(-1);
        SMGroupSet((smPage * SM_PAGE_SIZE) + smCursorNo);
        return;
    }
    if (smPadDStkDown & SM_KEY_DOWN) {
        SMCursorNoAdd(1);
        SMGroupSet((smPage * SM_PAGE_SIZE) + smCursorNo);
        return;
    }
    if (smPadDStkDown & SM_KEY_LEFT) {
        SMPageNoAdd(-1);
        SMGroupSet((smPage * SM_PAGE_SIZE) + smCursorNo);
        return;
    }
    if (smPadDStkDown & SM_KEY_RIGHT) {
        SMPageNoAdd(1);
        SMGroupSet((smPage * SM_PAGE_SIZE) + smCursorNo);
        return;
    }
    if ((smPadBtnDown & PAD_BUTTON_A) || (smPadBtnDown & PAD_BUTTON_START)) {
        obj->objFunc = SMCharSelInit;
        return;
    }
    if (smPadBtnDown & PAD_BUTTON_Y) {
        obj->objFunc = SMPlayerConfInit;
        return;
    }
    if (smPadBtnDown & PAD_TRIGGER_Z) {
        obj->objFunc = SMSound3DInit;
        return;
    }
    if (smPadBtnDown & PAD_BUTTON_X) {
        omOvlReturnEx(0, 1);
    }
}

static void SMGroupSet(int pos)
{
    int i;
    int j;
    OMOVL ovl;
    ovl = smPageData[pos].ovl;
    for(i=0; ; i++) {
        if(mgDataTbl[i].ovl == ovl) {
            break;
        }
        if(mgDataTbl[i].ovl == (u16)DLL_NONE) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                smPlayerConf[i].grp = i;
            }
            return;
        }
    }
    switch (mgDataTbl[i].type) {
        case MG_TYPE_4P:
        case MG_TYPE_BATTLE:
        case MG_TYPE_KUPA:
        case MG_TYPE_LAST:
        case MG_TYPE_KETTOU:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                smPlayerConf[i].grp = i;
            }
            break;

        case MG_TYPE_1VS3:
            smPlayerConf[0].grp = 0;
            for (i = 1; i < GW_PLAYER_MAX; i++) {
                smPlayerConf[i].grp = 1;
            }
            break;

        case MG_TYPE_2VS2:
            for (j = 0; j < GW_PLAYER_MAX/2; j++) {
                smPlayerConf[j].grp = 0;
            }
            for (j = GW_PLAYER_MAX/2; j < GW_PLAYER_MAX; j++) {
                smPlayerConf[j].grp = 1;
            }
            if(mgDataTbl[i].flag & MG_FLAG_GRPORDER) {
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    smPlayerConf[i].grp = i;
                }
            }
            break;
    }
}

static HU3DMODELID smCharMdlId[SM_CHAR_MAX];
static s16 smCharSelEndF[GW_PLAYER_MAX];
static s16 smCharOnF[SM_CHAR_MAX];

static int smCharFileTbl[13] = { //Should be CHARNO_MAX
    SELMENU_HSF_mario,
    SELMENU_HSF_luigi,
    SELMENU_HSF_peach,
    SELMENU_HSF_yoshi,
    SELMENU_HSF_wario,
    SELMENU_HSF_daisy,
    SELMENU_HSF_waluigi,
    SELMENU_HSF_kinopio,
    SELMENU_HSF_teresa,
    SELMENU_HSF_minikoopa,
    SELMENU_HSF_minikoopaR,
    SELMENU_HSF_minikoopaG,
    SELMENU_HSF_minikoopaB
};

static int smCharMotFileTbl[13] = { //Should be CHARNO_MAX
    SELMENU_HSF_marioMot,
    SELMENU_HSF_luigiMot,
    SELMENU_HSF_peachMot,
    SELMENU_HSF_yoshiMot,
    SELMENU_HSF_warioMot,
    SELMENU_HSF_daisyMot,
    SELMENU_HSF_waluigiMot,
    SELMENU_HSF_kinopioMot,
    SELMENU_HSF_teresaMot,
    SELMENU_HSF_minikoopaMot,
    SELMENU_HSF_minikoopaMot,
    SELMENU_HSF_minikoopaMot,
    SELMENU_HSF_minikoopaMot
};

static s16 SMCharNoAdd(s16 playerNo, s16 num)
{
    int i;
    s16 charNo;
    for (i = 0; i < SM_CHAR_MAX; i++) {
        smCharOnF[i] = FALSE;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (smPlayerConf[i].type) {
            continue;
        }
        smCharOnF[smPlayerConf[i].charNo] = TRUE;
    }
    if (!num) {
        return;
    }
    charNo = smPlayerConf[playerNo].charNo;
    do {
        charNo += num;
        if (charNo < 0) {
            charNo = SM_CHAR_MAX-1;
        }
        if (charNo >= SM_CHAR_MAX) {
            charNo = 0;
        }
    } while (smCharOnF[charNo] != 0);
    return charNo;
}

static void SMCharMdlKill(void)
{
    int i;
    for (i = 0; i < SM_CHAR_MAX; i++) {
        Hu3DModelKill(smCharMdlId[i]);
    }
}

static void SMCharComSet(void)
{
    int i;
    int j;
    int num;
    int charNo[SM_CHAR_MAX];
    smCharOnF[4] = TRUE;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        num = 0;
        for (j = 0; j < SM_CHAR_MAX; j++) {
            if (smCharOnF[j]) {
                continue;
            }
            charNo[num++] = j;
        }
        if (smPlayerConf[i].type == GW_TYPE_COM) {
            smPlayerConf[i].charNo = charNo[rand8() % num];
            smCharOnF[smPlayerConf[i].charNo] = TRUE;
        }
    }
}

static void SMCharSelMain(OMOBJ *obj);
static void SMExit(OMOBJ *obj);

static void SMCharSelInit(OMOBJ *obj)
{
    int i;
    for (i = 0; i < SM_CHAR_MAX; i++) {
        void *data = HuDataSelHeapReadNum(smCharFileTbl[i], HU_MEMNUM_OVL, HEAP_MODEL);
        smCharMdlId[i] = Hu3DModelCreate(data);
        Hu3DModelPosSet(smCharMdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(smCharMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(smCharMdlId[i], Hu3DJointMotionData(smCharMdlId[i], smCharMotFileTbl[i]));
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        smCharSelEndF[i] = FALSE;
    }
    SMCharNoAdd(0, 0);
    obj->objFunc = SMCharSelMain;
}

static void SMCharSelMain(OMOBJ *obj)
{
    int i;

    s16 port;
    int j;
    s16 w;
    s16 x;
    s16 y;
    s16 manDoneNum;
    s16 manNum;

    for (i = 0; i < SM_CHAR_MAX; i++) {
        Hu3DModelAttrSet(smCharMdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (smPlayerConf[i].type == GW_TYPE_MAN) {
            Hu3DModelAttrReset(smCharMdlId[smPlayerConf[i].charNo], HU3D_ATTR_DISPOFF);
            Hu3DModelCameraSet(smCharMdlId[smPlayerConf[i].charNo], smCharSelCamBitTbl[i]);
        }
    }
    for (i = 0, manDoneNum = 0, manNum = 0; i < GW_PLAYER_MAX; i++) {
        if (smCharSelEndF[i] == TRUE) {
            manDoneNum++;
        }
        if (smPlayerConf[i].type == GW_TYPE_MAN) {
            manNum++;
        }
    }
    if (manDoneNum == manNum) {
        SMCharNoAdd(0, 0);
        SMCharComSet();
        SMCopyConfig(GwPlayerConf, smPlayerConf);
        CharDataClose(-1); //Can't use CHARNO_NONE
        OSReport("%d,%d,%d,%d\n", GwPlayerConf[0].charNo, GwPlayerConf[1].charNo, GwPlayerConf[2].charNo, GwPlayerConf[3].charNo);
        if (!CharMotionAMemPGet(GwPlayerConf[0].charNo)) {
            CharMotionInit(GwPlayerConf[0].charNo);
        }
        if (!CharMotionAMemPGet(GwPlayerConf[1].charNo)) {
            CharMotionInit(GwPlayerConf[1].charNo);
        }
        if (!CharMotionAMemPGet(GwPlayerConf[2].charNo)) {
            CharMotionInit(GwPlayerConf[2].charNo);
        }
        if (!CharMotionAMemPGet(GwPlayerConf[3].charNo)) {
            CharMotionInit(GwPlayerConf[3].charNo);
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
        obj->objFunc = SMExit;
    }
    SMBtnRead();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        port = smPlayerConf[i].padNo;
        if (!smCharSelEndF[i]) {
            if (smPadDStkDownAll[port] & SM_KEY_LEFT) {
                smPlayerConf[port].charNo = SMCharNoAdd(i, -1);
            }
            if (smPadDStkDownAll[port] & SM_KEY_RIGHT) {
                smPlayerConf[port].charNo = SMCharNoAdd(i, 1);
            }
            if (smPadBtnDownAll[port] & PAD_BUTTON_A) {
                smCharSelEndF[i] = TRUE;
            }
        }
        if (HuPadBtnDown[port] & PAD_BUTTON_B) {
            if (manDoneNum != 0) {
                smCharSelEndF[i] = 0;
            }
            else {
                SMCharMdlKill();
                obj->objFunc = SMMain;
                return;
            }
        }
        if (smPadBtnDownAll[port] & PAD_BUTTON_START) {
            for (j = 0; j < GW_PLAYER_MAX; j++) {
                if (!smPlayerConf[j].type) {
                    smCharSelEndF[j] = TRUE;
                }
            }
            return;
        }
        if (!smPlayerConf[i].type) {
            w = strlen(smCharNameTbl[smPlayerConf[i].charNo]);
            w *= 16;
            x = ((320 - w) / 2) + ((i % 2) * 320);
            y = 176 + ((i / 2) * 240);
            if (smCharSelEndF[i] == TRUE) {
                fontcolor = FONT_COLOR_WHITE;
                print8(x, y, 2.0f,
                    "\xFD\x08"
                    "%s",
                    smCharNameTbl[smPlayerConf[i].charNo]);
            }
            else {
                fontcolor = FONT_COLOR_GREEN;
                print8(x, y, 2.0f,
                    "\xFD\x05"
                    "%s",
                    smCharNameTbl[smPlayerConf[i].charNo]);
            }
        }
        else {
            w = 72;
            x = ((320 - w) / 2) + ((i % 2) * 320);
            y = 108 + ((i / 2) * 240);
            fontcolor = FONT_COLOR_YELLOW;
            print8(x, y, 3.0f,
                "\xFD\x07"
                "COM");
        }
    }
}

static void SMExit(OMOBJ *object)
{
    int mg;
    if (WipeCheckEnd()) {
        return;
    }
    while (HuARDMACheck());
    mg = MgNoGet(smPageData[(smPage * SM_PAGE_SIZE) + smCursorNo].ovl);
    GwSystem.mgNo = mg;
    OSReport("mgNo=%d\n", mg);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    _ClearFlag(FLAG_INST_DECA);
    _SetFlag(FLAG_MGINST_ON);
    if(mg == -1) {
        omOvlCall(smPageData[(smPage * SM_PAGE_SIZE) + smCursorNo].ovl, 0, 0);
    } else {
        omOvlCall(DLL_instdll, 0, 0);
    }
    GameMesOvlPrev = smPageData[(smPage * SM_PAGE_SIZE) + smCursorNo].ovl;
    smChar1Prev = smCursorNo;
}

static char *smPlayerConfStrTbl[] = { "PLAYER:%d", " PAD%d:%s", "PADNO:%d", "  GRP:%d", "  DIF:%s" };

static s16 smPlayerConfEditF;
static s16 smPlayerConfNo;
static s16 smPlayerConfChoiceNo;

#define DO_HILITE(option)                                                                                                                            \
    do {                                                                                                                                             \
        s32 color;                                                                                                                                   \
        if (smPlayerConfChoiceNo == option) {                                                                                                          \
            color = fontcolor = FONT_COLOR_CYAN;                                                                                                     \
        }                                                                                                                                            \
        else {                                                                                                                                       \
            color = fontcolor = FONT_COLOR_GREEN;                                                                                                    \
        }                                                                                                                                            \
        (void)color;                                                                                                                                 \
    } while (0)

static void SMPlayerConfPrint(void)
{
    int i;
    s16 x;
    s16 y;
    s16 w;
    char *typeStr[] = { "HUMAN", "COM" };
    char *diffStr[] = { "EASY", "NORMAL", "HARD", "VERYHARD" };

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        w = 128;
        x = ((320 - w) / 2) + ((i % 2) * 320);
        y = ((i / 2) * 240) + 48;
        if (smPlayerConfEditF == 0 && smPlayerConfNo == i) {
            fontcolor = FONT_COLOR_CYAN;
        }
        else {
            fontcolor = FONT_COLOR_GREEN;
        }
        print8(x, y, 2.0f, smPlayerConfStrTbl[0], i);
        if (smPlayerConfEditF == 0) {
            fontcolor = FONT_COLOR_DARK_GREEN;
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 80, 2.0f, smPlayerConfStrTbl[1], smPlayerConf[i].padNo, typeStr[smPlayerConf[i].type]);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 96, 2.0f, smPlayerConfStrTbl[2], smPlayerConf[i].padNo);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 112, 2.0f, smPlayerConfStrTbl[3], smPlayerConf[i].grp);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 128, 2.0f, smPlayerConfStrTbl[4], diffStr[smPlayerConf[i].dif]);
        }
        else if (smPlayerConfEditF == 1 && smPlayerConfNo == i) {
            DO_HILITE(0);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 80, 2.0f, smPlayerConfStrTbl[1], smPlayerConf[i].padNo, typeStr[smPlayerConf[i].type]);
            DO_HILITE(1);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 96, 2.0f, smPlayerConfStrTbl[2], smPlayerConf[i].padNo);
            DO_HILITE(2);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 112, 2.0f, smPlayerConfStrTbl[3], smPlayerConf[i].grp);
            DO_HILITE(3);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 128, 2.0f, smPlayerConfStrTbl[4], diffStr[smPlayerConf[i].dif]);
        }
        else {
            fontcolor = FONT_COLOR_DARK_GREEN;
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 80, 2.0f, smPlayerConfStrTbl[1], smPlayerConf[i].padNo, typeStr[smPlayerConf[i].type]);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 96, 2.0f, smPlayerConfStrTbl[2], smPlayerConf[i].padNo);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 112, 2.0f, smPlayerConfStrTbl[3], smPlayerConf[i].grp);
            print8(((i % 2) * 320) + 64, ((i / 2) * 240) + 128, 2.0f, smPlayerConfStrTbl[4], diffStr[smPlayerConf[i].dif]);
        }
    }
}

#undef DO_HILITE
static void SMPlayerConfMain(OMOBJ *obj);

static void SMPlayerConfInit(OMOBJ *obj)
{
    smPlayerConfEditF = 0;
    smPlayerConfNo = 0;
    smPlayerConfChoiceNo = 0;
    obj->objFunc = SMPlayerConfMain;
}

static void SMPlayerConfMain(OMOBJ *obj)
{
    int offset;
    SMPlayerConfPrint();
    SMBtnRead();
    if (!smPlayerConfEditF) {
        if (smPadDStkDown & SM_KEY_LEFT) {
            offset = ((smPlayerConfNo & 0x1) - 1) & 0x1;
            smPlayerConfNo = (smPlayerConfNo & 0xFFFE) + offset;
        }
        else {
            if (smPadDStkDown & SM_KEY_RIGHT) {
                offset = ((smPlayerConfNo & 0x1) + 1) & 0x1;
                smPlayerConfNo = (smPlayerConfNo & 0xFFFE) + offset;
            }
            else if (smPadDStkDown & SM_KEY_DOWN) {
                if ((smPlayerConfNo += 2) >= 4) {
                    smPlayerConfNo -= 4;
                }
            }
            else if (smPadDStkDown & SM_KEY_UP) {
                if ((smPlayerConfNo -= 2) < 0) {
                    smPlayerConfNo += 4;
                }
            }
        }
        if (smPadBtnDown & PAD_BUTTON_A) {
            smPlayerConfEditF = 1;
            return;
        }
        if ((smPadBtnDown & PAD_BUTTON_B) || (smPadBtnDown & PAD_BUTTON_Y)) {
            obj->objFunc = SMMain;
            return;
        }
        if (smPadBtnDown & PAD_BUTTON_START) {
            int i;
            for(i=0; i<100; i++) {
                int order1 = frandmod(GW_PLAYER_MAX);
                int order2 = frandmod(GW_PLAYER_MAX);
                int temp = smPlayerConf[order1].grp;
                smPlayerConf[order1].grp = smPlayerConf[order2].grp;
                smPlayerConf[order2].grp = temp;
            }
        }
    }
    else if (smPadDStkDown & SM_KEY_UP) {
        if (--smPlayerConfChoiceNo < 0) {
            smPlayerConfChoiceNo = 3;
        }
    }
    else if (smPadDStkDown & SM_KEY_DOWN) {
        if (++smPlayerConfChoiceNo > 3) {
            smPlayerConfChoiceNo = 0;
        }
    }
    else if (smPadDStkDown & SM_KEY_LEFT) {
        switch (smPlayerConfChoiceNo) {
            case 0:
                smPlayerConf[smPlayerConfNo].type ^= 1;
                break;

            case 1:
                if (--smPlayerConf[smPlayerConfNo].padNo < 0) {
                    smPlayerConf[smPlayerConfNo].padNo = 3;
                }
                break;

            case 2:
                if (--smPlayerConf[smPlayerConfNo].grp < 0) {
                    smPlayerConf[smPlayerConfNo].grp = 3;
                }
                break;

            case 3:
                if (--smPlayerConf[smPlayerConfNo].dif < 0) {
                    smPlayerConf[smPlayerConfNo].dif = 3;
                }
                break;
        };
    }
    else if (smPadDStkDown & SM_KEY_RIGHT) {
        switch (smPlayerConfChoiceNo) {
            case 0:
                smPlayerConf[smPlayerConfNo].type ^= 1;
                break;

            case 1:
                if (++smPlayerConf[smPlayerConfNo].padNo > 3) {
                    smPlayerConf[smPlayerConfNo].padNo = 0;
                }
                break;

            case 2:
                if (++smPlayerConf[smPlayerConfNo].grp > 3) {
                    smPlayerConf[smPlayerConfNo].grp = 0;
                }
                break;

            case 3:
                if (++smPlayerConf[smPlayerConfNo].dif > 3) {
                    smPlayerConf[smPlayerConfNo].dif = 0;
                }
                break;
        }
    }
    else if ((smPadBtnDown & PAD_BUTTON_A) || (smPadBtnDown & PAD_BUTTON_B)) {
        smPlayerConfEditF = 0;
    }
    else if (smPadBtnDown & PAD_BUTTON_Y) {
        obj->objFunc = SMMain;
    }
}

static void SMRandMain(OMOBJ *obj)
{
    rand8();
}

static void SMStub(void) { }

static s16 smEmiCompDataNo;
static s16 smEmiCompVal;
static s16 smSound3DNo;
s16 lbl_1_bss_0;

static void SMSound3DExec(OMOBJ *obj);
static void SMSound3DPrint(void);

static void SMSound3DInit(OMOBJ *obj)
{
    MSMSE *se = msmSeGetIndexPtr(smEmiCompDataNo);
    smEmiCompVal = se->comp;
    obj->objFunc = SMSound3DExec;
}

static void SMSound3DExec(OMOBJ *obj)
{
    float speed;
    MSMSE *se;

    SMBtnRead();
    if (smPadDStkDown & SM_KEY_UP) {
        smSound3DNo--;
        if (smSound3DNo < 0) {
            smSound3DNo = 7;
        }
    }
    if (smPadDStkDown & SM_KEY_DOWN) {
        smSound3DNo++;
        if (smSound3DNo >= 8) {
            smSound3DNo = 0;
        }
    }
    if (smPadDStkDown & (SM_KEY_LEFT | SM_KEY_RIGHT)) {
        if (smPadDStkDown & SM_KEY_RIGHT) {
            speed = 10;
        }
        else {
            speed = -10;
        }
        if (HuPadBtn[0] & PAD_TRIGGER_Z) {
            speed *= 10.0f;
        }
        switch (smSound3DNo) {
            case 0:
                Snd3DDistOffset += speed;
                break;

            case 1:
                Snd3DSpeedOffset += speed;
                break;

            case 2:
                Snd3DStartDisOffset += speed;
                break;

            case 3:
                Snd3DFrontSurDisOffset += speed;
                break;

            case 4:
                Snd3DBackSurDisOffset += speed;
                break;

            case 5:
                smEmiCompDataNo += speed / 10.0f;
                if (smEmiCompDataNo < 0) {
                    smEmiCompDataNo = 0;
                }
                se = msmSeGetIndexPtr(smEmiCompDataNo);
                smEmiCompVal = se->comp;
                break;

            case 6:
                smEmiCompVal += speed / 10.0f;
                if (smEmiCompVal > 127) {
                    smEmiCompVal = 127;
                }
                if (smEmiCompVal < -127) {
                    smEmiCompVal = -127;
                }
                se = msmSeGetIndexPtr(smEmiCompDataNo);
                se->comp = smEmiCompVal;
                break;

            case 7:
                musicOffF = (musicOffF) ? 0 : 1;
                if (musicOffF) {
                    msmMusSetMasterVolume(0);
                }
                else {
                    msmMusSetMasterVolume(127);
                }
                break;
        }
    }
    if (smPadBtnDown & PAD_BUTTON_START) {
        switch (smSound3DNo) {
            case 0:
                Snd3DDistOffset = 0;
                break;

            case 1:
                Snd3DSpeedOffset = 0;
                break;

            case 2:
                Snd3DStartDisOffset = 0;
                break;

            case 3:
                Snd3DFrontSurDisOffset = 0;
                break;

            case 4:
                Snd3DBackSurDisOffset = 0;
                break;

            case 5:
                smEmiCompDataNo = 0;
                break;

            case 6:
                smEmiCompVal = 0;
                break;

            case 7:
                musicOffF = 0;
                break;
        }
    }

    if (smPadBtnDown & PAD_BUTTON_B) {
        obj->objFunc = SMMain;
    }
    SMSound3DPrint();
}

#define DO_HILITE(pos)                                                                                                                               \
    do {                                                                                                                                             \
        if (smSound3DNo == pos) {                                                                                                                   \
            fontcolor = FONT_COLOR_GREEN;                                                                                                            \
        }                                                                                                                                            \
        else {                                                                                                                                       \
            fontcolor = FONT_COLOR_DARK_GREEN;                                                                                                       \
        }                                                                                                                                            \
    } while (0)

static void SMSound3DPrint(void)
{
    char *onOffStr[] = { " ON", "OFF" };
    fontcolor = FONT_COLOR_YELLOW;
    print8(200, 64, 2.0f, "3DSound Config.");
    DO_HILITE(0);
    print8(140, 96, 2.0f, "Max Distance   %5.1f", Snd3DDistOffset);
    DO_HILITE(1);
    print8(140, 112, 2.0f, "Sound Speed    %5.1f", Snd3DSpeedOffset);
    DO_HILITE(2);
    print8(140, 128, 2.0f, "Start Distance %5.1f", Snd3DStartDisOffset);
    DO_HILITE(3);
    print8(140, 144, 2.0f, "Front Distance %5.1f", Snd3DFrontSurDisOffset);
    DO_HILITE(4);
    print8(140, 160, 2.0f, "Back Distance  %5.1f", Snd3DBackSurDisOffset);
    DO_HILITE(5);
    print8(140, 176, 2.0f, "emiComp DataNo  %04d", smEmiCompDataNo);
    DO_HILITE(6);
    print8(140, 192, 2.0f, "emiComp VAL      %3d", smEmiCompVal);
    DO_HILITE(7);
    print8(140, 208, 2.0f, "Music            %s", onOffStr[(musicOffF) ? 1 : 0]);
}

#undef DO_HILITE
