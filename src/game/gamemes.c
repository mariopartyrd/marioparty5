#include "game/gamemes.h"
#include "game/process.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/armem.h"
#include "game/main.h"
#include "game/hu3d.h"
#include "game/audio.h"
#include "game/window.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/pad.h"
#include "messnum/mginst.h"
#include "datanum/mgconst.h"

typedef struct gMesTbl_s {
    GAMEMESCREATE create;
    GAMEMESEXEC exec;
    HuVec2F pos;
	HuVec2F scale;
    int timeMax;
} GMESTBL;

GAMEMES gMesData[GMES_MAX];
s16 gMesTime;
u8 gMesCloseF;
static u8 gMesDebugF;
unsigned int gMesVWait;
static ANIMDATA *scoreWinMaskAnim;
static ANIMDATA *scoreWinAnim;
static BOOL pauseWaitF;
static BOOL pauseCancelF;
static BOOL pauseEnableF;
static HUPROCESS *pauseProc;
int gMesLanguageNo;

OMOVL gMesOvlPrev = DLL_NONE;

GMESTBL gMesTbl[] = {
    {
        NULL,
        NULL,
        { 292, 240 },
        { 1, 1 },
        60
    },
    {
        GMesTimerInit,
        GMesTimerExec,
        { 292, 64 },
        { 1, 1 },
        60
    },
    {
        GMesMgInit,
        GMesMg4PExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgInit,
        GMesMg2Vs2Exec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgWinInit,
        GMesMgWinExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMg1Vs3Init,
        GMesMg1Vs3Exec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgBattleInit,
        GMesMgBattleExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgInit,
        GMesMg4PExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgKupaInit,
        GMesMgKupaExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgInit,
        GMesMgLastExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgInit,
        GMesMgKettouExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgSdInit,
        GMesMgSdExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgInit,
        GMesMgDonkeyExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        GMesMgDrawInit,
        GMesMgDrawExec,
        { 292, 240 },
        { 1, 1 },
        60
    },
    {
        GMesMgRecordInit,
        GMesMgRecordExec,
        { 292, 240 },
        { 1, 1 },
        180
    },
    {
        NULL,
        NULL,
        { 292, 240 },
        { 1, 1 },
        60
    },
};

static s16 startMesTbl[MG_TYPE_MAX] = {
    GMES_MES_MG_4P,
    GMES_MES_MG_1VS3,
    GMES_MES_MG_2VS2,
    GMES_MES_MG_BATTLE,
    GMES_MES_MG_KUPA,
    GMES_MES_MG_LAST,
    GMES_MES_MG_KETTOU,
    GMES_MES_MG_SD,
    GMES_MES_MG_DONKEY
};

static char asciiTbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char kanaTbl[] = "ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｬｭｮｯｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎﾊﾋﾌﾍﾎｰ";
static char numberTbl[] = "0123456789";
static char punctTbl[] = "!?.";
static int *fontBufs[5] = {};

static void CreateFontBuf(void);

void GMesInit(void)
{
    GAMEMES *gMes = &gMesData[0];
    int i;
    for(i=GMES_MAX; i; i--, gMes++) {
        gMes->mesNo = GMES_MES_NULL;
        gMes->buf = NULL;
    }
    gMesCloseF = FALSE;
    gMesDebugF = FALSE;
    gMesTime = 30;
    HuAR_DVDtoARAM(DATA_gamemes);
    HuAR_DVDtoARAM(DATA_mgconst);
    while(HuARDMACheck());
    CreateFontBuf();
    gMesOvlPrev = DLL_NONE;
    gMesLanguageNo = GWLanguageGet();
    MgScoreWinInit();
}

