#include "game/gamemes.h"
#include "game/charman.h"


static int GameMesOfs;
static int GameMesOfsSpeed;
static float GameMesScale;
static float GameMesTPLvl;

static float SprPosX[10];
static float SprPosY[10];
static float SprAngle[10];

BOOL GameMesMg1Vs3Init(GAMEMES *mes, va_list args)
{
    s16 i;
    s16 j;
    GAMEMESID strMes;
    float x;
    float y;
    mes->subMode = va_arg(args, int);
    if(mes->subMode == GAMEMES_MG_TYPE_DRAW) {
        GameMesMgDrawInit(mes, args);
        return TRUE;
    }
    mes->mesMode = 1;
    mes->angle = 0;
    for(i=0; i<6; i++) {
        if(i == 0) {
            u32 mesId = (mes->subMode == GAMEMES_MG_TYPE_START) ? MESS_MGINSTSYS_MES_START : MESS_MGINSTSYS_MES_FINISH;
            strMes = GameMesStrWinCreate(mes, mesId);
            GameMesOfs = (mes->pos.x*2)-(mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+84)));
            GameMesOfsSpeed = GameMesOfs;
        } else {
            strMes = GameMesStrCopy(mes, strMes);
        }
        for(j=0; j<mes->charNum; j++) {
            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
            y = mes->pos.y;
            if(i == 0) {
                SprPosX[j] = x;
                SprPosY[j] = y;
            }
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(j == 0) {
                    y -= GameMesOfs;
                } else {
                    x += GameMesOfs;
                }
                HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
            } else {
                if(i == 0) {
                    HuSprScaleSet(mes->grpId[i], j, 0, 0);
                } else {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                    HuSprDispOff(mes->grpId[i], j);
                }
            }
            HuSprPosSet(mes->grpId[i], j, x, y);
            HuSprPriSet(mes->grpId[i], j, i+5);
            HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+1));
        }
    }
    if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
        GameMesOfs = 0;
        GameMesScale = 0;
    }
    GameMesPauseEnable(FALSE);
    return TRUE;
}

BOOL GameMesMg1Vs3Exec(GAMEMES *mes)
{
    s16 j; //r30
    s16 i; //r29
    
    float x; //f31
    float y; //f30
    
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode == 0) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    switch(mes->mesMode) {
        case 1:
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(mes->time <= 30) {
                    GameMesOfs -= (GameMesOfsSpeed/30)+6;
                    for(i=0; i<6; i++) {
                        for(j=0; j<mes->charNum; j++) {
                            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            y = mes->pos.y;
                            if(j == 0) {
                                y -= GameMesOfs;
                                y -= i*24;
                                if(y >= SprPosY[j]) {
                                    y = SprPosY[j];
                                    if(i != 0) {
                                        HuSprDispOff(mes->grpId[i], j);
                                    }
                                }
                                
                            } else {
                                x += GameMesOfs;
                                x += i*24;
                                if(x <= SprPosX[j]) {
                                    x = SprPosX[j];
                                    if(i != 0) {
                                        HuSprDispOff(mes->grpId[i], j);
                                    }
                                }
                            }
                            HuSprPosSet(mes->grpId[i], j, x, y);
                        }
                    }
                }
                if(mes->time >= 30) {
                    GameMesScale = (HuSin((mes->time-30)*9.0f)*0.5)+1.0;
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 30) {
                    HuAudFXPlay(MSM_SE_CMN_26);
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time == 50) {
                    mes->stat |= GAMEMES_STAT_TIMEEND;
                    GameMesStartSePlay();
                }
                if(mes->time >= 50) {
                    mes->mesMode = 0;
                    mes->timeMax = 95;
                }
            } else {
                if(mes->time <= 20) {
                    GameMesScale = HuSin(mes->time*4.5f);
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 20) {
                    if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                        HuAudFXPlay(MSM_SE_CMN_27);
                    } else {
                        HuAudFXPlay(MSM_SE_CMN_29);
                    }
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time >= 65) {
                    GameMesOfs += 30;
                    for(i=0; i<6; i++) {
                        for(j=0; j<mes->charNum; j++) {
                            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            y = mes->pos.y;
                            if(j == 0) {
                                y += GameMesOfs;
                                y -= i*24;
                                if(y < SprPosY[j]) {
                                    y = SprPosY[j];
                                }
                                HuSprDispOn(mes->grpId[i], j);
                            } else {
                                x -= GameMesOfs;
                                x += i*24;
                                if(x > SprPosX[j]) {
                                    x = SprPosX[j];
                                }
                                HuSprDispOn(mes->grpId[i], j);
                            }
                            HuSprPosSet(mes->grpId[i], j, x, y);
                        }
                    }
                }
                if(mes->time >= 100) {
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                }
            }
            break;
        
        case -1:
            mes->angle += GameMesVWait*0.1f;
            GameMesTPLvl = 1.0f-mes->angle;
            if(GameMesTPLvl <= 0) {
                GameMesTPLvl = 0;
                mes->mesMode = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            for(j=0; j<mes->charNum; j++) { 
                HuSprTPLvlSet(mes->grpId[0], j, GameMesTPLvl);
            }
            break;
        
        default:
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        if(mes->subMode == GAMEMES_MG_TYPE_START) {
            GameMesPauseEnable(TRUE);
        }
        return FALSE;
    }
    return TRUE;
}

static int BattleMesDelay;

BOOL GameMesMgBattleInit(GAMEMES *mes, va_list args)
{
    s16 i;
    s16 j;
    GAMEMESID strMes;
    float x;
    float y;
    mes->subMode = va_arg(args, int);
    if(mes->subMode == GAMEMES_MG_TYPE_DRAW) {
        GameMesMgDrawInit(mes, args);
        return TRUE;
    }
    mes->mesMode = 1;
    mes->angle = 0;
    for(i=0; i<6; i++) {
        if(i == 0) {
            u32 mesId = (mes->subMode == GAMEMES_MG_TYPE_START) ? MESS_MGINSTSYS_MES_START : MESS_MGINSTSYS_MES_FINISH;
            strMes = GameMesStrWinCreate(mes, mesId);
            GameMesOfs = mes->pos.x+28;
            GameMesOfsSpeed = GameMesOfs;
        } else {
            strMes = GameMesStrCopy(mes, strMes);
        }
        for(j=0; j<mes->charNum; j++) {
            if(i == 0) {
                SprPosX[j] = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                SprPosY[j] = mes->pos.y;
            }
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                switch(j%4) {
                    case 0:
                        x = mes->pos.x-GameMesOfs;
                        y = mes->pos.y-GameMesOfs;
                        break;
                    
                    case 1:
                        x = mes->pos.x+GameMesOfs;
                        y = mes->pos.y-GameMesOfs;
                        break;
                    
                    case 2:
                        x = mes->pos.x-GameMesOfs;
                        y = mes->pos.y+GameMesOfs;
                        break;
                    
                    case 3:
                        x = mes->pos.x+GameMesOfs;
                        y = mes->pos.y+GameMesOfs;
                        break;
                }
                HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
            } else {
                x = SprPosX[j];
                y = SprPosY[j];
                if(i == 0) {
                    HuSprScaleSet(mes->grpId[i], j, 0, 0);
                } else {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                    HuSprDispOff(mes->grpId[i], j);
                }
            }
            HuSprPosSet(mes->grpId[i], j, x, y);
            HuSprPriSet(mes->grpId[i], j, 5+i);
            HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+1));
        }
    }
    
    if(mes->subMode == GAMEMES_MG_TYPE_START) {
        BattleMesDelay = 0;
    } else {
        GameMesOfs = 0;
        GameMesScale = 0;
        BattleMesDelay = 0;
        SprAngle[0] = 0;
    }
    GameMesPauseEnable(FALSE);
    return TRUE;
}

BOOL GameMesMgBattleExec(GAMEMES *mes)
{
    s16 j; //r30
    s16 i; //r29
    s16 hideNum; //r28
    
    float x; //f31
    float y; //f30
    
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode == 0) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    switch(mes->mesMode) {
        case 1:
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(mes->time <= 60) {
                    if(BattleMesDelay == 0) {
                        GameMesOfs -= (GameMesOfsSpeed/30)+1;
                        for(hideNum=0, i=0; i<6; i++) {
                            for(j=0; j<mes->charNum; j++) { 
                                switch(j%4) {
                                    case 0:
                                        x = mes->pos.x-GameMesOfs;
                                        y = mes->pos.y-GameMesOfs;
                                        x -= i*24;
                                        y -= i*24;
                                        if(x > mes->pos.x) {
                                            x = mes->pos.x;
                                        }
                                        if(y > mes->pos.y) {
                                            y = mes->pos.y;
                                        }
                                        break;
                                    
                                    case 1:
                                        x = mes->pos.x+GameMesOfs;
                                        y = mes->pos.y-GameMesOfs;
                                        x += i*24;
                                        y -= i*24;
                                        if(x < mes->pos.x) {
                                            x = mes->pos.x;
                                        }
                                        if(y > mes->pos.y) {
                                            y = mes->pos.y;
                                        }
                                        break;
                                    
                                    case 2:
                                        x = mes->pos.x-GameMesOfs;
                                        y = mes->pos.y+GameMesOfs;
                                        x -= i*24;
                                        y += i*24;
                                        if(x > mes->pos.x) {
                                            x = mes->pos.x;
                                        }
                                        if(y < mes->pos.y) {
                                            y = mes->pos.y;
                                        }
                                        break;
                                    
                                    case 3:
                                        x = mes->pos.x+GameMesOfs;
                                        y = mes->pos.y+GameMesOfs;
                                        x += i*24;
                                        y += i*24;
                                        if(x < mes->pos.x) {
                                            x = mes->pos.x;
                                        }
                                        if(y < mes->pos.y) {
                                            y = mes->pos.y;
                                        }
                                        break;
                                }
                                HuSprPosSet(mes->grpId[i], j, x, y);
                                if(x == mes->pos.x && y == mes->pos.y && i != 0) {
                                    HuSprDispOff(mes->grpId[i], j);
                                    hideNum++;
                                }
                            }
                        }
                        if(hideNum == mes->charNum*5) {
                            BattleMesDelay = 1;
                            GameMesOfs = 0;
                        }
                    } else {
                        GameMesOfs += 10;
                        for(j=0; j<mes->charNum; j++) {
                            if(mes->charNum % 2) {
                                if(j < (mes->charNum-1)/2) {
                                    x = mes->pos.x-GameMesOfs;
                                    if(x < SprPosX[j]) {
                                        x = SprPosX[j];
                                    }
                                } else if(j > (mes->charNum-1)/2) {
                                    x = mes->pos.x+GameMesOfs;
                                    if(x > SprPosX[j]) {
                                        x = SprPosX[j];
                                    }
                                } else {
                                    x = SprPosX[j];
                                }
                            } else {
                                if(j < mes->charNum/2) {
                                    x = mes->pos.x-GameMesOfs;
                                    if(x < SprPosX[j]) {
                                        x = SprPosX[j];
                                    }
                                } else {
                                    x = mes->pos.x+GameMesOfs;
                                    if(x > SprPosX[j]) {
                                        x = SprPosX[j];
                                    }
                                }
                            }
                            y = mes->pos.y;
                            HuSprPosSet(mes->grpId[0], j, x, y);
                        }
                    }
                }
                if(mes->time >= 60) {
                    GameMesScale = (HuSin((mes->time-60)*9.0f)*0.5)+1.0;
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 60) {
                    HuAudFXPlay(MSM_SE_CMN_26);
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time == 80) {
                    mes->stat |= GAMEMES_STAT_TIMEEND;
                    GameMesStartSePlay();
                }
                if(mes->time >= 80) {
                    mes->mesMode = 0;
                    mes->timeMax = 125;
                }
            } else {
                if(mes->time <= 20) {
                    GameMesScale = HuSin(mes->time*4.5f);
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 20) {
                    if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                        HuAudFXPlay(MSM_SE_CMN_27);
                    } else {
                        HuAudFXPlay(MSM_SE_CMN_29);
                    }
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time >= 65) {
                    if(BattleMesDelay == 0) {
                        hideNum = 0;
                        GameMesOfs += 10;
                        for(j=0; j<mes->charNum; j++) {
                            x = SprPosX[j];
                            y = mes->pos.y;
                            if(SprPosX[j] < mes->pos.x) {
                                x += GameMesOfs;
                                if(x > mes->pos.x) {
                                    x = mes->pos.x;
                                }
                            }
                            if(SprPosX[j] > mes->pos.x) {
                                x -= GameMesOfs;
                                if(x < mes->pos.x) {
                                    x = mes->pos.x;
                                }
                            }
                            if(x == mes->pos.x) {
                                hideNum++;
                            }
                            HuSprPosSet(mes->grpId[0], j, x, y);
                        }
                        if(hideNum == mes->charNum) {
                            BattleMesDelay = 1;
                            GameMesOfs = 0;
                        }
                    } else {
                        if(++BattleMesDelay > 10) {
                            GameMesOfs += 5;
                            for(i=0; i<6; i++) {
                                for(j=0; j<mes->charNum; j++) { 
                                    float baseX = mes->pos.x;
                                    float baseY = mes->pos.y;
                                    switch(j%4) {
                                        case 0:
                                            baseX -= GameMesOfs;
                                            baseY -= GameMesOfs;
                                            x = (baseX+(50.0*HuCos(SprAngle[0])))-50.0f;
                                            y = baseY-(50.0*HuSin(SprAngle[0]));
                                            break;
                                        
                                        case 1:
                                            baseX += GameMesOfs;
                                            baseY -= GameMesOfs;
                                            x = (baseX-(50.0*HuCos(-SprAngle[0])))+50.0f;
                                            y = baseY+(50.0*HuSin(-SprAngle[0]));
                                            break;
                                        
                                        case 2:
                                            baseX -= GameMesOfs;
                                            baseY += GameMesOfs;
                                            x = (baseX+(50.0*HuCos(SprAngle[0])))-50.0f;
                                            y = baseY-(50.0*HuSin(-SprAngle[0]));
                                            break;
                                        
                                        case 3:
                                            baseX += GameMesOfs;
                                            baseY += GameMesOfs;
                                            x = (baseX-(50.0*HuCos(-SprAngle[0])))+50.0f;
                                            y = baseY+(50.0*HuSin(SprAngle[0]));
                                            break;
                                    }
                                    HuSprPosSet(mes->grpId[i], j, x, y);
                                    HuSprZRotSet(mes->grpId[i], j, ((j%4 == 1) || (j%4 == 2)) ? SprAngle[0] : -SprAngle[0]);
                                }
                            }
                            SprAngle[0] += 25;
                        }
                    }
                }
                if(mes->time >= 140) {
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                }
            }
            break;
        
        case -1:
            mes->angle += GameMesVWait*0.1f;
            GameMesTPLvl = 1.0f-mes->angle;
            if(GameMesTPLvl <= 0) {
                GameMesTPLvl = 0;
                mes->mesMode = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            for(j=0; j<mes->charNum; j++) { 
                HuSprTPLvlSet(mes->grpId[0], j, GameMesTPLvl);
            }
            break;
        
        default:
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        if(mes->subMode == GAMEMES_MG_TYPE_START) {
            GameMesPauseEnable(TRUE);
        }
        return FALSE;
    }
    return TRUE;
}

static int KoopaMesSpeed;
static int KoopaMesYOfs[5];

BOOL GameMesMgKupaInit(GAMEMES *mes, va_list args)
{
    s16 i;
    s16 j;
    GAMEMESID strMes;
    float x;
    float y;
    
    
    mes->subMode = va_arg(args, int);
    if(mes->subMode == GAMEMES_MG_TYPE_DRAW) {
        GameMesMgDrawInit(mes, args);
        return TRUE;
    }
    mes->mesMode = 1;
    mes->angle = 0;
    for(i=0; i<6; i++) {
        if(i == 0) {
            u32 mesId = (mes->subMode == GAMEMES_MG_TYPE_START) ? MESS_MGINSTSYS_MES_START : MESS_MGINSTSYS_MES_FINISH;
            strMes = GameMesStrWinCreate(mes, mesId);
            GameMesOfs = mes->pos.y+28;
            GameMesOfsSpeed = GameMesOfs;
        } else {
            strMes = GameMesStrCopy(mes, strMes);
        }
        for(j=0; j<mes->charNum; j++) {
            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
            y = mes->pos.y;
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(i == 0) {
                    SprPosX[j] = x;
                    SprPosY[j] = y;
                }
                if(j%2 == 0) {
                    y -= GameMesOfs;
                } else {
                    y += GameMesOfs;
                }
                HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+1));
            } else {
                if(i == 0) {
                    SprPosX[j] = x;
                    SprPosY[j] = y;
                    HuSprScaleSet(mes->grpId[i], j, 0, 0);
                } else {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                    HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+3));
                    HuSprDispOff(mes->grpId[i], j);
                }
            }
            HuSprPosSet(mes->grpId[i], j, x, y);
            HuSprPriSet(mes->grpId[i], j, i+5);
        }
    }
    if(mes->subMode == GAMEMES_MG_TYPE_START) {
        for(i=0; i<5; i++) {
            KoopaMesYOfs[i] = GameMesOfs;
        }
    } else {
        GameMesOfs = 0;
        GameMesScale = 0;
        SprAngle[0] = 0;
    }
    GameMesPauseEnable(FALSE);
    return TRUE;
}

BOOL GameMesMgKupaExec(GAMEMES *mes)
{
    s16 j;
    s16 i;
    
    float x;
    float y;
    
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode == 0) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    switch(mes->mesMode) {
        case 1:
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(mes->time <= 60) {
                    if(mes->time < 8) {
                        KoopaMesSpeed = 0;
                    }
                    if(mes->time == 8) {
                        KoopaMesSpeed = 8;
                    }
                    if(mes->time == 12) {
                        KoopaMesSpeed = 10;
                    }
                    if(mes->time >= 14) {
                        KoopaMesSpeed += 3;
                    }
                    GameMesOfs -= GameMesOfsSpeed/(KoopaMesSpeed+15);
                    for(i=0; i<5; i++) {
                        int temp[2];
                        if(i == 0) {
                            temp[0] = KoopaMesYOfs[0];
                            KoopaMesYOfs[0] = GameMesOfs;
                        } else {
                            temp[1] = KoopaMesYOfs[i];
                            KoopaMesYOfs[i] = temp[0];
                            temp[0] = temp[1];
                        }
                    }
                    for(i=0; i<6; i++) {
                        for(j=0; j<mes->charNum; j++) { 
                            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            y = mes->pos.y;
                            if(j%2 == 0) {
                                if(i == 0) {
                                    y -= GameMesOfs;
                                } else {
                                    y -= KoopaMesYOfs[i-1];
                                }
                                y -= i*8;
                                if(y >= SprPosY[j]) {
                                    y = SprPosY[j];
                                    if(i != 0) {
                                        HuSprDispOff(mes->grpId[i], j);
                                    }
                                }
                            } else {
                                if(i == 0) {
                                    y += GameMesOfs;
                                } else {
                                    y += KoopaMesYOfs[i-1];
                                }
                                y += i*8;
                                if(y <= SprPosY[j]) {
                                    y = SprPosY[j];
                                    if(i != 0) {
                                        HuSprDispOff(mes->grpId[i], j);
                                    }
                                }
                            }
                            HuSprPosSet(mes->grpId[i], j, x, y);
                        }
                    }
                }
                if(mes->time >= 60) {
                    GameMesScale = (HuSin((mes->time-60)*9.0f)*0.5)+1.0;
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 60) {
                    HuAudFXPlay(MSM_SE_CMN_26);
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time == 80) {
                    mes->stat |= GAMEMES_STAT_TIMEEND;
                    GameMesStartSePlay();
                }
                if(mes->time >= 80) {
                    mes->mesMode = 0;
                    mes->timeMax = 125;
                }
            } else {
                if(mes->time <= 20) {
                    GameMesScale = HuSin(mes->time*4.5f);
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 20) {
                    if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                        HuAudFXPlay(MSM_SE_CMN_27);
                    } else {
                        HuAudFXPlay(MSM_SE_CMN_29);
                    }
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                    KoopaMesSpeed = 24;
                    for(i=0; i<5; i++) {
                        KoopaMesYOfs[i] = 0;
                    }
                }
                if(mes->time >= 65) {
                    KoopaMesSpeed--;
                    if(KoopaMesSpeed < 3) {
                        KoopaMesSpeed = 3;
                    }
                    GameMesOfs += 45/(KoopaMesSpeed/3);
                    if(mes->time >= 66) {
                        for(i=0; i<5; i++) {
                            int temp[2];
                            if(i == 0) {
                                temp[0] = KoopaMesYOfs[0];
                                KoopaMesYOfs[0] = GameMesOfs;
                            } else {
                                temp[1] = KoopaMesYOfs[i];
                                KoopaMesYOfs[i] = temp[0];
                                temp[0] = temp[1];
                            }
                        }
                    }
                    for(i=0; i<6; i++) {
                        for(j=0; j<mes->charNum; j++) {
                            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            y = mes->pos.y;
                            if(j%2 == 0) {
                                if(i == 0) {
                                    y -= GameMesOfs;
                                } else {
                                    y -= KoopaMesYOfs[i-1];
                                }
                                y += i*12;
                                if(y > SprPosY[j]) {
                                    y = SprPosY[j];
                                }
                            } else {
                                if(i == 0) {
                                    y += GameMesOfs;
                                } else {
                                    y += KoopaMesYOfs[i-1];
                                }
                                y -= i*12;
                                if(y < SprPosY[j]) {
                                    y = SprPosY[j];
                                }
                            }
                            if(i != 0) {
                                HuSprDispOn(mes->grpId[i], j);
                            }
                            HuSprPosSet(mes->grpId[i], j, x, y);
                            HuSprZRotSet(mes->grpId[i], j, (j%2) ? SprAngle[0] : -SprAngle[0]);
                        }
                    }
                    SprAngle[0] += 30;
                }
                if(mes->time >= 100) {
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                }
            }
            break;
        
        case -1:
            mes->angle += GameMesVWait*0.1f;
            GameMesTPLvl = 1.0f-mes->angle;
            if(GameMesTPLvl <= 0) {
                GameMesTPLvl = 0;
                mes->mesMode = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            for(j=0; j<mes->charNum; j++) { 
                HuSprTPLvlSet(mes->grpId[0], j, GameMesTPLvl);
            }
            break;
        
        default:
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        if(mes->subMode == GAMEMES_MG_TYPE_START) {
            GameMesPauseEnable(TRUE);
        }
        return FALSE;
    }
    return TRUE;
}

static float SdMesScale[10];
static int SdMesScaleTime[10];
static int SdMesWork[2];
static int MesUnk1[10];
static float SdMesTPLvl[6][10];

BOOL GameMesMgSdInit(GAMEMES *mes, va_list args)
{
    s16 i;
    s16 j;
    GAMEMESID strMes;
    float x;
    float y;
    mes->subMode = va_arg(args, int);
    if(mes->subMode == GAMEMES_MG_TYPE_DRAW) {
        GameMesMgDrawInit(mes, args);
        return TRUE;
    }
    mes->mesMode = 1;
    mes->angle = 0;
    if(mes->subMode == GAMEMES_MG_TYPE_START) {
        for(j=0; j<10; j++) {
            SdMesScale[j] = 20;
            SdMesScaleTime[j] = 0;
        }
        GameMesScale = 0;
        SdMesWork[0] = SdMesWork[1] = 0;
    } else {
        for(j=0; j<10; j++) {
            MesUnk1[j] = 0;
            SdMesScale[j] = 1;
        }
        GameMesScale = 0;
        SdMesWork[0] = 1;
        SdMesWork[1] = 0;
    }
    for(i=0; i<6; i++) {
        if(i == 0) {
            u32 mesId = (mes->subMode == GAMEMES_MG_TYPE_START) ? MESS_MGINSTSYS_MES_START : MESS_MGINSTSYS_MES_FINISH;
            strMes = GameMesStrWinCreate(mes, mesId);
        } else {
            strMes = GameMesStrCopy(mes, strMes);
        }
        for(j=0; j<mes->charNum; j++) {
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                y = mes->pos.y;
                if(i == 0) {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x*SdMesScale[j], mes->scale.y*SdMesScale[j]);
                } else {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                }
            } else {
                x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                y = mes->pos.y;
                if(i == 0) {
                    SprPosX[j] = x;
                    SprPosY[j] = y;
                    
                    HuSprScaleSet(mes->grpId[i], j, 0, 0);
                } else {
                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                }
            }
            HuSprPosSet(mes->grpId[i], j, x, y);
            HuSprPriSet(mes->grpId[i], j, (mes->subMode == GAMEMES_MG_TYPE_START) ? (i+5) : (j+(i+5)));
            if(i == 0) {
                HuSprTPLvlSet(mes->grpId[i], j, (mes->subMode == GAMEMES_MG_TYPE_START) ? 0.0f : 1.0f);
                SdMesTPLvl[i][j] = (mes->subMode == GAMEMES_MG_TYPE_START) ? 0.0f : 1.0f;
            } else {
                HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+1));
                SdMesTPLvl[i][j] = 1.0f/(i+1);
            }
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                HuSprDispOff(mes->grpId[i], j);
            }
            if(i != 0 && mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                HuSprDispOff(mes->grpId[i], j);
            }
        }
    }
    GameMesPauseEnable(FALSE);
    return TRUE;
}