void *GMesDataRead(unsigned int dataNum)
{
    return HuAR_ARAMtoMRAMFileRead(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
}

void GMesExec(void)
{
    GAMEMES *gMes;
    GMESTBL *mesTbl;
    int j;
    u8 stat;
    int i;
    BOOL result;
    gMesVWait = HuSysVWaitGet(gMesVWait);
    if(!Hu3DPauseF) {
        result = FALSE;
        stat = GMES_STAT_NONE;
        gMes = &gMesData[0];
        for(i=0; i<GMES_MAX; i++, gMes++) {
            if(gMes->stat == GMES_STAT_NONE) {
                continue;
            }
            if(gMes->exec) {
                result = gMes->exec(gMes);
            } else {
                mesTbl = &gMesTbl[gMes->mesNo];
                if(gMes->mesNo != GMES_MES_NULL && NULL != mesTbl->exec) {
                    result = mesTbl->exec(gMes);
                }
            }
            if(result == FALSE) {
                gMes->stat = GMES_STAT_NONE;
                if(!gMesDebugF) {
                    for(j=0; j<GMES_MAX; j++) {
                        if(gMesData[j].stat != GMES_STAT_NONE) {
                            break;
                        }
                    }
                }
                if(!gMes->buf) {
                    HuMemDirectFree(gMes->buf);
                    gMes->buf = NULL;
                }
            }
            stat |= gMes->stat;
        }
        if(stat == GMES_STAT_NONE || (stat & GMES_STAT_TIMEEND)) {
            if(gMesTime > 0) {
                gMesTime -= gMesVWait;
            }
        }
    }
}

static GMESID InitGMes(s16 mesNo, va_list args)
{
    GAMEMES *gMes = &gMesData[0];
    GMESTBL *mesTbl = &gMesTbl[(u8)mesNo];
    int i;
    int mesId;
    
    for(mesId=0; mesId<GMES_MAX; mesId++, gMes++) {
        if(gMes->stat == GMES_STAT_NONE) {
            break;
        }
    }
    if(mesId >= GMES_MAX) {
        return GMES_ID_NONE;
    }
    gMes->stat |= GMES_STAT_EXIST;
    if(gMes->buf) {
        HuMemDirectFree(gMes->buf);
    }
    gMes->buf = NULL;
    gMes->mesNo = mesNo;
    gMes->time = 0;
    gMes->pos.x = mesTbl->pos.x;
    gMes->pos.y = mesTbl->pos.y;
    gMes->scale.x = mesTbl->scale.x;
    gMes->scale.y = mesTbl->scale.y;
    gMes->unk18 = 0;
    gMes->color.g = 255;
    gMes->timeMax = mesTbl->timeMax;
    gMes->timerVal = gMes->subMode = gMes->work = gMes->charNum = 0;
    gMes->angle = gMes->winScale = 0;
    gMes->dispMode = gMes->dispValue = 0;
    for(i=0; i<GMES_STRMAX; i++) {
        gMes->strSprId[i] = gMes->strGId[i] = HUSPR_NONE;
    }
    if(NULL != mesTbl->create) {
        i = mesTbl->create(gMes, args);
        if(i == FALSE) {
            gMes->stat = GMES_STAT_NONE;
            return GMES_ID_NONE;
        }
    }
    gMesTime = 30;
    return mesId;
}

GMESID GMesCreate(s16 mesNo, ...)
{
    GMESID id;
    va_list args;
	va_start(args, mesNo);
    
    if(mesNo == GMES_MES_MG) {
        if(omcurovl == DLL_m433dll || omcurovl == DLL_m580dll) {
            mesNo = GMES_MES_MG_2VS2;
        } else if(omcurovl == DLL_sd00dll) {
            mesNo = GMES_MES_MG_SD;
        } else if(omcurovl == DLL_m562dll) {
            mesNo = GMES_MES_MG_LAST;
        } else if(GwSystem.mgNo == GW_MGNO_NONE) {
            mesNo = GMES_MES_MG_4P;
        } else {
            mesNo = startMesTbl[mgDataTbl[GwSystem.mgNo].type];
        }
    }
    id = InitGMes(mesNo, args);
    va_end(args);
    return id;
}

u8 GMesStatGet(GMESID id)
{
    GAMEMES *gMes;
    u8 stat;
    stat = GMES_STAT_NONE;
    if(id < 0) {
        int i;
        gMes = &gMesData[0]; 
        for(i=GMES_MAX; i; i--, gMes++) {
            stat |= gMes->stat;
        }
    } else {
        if(id < GMES_MAX) {
            stat = gMesData[id].stat;
        }
    }
    return stat;
}

void GMesPosSet(GMESID id, float posX, float posY)
{
    if(id < 0) {
        return;
    }
    if(id >= GMES_MAX) {
        return;
    }
    gMesData[id].pos.x = posX;
    gMesData[id].pos.y = posY;
}

void GMesDispSet(GMESID id, s16 mode, s16 value)
{
    if(id < 0) {
        return;
    }
    if(id >= GMES_MAX) {
        return;
    }
    gMesData[id].dispMode = mode;
    gMesData[id].dispValue = value;
}

void GMesKill(GMESID id)
{
    if(id < 0) {
        return;
    }
    if(id >= GMES_MAX) {
        return;
    }
    if(gMesData[id].stat == GMES_STAT_NONE) {
        return;
    }
    gMesData[id].stat = GMES_STAT_KILL;
}

void GMesClose(void)
{
    gMesCloseF = TRUE;
    GMesExec();
    gMesCloseF = FALSE;
    GMesStub();
    gMesDebugF = FALSE;
}

BOOL GMesKillCheck(void)
{
    u8 stat = GMesStatGet(GMES_ID_NONE);
    if(stat == GMES_STAT_NONE || (stat & (GMES_STAT_TIMEEND|GMES_STAT_KILL))) {
        if(gMesTime <= 0 || (stat & GMES_STAT_KILL)) {
            return TRUE;
        }
    }
    return FALSE;
}

void GMesStub(void)
{
    
}

void GMesSprKill(GAMEMES *gMes)
{
    int i;
    for(i=0; i<GMES_STRMAX; i++) {
        if(gMes->strGId[i] >= 0) {
            HuSprGrpKill(gMes->strGId[i]);
        }
        if(gMes->strSprId[i] >= 0) {
            HuSprKill(gMes->strSprId[i]);
        }
    }
}

#define TIMER_SUBMODE_OFF 1
#define TIMER_SUBMODE_SHOW 2
#define TIMER_SUBMODE_DIGITUP 3

BOOL GMesTimerInit(GAMEMES *gMes, va_list args)
{
    HUSPRGRPID gid;
    ANIMDATA *anim;
    HUSPRID sprid;
    
    int maxTime, posX, posY;
    s16 i;
    maxTime = va_arg(args, int);
    posX = va_arg(args, int);
    posY = va_arg(args, int);
    if(maxTime <= 0 && maxTime > 99) {
        maxTime = 99;
    }
    gMes->timerVal = maxTime;
    if(posX >= 0) {
        gMes->pos.x = posX;
    }
    if(posY >= 0) {
        gMes->pos.y = posY;
    }
    gMes->work = 0;
    gMes->subMode = TIMER_SUBMODE_SHOW;
    gMes->angle = 0;
    gid = HuSprGrpCreate(4);
    gMes->strGId[0] = gid;
    HuSprGrpScaleSet(gid, gMes->scale.x, gMes->scale.y);
    anim = HuSprAnimRead(GMesDataRead(GAMEMES_ANM_timerDigit));
    posX = 12;
    for(i=0; i<2; posX -= 24, i++) {
        sprid = HuSprCreate(anim, 5, 0);
        HuSprGrpMemberSet(gid, i, sprid);
        HuSprSpeedSet(gid, i, 0);
        HuSprPosSet(gid, i, posX, 0);
        HuSprColorSet(gid, i, 112, 233, 255);
    }
    anim = HuSprAnimRead(GMesDataRead(GAMEMES_ANM_timerBack));
    sprid = HuSprCreate(anim, 7, 0);
    HuSprGrpMemberSet(gid, 2, sprid);
    HuSprPosSet(gid, 2, 0, 0);
    HuSprTPLvlSet(gid, 2, 0.5f);
    HuSprColorSet(gid, 2, 0, 0, 0);
    anim = HuSprAnimRead(GMesDataRead(GAMEMES_ANM_timer));
    sprid = HuSprCreate(anim, 6, 0);
    HuSprGrpMemberSet(gid, 3, sprid);
    HuSprPosSet(gid, 3, 0, 0);
    HuSprGrpPosSet(gid, -100, -100);
    return TRUE;
}

BOOL GMesTimerExec(GAMEMES *gMes)
{
    HUSPRGRPID gid = gMes->strGId[0];
    s16 i;
    u8 digit[2];
    float scale, tpLvl;
    if(gMes->dispMode != 0 && gMes->subMode != GMES_SUBMODE_NULL) {
        switch(gMes->dispMode) {
            case GMES_DISP_SET:
                switch(gMes->dispValue) {
                    case -1:
                        gMes->stat |= GMES_STAT_TIMEEND;
                        gMes->subMode = GMES_SUBMODE_NULL;
                        gMes->angle = 0;
                        break;
                    
                    case 0:
                        gMes->subMode = TIMER_SUBMODE_SHOW;
                        gMes->angle = 0;
                        break;
                    
                    case 1:
                        gMes->subMode = TIMER_SUBMODE_DIGITUP;
                        gMes->angle = 0;
                        break;
                }
                gMes->dispMode = GMES_DISP_NONE;
                break;
            
            case GMES_DISP_UPDATE:
                if(gMes->dispValue < 0 && !(gMes->stat & GMES_STAT_TIMEEND)) {
                    gMes->stat |= GMES_STAT_TIMEEND;
                    gMes->subMode = GMES_SUBMODE_NULL;
                    gMes->angle = 0;
                } else {
                    if(gMes->dispValue > 99) {
                        gMes->timerVal = 99;
                    } else {
                        if(gMes->timerVal != gMes->dispValue) {
                            gMes->timerVal = gMes->dispValue;
                            if(gMes->dispValue <= 5) {
                                HuAudFXPlay(MSM_SE_CMN_07);
                                gMes->subMode = TIMER_SUBMODE_DIGITUP;
                                gMes->angle = 0;
                                HuSprColorSet(gid, 0, 255, 112, 160);
                                HuSprColorSet(gid, 1, 255, 112, 160);
                            } else {
                                HuSprColorSet(gid, 0, 112, 233, 255);
                                HuSprColorSet(gid, 1, 112, 233, 255);
                            }
                        }
                    }
                }
                gMes->dispMode = GMES_DISP_NONE;
                break;
            
            case GMES_DISP_HIDE:
                for(i=0; i<2; i++) {
                    HuSprDispOn(gid, i);
                }
                gMes->subMode = TIMER_SUBMODE_DIGITUP;
                gMes->dispMode = GMES_DISP_NONE;
                break;
            
            default:
                gMes->dispMode = GMES_DISP_NONE;
                break;
        }
    }
    if(gMes->subMode == TIMER_SUBMODE_OFF) {
        return TRUE;
    }
    if(gMes->timerVal > 99) {
        digit[0] = digit[1] = 9;
    } else {
        int value = gMes->timerVal/10;
        digit[1] = value;
        digit[0] = gMes->timerVal-(value*10);
    }
    HuSprGrpPosSet(gid, gMes->pos.x, gMes->pos.y);
    HuSprGrpScaleSet(gid, gMes->scale.x, gMes->scale.y);
    for(i=0; i<2; i++) {
        HuSprBankSet(gid, i, digit[i]);
    }
    if(gMes->subMode != GMES_SUBMODE_NONE) {
        switch(gMes->subMode) {
            case TIMER_SUBMODE_SHOW:
            {
                float scaleX, scaleY;
                scale = fabs(((5*HuSin(gMes->angle))+1)-(HuSin(130)*5));
                scaleX = gMes->scale.x*scale;
                scaleY = gMes->scale.y*scale;
                gMes->angle += 5.0f*gMesVWait;
                if(gMes->angle > 130) {
                    gMes->subMode = GMES_SUBMODE_NONE;
                } else {
                    HuSprGrpScaleSet(gid, scaleX, scaleY);
                }
            }
                break;
            
            case TIMER_SUBMODE_DIGITUP:
                scale = 1+HuSin(gMes->angle);
                tpLvl = 1-(0.5*HuSin(gMes->angle));
                gMes->angle += 18.0f*gMesVWait;
                if(gMes->angle > 180) {
                    gMes->subMode = GMES_SUBMODE_NONE;
                    scale = 1;
                    tpLvl = 1;
                }
                for(i=0; i<2; i++) {
                    HuSprScaleSet(gid, i, scale, scale);
                    HuSprTPLvlSet(gid, i, tpLvl);
                }
                break;
            
            case GMES_SUBMODE_NULL:
                HuSprGrpScaleSet(gid, gMes->scale.x, gMes->scale.y);
                for(i=0; i<2; i++) {
                    HuSprScaleSet(gid, i, 1, 1);
                    HuSprTPLvlSet(gid, i, 1);
                }
                gMes->angle++;
                if(gMes->angle < 60) {
                    break;
                }
                tpLvl = 1.0-((gMes->angle-60)/20);
                if(tpLvl <= 0) {
                    tpLvl = 0;
                    gMes->subMode = GMES_SUBMODE_NONE;
                    gMes->stat |= GMES_STAT_KILL;
                }
                for(i=0; i<4; i++) {
                    HuSprTPLvlSet(gid, i, tpLvl);
                }
                break;
        }
    }
    if(gMesCloseF || (gMes->stat & GMES_STAT_KILL)) {
        GMesSprKill(gMes);
        return FALSE;
    }
    return TRUE;
}

#undef TIMER_SUBMODE_OFF
#undef TIMER_SUBMODE_SHOW
#undef TIMER_SUBMODE_DIGITUP


static void CreateFontBuf(void)
{
    if(!fontBufs[0]) {
        fontBufs[0] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, strlen(asciiTbl)*sizeof(int));
        memset(fontBufs[0], 0, strlen(asciiTbl)*sizeof(int));
    }
    if(!fontBufs[1]) {
        fontBufs[1] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, strlen(kanaTbl)*sizeof(int));
        memset(fontBufs[1], 0, strlen(kanaTbl)*sizeof(int));
    }
    if(!fontBufs[2]) {
        fontBufs[2] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, strlen(kanaTbl)*sizeof(int));
        memset(fontBufs[2], 0, strlen(kanaTbl)*sizeof(int));
    }
    if(!fontBufs[3]) {
        fontBufs[3] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, strlen(numberTbl)*sizeof(int));
        memset(fontBufs[3], 0, strlen(numberTbl)*sizeof(int));
    }
    if(!fontBufs[4]) {
        fontBufs[4] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, strlen(punctTbl)*sizeof(int));
        memset(fontBufs[4], 0, strlen(punctTbl)*sizeof(int));
    }
}

static ANIMDATA *CreateFontChar(char *str, s16 flag);

#define STR_CHAR_MAX 100

int GMesStrCreate(GAMEMES *gMes, char *str, s16 flag)
{
    s16 charNum;
    s16 x;
    s16 i;
    s16 strId;
    char *s;
    ANIMDATA **charAnimTbl;
    s16 *charX;
    s16 spaceNum;
    HUSPRGRPID gid;
    
    for(strId=0; strId<GMES_STRMAX; strId++) {
        if(gMes->strGId[strId] == HUSPR_GRP_NONE) {
            break;
        }
    }
    if(strId == GMES_STRMAX) {
        return GMES_STR_NONE;
    }
    charAnimTbl = HuMemDirectMalloc(HUHEAPTYPE_HEAP, STR_CHAR_MAX*sizeof(ANIMDATA *));
    charX = HuMemDirectMalloc(HUHEAPTYPE_HEAP, STR_CHAR_MAX*sizeof(s16));

    for(s=str, x=0, charNum=spaceNum=0; *s; s++) {
        if(*s == ' ') {
            x += 56;
            spaceNum++;
        } else {
            charAnimTbl[charNum] = CreateFontChar(s, flag);
            if(charAnimTbl[charNum]) {
                charX[charNum] = x;
                x += 56;
                charNum++;
            }
        }
    }
    gid = HuSprGrpCreate(charNum+spaceNum);
    gMes->strGId[strId] = gid;
    x = (x/2)-28;
    for(i=0; i<charNum; i++) {
        HUSPRID sprid = HuSprCreate(charAnimTbl[i], 5, 0);
        HuSprGrpMemberSet(gid, i, sprid);
        HuSprPosSet(gid, i, charX[i]-x, 0);
    }
    gMes->charNum = charNum;
    HuMemDirectFree(charAnimTbl);
    HuMemDirectFree(charX);
    return strId;
}