BOOL GameMesMgSdExec(GAMEMES *mes)
{
    s16 j; //r29
    s16 i; //r28
    
    int timeStart; //r27
    int timeEnd; //r26
    
    float x; //f31
    float y; //f30
    
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode == 0) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    switch(mes->mesMode) {
        case 1:
            if(mes->subMode == GAMEMES_MG_TYPE_START) {
                if(GameMesLanguageNo == HUWIN_LANG_JAPAN) {
                    timeStart = 110;
                    timeEnd = 130;
                } else {
                    timeStart = 123;
                    timeEnd = 143;
                }
                if(mes->time <= timeStart) {
                    if(SdMesWork[0] < mes->charNum) {
                        if(mes->time > 0) {
                            SdMesWork[0] = 1;
                        }
                        if(mes->time > 8) {
                            SdMesWork[0]++;
                        }
                        for(j=2; j<mes->charNum; j++) {
                            if(mes->time > ((j-2)*7)+45) {
                                SdMesWork[0]++;
                            }
                        }
                        if(SdMesWork[0] > mes->charNum) {
                            SdMesWork[0] = mes->charNum;
                        }
                    }
                    for(i=0; i<6; i++) {
                        for(j=0; j<SdMesWork[0]; j++) {
                            if(i == 0) {
                                SdMesScale[j] -= 20.0f/(timeStart/5);
                                if(SdMesScale[j] < 1) {
                                    SdMesScale[j] = 1;
                                }
                                HuSprDispOn(mes->grpId[i], j);
                            }
                            HuSprScaleSet(mes->grpId[i], j, mes->scale.x*SdMesScale[j], mes->scale.y*SdMesScale[j]);
                            if(i == 0) {
                                SdMesTPLvl[0][j] += 0.017f;
                                if(SdMesTPLvl[0][j] > 1) {
                                    SdMesTPLvl[0][j] = 1;
                                }
                                HuSprTPLvlSet(mes->grpId[i], j, SdMesTPLvl[0][j]);
                                x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            } else {
                                x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            }
                            y = mes->pos.y;
                            HuSprPosSet(mes->grpId[i], j, x, y);
                            if(i == 0) {
                                if(SdMesScale[j] == 1.0f) {
                                    SdMesScaleTime[j]++;
                                }
                            }
                            if(SdMesScaleTime[j] != 0) {
                                HuSprDispOn(mes->grpId[i], j);
                                HuSprScaleSet(mes->grpId[i], j, mes->scale.x*(1.0f+(0.04f*((SdMesScaleTime[j]/2)*i))), mes->scale.y*(1.0f+(0.04f*((SdMesScaleTime[j]/2)*i))));
                                if(i != 0) {
                                    SdMesTPLvl[i][j] -= 0.02f;
                                    if(SdMesTPLvl[i][j] < 0.01f) {
                                        SdMesTPLvl[i][j] = 0;
                                        HuSprDispOff(mes->grpId[i], j);
                                    } else {
                                        HuSprTPLvlSet(mes->grpId[i], j, SdMesTPLvl[i][j]);
                                        x += frandmod(7)*(0.1f*(frandmod(2) != 0 ? 1 : -1));
                                        y += frandmod(7)*(0.1f*(frandmod(2) != 0 ? 1 : -1));
                                        HuSprPosSet(mes->grpId[i], j, x, y);
                                        if(i == 1) {
                                            HuSprPosSet(mes->grpId[0], j, x, y);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if(mes->time >= timeStart) {
                    GameMesScale = (HuSin((mes->time-timeStart)*9.0f)*0.5)+1.0;
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == timeStart) {
                    HuAudFXPlay(MSM_SE_CMN_26);
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time == timeEnd) {
                    mes->stat |= GAMEMES_STAT_TIMEEND;
                    GameMesStartSePlay();
                }
                if(mes->time >= timeEnd) {
                    mes->mesMode = 0;
                    mes->timeMax = timeEnd+45;
                }
            } else {
                if(mes->time <= 20) {
                    GameMesScale = HuSin(mes->time*4.5f);
                    for(j=0; j<mes->charNum; j++) {
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                        y = mes->pos.y;
                        HuSprPosSet(mes->grpId[0], j, x, y);
                        HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                    }
                }
                if(mes->time == 20) {
                    if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                        HuAudFXPlay(MSM_SE_CMN_27);
                    } else {
                        HuAudFXPlay(MSM_SE_CMN_29);
                    }
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time >= 65) {
                    for(i=0; i<1; i++) {
                        for(j=0; j<SdMesWork[0]; j++) {
                            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                            y = mes->pos.y;
                            HuSprPosSet(mes->grpId[i], j, x, y);
                            if(SdMesScale[j] == 1.0f) {
                                if(SdMesWork[1] < 3) {
                                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x*(1.0f-(0.2f*(SdMesWork[1]+1))), mes->scale.y*(1.0f-(0.2f*(SdMesWork[1]+1))));
                                } else if(SdMesWork[1] < 5) {
                                    HuSprScaleSet(mes->grpId[i], j, mes->scale.x*(1.0f-(0.3f*(4-SdMesWork[1]))), mes->scale.y*(1.0f-(0.3f*(4-SdMesWork[1]))));
                                }
                            }
                            if(SdMesWork[1] >= 5 || SdMesScale[j] > 1.0f) {
                                if(i == 0) {
                                    SdMesScale[j] += 1.5f;
                                    if(SdMesScale[j] < 0) {
                                        SdMesScale[j] = 0;
                                    }
                                }
                                HuSprScaleSet(mes->grpId[i], j, mes->scale.x*SdMesScale[j], mes->scale.y*SdMesScale[j]);
                                SdMesTPLvl[i][j] -= 0.07f;
                                if(SdMesTPLvl[i][j] < 0) {
                                    SdMesTPLvl[i][j] = 0;
                                    HuSprDispOff(mes->grpId[i], j);
                                } else {
                                    HuSprTPLvlSet(mes->grpId[i], j, SdMesTPLvl[i][j]);
                                }
                            }
                        }
                    }
                    if(SdMesWork[0] <= mes->charNum && ++SdMesWork[1] == 8) {
                        SdMesWork[0]++;
                        if(SdMesWork[0] > mes->charNum) {
                            SdMesWork[0] = mes->charNum;
                        }
                        SdMesWork[1] = 0;
                    }
                }
                if(mes->time >= 160) {
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                }
            }
            break;
        case -1:
            mes->angle += GameMesVWait*0.1f;
            GameMesTPLvl = 1.0f-mes->angle;
            if(GameMesTPLvl <= 0) {
                GameMesTPLvl = 0;
                mes->mesMode = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            for(j=0; j<mes->charNum; j++) { 
                HuSprTPLvlSet(mes->grpId[0], j, GameMesTPLvl);
            }
            break;
        
        default:
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        if(mes->subMode == GAMEMES_MG_TYPE_START) {
            GameMesPauseEnable(TRUE);
        }
        return FALSE;
    }
    return TRUE;
}

static int DrawMesTime[10][6];
static float DrawMesY[10][6];

BOOL GameMesMgDrawInit(GAMEMES *mes, va_list args)
{
    s16 i;
    s16 j;
    float x;
    float y;
    
    GAMEMESID strMes;
    mes->subMode = GAMEMES_MG_TYPE_DRAW;
    mes->mesNo = GAMEMES_MES_MG_DRAW;
    mes->mesMode = 1;
    mes->angle = 0;
    for(j=0; j<10; j++) {
        MesUnk1[j] = 0;
        for(i=0; i<6; i++) {
            DrawMesTime[j][i] = 0;
            DrawMesY[j][i] = mes->pos.y;
        }
    }
    GameMesScale = 0;
    SdMesWork[0] = 1;
    SdMesWork[1] = 0;
    for(i=0; i<6; i++) {
        if(i == 0) {
            strMes = GameMesStrWinCreate(mes, MESS_MGINSTSYS_MES_DRAW);
        } else {
            strMes = GameMesStrCopy(mes, strMes);
        }
        for(j=0; j<mes->charNum; j++) {
            x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
            y = mes->pos.y;
            if(i == 0) {
                SprPosX[j] = x;
                SprPosY[j] = y;
            }
            HuSprPosSet(mes->grpId[i], j, x, y);
            HuSprPriSet(mes->grpId[i], j, (5+i));
            HuSprTPLvlSet(mes->grpId[i], j, 1.0f/(i+1));
            if(i == 0) {
                HuSprScaleSet(mes->grpId[i], j, 0, 0);
            } else {
                HuSprScaleSet(mes->grpId[i], j, mes->scale.x, mes->scale.y);
                HuSprDispOff(mes->grpId[i], j);
            }
        }
    }
    return TRUE;
}

BOOL GameMesMgDrawExec(GAMEMES *mes)
{
    s16 i;
    s16 j;
    s16 k;
    float x;
    float y;
    float t;
    
    mes->time += GameMesVWait;
    switch(mes->mesMode) {
        case 1:
            if(mes->time <= 20) {
                GameMesScale = HuSin(mes->time*4.5f);
                for(j=0; j<mes->charNum; j++) {
                    x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28)*GameMesScale);
                    y = mes->pos.y;
                    HuSprPosSet(mes->grpId[0], j, x, y);
                    HuSprScaleSet(mes->grpId[0], j, mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                }
            }
            if(mes->time == 20) {
                HuAudFXPlay(MSM_SE_CMN_29);
                mes->stat |= GAMEMES_STAT_FXPLAY;
            }
            if(mes->time >= 140) {
                for(i=0; i<6; i++) {
                    for(j=0; j<SdMesWork[0]; j++) {
                        DrawMesTime[j][i]++;
                        x = mes->pos.x+(mes->scale.x*(((-mes->charNum*56)/2)+(j*56)+28));
                        if(i == 0) {
                            t = DrawMesTime[j][i];
                            y = mes->pos.y-((36.0f*t)-(1.8f*t*t));
                            for(k=5; k>0; k--) {
                                DrawMesY[j][k] = DrawMesY[j][k-1];
                            }
                            DrawMesY[j][0] = y;
                        } else {
                            y = DrawMesY[j][i];
                        }
                        HuSprPosSet(mes->grpId[i], j, x, y);
                        HuSprDispOn(mes->grpId[i], j);
                        DrawMesY[j][i] = y;
                    }
                }
                if(SdMesWork[0] < mes->charNum && ++SdMesWork[1] == 10) {
                    SdMesWork[0]++;
                    SdMesWork[1] = 0;
                }
            }
            if(mes->time >= 210) {
                mes->mesMode = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

static int RecordMesValue;
static int RecordMesType;

BOOL GameMesMgRecordInit(GAMEMES *mes, va_list args)
{
    s16 no; //r30
    int num; //r29
    s16 i; //r28
    ANIMDATA *animP; //r27
    HUSPRID sprId; //r26
    s16 x; //r25
    int center; //r24
    s16 separatorNo[2]; //sp+0x14
    int decimal; //sp+0x10
        
    RecordMesValue = va_arg(args, int);
    mes->mesMode = 1;
    mes->angle = 0;
    switch(omcurovl) {
        case DLL_m510dll:
        case DLL_m529dll:
        case DLL_m552dll:
        case DLL_m574dll:
            RecordMesType = 0;
            num = 0;
            break;
        
        case DLL_m506dll:
        case DLL_m547dll:
            RecordMesType = 1;
            num = 1;
            break;
        
        case DLL_m511dll:
        case DLL_m512dll:
        case DLL_m513dll:
            RecordMesType = 2;
            num = 0;
            break;
        
        case DLL_m514dll:
            RecordMesType = -2;
            num = 2;
            break;
        
        default:
            RecordMesType = -1;
            num = 0;
            break;
    }
    mes->grpId[0] = HuSprGrpCreate(num+10);
    num = (GameMesLanguageNo == HUWIN_LANG_JAPAN) ? GAMEMES_ANM_recordJpn
        : (GameMesLanguageNo == HUWIN_LANG_ENGLISH) ? GAMEMES_ANM_recordEng
        : (GameMesLanguageNo == HUWIN_LANG_GERMANY) ? GAMEMES_ANM_recordGer
        : (GameMesLanguageNo == HUWIN_LANG_FRANCE) ? GAMEMES_ANM_recordFra
        : (GameMesLanguageNo == HUWIN_LANG_ITALY) ? GAMEMES_ANM_recordIta
        : (GameMesLanguageNo == HUWIN_LANG_SPAIN) ? GAMEMES_ANM_recordSpa
        : GAMEMES_ANM_recordEng;
    animP = HuSprAnimRead(GameMesDataRead(num));
    sprId = HuSprCreate(animP, 0, 0);
    HuSprGrpMemberSet(mes->grpId[0], 0, sprId);
    HuSprPosSet(mes->grpId[0], 0, 0, -32);
    HuSprPriSet(mes->grpId[0], 0, 2);
    animP = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_recordIcon));
    sprId = HuSprCreate(animP, 0, 0);
    HuSprGrpMemberSet(mes->grpId[0], 1, sprId);
    HuSprPriSet(mes->grpId[0], 1, 2);
    animP = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_digitRecord));
    
    for(i=0; i<8; i++) {
        sprId = HuSprCreate(animP, 0, 0);
        HuSprGrpMemberSet(mes->grpId[0], i+2, sprId);
        HuSprPriSet(mes->grpId[0], i+2, 2);
    }
    if(RecordMesType == 1 || RecordMesType == -2) {
        if(RecordMesType == -2 && GameMesLanguageNo == HUWIN_LANG_ENGLISH) {
            animP = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_unitFoot));
        } else {
            animP = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_unitYard));
        }
        sprId = HuSprCreate(animP, 0, 0);
        HuSprGrpMemberSet(mes->grpId[0], 10, sprId);
        HuSprPriSet(mes->grpId[0], 10, 2);
    }
    if(RecordMesType == -2) {
        animP = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_digitRecord));
        sprId = HuSprCreate(animP, 0, 0);
        HuSprGrpMemberSet(mes->grpId[0], 11, sprId);
        HuSprBankSet(mes->grpId[0], 11, 12);
        HuSprPriSet(mes->grpId[0], 11, 2);
    }
    if(RecordMesType < 2) {
        no = 0;
        if(RecordMesValue > 999999) {
            RecordMesValue = 999999;
        }
        num = RecordMesValue/100000;
        if(num != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*100000;
            no++;
        }
        num = RecordMesValue/10000;
        if(num != 0 || no != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*10000;
            no++;
        }
        num = RecordMesValue/1000;
        if(num != 0 || no != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*1000;
            no++;
        }
        num = RecordMesValue/100;
        if(num != 0 || no != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*100;
            no++;
        }
        num = RecordMesValue/10;
        if(num != 0 || no != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*10;
            no++;
        }
        HuSprBankSet(mes->grpId[0], no+2, RecordMesValue);
        no++;
        for(i=no; i<8; i++) {
            HuSprDispOff(mes->grpId[0], i+2);
        }
        if(RecordMesType == -2) {
            HuSprPosSet(mes->grpId[0], 11, (no == 5) ? 28 : 4, 32);
            center = 8;
            num = 16;
            HuSprPosSet(mes->grpId[0], 10, 48+(-((no*44)+48)/2)+(no*44)+num+center, 32);
            HuSprPosSet(mes->grpId[0], 1, 24+(-((no*44)+48)/2)-num-center, 32);
            for(i=0; i<no; i++) {
                decimal = (no == 5) ? 3 : 2;
                if(i < decimal) {
                    HuSprPosSet(mes->grpId[0], i+2, 70+(-((no*44)+48)/2)+(i*44)-num-center, 32);
                } else {
                    HuSprPosSet(mes->grpId[0], i+2, 70+(-((no*44)+48)/2)+(i*44)-num+center, 32);
                }
            }
        } else {
            if(RecordMesType == 1) {
                num = 16;
                HuSprPosSet(mes->grpId[0], 10, 48+(-((no*44)+48)/2)+(no*44)+num, 32);
            } else {
                num = 0;
            }
            HuSprPosSet(mes->grpId[0], 1, 24+(-((no*44)+48)/2)-num, 32);
            for(i=0; i<no; i++) {
                HuSprPosSet(mes->grpId[0], i+2, 70+(-((no*44)+48)/2)+(i*44)-num, 32);
                if(RecordMesType == -1) {
                    HuSprDispOff(mes->grpId[0], i+2);
                }
            }
        }
    } else if(RecordMesType == 2) {
        no = 0;
        num = RecordMesValue/36000;
        if(num != 0) {
            HuSprBankSet(mes->grpId[0], no+2, num);
            RecordMesValue -= num*36000;
            no++;
        }
        num = RecordMesValue/3600;
        HuSprBankSet(mes->grpId[0], no+2, num);
        RecordMesValue -= num*3600;
        no++;
        
        HuSprBankSet(mes->grpId[0], no+2, 10);
        separatorNo[0] = no;
        no++;
        
        num = RecordMesValue/600;
        HuSprBankSet(mes->grpId[0], no+2, num);
        RecordMesValue -= num*600;
        no++;
        
        num = RecordMesValue/60;
        HuSprBankSet(mes->grpId[0], no+2, num);
        RecordMesValue -= num*60;
        no++;
        
        HuSprBankSet(mes->grpId[0], no+2, 11);
        separatorNo[1] = no;
        no++;
        
        RecordMesValue = (100.0/60.0)*(float)RecordMesValue;
        
        num = RecordMesValue/10;
        HuSprBankSet(mes->grpId[0], no+2, num);
        RecordMesValue -= num*10;
        no++;
        
        HuSprBankSet(mes->grpId[0], no+2, RecordMesValue);
        no++;
        for(i=no; i<8; i++) {
            HuSprDispOff(mes->grpId[0], i+2);
        }
        HuSprPosSet(mes->grpId[0], 1, 24+(-((no*44)+4)/2), 32);
        for(x=70+(-((no*44)+4)/2), i=0; i<no; i++) {
            HuSprPosSet(mes->grpId[0], i+2, x, 32);
            if(separatorNo[0] == i || separatorNo[1] == i) {
                x += 22;
            } else {
                x += 44;
            }
        }
    }
    HuSprGrpScaleSet(mes->grpId[0], 0, 0);
    return TRUE;
}