int GMesStrCopy(GAMEMES *gMes, s16 id)
{
    s16 strId;
    for(strId=0; strId<GMES_STRMAX; strId++) {
        if(gMes->strGId[strId] == HUSPR_GRP_NONE) {
            break;
        }
    }
    if(strId == GMES_STRMAX) {
        return GMES_STR_NONE;
    }
    gMes->strGId[strId] = HuSprGrpCopy(gMes->strGId[id]);
    return strId;
}

static ANIMDATA *CreateFontChar(char *str, s16 flag)
{
    char *tbl;
    s16 i;
    unsigned int dataNum;
    char c;
    c = *str;
    if(c == 222 || c == 223) {
        return NULL;
    }
    for(i=0,tbl=asciiTbl; *tbl; i++, tbl++) {
        if(*tbl == c) {
            dataNum = GAMEMES_ANM_letterAUpper+i;
            return HuSprAnimRead(GMesDataRead(dataNum));
        }
    }
    for(i=0,tbl=numberTbl; *tbl; i++, tbl++) {
        if(*tbl == c) {
            dataNum = GAMEMES_ANM_number0+i;
            return HuSprAnimRead(GMesDataRead(dataNum));
        }
    }
    for(i=0,tbl=punctTbl; *tbl; i++, tbl++) {
        if(*tbl == c) {
            dataNum = GAMEMES_ANM_punctExcl+i;
            return HuSprAnimRead(GMesDataRead(dataNum));
        }
    }
    return NULL;
}

static void ExecPause(void);

void GMesPauseCreate(void)
{
    GMesPauseEnable(FALSE);
    HuWinInit(1);
    pauseProc = HuPrcCreate(ExecPause, 100, 4096, 0);
    HuPrcSetStat(pauseProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    pauseEnableF = TRUE;
    pauseCancelF = FALSE;
    pauseWaitF = FALSE;
}

static u32 m562CtrlMesTbl[] = {
    0,
    MGINST_M562_2,
    0,
    0,
    MGINST_M562_CTRL2,
    0,
    0,
    MGINST_M562_CTRL2,
    0,
    0,
    MGINST_M562_2,
    0,
};

static void ExecPause(void)
{
    GAMEMES gMes;
    s16 charNo[GW_PLAYER_MAX][GW_PLAYER_MAX];
    s16 charNum[GW_PLAYER_MAX];
    HUWINID winId[3] = { HUWIN_NONE, HUWIN_NONE, HUWIN_NONE };
    static char *pauseStr[] = {
		"ﾎﾟｰｽﾞ",
		"PAUSE"
	};

    s16 i;
    u32 *instMes;
    s16 j;
    s16 mgNo;
    float time;
    
    HuAudFXPlay(MSM_SE_CMN_06);
    for(i=0; i<GMES_STRMAX; i++) {
        gMes.strSprId[i] = gMes.strGId[i] = HUSPR_NONE;
    }
    GMesStrCreate(&gMes, pauseStr[(gMesLanguageNo == GW_LANGUAGE_JPN) ? 0 : 1], FALSE);
    for(i=0; i<gMes.charNum; i++) {
        HuSprPriSet(gMes.strGId[0], i, 0);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charNum[i] = 0;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charNo[GwPlayerConf[i].grp][charNum[GwPlayerConf[i].grp]] = GwPlayerConf[i].charNo;
		charNum[GwPlayerConf[i].grp]++;
    }
    mgNo = MgNoGet(omcurovl);
    if(_CheckFlag(FLAG_DECA_INST)) {
        instMes = &mgDataTbl[mgNo].instMes[1][0];
    } else {
        instMes = &mgDataTbl[mgNo].instMes[0][0];
    }
    if(omcurovl == DLL_m562dll) {
        instMes = &m562CtrlMesTbl[(m562CtrlNo-1)*3];
    }
    if(!instMes[1] && !instMes[2]) {
        for(i=1; i<=20; i++) {
            time = HuSin(i*(90.0f/20.0f));
            HuSprGrpPosSet(gMes.strGId[0], 288, (290*time)-50);
            HuPrcVSleep();
        }
    } else {
        if(mgPauseExitF && !_CheckFlag(FLAG_MG_PRACTICE)) {
            winId[2] = HuWinExCreateFrame(-10000, 400, 412, 42, HUWIN_SPEAKER_NONE, 0);
            HuWinPriSet(winId[2], 0);
            HuWinDispOn(winId[2]);
            HuWinMesSpeedSet(winId[2], 0);
            HuWinAttrSet(winId[2], HUWIN_ATTR_ALIGN_CENTER);
            HuWinMesSet(winId[2], MGINSTSYS_BTN_EXIT);
        }
        if(instMes[2]) {
            s16 insertMesNo;
            winId[0] = HuWinExCreateFrame(-10000, 140, 412, 120, HUWIN_SPEAKER_NONE, 0);
            HuWinPriSet(winId[0], 0);
            HuWinDispOn(winId[0]);
            HuWinMesSpeedSet(winId[0], 0);
            HuWinMesSet(winId[0], instMes[1]);
            winId[1] = HuWinExCreateFrame(-10000, 276, 412, 120, HUWIN_SPEAKER_NONE, 0);
            HuWinPriSet(winId[1], 0);
            HuWinDispOn(winId[1]);
            HuWinMesSpeedSet(winId[1], 0);
            HuWinMesSet(winId[1], instMes[2]);
            for(i=insertMesNo=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<charNum[i]; j++) {
                    HuWinInsertMesSet(winId[0], charNo[i][j], (int)insertMesNo);
                    HuWinInsertMesSet(winId[1], charNo[i][j], (int)insertMesNo);
                    insertMesNo++;
                }
            }
            for(i=1; i<=20; i++) {
                time = HuSin(i*(90.0f/20.0f));
                HuSprGrpPosSet(gMes.strGId[0], 288, (150*time)-50);
                HuWinPosSet(winId[0], (482*time)-400, 140);
                HuWinPosSet(winId[1], (-318*time)+400, 272);
                if(winId[2] != HUWIN_NONE) {
                    HuWinPosSet(winId[2], 82, (100*(1.0-time))+404);
                }
                HuPrcVSleep();
            }
        } else {
            winId[0] = HuWinExCreateFrame(-10000, 170, 412, 120, HUWIN_SPEAKER_NONE, 0);
            HuWinPriSet(winId[0], 0);
            HuWinDispOn(winId[0]);
            HuWinMesSpeedSet(winId[0], 0);
            HuWinMesSet(winId[0], instMes[1]);
            for(i=1; i<=20; i++) {
                time = HuSin(i*(90.0f/20.0f));
                HuSprGrpPosSet(gMes.strGId[0], 288, (150*time)-50);
                HuWinPosSet(winId[0], (482*time)-400, 170);
                if(winId[2] != HUWIN_NONE) {
                    HuWinPosSet(winId[2], (-318*time)+400, 404);
                }
                HuPrcVSleep();
            }
        }
    }
    GMesPauseEnable(TRUE);
    pauseWaitF = TRUE;
    while(!pauseCancelF) {
        HuPrcVSleep();
    }
    pauseWaitF = FALSE;
    if(winId[0] == HUWIN_NONE && winId[1] == HUWIN_NONE) {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(gMes.strGId[0], 288, (290*time)-50);
            HuPrcVSleep();
        }
    } else if(winId[1] != HUWIN_NONE) {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(gMes.strGId[0], 288, (150*time)-50);
            HuWinPosSet(winId[0], (482*time)-400, 140);
            HuWinPosSet(winId[1], (-318*time)+400, 272);
            if(winId[2] != HUWIN_NONE) {
                HuWinPosSet(winId[2], 82, (100*(1.0-time))+404);
            }
            HuPrcVSleep();
        }
    } else {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(gMes.strGId[0], 288, (150*time)-50);
            HuWinPosSet(winId[0], (482*time)-400, 170);
            if(winId[2] != HUWIN_NONE) {
                HuWinPosSet(winId[2], (-318*time)+400, 404);
            }
            HuPrcVSleep();
        }
    }
    omSysPauseCtrl(FALSE);
    if(winId[0] != HUWIN_NONE) {
        HuWinKill(winId[0]);
    }
    if(winId[1] != HUWIN_NONE) {
        HuWinKill(winId[1]);
    }
    if(winId[2] != HUWIN_NONE) {
        HuWinKill(winId[2]);
    }
    HuSprGrpKill(gMes.strGId[0]);
    pauseProc = FALSE;
    pauseEnableF = FALSE;
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void GMesPauseCancel(void)
{
    pauseCancelF = TRUE;
}

void GMesPauseEnable(BOOL enable)
{
    if(!_CheckFlag(FLAG_MG_PAUSE_OFF)) {
        omSysPauseEnable(enable);
    }
}

#define PRACTICE_POS_TOP 0
#define PRACTICE_POS_BOTTOM 1
#define PRACTICE_POS_CENTER 2
#define PRACTICE_POS_NUM 3

typedef struct Practice_s {
    s16 ovl;
    s16 posNo;
} PRACTICE;

static PRACTICE practiceTbl[] = {
    DLL_m501dll, PRACTICE_POS_BOTTOM,
    DLL_m502dll, PRACTICE_POS_BOTTOM,
    DLL_m503dll, PRACTICE_POS_BOTTOM,
    DLL_m504dll, PRACTICE_POS_CENTER,
    DLL_m505dll, PRACTICE_POS_BOTTOM,
    DLL_m506dll, PRACTICE_POS_BOTTOM,
    DLL_m507dll, PRACTICE_POS_CENTER,
    DLL_m508dll, PRACTICE_POS_BOTTOM,
    DLL_m509dll, PRACTICE_POS_BOTTOM,
    DLL_m510dll, PRACTICE_POS_TOP,
    DLL_m511dll, PRACTICE_POS_CENTER,
    DLL_m512dll, PRACTICE_POS_TOP,
    DLL_m513dll, PRACTICE_POS_BOTTOM,
    DLL_m514dll, PRACTICE_POS_TOP,
    DLL_m515dll, PRACTICE_POS_CENTER,
    DLL_m516dll, PRACTICE_POS_BOTTOM,
    DLL_m517dll, PRACTICE_POS_BOTTOM,
    DLL_m518dll, PRACTICE_POS_BOTTOM,
    DLL_m519dll, PRACTICE_POS_TOP,
    DLL_m520dll, PRACTICE_POS_BOTTOM,
    DLL_m521dll, PRACTICE_POS_TOP,
    DLL_m522dll, PRACTICE_POS_BOTTOM,
    DLL_m523dll, PRACTICE_POS_BOTTOM,
    DLL_m524dll, PRACTICE_POS_BOTTOM,
    DLL_m525dll, PRACTICE_POS_BOTTOM,
    DLL_m526dll, PRACTICE_POS_TOP,
    DLL_m527dll, PRACTICE_POS_BOTTOM,
    DLL_m528dll, PRACTICE_POS_BOTTOM,
    DLL_m529dll, PRACTICE_POS_BOTTOM,
    DLL_m530dll, PRACTICE_POS_TOP,
    DLL_m531dll, PRACTICE_POS_BOTTOM,
    DLL_m532dll, PRACTICE_POS_BOTTOM,
    DLL_m533dll, PRACTICE_POS_BOTTOM,
    DLL_m534dll, PRACTICE_POS_BOTTOM,
    DLL_m535dll, PRACTICE_POS_BOTTOM,
    DLL_m536dll, PRACTICE_POS_BOTTOM,
    DLL_m537dll, PRACTICE_POS_BOTTOM,
    DLL_m538dll, PRACTICE_POS_TOP,
    DLL_m539dll, PRACTICE_POS_CENTER,
    DLL_m540dll, PRACTICE_POS_TOP,
    DLL_m541dll, PRACTICE_POS_BOTTOM,
    DLL_m542dll, PRACTICE_POS_BOTTOM,
    DLL_m543dll, PRACTICE_POS_BOTTOM,
    DLL_m544dll, PRACTICE_POS_BOTTOM,
    DLL_m545dll, PRACTICE_POS_BOTTOM,
    DLL_m546dll, PRACTICE_POS_BOTTOM,
    DLL_m547dll, PRACTICE_POS_BOTTOM,
    DLL_m548dll, PRACTICE_POS_BOTTOM,
    DLL_m549dll, PRACTICE_POS_BOTTOM,
    DLL_m550dll, PRACTICE_POS_BOTTOM,
    DLL_m551dll, PRACTICE_POS_TOP,
    DLL_m552dll, PRACTICE_POS_BOTTOM,
    DLL_m553dll, PRACTICE_POS_BOTTOM,
    DLL_m554dll, PRACTICE_POS_BOTTOM,
    DLL_m555dll, PRACTICE_POS_BOTTOM,
    DLL_m559dll, PRACTICE_POS_BOTTOM,
    DLL_m560dll, PRACTICE_POS_TOP,
    DLL_m561dll, PRACTICE_POS_BOTTOM,
    DLL_m562dll, PRACTICE_POS_BOTTOM,
    DLL_m563dll, PRACTICE_POS_BOTTOM,
    DLL_m564dll, PRACTICE_POS_BOTTOM,
    DLL_m565dll, PRACTICE_POS_BOTTOM,
    DLL_m566dll, PRACTICE_POS_BOTTOM,
    DLL_m567dll, PRACTICE_POS_TOP,
    DLL_m568dll, PRACTICE_POS_BOTTOM,
    DLL_m569dll, PRACTICE_POS_BOTTOM,
    DLL_m570dll, PRACTICE_POS_BOTTOM,
    DLL_m571dll, PRACTICE_POS_BOTTOM,
    DLL_m572dll, PRACTICE_POS_BOTTOM,
    DLL_m573dll, PRACTICE_POS_TOP,
    DLL_m574dll, PRACTICE_POS_BOTTOM,
    DLL_m575dll, PRACTICE_POS_BOTTOM,
    DLL_m576dll, PRACTICE_POS_BOTTOM,
    DLL_m577dll, PRACTICE_POS_BOTTOM,
    DLL_m579dll, PRACTICE_POS_TOP,
    DLL_NONE, 0
};