BOOL GameMesMgRecordExec(GAMEMES *mes)
{
    if(!_CheckFlag(FLAG_MG_PRACTICE)) {
        mes->time += GameMesVWait;
        if(mes->time >= mes->timeMax && mes->mesMode == 0) {
            mes->stat |= GAMEMES_STAT_TIMEEND;
            mes->mesMode = -1;
            mes->angle = 0;
        }
        switch(mes->mesMode) {
            case 1:
                if(mes->time <= 20) {
                    GameMesScale = HuSin(mes->time*4.5f);
                    HuSprGrpPosSet(mes->grpId[0], 288, 240);
                    HuSprGrpScaleSet(mes->grpId[0], mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                }
                if(mes->time == 20) {
                    HuAudFXPlay(MSM_SE_CMN_28);
                    mes->stat |= GAMEMES_STAT_FXPLAY;
                }
                if(mes->time >= 100) {
                    mes->mesMode = 0;
                    mes->timeMax = 140;
                }
                break;
            
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                GameMesTPLvl = 1.0f-mes->angle;
                if(GameMesTPLvl <= 0.0f) {
                    GameMesTPLvl = 0;
                }
                HuSprGrpTPLvlSet(mes->grpId[0], GameMesTPLvl);
                GameMesScale = 1.0f-mes->angle;
                if(GameMesScale <= 0.0f) {
                    GameMesScale = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                }
                HuSprGrpScaleSet(mes->grpId[0], mes->scale.x*GameMesScale, mes->scale.y*GameMesScale);
                break;
        }
    } else {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->stat |= GAMEMES_STAT_KILL;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}


static char *WinnerMesNameTbl[15*6] = {
        "ﾏﾘｵ",
        "MARIO",
        "MARIO",
        "MARIO",
        "MARIO",
        "MARIO",
        
        "ﾙｲｰｼﾞ",
        "LUIGI",
        "LUIGI",
        "LUIGI",
        "LUIGI",
        "LUIGI",

        "ﾋﾟｰﾁ",
        "PEACH",
        "PEACH",
        "PEACH",
        "PEACH",
        "PEACH",

        "ﾖｯｼｰ",
        "YOSHI",
        "YOSHI",
        "YOSHI",
        "YOSHI",
        "YOSHI",
        
        "ﾜﾘｵ",
        "WARIO",
        "WARIO",
        "WARIO",
        "WARIO",
        "WARIO",

        "ﾃﾞｲｼﾞｰ",
        "DAISY",
        "DAISY",
        "DAISY",
        "DAISY",
        "DAISY",

        "ﾜﾙｲｰｼﾞ",
        "WALUIGI",
        "WALUIGI",
        "WALUIGI",
        "WALUIGI",
        "WALUIGI",
        
        "ｷﾉﾋﾟｵ",
        "TOAD",
        "TOAD",
        "TOAD",
        "TOAD",
        "TOAD",
        
        "ﾃﾚｻ",
        "BOO",
        "BUU HUU",
        "BOO",
        "BOO",
        "BOO",
        
        "ﾐﾆｸｯﾊﾟ",
        "KOOPA KID",
        "MINI BOWSER",
        "MINI BOWSER",
        "MINI BOWSER",
        "MINI BOWSER",
        
        "ﾐﾆｸｯﾊﾟR",
        "RED K.KID",
        "M.BOWSER R",
        "MINI B.ROUGE",
        "MINI B.ROSSO",
        "MINI B.ROJO",
        
        "ﾐﾆｸｯﾊﾟG",
        "GREEN K.KID",
        "M.BOWSER G",
        "MINI B.VERT",
        "MINI B.VERDE",
        "MINI B.VERDE",
        
        "ﾐﾆｸｯﾊﾟB",
        "BLUE K.KID",
        "M.BOWSER B",
        "MINI B.BLEU",
        "MINI B.BLU",
        "MINI B.AZUL",
        
        "ﾄﾞﾝｷｰ",
        "DK",
        "DONKEY KONG",
        "DK",
        "DK",
        "DK",
        
        "ｸｯﾊﾟ",
        "BOWSER",
        "BOWSER",
        "BOWSER",
        "BOWSER",
        "BOWSER"
};

static int WinnerMesGrpInfo[15*6] = {
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
    255, 255, 48, 255, 255, 255,
    255, 80, 64, 64, 64, 64,
    255, 48, 128, 64, 64, 64,
    255, 80, 128, 64, 64, 64,
    255, 64, 128, 64, 64, 64,
    255, 255, 96, 255, 255, 255,
    255, 255, 255, 255, 255, 255,
};

static s16 WinnerMesOfsTbl[4][5][2] = {
    {
        { 144, 0 },
        { -144, 0 },
    },
    {
        { 0, 35 },
        { -144, -35 },
        { 144, -35 },
    },
    {
        { 0, 105 },
        { 0, -105 },
        { -144, 0 },
        { 144, 0 },
    },
    {
        { 0, 105 },
        { -144, -105 },
        { 144, -105 },
        { -144, 0 },
        { 144, 0 },
    }
};

static s16 WinnerMesSprX[6];
static int WinnerMesSprNum[5];
static int WinnerMesCharNo[GW_PLAYER_MAX];
static float WinnerMesNameX[GW_PLAYER_MAX];
static float WinnerMesNameY[GW_PLAYER_MAX];

static int WinnerMesLanguageNo;
static int WinnerMesCharNum;

static void FixWinnerSprPos(int grpId, int charNo, float scale, int sprNum);

BOOL GameMesMgWinnerInit(GAMEMES *mes, va_list args)
{
    int i; //r29
    int charNo; //r28
    int type; //r27
    int nameLen; //r26
    GAMEMESID strMes; //r20
    
    char *nameMes; //r19
    int nameFlag; //r18
    u32 mesId; //r17
    
    float x; //f31
    float y; //f30
    float ofs; //f29
    
    
    type = va_arg(args, int);
    if(type < 0 && type >= 6) {
        return FALSE;
    }
    mesId = MESS_MGINSTSYS_MES_WIN;
    strMes = GameMesStrWinCreate(mes, mesId);
    WinnerMesSprX[0] = mes->charNum*56;
    WinnerMesSprNum[0] = mes->charNum;
    WinnerMesCharNum = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charNo = va_arg(args, int);
        if(charNo < 0) {
            continue;
        }
        WinnerMesLanguageNo = (GameMesLanguageNo == HUWIN_LANG_JAPAN) ? 0
        : (GameMesLanguageNo == HUWIN_LANG_ENGLISH) ? 1
        : (GameMesLanguageNo == HUWIN_LANG_GERMANY) ? 2
        : (GameMesLanguageNo == HUWIN_LANG_FRANCE) ? 3
        : (GameMesLanguageNo == HUWIN_LANG_ITALY) ? 4
        : 5;
        nameMes = WinnerMesNameTbl[(charNo*6)+WinnerMesLanguageNo];
        nameFlag = 0;
        strMes = GameMesStrCreate(mes, nameMes, nameFlag);
        if(GameMesLanguageNo == HUWIN_LANG_ENGLISH) {
            if(charNo == CHARNO_MINIKOOPA
                || charNo == CHARNO_MINIKOOPAR
                || charNo == CHARNO_MINIKOOPAG
                || charNo == CHARNO_MINIKOOPAB) {
                    mes->charNum++;
                }
        } else if(GameMesLanguageNo == HUWIN_LANG_GERMANY) {
            if(charNo == CHARNO_TERESA
                || charNo == CHARNO_MINIKOOPA
                || charNo == CHARNO_MINIKOOPAR
                || charNo == CHARNO_MINIKOOPAG
                || charNo == CHARNO_MINIKOOPAB
                || charNo == 13) {
                    mes->charNum++;
                }
        } else if(GameMesLanguageNo == HUWIN_LANG_FRANCE || GameMesLanguageNo == HUWIN_LANG_ITALY || GameMesLanguageNo == HUWIN_LANG_SPAIN) {
            if(charNo == CHARNO_MINIKOOPA
                || charNo == CHARNO_MINIKOOPAR
                || charNo == CHARNO_MINIKOOPAG
                || charNo == CHARNO_MINIKOOPAB) {
                    mes->charNum++;
                }
        }
        WinnerMesCharNo[WinnerMesCharNum] = charNo;
        WinnerMesCharNum++;
        WinnerMesSprX[WinnerMesCharNum] = mes->charNum*56;
        WinnerMesSprNum[WinnerMesCharNum] = mes->charNum;
    }
    if(WinnerMesCharNum == 0) {
        return FALSE;
    }
    mes->charNum = WinnerMesCharNum+1;
    mes->mesMode = 1;
    mes->angle = 0;
    GameMesOfs = 600;
    for(y=0, i=0; i<mes->charNum; i++) {
        if(WinnerMesCharNum == 1) {
            nameLen = WinnerMesSprX[1]/56;
            if(nameLen <= 5) {
                mes->winScale = 1;
                ofs = WinnerMesSprX[1];
            } else {
                mes->winScale = 5.0f/nameLen;
                ofs = 280;
            }
            if(i != 0) {
                FixWinnerSprPos(mes->grpId[i], WinnerMesCharNo[i-1], mes->winScale, WinnerMesSprX[i]/56);
            }
            x = ofs+WinnerMesSprX[0]+32.0f;
            if(i == 0) {
                x = ((576.0f-x)/2)+(x-(WinnerMesSprX[0]/2));
            } else {
                x = ((576.0f-x)/2)+(ofs/2);
            }
        } else if(i == 0) {
            x = 288.0f;
        } else if(WinnerMesCharNum == 2) {
            if(WinnerMesSprX[i]/56 <= 4) {
                mes->winScale = 1;
            } else {
                mes->winScale = 4.0f/(WinnerMesSprX[i]/56);
            }
            if(i != 0) {
                FixWinnerSprPos(mes->grpId[i], WinnerMesCharNo[i-1], mes->winScale, WinnerMesSprX[i]/56);
            }
            if(i == 1) {
                x = 272.0f-((WinnerMesSprX[1]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x -= ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            } else {
                x = 304.0f+((WinnerMesSprX[2]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x += ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            }
        } else if(WinnerMesCharNum == 3) {
            if(WinnerMesSprX[i]/56 <= 4) {
                mes->winScale = 1;
            } else {
                mes->winScale = 4.0f/(WinnerMesSprX[i]/56);
            }
            if(i != 0) {
                FixWinnerSprPos(mes->grpId[i], WinnerMesCharNo[i-1], mes->winScale, WinnerMesSprX[i]/56);
            }
            if(i == 1) {
                x = 288;
            } else if(i == 2) {
                x = 272.0f-((WinnerMesSprX[2]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x -= ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            } else {
                x = 304.0f+((WinnerMesSprX[3]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x += ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            }
        } else if(WinnerMesCharNum == 4) {
            if(WinnerMesSprX[i]/56 <= 4) {
                mes->winScale = 1;
            } else {
                mes->winScale = 4.0f/(WinnerMesSprX[i]/56);
            }
            if(i != 0) {
                FixWinnerSprPos(mes->grpId[i], WinnerMesCharNo[i-1], mes->winScale, WinnerMesSprX[i]/56);
            }
            if((i%2) != 0) {
                x = 272.0f-((WinnerMesSprX[i]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x -= ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            } else {
                x = 304.0f+((WinnerMesSprX[i]*mes->winScale)/2);
                if(WinnerMesSprX[i]/56 < 4) {
                    x += ((4-(WinnerMesSprX[i]/56))*56)/2.0f;
                }
            }
        }
        WinnerMesNameX[i] = x;
        if(WinnerMesCharNum == 1) {
            switch(omcurovl) {
                case DLL_m512dll:
                case DLL_m519dll:
                case DLL_m525dll:
                case DLL_m545dll:
                case DLL_m546dll:
                case DLL_m547dll:
                case DLL_m550dll:
                case DLL_sd00dll:
                    WinnerMesNameY[i] = 123;
                    break;
                
                case DLL_m501dll:
                case DLL_m503dll:
                case DLL_m504dll:
                case DLL_m509dll:
                case DLL_m514dll:
                case DLL_m515dll:
                case DLL_m517dll:
                case DLL_m518dll:
                case DLL_m520dll:
                case DLL_m522dll:
                case DLL_m541dll:
                case DLL_m542dll:
                case DLL_m543dll:
                case DLL_m544dll:
                case DLL_m548dll:
                case DLL_m549dll:
                case DLL_m551dll:
                case DLL_m553dll:
                case DLL_m565dll:
                case DLL_m566dll:
                case DLL_m568dll:
                    WinnerMesNameY[i] = 357;
                    break;
                
                default:
                    WinnerMesNameY[i] = 240.0f+WinnerMesOfsTbl[WinnerMesCharNum-1][i][1];
                    break;
            }
        } else if(WinnerMesCharNum == 2) {
            switch(omcurovl) {
                case DLL_m501dll:
                case DLL_m503dll:
                case DLL_m514dll:
                case DLL_m521dll:
                case DLL_m526dll:
                case DLL_m533dll:
                case DLL_m565dll:
                    WinnerMesNameY[i] = 407.0f;
                    if(i != 0) {
                        WinnerMesNameY[i] -= 70.0f;
                    }
                    break;
                
                default:
                    WinnerMesNameY[i] = 240.0f+WinnerMesOfsTbl[WinnerMesCharNum-1][i][1];
                    break;
            }
        } else {
            WinnerMesNameY[i] = 240.0f+WinnerMesOfsTbl[WinnerMesCharNum-1][i][1];
        }
        x = WinnerMesNameX[i];
        y = WinnerMesNameY[i];
        if(WinnerMesCharNum == 1) {
            if(i == 1) {
                x -= GameMesOfs;
            }
        } else if(WinnerMesCharNum == 2) {
            if(i == 1) {
                x -= GameMesOfs;
            } else if(i == 2) {
                x += GameMesOfs;
            }
        } else if(WinnerMesCharNum == 3) {
            if(i == 1) {
                y -= GameMesOfs;
            } else if(i == 2) {
                x -= GameMesOfs;
            } else if(i == 3) {
                x += GameMesOfs;
            }
        } else if(WinnerMesCharNum == 4) {
            if(i == 1) {
                x -= GameMesOfs;
            } else if(i == 2) {
                x += GameMesOfs;
            } else if(i == 3) {
                x -= GameMesOfs;
            } else if(i == 4) {
                x += GameMesOfs;
            }
        }
        HuSprGrpPosSet(mes->grpId[i], x, y);
        if(i == 0) {
            HuSprGrpScaleSet(mes->grpId[0], 0, 0);
        } else {
            HuSprGrpScaleSet(mes->grpId[i], mes->scale.x*mes->winScale, mes->scale.y);
        }
        for(charNo=0; charNo<WinnerMesSprNum[i]; charNo++) {
            HuSprPriSet(mes->grpId[i], charNo, 2);
        }
    }
    return TRUE;
}

BOOL GameMesMgWinnerExec(GAMEMES *mes)
{
    int i; //r30
    
    float x; //f31
    float y; //f30
    float t; //f29
    float scale; //f28
    float zRot; //f27
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode == 0) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    switch(mes->mesMode) {
        case 1:
            if(mes->time <= 20) {
                t = mes->time;
                zRot = (t/20.0f)*360.0f;
                HuSprGrpZRotSet(mes->grpId[0], zRot);
                scale = HuSin((t/20.0f)*90.0f);
                HuSprGrpScaleSet(mes->grpId[0], mes->scale.x*scale, mes->scale.y*scale);
                GameMesOfs -= 30;
                for(i=1; i<mes->charNum; i++) {
                    x = WinnerMesNameX[i];
                    y = WinnerMesNameY[i];
                    if(WinnerMesCharNum == 1) {
                        if(i == 1) {
                            x -= GameMesOfs;
                        }
                    } else if(WinnerMesCharNum == 2) {
                        if(i == 1) {
                            x -= GameMesOfs;
                        } else if(i == 2) {
                            x += GameMesOfs;
                        }
                    } else if(WinnerMesCharNum == 3) {
                        if(i == 1) {
                            y -= GameMesOfs;
                        } else if(i == 2) {
                            x -= GameMesOfs;
                        } else if(i == 3) {
                            x += GameMesOfs;
                        }
                    } else if(WinnerMesCharNum == 4) {
                        if(i == 1) {
                            x -= GameMesOfs;
                        } else if(i == 2) {
                            x += GameMesOfs;
                        } else if(i == 3) {
                            x -= GameMesOfs;
                        } else if(i == 4) {
                            x += GameMesOfs;
                        }
                    }
                    HuSprGrpPosSet(mes->grpId[i], x, y);
                }
            }
            if(mes->time == 140) {
                mes->mesMode = 0;
                mes->timeMax = 170;
            }
            break;
           
        case -1:
            mes->angle += GameMesVWait*0.1f;
            GameMesTPLvl = 1.0f-mes->angle;
            if(GameMesTPLvl <= 0) {
                GameMesTPLvl = 0;
                mes->stat |= GAMEMES_STAT_KILL;
            }
            for(i=0; i<mes->charNum; i++) {
                HuSprGrpTPLvlSet(mes->grpId[i], GameMesTPLvl);
            }
            break;
        
        default:
            break;
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

static void FixWinnerSprPos(int grpId, int charNo, float scale, int sprNum)
{
    int i;
    HUSPRITE *sp;
    BOOL needFix = FALSE;
    if(GameMesLanguageNo == HUWIN_LANG_ENGLISH) {
        if(charNo == CHARNO_MINIKOOPA
            || charNo == CHARNO_MINIKOOPAR
            || charNo == CHARNO_MINIKOOPAG
            || charNo == CHARNO_MINIKOOPAB) {
                needFix = TRUE;
            }
    } else if(GameMesLanguageNo == HUWIN_LANG_GERMANY) {
        if(charNo == CHARNO_TERESA
            || charNo == CHARNO_MINIKOOPA
            || charNo == CHARNO_MINIKOOPAR
            || charNo == CHARNO_MINIKOOPAG
            || charNo == CHARNO_MINIKOOPAB
            || charNo == 13) {
                needFix = TRUE;
            }
    } else if(GameMesLanguageNo == HUWIN_LANG_FRANCE || GameMesLanguageNo == HUWIN_LANG_ITALY || GameMesLanguageNo == HUWIN_LANG_SPAIN) {
        if(charNo == CHARNO_MINIKOOPA
            || charNo == CHARNO_MINIKOOPAR
            || charNo == CHARNO_MINIKOOPAG
            || charNo == CHARNO_MINIKOOPAB) {
                needFix = TRUE;
            }
    }
    if(needFix) {
        int num = (WinnerMesGrpInfo[(charNo*6)+WinnerMesLanguageNo] & 0xF0) >> 4;
        for(i=0; i<num; i++) {
            sp = &HuSprData[HuSprGrpData[grpId].sprId[i]];
            sp->pos.x += (56.0f*scale)/2;
        }
        for(i=num; i<sprNum-1; i++) {
            sp = &HuSprData[HuSprGrpData[grpId].sprId[i]];
            sp->pos.x -= (56.0f*scale)/2;
        }
    }
}

static void FixMgMesScale(GAMEMES *mes);

BOOL GameMesMgInit(GAMEMES *mes, va_list args)
{
    s16 i;
    GAMEMESID strMes;
    u32 mesId;
    mes->subMode = va_arg(args, int);
    if(mes->subMode == GAMEMES_MG_TYPE_DRAW) {
        return GameMesMgDrawInit(mes, args);
    }
    mes->mesMode = 0;
    mes->angle = 0;
    mesId = (mes->subMode == GAMEMES_MG_TYPE_START) ? MESS_MGINSTSYS_MES_START : MESS_MGINSTSYS_MES_FINISH;
    strMes = GameMesStrWinCreate(mes, mesId);
    HuSprGrpPosSet(mes->grpId[strMes], 0, 0);
    FixMgMesScale(mes);
    for(i=0; i<mes->charNum; i++) {
        HuSprTPLvlSet(mes->grpId[strMes], i, 1);
        HuSprPosSet(mes->grpId[strMes], i, 600+(i*56), 240);
        HuSprScaleSet(mes->grpId[strMes], i, mes->scale.x, mes->scale.y);
        HuSprPriSet(mes->grpId[strMes], i, 2);
    }
    mes->dispMode = 3;
    GameMesPauseEnable(FALSE);
    return TRUE;
}

static float GetMgMesSprXPos(GAMEMES *mes, float ofs);
static void UpdateMgStartMes(GAMEMES *mes, int time);
static void UpdateMgFinishMes(GAMEMES *mes, int time);

BOOL GameMesMg4PExec(GAMEMES *mes)
{
    int i; //r30
    int idleTime; //r29
    int charTime; //r28
    int inTime; //r27
    int beepTime; //r26
    int seTime; //r25
    int yStep; //r24
    
    float t; //f31
    float x; //f30
    float tpLvl; //f29
    float y; //f28
    float angle; //f27
    
    idleTime = 42;
    inTime = idleTime+20;
    beepTime = inTime;
    seTime = beepTime+20;
    if(mes->dispMode != 0 && mes->mesMode != -1) {
        switch(mes->dispMode) {
            case 2:
               if(mes->dispValue != -1) {
                   (void)mes;
               } else {
                   mes->mesMode = -1;
                   mes->angle = 0;
               }
               mes->dispMode = 0;
               break;
               
            case 1:
                mes->timeMax = mes->dispValue;
                mes->dispMode = 0;
                break;
            
            case 3:
                mes->mesMode = 1;
                mes->dispMode = 0;
                break;
            
            default:
                mes->dispMode = 0;
                break;
        }
    }
    if(mes->mesMode == 2) {
        return TRUE;
    }
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode != -1) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    if(mes->mesMode != 0) {
        switch(mes->mesMode) {
            case 1:
                if(mes->subMode == GAMEMES_MG_TYPE_START) {
                    if(mes->time <= inTime) {
                        for(i=0; i<mes->charNum; i++) {
                            charTime = mes->time-((i*20.0f)/mes->charNum);
                            t = mes->time-((i*20.0f)/mes->charNum);
                            if(t < 0) {
                                continue;
                            }
                            if((float)charTime > idleTime) {
                                continue;
                            }
                            x = GetMgMesSprXPos(mes, i);
                            yStep = t/(idleTime/3.0f);
                            angle = t-((idleTime/3.0f)*yStep);
                            y = mes->pos.y-(100*HuSin((angle*180.0f)/(idleTime/3.0f)));
                            if(charTime == idleTime) {
                                HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                HuSprZRotSet(mes->grpId[0], i, 0);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, x+((((600.0f-x)+(i*56))*(idleTime-t))/idleTime), y);
                            }
                        }
                    } else if(mes->time >= beepTime && mes->time <= seTime) {
                        UpdateMgStartMes(mes, mes->time-beepTime);
                    } else if(mes->time >= seTime+45) {
                        mes->mesMode = -1;
                        mes->angle = 0;
                    }
                    if(mes->time == beepTime) {
                        i = HuAudFXPlay(MSM_SE_CMN_26);
                        mes->stat |= GAMEMES_STAT_FXPLAY;
                    }
                    if(mes->time == seTime) {
                        GameMesStartSePlay();
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                } else {
                    if(mes->time <= 20) {
                        UpdateMgFinishMes(mes, mes->time);
                        if(mes->time == 20) {
                            if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                                HuAudFXPlay(MSM_SE_CMN_27);
                            } else {
                                HuAudFXPlay(MSM_SE_CMN_29);
                            }
                            mes->stat |= GAMEMES_STAT_FXPLAY;
                        }
                    } else if(mes->time > 65) {
                        for(i=0; i<mes->charNum; i++) {
                            charTime = (mes->time-65)-((i*20.0f)/mes->charNum);
                            t = (mes->time-65)-((i*20.0f)/mes->charNum);
                            if(t < 0) {
                                continue;
                            }
                            if((float)charTime > idleTime) {
                                continue;
                            }
                            x = GetMgMesSprXPos(mes, i);
                            yStep = t/(idleTime/3.0f);
                            angle = t-((idleTime/3.0f)*yStep);
                            y = mes->pos.y-(100*HuSin((angle*180.0f)/(idleTime/3.0f)));
                            if(charTime == idleTime) {
                                HuSprDispOff(mes->grpId[0], i);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, x-((t*(x+((mes->charNum-i)*56)))/idleTime), y);
                            }
                        }
                    }
                    if(mes->time == inTime+65) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                    if(mes->time >= inTime+65) {
                        mes->mesMode = 0;
                        mes->stat |= GAMEMES_STAT_KILL;
                    }
                }
                break;
            
            
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                tpLvl = 1.0f-mes->angle;
                if(tpLvl <= 0.0f) {
                    tpLvl = 0;
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                    if(mes->subMode == GAMEMES_MG_TYPE_START) {
                        GameMesPauseEnable(TRUE);
                    }
                }
                for(i=0; i<mes->charNum; i++) {
                    HuSprTPLvlSet(mes->grpId[0], i, tpLvl);
                }
                break;
           
            default:
                break;
        }
    }
    
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

BOOL GameMesMg2Vs2Exec(GAMEMES *mes)
{
    int i; //r30
    int idleTime; //r29
    int centerChar; //r28
    int time; //r27
    int endTime; //r26
    int startTime; //r25
    int beepTime; //r24
    int seTime; //r23
    int inTime; //r22
    int centerOfs; //r21
    
    float x; //f31
    float t; //f30
    float tpLvl; //f29
    float baseSprX; //f28
    float startFrac; //f27
    float maxSprX; //f26
    float sprX; //f25
    float xTime; //f24
    float y; //f23
    float baseTime; //f22
    float maxTime; //f21
    float maxDist; //f20
    
    startFrac = 0.25f;
    idleTime = 20;
    inTime = 40;
    beepTime = inTime+2;
    seTime = beepTime+20;
    centerChar = (mes->charNum/2)+(mes->charNum%2)-1;
    if(mes->dispMode != 0 && mes->mesMode != -1) {
        switch(mes->dispMode) {
            case 2:
               if(mes->dispValue != -1) {
                   (void)mes;
               } else {
                   mes->mesMode = -1;
                   mes->angle = 0;
               }
               mes->dispMode = 0;
               break;
               
            case 1:
                mes->timeMax = mes->dispValue;
                mes->dispMode = 0;
                break;
            
            case 3:
                mes->mesMode = 1;
                mes->dispMode = 0;
                break;
            
            default:
                mes->dispMode = 0;
                break;
        }
    }
    if(mes->mesMode == 2) {
        return TRUE;
    }
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode != -1) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    if(mes->mesMode != 0) {
        switch(mes->mesMode) {
            case 1:
                if(mes->subMode == GAMEMES_MG_TYPE_START) {
                    if(mes->time <= inTime) {
                        for(i=0; i<mes->charNum; i++) {
                            if(i == 0 || i == mes->charNum-1) {
                                x = GetMgMesSprXPos(mes, i);
                                if(i == 0) {
                                    baseSprX = GetMgMesSprXPos(mes, centerChar);
                                    maxSprX = GetMgMesSprXPos(mes, centerChar-0.5f);
                                } else {
                                    baseSprX = GetMgMesSprXPos(mes, (mes->charNum-1)-centerChar);
                                    maxSprX = GetMgMesSprXPos(mes, (mes->charNum-1)-centerChar+0.5f);
                                }
                                time = mes->time;
                                if(time < idleTime) {
                                    if(time < idleTime*startFrac) {
                                        HuSprPosSet(mes->grpId[0], i, -30, mes->pos.y);
                                    } else {
                                        endTime = idleTime*(1.0f-startFrac);
                                        startTime = time-(idleTime*startFrac);
                                        if(i == 0) {
                                            sprX = baseSprX-(((56.0f+mes->pos.x)*(endTime-startTime))/endTime);
                                            if(sprX > maxSprX) {
                                                sprX = maxSprX;
                                            }
                                        } else {
                                            sprX = baseSprX+(((56.0f+(576.0f-mes->pos.x))*(endTime-startTime))/endTime);
                                            if(sprX < maxSprX) {
                                                sprX = maxSprX;
                                            }
                                        }
                                        HuSprPosSet(mes->grpId[0], i, sprX, mes->pos.y);
                                    }
                                } else if(time < 40) {
                                    y = mes->pos.y-(80.0*HuSin(((time-idleTime)*180.0f)/20.0f));
                                    HuSprPosSet(mes->grpId[0], i, maxSprX+(((x-maxSprX)*(time-idleTime))/20.0f), y);
                                } else {
                                    HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                }
                            } else {
                                if(i != centerChar && i != ((mes->charNum-1)-centerChar)) {
                                    x = GetMgMesSprXPos(mes, i);
                                    time = mes->time;
                                    if(time >= 0) {
                                        if(time < idleTime) {
                                            if(time < idleTime*startFrac) {
                                                HuSprPosSet(mes->grpId[0], i, -30, mes->pos.y);
                                            } else {
                                                endTime = idleTime*(1.0f-startFrac);
                                                startTime = time-(idleTime*startFrac);
                                                if(i < centerChar) {
                                                    HuSprPosSet(mes->grpId[0], i, x-(((56.0f+mes->pos.x)*(endTime-startTime))/endTime), mes->pos.y);
                                                } else {
                                                    HuSprPosSet(mes->grpId[0], i, x+(((56.0f+(576.0f-mes->pos.x))*(endTime-startTime))/endTime), mes->pos.y);
                                                }
                                            }
                                        } else {
                                            HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                        }
                                    }
                                } else {
                                    time = mes->time-16;
                                    if(time >= 0) {
                                        if(time < 20) {
                                            tpLvl = HuSin(time*9.0f)+(time/(float)idleTime);
                                            x = mes->pos.x+(mes->scale.x*((tpLvl*28.0f)+(tpLvl*(mes->charNum*56)*-0.5f)+(tpLvl*(i*56))));
                                            HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                            HuSprScaleSet(mes->grpId[0], i, tpLvl*mes->scale.x, HuSin(time*9.0f)+((mes->scale.y*time)/idleTime));
                                        } else {
                                            x = GetMgMesSprXPos(mes, i);
                                            HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                            HuSprScaleSet(mes->grpId[0], i, mes->scale.x, mes->scale.y);
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if(mes->time >= beepTime && mes->time <= seTime) {
                            UpdateMgStartMes(mes, mes->time-beepTime);
                        } else if(mes->time >= seTime+45) {
                            mes->mesMode = -1;
                            mes->angle = 0;
                        }
                    }
                    if(mes->time == beepTime) {
                        HuAudFXPlay(MSM_SE_CMN_26);
                        mes->stat |= GAMEMES_STAT_FXPLAY;
                    }
                    if(mes->time == seTime) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                        GameMesStartSePlay();
                    }
                } else {
                    if(mes->time <= 20) {
                        UpdateMgFinishMes(mes, mes->time);
                        if(mes->time == 20) {
                            if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                                HuAudFXPlay(MSM_SE_CMN_27);
                            } else {
                                HuAudFXPlay(MSM_SE_CMN_29);
                            }
                            mes->stat |= GAMEMES_STAT_FXPLAY;
                        }
                    } else if(mes->time >= 65) {
                        for(i=0; i<mes->charNum; i++) {
                            t = mes->time-65;
                            if(i < centerChar-1 || i > (mes->charNum-centerChar)) {
                                x = GetMgMesSprXPos(mes, i);
                                if(i < centerChar) {
                                    baseSprX = GetMgMesSprXPos(mes, centerChar-1);
                                    centerOfs = (centerChar-2)-i;
                                } else {
                                    baseSprX = GetMgMesSprXPos(mes, mes->charNum-centerChar);
                                    centerOfs = i-((mes->charNum+1)-centerChar);
                                }
                                if(centerChar  == 2) {
                                    baseTime = 0;
                                } else {
                                    baseTime = (5.0f*centerOfs)/(centerChar-2);
                                }
                                maxTime = 5.0f-baseTime;
                                t -= baseTime;
                                if(t < 0) {
                                    continue;
                                }
                                if(t < idleTime) {
                                    y = mes->pos.y-(80.0*HuSin(((t*180.0f)/idleTime)));
                                    HuSprPosSet(mes->grpId[0], i, x+((t*(baseSprX-x))/idleTime), y);
                                } else if(t < (idleTime+maxTime)) {
                                    HuSprPosSet(mes->grpId[0], i, baseSprX, mes->pos.y);
                                } else {
                                    if(t < 40.0f) {
                                        maxDist = t-(idleTime+maxTime);
                                        xTime = idleTime*(1.0f-startFrac);
                                        if(i < centerChar) {
                                            HuSprPosSet(mes->grpId[0], i, baseSprX-(((56.0f+baseSprX)*maxDist)/xTime), mes->pos.y);
                                        } else {
                                            HuSprPosSet(mes->grpId[0], i, baseSprX+(((56.0f+(576.0f-baseSprX))*maxDist)/xTime), mes->pos.y);
                                        }
                                    } else {
                                        HuSprDispOff(mes->grpId[0], i);
                                    }
                                }
                            } else {
                                if(i != centerChar && i != ((mes->charNum-1)-centerChar)) {
                                    x = GetMgMesSprXPos(mes, i);
                                    t -= 25;
                                    if(t < 0) {
                                        continue;
                                    }
                                    if(t < idleTime) {
                                        xTime = idleTime*(1.0f-startFrac);
                                        if(i < centerChar) {
                                            HuSprPosSet(mes->grpId[0], i, x-(((56.0f+x)*t)/xTime), mes->pos.y);
                                        } else {
                                            HuSprPosSet(mes->grpId[0], i, x+(((56.0f+(576.0f-x))*t)/xTime), mes->pos.y);
                                        }
                                    } else {
                                        HuSprDispOff(mes->grpId[0], i);
                                    }
                                } else {
                                    if(t < 0) {
                                        continue;
                                    }
                                    if(t < idleTime) {
                                        tpLvl = 1.0f+(t*0.1f);
                                        x = mes->pos.x+(mes->scale.x*((tpLvl*28.0f)+(tpLvl*(mes->charNum*56)*-0.5f)+(tpLvl*(i*56))));
                                        HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                        HuSprScaleSet(mes->grpId[0], i, tpLvl*mes->scale.x, mes->scale.y+(0.1f*t));
                                        HuSprTPLvlSet(mes->grpId[0], i, (idleTime-t)/idleTime);
                                    } else {
                                        HuSprDispOff(mes->grpId[0], i);
                                    }
                                }
                            } 
                        }
                    }
                    if(mes->time == 120) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                    if(mes->time >= 120) {
                        mes->mesMode = 0;
                        mes->stat |= GAMEMES_STAT_KILL;
                    }
                }
                
                break;
           
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                tpLvl = 1.0f-mes->angle;
                if(tpLvl <= 0.0f) {
                    tpLvl = 0;
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                    if(mes->subMode == GAMEMES_MG_TYPE_START) {
                        GameMesPauseEnable(TRUE);
                    }
                }
                for(i=0; i<mes->charNum; i++) {
                    HuSprTPLvlSet(mes->grpId[0], i, tpLvl);
                }
                break;
            
            default:
                break;
        }
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

BOOL GameMesMgLastExec(GAMEMES *mes)
{
    int i; //r30
    int idleTime; //r29
    int time; //r28
    int beepTime; //r27
    int seTime; //r26
    int fracTime; //r25
    int startTime; //r24
    
    float t; //f31
    float x; //f30
    float zRot; //f29
    float y; //f28
    float startFrac; //f27
    float tpLvl; //f26
    float scale; //f25
    float s; //f24
    
    startFrac = 0.25f;
    scale = 1;
    idleTime = 40;
    beepTime = idleTime+10;
    seTime = beepTime+20;
    
    if(mes->dispMode != 0 && mes->mesMode != -1) {
        switch(mes->dispMode) {
            case 2:
               if(mes->dispValue != -1) {
                   (void)mes;
               } else {
                   mes->mesMode = -1;
                   mes->angle = 0;
               }
               mes->dispMode = 0;
               break;
               
            case 1:
                mes->timeMax = mes->dispValue;
                mes->dispMode = 0;
                break;
            
            case 3:
                mes->mesMode = 1;
                mes->dispMode = 0;
                break;
            
            default:
                mes->dispMode = 0;
                break;
        }
    }
    if(mes->mesMode == 2) {
        return TRUE;
    }
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode != -1) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    if(mes->mesMode != 0) {
        switch(mes->mesMode) {
            case 1:
                if(mes->subMode == GAMEMES_MG_TYPE_START) {
                    if(mes->time <= idleTime) {
                        time = mes->time;
                        zRot = -((time*1080.0f)/idleTime);
                        t = ((1-HuSin((90.0f*time)/idleTime))*8)+1;
                        for(i=0; i<mes->charNum; i++) {
                            x = (t*mes->scale.x)*((28.0f+(-0.5f*(mes->charNum*56)))+(i*56));
                            y = x*HuSin(zRot);
                            x = x*HuCos(zRot);
                            if(time == idleTime) {
                                HuSprPosSet(mes->grpId[0], i, mes->pos.x+x, mes->pos.y+y);
                                HuSprZRotSet(mes->grpId[0], i, 0);
                                HuSprScaleSet(mes->grpId[0], i, mes->scale.x, mes->scale.y);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, mes->pos.x+x, mes->pos.y+y);
                                HuSprZRotSet(mes->grpId[0], i, zRot);
                                HuSprScaleSet(mes->grpId[0], i, t*mes->scale.x, t);
                            }
                        }
                    } else if(mes->time >= beepTime && mes->time <= seTime) {
                        UpdateMgStartMes(mes, mes->time-beepTime);
                    } else if(mes->time >= seTime+45) {
                        mes->mesMode = -1;
                        mes->angle = 0;
                    }
                    if(mes->time == beepTime) {
                        i = HuAudFXPlay(MSM_SE_CMN_26);
                        mes->stat |= GAMEMES_STAT_FXPLAY;
                    }
                    if(mes->time == seTime) {
                        GameMesStartSePlay();
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                } else {
                    if(mes->time <= 20) {
                        UpdateMgFinishMes(mes, mes->time);
                        if(mes->time == 20) {
                            if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                                HuAudFXPlay(MSM_SE_CMN_27);
                            } else {
                                HuAudFXPlay(MSM_SE_CMN_29);
                            }
                            mes->stat |= GAMEMES_STAT_FXPLAY;
                        }
                    } else if(mes->time >= 65) {
                        time = mes->time-65;
                        s = HuSin((time*90.0f)/idleTime);
                        zRot = s*(s*((720.0f*time)/idleTime));
                        startTime = time-(idleTime*startFrac);
                        fracTime = idleTime*(1.0f-startFrac);
                        if(time < (idleTime*startFrac)) {
                            t = 1.0f+(scale*HuSin((time*90.0f)/(idleTime*startFrac)));
                        } else {
                            t = (1.0f+scale)*HuSin(90.0f-((startTime*90.0f)/fracTime));
                        }
                        for(i=0; i<mes->charNum; i++) {
                            if(t <= 1.0f) {
                                x = (t*mes->scale.x)*((28.0f+(-(0.5f*(mes->charNum*56))))+(i*56));
                            } else {
                                x = ((((t-1.0f)*0.5f)+1.0f)*mes->scale.x)*((28.0f+(-(0.5f*(mes->charNum*56))))+(i*56));
                            }
                            y = x*HuSin(zRot);
                            x = x*HuCos(zRot);
                            if(time == idleTime) {
                                HuSprDispOff(mes->grpId[0], i);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, mes->pos.x+x, mes->pos.y+y);
                                HuSprZRotSet(mes->grpId[0], i, zRot);
                                if(t <= 1.0f) {
                                    HuSprScaleSet(mes->grpId[0], i, t*mes->scale.x, t);
                                } else {
                                    HuSprScaleSet(mes->grpId[0], i, (((t-1.0f)*0.5f)+1.0f)*mes->scale.x, t);
                                }
                                
                            }
                        }
                    }
                    if(mes->time == 120) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                    if(mes->time >= 120) {
                        mes->mesMode = 0;
                        mes->stat |= GAMEMES_STAT_KILL;
                    }
                }
                break;
                
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                tpLvl = 1.0f-mes->angle;
                if(tpLvl <= 0.0f) {
                    tpLvl = 0;
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                    if(mes->subMode == GAMEMES_MG_TYPE_START) {
                        GameMesPauseEnable(TRUE);
                    }
                }
                for(i=0; i<mes->charNum; i++) {
                    HuSprTPLvlSet(mes->grpId[0], i, tpLvl);
                }
                break;
            
            default:
                break;
        }
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

BOOL GameMesMgKettouExec(GAMEMES *mes)
{
    int i; //r30
    int idleTime; //r29
    int posIdx; //r28
    int charNo; //r27
    int time; //r26
    int beepTime; //r25
    int seTime; //r24
    int centerChar; //r23
    
    float t; //f31
    float x; //f30
    float width; //f29
    float zRot; //f28
    float tpLvl; //f27
    float y; //f26
    float exitSpeed; //f25
    
    static HuVec2f posTbl[7] = {
        { -30, 190 },
        { 170, 64 },
        { 400, 260 },
        { 200, 416 },
        { 42, 240 },
        { 180, 130 },
        { 0, 0 },
    };
    
    idleTime = 7;
    beepTime = (idleTime*7)+10;
    seTime = beepTime+20;
    centerChar = (mes->charNum/2)+(mes->charNum%2)-1;
    if(mes->dispMode != 0 && mes->mesMode != -1) {
        switch(mes->dispMode) {
            case 2:
               if(mes->dispValue != -1) {
                   (void)mes;
               } else {
                   mes->mesMode = -1;
                   mes->angle = 0;
               }
               mes->dispMode = 0;
               break;
               
            case 1:
                mes->timeMax = mes->dispValue;
                mes->dispMode = 0;
                break;
            
            case 3:
                mes->mesMode = 1;
                mes->dispMode = 0;
                break;
            
            default:
                mes->dispMode = 0;
                break;
        }
    }
    if(mes->mesMode == 2) {
        return TRUE;
    }
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode != -1) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    posTbl[6].x = mes->pos.x;
    posTbl[6].y = mes->pos.y;
    if(mes->mesMode != 0) {
        switch(mes->mesMode) {
            case 1:
                if(mes->subMode == GAMEMES_MG_TYPE_START) {
                    if(mes->time <= beepTime) {
                        for(i=0; i<mes->charNum; i++) {
                            if(i < mes->charNum/2) {
                                charNo = i;
                            } else {
                                charNo = (mes->charNum-1)-i;
                            }
                            time = mes->time-((20*charNo)/mes->charNum);
                            t = mes->time-((20.0f*charNo)/mes->charNum);
                            exitSpeed = t-(mes->time-((20.0f*centerChar)/mes->charNum));
                            if(t < 0) {
                                continue;
                            }
                            posIdx = t/idleTime;
                            width = t-(posIdx*idleTime);
                            x = GetMgMesSprXPos(mes, i);
                            if(t < (6.0f*idleTime)) {
                                x = posTbl[posIdx].x+((width*(posTbl[posIdx+1].x-posTbl[posIdx].x))/idleTime);
                                if(i >= (mes->charNum/2)) {
                                    x = 576.0f-x;
                                }
                                y = posTbl[posIdx].y+((width*(posTbl[posIdx+1].y-posTbl[posIdx].y))/idleTime);
                                HuSprPosSet(mes->grpId[0], i, x, y);
                                zRot = (2160.0f*t)/(6.0f*idleTime);
                                if(i < (mes->charNum/2)) {
                                    HuSprZRotSet(mes->grpId[0], i, zRot);
                                } else {
                                    HuSprZRotSet(mes->grpId[0], i, -zRot);
                                }
                            } else {
                                if(t < (6.0f*idleTime)+exitSpeed) {
                                    t -= 6.0f*idleTime;
                                    HuSprZRotSet(mes->grpId[0], i, 0);
                                    x = posTbl[6].x+((t*(x-posTbl[6].x))/exitSpeed);
                                    HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                } else {
                                    HuSprZRotSet(mes->grpId[0], i, 0);
                                    HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                                }
                            }
                        }
                    } else if(mes->time <= seTime) {
                        UpdateMgStartMes(mes, mes->time-beepTime);
                    } else if(mes->time >= seTime+45) {
                        mes->mesMode = -1;
                        mes->angle = 0;
                    }
                    if(mes->time == beepTime) {
                        HuAudFXPlay(MSM_SE_CMN_26);
                        mes->stat |= GAMEMES_STAT_FXPLAY;
                    }
                    if(mes->time == seTime) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                        GameMesStartSePlay();
                    }
                } else {
                    if(mes->time <= 20) {
                        UpdateMgFinishMes(mes, mes->time);
                        if(mes->time == 20) {
                            if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                                HuAudFXPlay(MSM_SE_CMN_27);
                            } else {
                                HuAudFXPlay(MSM_SE_CMN_29);
                            }
                            mes->stat |= GAMEMES_STAT_FXPLAY;
                        }
                    } else if(mes->time >= 65) {
                        for(i=0; i<mes->charNum; i++) {
                            if(i < mes->charNum/2) {
                                charNo = centerChar-i;
                            } else {
                                charNo = i-((mes->charNum-1)-centerChar);
                            }
                            time = mes->time-65;
                            t = time-((28.0f*charNo)/mes->charNum);
                            time -= (charNo*28)/mes->charNum;
                            if(t < 0) {
                                continue;
                            }
                            posIdx = t/idleTime;
                            width = t-(posIdx*idleTime);
                            x = GetMgMesSprXPos(mes, i);
                            if(t < idleTime) {
                                x += (width*(posTbl[6].x-x))/idleTime;
                                HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                            } else if(t < (idleTime+(6.0f*idleTime))) {
                                time -= idleTime;
                                t -= idleTime;
                                posIdx = 6-(int)(t/idleTime);
                                x = posTbl[posIdx].x+((width*(posTbl[posIdx-1].x-posTbl[posIdx].x))/idleTime);
                                if(i >= (mes->charNum/2)) {
                                    x = 576.0f-x;
                                }
                                y = posTbl[posIdx].y+((width*(posTbl[posIdx-1].y-posTbl[posIdx].y))/idleTime);
                                HuSprPosSet(mes->grpId[0], i, x, y);
                                zRot = (2160.0f*t)/(6.0f*idleTime);
                                if(i < (mes->charNum/2)) {
                                    HuSprZRotSet(mes->grpId[0], i, -zRot);
                                } else {
                                    HuSprZRotSet(mes->grpId[0], i, zRot);
                                }
                            } else {
                                HuSprDispOff(mes->grpId[0], i);
                            }
                        }
                    }
                    if(mes->time == 135) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                    if(mes->time >= 135) {
                        mes->mesMode = 0;
                        mes->stat |= GAMEMES_STAT_KILL;
                    }
                }
                break;
                
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                tpLvl = 1.0f-mes->angle;
                if(tpLvl <= 0.0f) {
                    tpLvl = 0;
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                    if(mes->subMode == GAMEMES_MG_TYPE_START) {
                        GameMesPauseEnable(TRUE);
                    }
                }
                for(i=0; i<mes->charNum; i++) {
                    HuSprTPLvlSet(mes->grpId[0], i, tpLvl);
                }
                break;
            
            default:
                break;
        }
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

BOOL GameMesMgDonkeyExec(GAMEMES *mes)
{
    int i; //r30
    int inTime; //r29
    int beepTime; //r28
    int seTime; //r27
    int idleTime; //r26
    
    float c; //f31
    float ofsX; //f30
    float x; //f29
    float ofsY; //f28
    float tpLvl; //f27
    float y; //f26
    float t; //f25
    float angle; //f24
    float r; //f23
    
    idleTime = 24;
    inTime = 2.5f*idleTime;
    beepTime = inTime+5;
    seTime = beepTime+20;
    
    if(mes->dispMode != 0 && mes->mesMode != -1) {
        switch(mes->dispMode) {
            case 2:
               if(mes->dispValue != -1) {
                   (void)mes;
               } else {
                   mes->mesMode = -1;
                   mes->angle = 0;
               }
               mes->dispMode = 0;
               break;
               
            case 1:
                mes->timeMax = mes->dispValue;
                mes->dispMode = 0;
                break;
            
            case 3:
                mes->mesMode = 1;
                mes->dispMode = 0;
                break;
            
            default:
                mes->dispMode = 0;
                break;
        }
    }
    if(mes->mesMode == 2) {
        return TRUE;
    }
    mes->time += GameMesVWait;
    if(mes->time >= mes->timeMax && mes->mesMode != -1) {
        mes->stat |= GAMEMES_STAT_TIMEEND;
        mes->mesMode = -1;
        mes->angle = 0;
    }
    if(mes->mesMode != 0) {
        switch(mes->mesMode) {
            case 1:
                if(mes->subMode == GAMEMES_MG_TYPE_START) {
                    if(mes->time <= inTime) {
                        r = 0.5f+(0.5f*((float)(inTime-mes->time)/inTime));
                        c = -HuCos((450.0f*mes->time)/inTime);
                        c *= r;
                        ofsX = 288.0f*c;
                        angle = (90.0f*ofsX)/288.0f;
                        ofsY = -240.0*(1.0-HuSin(angle+90.0f));
                        for(i=0; i<mes->charNum; i++) {
                            x = ofsX+GetMgMesSprXPos(mes, i);
                            y = ofsY+mes->pos.y;
                            if(mes->time >= inTime) {
                                HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, x, y);
                            }
                        }
                    } else if(mes->time >= beepTime && mes->time <= seTime) {
                        ofsX = ofsY = 0;
                        UpdateMgStartMes(mes, mes->time-beepTime);
                    } else if(mes->time >= seTime+45) {
                        mes->mesMode = -1;
                        mes->angle = 0;
                    }
                    if(mes->time == beepTime) {
                        i = HuAudFXPlay(MSM_SE_CMN_26);
                        mes->stat |= GAMEMES_STAT_FXPLAY;
                    }
                    if(mes->time == seTime) {
                        GameMesStartSePlay();
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                        
                    }
                } else {
                    if(mes->time <= 20) {
                        UpdateMgFinishMes(mes, mes->time);
                        if(mes->time == 20) {
                            if(mes->subMode == GAMEMES_MG_TYPE_FINISH) {
                                HuAudFXPlay(MSM_SE_CMN_27);
                            } else {
                                HuAudFXPlay(MSM_SE_CMN_29);
                            }
                            mes->stat |= GAMEMES_STAT_FXPLAY;
                        }
                    } else if(mes->time >= 65) {
                        t = mes->time-65;
                        r = 0.5f+(0.5f*(t/inTime));
                        c = -HuCos(90.0f+((450.0f*t)/inTime));
                        c *= r;
                        ofsX = 288.0f*c;
                        angle = (90.0f*ofsX)/288.0f;
                        ofsY = -240.0*(1.0-HuSin(angle+90.0f));
                        for(i=0; i<mes->charNum; i++) {
                            x = ofsX+GetMgMesSprXPos(mes, i);
                            y = ofsY+mes->pos.y;
                            if(t >= inTime) {
                                HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
                            } else {
                                HuSprPosSet(mes->grpId[0], i, x, y);
                            }
                        }
                    }
                    if(mes->time == 125) {
                        mes->stat |= GAMEMES_STAT_TIMEEND;
                    }
                    if(mes->time >= 125) {
                        mes->mesMode = 0;
                        mes->stat |= GAMEMES_STAT_KILL;
                    }
                }
                break;
            
            case -1:
                mes->angle += 0.1f*GameMesVWait;
                tpLvl = 1.0f-mes->angle;
                if(tpLvl <= 0.0f) {
                    tpLvl = 0;
                    mes->mesMode = 0;
                    mes->stat |= GAMEMES_STAT_KILL;
                    if(mes->subMode == GAMEMES_MG_TYPE_START) {
                        GameMesPauseEnable(TRUE);
                    }
                }
                for(i=0; i<mes->charNum; i++) {
                    HuSprTPLvlSet(mes->grpId[0], i, tpLvl);
                }
                break;
            
            default:
                break;
        }
    }
    if(GameMesCloseF || (mes->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(mes);
        return FALSE;
    }
    return TRUE;
}

static void UpdateMgStartMes(GAMEMES *mes, int time)
{
    int i; //r30
    float scaleX; //f31
    float t; //f30
    float x; //f29
    if(time < 0) {
        time = 0;
    } else if(time > 20) {
        time = 20;
    }
    t = HuSin(time*9.0f);
    scaleX = 1.0f+(0.5f*t);
    for(i=0; i<mes->charNum; i++) {
        HuSprDispOn(mes->grpId[0], i);
        x = mes->pos.x+(mes->scale.x*((scaleX*28.0f)+(scaleX*(mes->charNum*56)*-0.5f)+(scaleX*(i*56))));
        HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
        HuSprScaleSet(mes->grpId[0], i, scaleX*mes->scale.x, mes->scale.y+t);
    }
}

static void UpdateMgFinishMes(GAMEMES *mes, int time)
{
    int i; //r30
    float t; //f30
    float x; //f29
    if(time < 0) {
        time = 0;
    } else if(time > 20) {
        time = 20;
    }
    t = HuSin(time*4.5f);
    for(i=0; i<mes->charNum; i++) {
        x = mes->pos.x+(mes->scale.x*((t*28.0f)+(t*(mes->charNum*56)*-0.5f)+(t*(i*56))));
        HuSprPosSet(mes->grpId[0], i, x, mes->pos.y);
        HuSprScaleSet(mes->grpId[0], i, mes->scale.x*t, mes->scale.y*t);
    }
}

static float GetMgMesSprXPos(GAMEMES *mes, float ofs)
{
    return mes->pos.x+(mes->scale.x*((1*28.0f)+(1*(mes->charNum*56)*-0.5f)+(1*(ofs*56))));
}

static void FixMgMesScale(GAMEMES *mes)
{
    if(mes->charNum <= 8) {
        mes->scale.x = 1.0f;
    } else {
        mes->scale.x = 8.0f/mes->charNum;
    }
}