static void ExecPractice(void);

void GMesPracticeCreate(void)
{
    HUPROCESS *parent = HuPrcCurrentGet();
    s16 i;
    if(!_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    wipeFadeInF = FALSE;
    for(i=0; practiceTbl[i].ovl != DLL_NONE; i++) {
        if(omcurovl == practiceTbl[i].ovl) {
            break;
        }
    }
    if(practiceTbl[i].ovl != DLL_NONE) {
        HuPrcChildCreate(ExecPractice, 10, 8192, 0, parent);
    }
}

static void ExecPractice(void)
{
    float angle;
    s16 id;
    HUSPRGRPID gid;
    MGDATA *mgP;
    BOOL exitF;
    PRACTICE *practiceP;
    HUSPRID sprid;
    ANIMDATA *anim;
    
    static float posTbl[PRACTICE_POS_NUM] = {
        53,
        424,
        240
    };

    angle = 0;
    for(id=0; practiceTbl[id].ovl != DLL_NONE; id++) {
        if(omcurovl == practiceTbl[id].ovl) {
            break;
        }
    }
    practiceP = &practiceTbl[id];
    id = MgNoGet(omcurovl);
    if(id != -1) {
        mgP = &mgDataTbl[id];
    } else {
        mgP = &mgDataTbl[0];
    }
    gid = HuSprGrpCreate(1);
    HuSprGrpPosSet(gid, 0, 0);
    anim = HuSprAnimRead(GMesDataRead(GAMEMES_ANM_practiceExit));
    sprid = HuSprCreate(anim, 1, 0);
    HuSprGrpMemberSet(gid, 0, sprid);
    HuSprPosSet(gid, 0, 288, posTbl[practiceP->posNo]);
    do {
        if(!wipeFadeInF || WipeCheck()) {
            HuSprDispOff(gid, 0);
            HuPrcVSleep();
            continue;
        }
        HuSprDispOn(gid, 0);
        exitF = FALSE;
        switch(mgP->type) {
            case MG_TYPE_4P:
            case MG_TYPE_1VS3:
            case MG_TYPE_2VS2:
            case MG_TYPE_BATTLE:
            case MG_TYPE_KUPA:
            case MG_TYPE_LAST:
            case MG_TYPE_DONKEY:
                for(id=0; id<GW_PLAYER_MAX; id++) {
                    if((HuPadBtnDown[GwPlayerConf[id].padNo] & PAD_TRIGGER_Z) && GwPlayerConf[id].type == GW_TYPE_MAN) {
                        break;
                    }
                }
                if(id != GW_PLAYER_MAX) {
                    exitF = TRUE;
                }
                break;
            
            case MG_TYPE_KETTOU:
                for(id=0; id<GW_PLAYER_MAX; id++) {
                    if((HuPadBtnDown[GwPlayerConf[id].padNo] & PAD_TRIGGER_Z) && GwPlayerConf[id].type == GW_TYPE_MAN  && GwPlayerConf[id].grp < 2) {
                        break;
                    }
                }
                if(id != GW_PLAYER_MAX) {
                    exitF = TRUE;
                }
                break;
            
            default:
                break;
        }
        if(exitF) {
            break;
        }
        HuSprTPLvlSet(gid, 0, fabs(HuSin(angle)));
        angle += 2;
        HuPrcVSleep();
    } while(1);
    HuSprDispOff(gid, 0);
    omSysExitReq = TRUE;
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

typedef struct StartSe_s {
    s16 ovl;
    s16 seId;
} STARTSE;


static STARTSE startSeTbl[] = {
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m502dll, MSM_SE_CMN_17,
    DLL_m503dll, MSM_SE_CMN_17,
    DLL_m504dll, MSM_SE_CMN_17,
    DLL_m505dll, MSM_SE_CMN_17,
    DLL_m506dll, MSM_SE_CMN_17,
    DLL_m507dll, MSM_SE_CMN_17,
    DLL_m508dll, MSM_SE_CMN_17,
    DLL_m509dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m512dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m515dll, MSM_SE_CMN_17,
    DLL_m516dll, MSM_SE_CMN_17,
    DLL_m517dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m519dll, MSM_SE_CMN_17,
    DLL_m520dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m522dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m524dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m526dll, MSM_SE_CMN_17,
    DLL_m527dll, MSM_SE_CMN_17,
    DLL_m528dll, MSM_SE_CMN_17,
    DLL_m529dll, MSM_SE_CMN_17,
    DLL_m530dll, MSM_SE_CMN_17,
    DLL_m531dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m533dll, MSM_SE_CMN_17,
    DLL_m534dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m536dll, MSM_SE_CMN_17,
    DLL_m537dll, MSM_SE_CMN_17,
    DLL_m538dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m540dll, MSM_SE_CMN_17,
    DLL_m541dll, MSM_SE_CMN_17,
    DLL_m542dll, MSM_SE_CMN_17,
    DLL_m543dll, MSM_SE_CMN_17,
    DLL_m544dll, MSM_SE_CMN_17,
    DLL_m545dll, MSM_SE_CMN_17,
    DLL_m546dll, MSM_SE_CMN_17,
    DLL_m547dll, MSM_SE_CMN_17,
    DLL_m548dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m551dll, MSM_SE_CMN_17,
    DLL_m552dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m563dll, MSM_SE_CMN_17,
    DLL_m564dll, MSM_SE_CMN_17,
    DLL_m565dll, MSM_SE_CMN_17,
    DLL_m566dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m568dll, MSM_SE_CMN_17,
    DLL_m569dll, MSM_SE_CMN_17,
    DLL_m501dll, MSM_SE_CMN_17,
    DLL_m571dll, MSM_SE_CMN_17,
    DLL_m572dll, MSM_SE_CMN_17,
    DLL_m573dll, MSM_SE_CMN_17,
    DLL_m574dll, MSM_SE_CMN_17,
    DLL_m575dll, MSM_SE_CMN_17,
    DLL_m576dll, MSM_SE_CMN_17,
    DLL_NONE, 0
};

void GMesStartSePlay(void)
{
    s16 i;
    for(i=0; startSeTbl[i].ovl != DLL_NONE; i++) {
        if(omcurovl == startSeTbl[i].ovl) {
            break;
        }
    }
    if(startSeTbl[i].ovl == DLL_NONE || startSeTbl[i].seId == MSM_SE_NONE) {
        return;
    }
    HuAudFXPlay(startSeTbl[i].seId);
}

void GMesExitCheck(OMOBJ *obj)
{
    mgExitStartF = FALSE;
    if(obj->work[0] == 0) {
        if(MgNoGet(omcurovl) == -1) {
            omDelObjEx(HuPrcCurrentGet(), obj);
            return;
        } else {
            obj->work[0]++;
        }
    }
    if(!omPauseChk() || _CheckFlag(FLAG_MG_PRACTICE) || !pauseWaitF) {
        return;
    }
    if(HuPadBtnDown[omDBGSysKeyObj->work[1]] & PAD_TRIGGER_Z) {
        HuAudFXPlay(MSM_SE_CMN_04);
        GMesPauseCancel();
        omSysPauseCtrl(FALSE);
        omSysExitReq = TRUE;
        mgExitStartF = TRUE;
        omDelObjEx(HuPrcCurrentGet(), obj);
    }
}

static void DrawScoreWin(HUSPRITE *sp)
{
    ANIMBMP *bmp = sp->bg->bmp;
    float sizeX = bmp->sizeX*16;
    float sizeY = bmp->sizeY*16;
    Mtx modelview;
    HuVec2F vtx[4];
    GXColor color;
    GXSetScissor(sp->scissorX, sp->scissorY, sp->scissorW, sp->scissorH);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    color.r = sp->r;
    color.g = sp->g;
    color.b = sp->b;
    color.a = sp->a;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_TEXA, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_TRUE);
    MTXScale(modelview, sp->scale.x, sp->scale.y, 1.0f);
    mtxTransCat(modelview, sp->pos.x, sp->pos.y, 0);
    MTXConcat(*sp->groupMtx, modelview, modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    HuSprTexLoad(scoreWinMaskAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(sp->bg, 0, GX_TEXMAP1, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(scoreWinAnim, 0, GX_TEXMAP2, GX_CLAMP, GX_CLAMP, GX_NEAR);
    GXSetNumIndStages(2);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, sizeX, sizeY);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_16, GX_ITS_16);
    GXSetTevIndTile(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0, GX_ITB_NONE, GX_ITBA_OFF);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD0, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_16, GX_ITS_16);
    GXSetTevIndTile(GX_TEVSTAGE1, GX_INDTEXSTAGE1, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0, GX_ITB_NONE, GX_ITBA_OFF);
    vtx[0].x = (-sizeX/2)/2;
    vtx[0].y = (-sizeY/2)/2;
    vtx[1].x = (sizeX/2)/2;
    vtx[1].y = (-sizeY/2)/2;
    vtx[2].x = (sizeX/2)/2;
    vtx[2].y = (sizeY/2)/2;
    vtx[3].x = (-sizeX/2)/2;
    vtx[3].y = (sizeY/2)/2;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(vtx[0].x, vtx[0].y, 0);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(vtx[1].x, vtx[1].y, 0);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(vtx[2].x, vtx[2].y, 0);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(vtx[3].x, vtx[3].y, 0);
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
}

static void ScoreWinBGMake(ANIMDATA *anim)
{
    s16 i, j;
    u8 *bmpData;
    s16 blockW;
    s16 w;
    s16 blockH;
    s16 h;

    w = anim->bmp->sizeX;
    h = anim->bmp->sizeY;
    blockW = (w+7) & 0xF8;
    blockH = (h+3) & 0xFC;
    bmpData = anim->bmp->data = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, blockW*blockH, HU_MEMNUM_OVL);
    for(i=0; i<h; i++) {
        if(i == 0) {
           for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x00;
                } else if(j == 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x10;
                } else if(j == w-2) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x20;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x30;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xE0;
                }
            }
        } else if(i == 1) {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x01;
                } else if(j == 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x11;
                } else if(j == w-2) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x21;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x31;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xE1;
                }
            }
        } else if(i == h-2) {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x40;
                } else if(j == 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x50;
                } else if(j == w-2) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x60;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x70;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xE1;
                }
            }
        } else if(i == h-1) {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x41;
                } else if(j == 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x51;
                } else if(j == w-2) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x61;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x71;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xC1;
                }
            }
        } else {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x80;
                } else if(j == 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x90;
                } else if(j == w-2) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xA0;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xB0;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0xE1;
                }
            }
        }
    }
    DCStoreRangeNoSync(anim->bmp->data, blockW*blockH);
}

void MgScoreWinInit(void)
{
    scoreWinMaskAnim = scoreWinAnim = NULL;
}

HUSPRGRPID MgScoreWinCreate(s16 sizeX, s16 sizeY)
{
    HUSPRGRPID gid;
    HUSPRID sprid;
    ANIMDATA *anim;
    sizeX = (sizeX+7) & 0xFFF8;
    sizeY = (sizeY+7) & 0xFFF8;
    gid = HuSprGrpCreate(1);
    if(!scoreWinMaskAnim) {
        scoreWinMaskAnim = HuSprAnimDataRead(MGCONST_ANM_scoreWinMask);
    }
    if(!scoreWinAnim) {
        scoreWinAnim = HuSprAnimDataRead(MGCONST_ANM_scoreWin);
    }
    sprid = HuSprFuncCreate(DrawScoreWin, 100);
    HuSprGrpMemberSet(gid, 0, sprid);
    HuSprData[sprid].bg = anim = HuSprAnimMake(sizeX/8, sizeY/8, ANIM_BMP_IA4);
    ScoreWinBGMake(anim);
    HuSprColorSet(gid, 0, 64, 64, 64);
    HuSprTPLvlSet(gid, 0, 0.7f);
    return gid;
}

void MgScoreWinColorSet(HUSPRGRPID gid, u8 r, u8 g, u8 b)
{
    HuSprColorSet(gid, 0, r, g, b);
}

void MgScoreWinTPLvlSet(HUSPRGRPID gid, float tpLvl)
{
    HuSprTPLvlSet(gid, 0, tpLvl);
}

void MgScoreWinPosSet(HUSPRGRPID gid, float posX, float posY)
{
    HuSprGrpPosSet(gid, posX, posY);
}

void MgScoreWinDispSet(HUSPRGRPID gid, BOOL disp)
{
    if(disp) {
        HuSprDispOn(gid, 0);
    } else {
        HuSprDispOff(gid, 0);
    }
}

#define FONT_NUMBER(c) (GAMEMES_ANM_number0+((c)-'0'))
#define FONT_LETTERUPPER(c) (GAMEMES_ANM_letterAUpper+((c)-'A'))
#define FONT_LETTERLOWER(c) (GAMEMES_ANM_letterALower+((c)-'a'))

static unsigned int fontFileTbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    FONT_NUMBER('0'),
    FONT_NUMBER('1'),
    FONT_NUMBER('2'),
    FONT_NUMBER('3'),
    FONT_NUMBER('4'),
    FONT_NUMBER('5'),
    FONT_NUMBER('6'),
    FONT_NUMBER('7'),
    FONT_NUMBER('8'),
    FONT_NUMBER('9'),
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontDash,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    FONT_LETTERUPPER('A'),
    FONT_LETTERUPPER('B'),
    FONT_LETTERUPPER('C'),
    FONT_LETTERUPPER('D'),
    FONT_LETTERUPPER('E'),
    FONT_LETTERUPPER('F'),
    FONT_LETTERUPPER('G'),
    FONT_LETTERUPPER('H'),
    FONT_LETTERUPPER('I'),
    FONT_LETTERUPPER('J'),
    FONT_LETTERUPPER('K'),
    FONT_LETTERUPPER('L'),
    FONT_LETTERUPPER('M'),
    FONT_LETTERUPPER('N'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('P'),
    FONT_LETTERUPPER('Q'),
    FONT_LETTERUPPER('R'),
    FONT_LETTERUPPER('S'),
    FONT_LETTERUPPER('T'),
    FONT_LETTERUPPER('U'),
    FONT_LETTERUPPER('V'),
    FONT_LETTERUPPER('W'),
    FONT_LETTERUPPER('X'),
    FONT_LETTERUPPER('Y'),
    FONT_LETTERUPPER('Z'),
    FONT_LETTERUPPER('O'),
    GAMEMES_ANM_fontApos,
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERLOWER('a'),
    FONT_LETTERLOWER('b'),
    FONT_LETTERLOWER('c'),
    FONT_LETTERLOWER('d'),
    FONT_LETTERLOWER('e'),
    FONT_LETTERLOWER('f'),
    FONT_LETTERLOWER('g'),
    FONT_LETTERLOWER('h'),
    FONT_LETTERLOWER('i'),
    FONT_LETTERLOWER('j'),
    FONT_LETTERLOWER('k'),
    FONT_LETTERLOWER('l'),
    FONT_LETTERLOWER('m'),
    FONT_LETTERLOWER('n'),
    FONT_LETTERLOWER('o'),
    FONT_LETTERLOWER('p'),
    FONT_LETTERLOWER('q'),
    FONT_LETTERLOWER('r'),
    FONT_LETTERLOWER('s'),
    FONT_LETTERLOWER('t'),
    FONT_LETTERLOWER('u'),
    FONT_LETTERLOWER('v'),
    FONT_LETTERLOWER('w'),
    FONT_LETTERLOWER('x'),
    FONT_LETTERLOWER('y'),
    FONT_LETTERLOWER('z'),
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontTilde,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontDash,
    GAMEMES_ANM_punctDot,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_punctExcl,
    GAMEMES_ANM_punctHatena,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0
};

GMESID GMesStrWinCreate(GAMEMES *gMes, u32 winMessId)
{
    char *mesPtr;
    s16 x;
    s16 *charY;
    unsigned int *fileTbl;
    s16 charNum;
    s16 i;
    GMESID strId;
    unsigned int dataNum;
    s16 *charX;
    ANIMDATA **charAnimTbl;
    HUSPRGRPID gid;
    HUSPRID sprid;
    HuWinInit(0);
    for(strId=0; strId<GMES_STRMAX; strId++) {
        if(gMes->strGId[strId] == HUSPR_GRP_NONE) {
            break;
        }
    }
    if(strId == GMES_STRMAX) {
        return GMES_STR_NONE;
    }
    fileTbl = fontFileTbl;
    charAnimTbl = HuMemDirectMalloc(HUHEAPTYPE_HEAP, STR_CHAR_MAX*sizeof(ANIMDATA *));
    charX = HuMemDirectMalloc(HUHEAPTYPE_HEAP, STR_CHAR_MAX*sizeof(s16));
    charY = HuMemDirectMalloc(HUHEAPTYPE_HEAP, STR_CHAR_MAX*sizeof(s16));
    for(mesPtr=HuWinMesPtrGet(winMessId), x=0, charNum=0; *mesPtr; mesPtr++) {
        if(mesPtr[0] == ' ' || mesPtr[0] == 16) {
            x += 56;
            continue;
        }
        if(mesPtr[0] < 48) {
            continue;
        }
        if(mesPtr[0] == 128) {
            continue;
        }
        if(mesPtr[0] == 129) {
            continue;
        }
        if(mesPtr[1] == 128) {
            if(mesPtr[0] >= 150 && mesPtr[0] <= 164) {
                dataNum = fileTbl[mesPtr[0]+106];
            } else if(mesPtr[0] >= 170 && mesPtr[0] <= 174) {
                dataNum = fileTbl[mesPtr[0]+101];
            } else if(mesPtr[0] >= 214 && mesPtr[0] <= 228) {
                dataNum = fileTbl[mesPtr[0]+67];
            } else if(mesPtr[0] >= 234 && mesPtr[0] <= 238) {
                dataNum = fileTbl[mesPtr[0]+62];
            }
        } else if(mesPtr[1] == 129) {
            if(mesPtr[0] >= 170 && mesPtr[0] <= 174) {
                dataNum = fileTbl[mesPtr[0]+106];
            } else if(mesPtr[0] >= 234 && mesPtr[0] <= 238) {
                dataNum = fileTbl[mesPtr[0]+67];
            }
        } else {
            dataNum = fileTbl[mesPtr[0]];
        }
        charAnimTbl[charNum] = HuSprAnimRead(GMesDataRead(dataNum));
        charX[charNum] = x;
        if(mesPtr[0] >= 'a' && mesPtr[0] <= 'z') {
            charY[charNum] = 2;
            x += 56;
        } else if(mesPtr[0] == 194 || mesPtr[0]  == 195) {
            charY[charNum] = 0;
            x += 56;
        } else if(mesPtr[0] == '\\') {
            charY[charNum] = 0;
            x += 56;
        } else if(mesPtr[0] >= 135 && mesPtr[0] <= 143) {
            charY[charNum] = 4;
            x += 56;
        } else if(mesPtr[0] >= 199 && mesPtr[0] <= 207) {
            charY[charNum] = 4;
            x += 56;
        } else {
            charY[charNum] = 0;
            x += 56;
        }
        charNum++;
    }
    gid = HuSprGrpCreate(charNum);
    gMes->strGId[strId] = gid;
    x = (x/2)-28;
    for(i=0; i<charNum; i++) {
        sprid = HuSprCreate(charAnimTbl[i], 0, 0);
        HuSprGrpMemberSet(gid, i, sprid);
        HuSprPosSet(gid, i, charX[i]-x, charY[i]);
    }
    gMes->charNum = charNum;
    HuMemDirectFree(charAnimTbl);
    HuMemDirectFree(charX);
    HuMemDirectFree(charY);
    return strId;
}