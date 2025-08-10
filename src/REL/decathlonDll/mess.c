#include "REL/decathlonDll.h"

static s8 charMessTbl[14] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    9,
    9,
    9,
    9,
};

void DecaMessInst(void)
{
    HUWINID winId = DecaWinIdGet();
    int choice;
    DecaMesFlagReset(DECA_MESSFLAG_INST);
    DecaWinOpen();
    HuWinMesSet(winId, MESS_MG_DECATHLON_WELCOME);
    HuWinMesWait(winId);
    choice = HuWinChoiceGet(winId, -1);
    if(choice == 0) {
        HuWinMesSet(winId, MESS_MG_DECATHLON_INST);
        HuWinMesWait(winId);
        DecaFlagSet(DECAFLAG_SEE_INST);
    }
    DecaMesFlagSet(DECA_MESSFLAG_INST);
    HuPrcEnd();
}

void DecaMessBegin(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_BEGIN);
    DecaWinOpen();
    HuWinMesSet(winId, MESS_MG_DECATHLON_BEGIN);
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_BEGIN);
    HuPrcEnd();
}

void DecaMessMgNo(void)
{
    HUWINID winId = DecaWinIdGet();
    char strBuf[8];
    DecaMesFlagReset(DECA_MESSFLAG_MGNO);
    DecaWinOpen();
    sprintf(strBuf, "%d", DecaMgNoGet()+1);
    HuWinMesSet(winId, MESS_MG_DECATHLON_MGNO);
    HuWinInsertMesSet(winId, MESSNUM_PTR(strBuf), 0);
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_MGNO);
    HuPrcEnd();
}

void DecaMessMgStart(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_MGSTART);
    DecaFlagReset(DECAFLAG_EXIT|DECAFLAG_INSTEXIT);
    DecaWinOpen();
    while(1) {
        int choice, mode;
        mode = 0;
        HuWinMesSet(winId, MESS_MG_DECATHLON_MGSTART);
        HuWinMesWait(winId);
        choice = HuWinChoiceGet(winId, -1);
        if(choice < 0) {
            choice = 1;
        }
        if(choice == 1) {
            HuWinMesSet(winId, MESS_MG_DECATHLON_QUIT);
            HuWinMesWait(winId);
            if(HuWinChoiceGet(winId, -1) == 0) {
                DecaFlagSet(DECAFLAG_EXIT);
            } else {
                mode = 1;
            }
        } else if(choice == 2) {
            DecaFlagSet(DECAFLAG_INSTEXIT);
        }
        if(mode == 0) {
            break;
        }
    }
    DecaMesFlagSet(DECA_MESSFLAG_MGSTART);
    HuPrcEnd();
}

void DecaMessResultStart(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_RESULTSTART);
    DecaWinOpen();
    HuAudFXPlayPan(MSM_SE_GUIDE_56, 32);
    HuWinMesSet(winId, MESS_MG_DECATHLON_RESULT_START);
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_RESULTSTART);
    HuPrcEnd();
}

void DecaMessEnd(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_END);
    DecaWinOpen();
    HuAudFXPlayPan(MSM_SE_GUIDE_56, 64);
    HuWinMesSet(winId, MESS_MG_DECATHLON_END);
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_END);
    HuPrcEnd();
}

void DecaMessMaxScore(void)
{
    HUWINID winId = DecaWinIdGet();
    char strBuf[32];
    DecaMesFlagReset(DECA_MESSFLAG_MAXSCORE);
    sprintf(strBuf, "%d", DecaScoreMaxGet());
    DecaWinOpen();
    HuWinMesSet(winId, MESS_MG_DECATHLON_MAXSCORE);
    HuWinInsertMesSet(winId, MESSNUM_PTR(strBuf), 0);
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_MAXSCORE);
    HuPrcEnd();
}

void DecaMessWinner(void)
{
    HUWINID winId = DecaWinIdGet();
    int order[GW_PLAYER_MAX];
    DecaMesFlagReset(DECA_MESSFLAG_WINNER);
    DecaWinOpen();
    switch(DecaPlayerOrderGet(order)) {
        case 1:
            HuAudFXPlayPan(MSM_SE_GUIDE_55, 64);
            HuWinMesSet(winId, MESS_MG_DECATHLON_WINNER1);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[0]].charNo], 0);
            break;
      
        case 2:
            HuAudFXPlayPan(MSM_SE_GUIDE_55, 64);
            HuWinMesSet(winId, MESS_MG_DECATHLON_WINNER2);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[0]].charNo], 0);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[1]].charNo], 1);
            break;
      
        case 3:
            HuAudFXPlayPan(MSM_SE_GUIDE_55, 64);
            HuWinMesSet(winId, MESS_MG_DECATHLON_WINNER3);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[0]].charNo], 0);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[1]].charNo], 1);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[2]].charNo], 2);
            break;
        
        case 4:
            HuAudFXPlayPan(MSM_SE_GUIDE_55, 64);
            HuWinMesSet(winId, MESS_MG_DECATHLON_WINNER4);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[0]].charNo], 0);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[1]].charNo], 1);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[2]].charNo], 2);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[order[3]].charNo], 3);
            break;
    }
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_WINNER);
    HuPrcEnd();
}

void DecaMessReset(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_RESET);
    DecaWinOpen();
    HuWinMesSet(winId, MESS_MG_DECATHLON_RESET);
    HuWinMesWait(winId);
    if(HuWinChoiceGet(winId, -1) == 0) {
        DecaFlagSet(DECAFLAG_RESET);
    } else {
        HuWinMesSet(winId, MESS_MG_DECATHLON_BYE);
        HuWinMesWait(winId);
    }
    DecaMesFlagSet(DECA_MESSFLAG_RESET);
    HuPrcEnd();
}

void DecaMessBtnWait(void)
{
    int i;
    HUWINID winId;
    int num;
    DecaMesFlagReset(DECA_MESSFLAG_BTNWAIT);
    for(num=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(GwPlayerConf[i].type != GW_TYPE_MAN) {
            num++;
        }
    }
    if(num == GW_PLAYER_MAX) {
        DecaMesFlagSet(DECA_MESSFLAG_BTNWAIT);
        HuPrcEnd();
        return;
    }
    winId = HuWinCreate(230, 265, 240, 40, 0);
    HuWinMesSpeedSet(winId, 0);
    HuWinBGTPLvlSet(winId, 0);
    HuWinPriSet(winId, 5);
    HuWinAttrSet(winId, HUWIN_ATTR_ALIGN_RIGHT);
    HuWinMesSet(winId, MESS_SYSHELP_PARTYRESULT_END);
    HuWinMesWait(winId);
    while(1) {
        for(num=0, i=0; i<GW_PLAYER_MAX; i++) {
            if(HuPadBtnDown[i] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_01);
                num = GW_PLAYER_MAX;
                break;
            }
        }
        if(num == GW_PLAYER_MAX) {
            break;
        }
        HuPrcVSleep();
    }
    HuWinKill(winId);
    DecaMesFlagSet(DECA_MESSFLAG_BTNWAIT);
    HuPrcEnd();
}

void DecaMessGameEmotion(void)
{
    HUWINID winId = DecaWinIdGet();
    DecaMesFlagReset(DECA_MESSFLAG_EMOTION);
    DecaWinOpen();
    switch(DecaGameEmotionGet()) {
        case 0:
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_LEAD);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[DecaPlayerRankGet(0, NULL, NULL)].charNo], 0);
            break;
       
        case 1:
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_CLOSE);
            break;
       
        case 2:
            HuAudFXPlayPan(MSM_SE_GUIDE_57, 56);
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_PLAYER_FAIL);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[DecaPlayerRankGet(GW_PLAYER_MAX-1, NULL, NULL)].charNo], 0);
            break;
        
        case 3:
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_VERYCLOSE);
            break;
       
        case 4:
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_LEAD_GROW);
            break;
            
        case 5:
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_DEADHEAT);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[DecaPlayerRankGet(0, NULL, NULL)].charNo], 0);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[DecaPlayerRankGet(1, NULL, NULL)].charNo], 1);
            break;
       
        case 6:
            HuAudFXPlayPan(MSM_SE_GUIDE_57, 56);
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_TRYHARD);
            break;
        
        case 7:
        {
            int feelMes[7] = {
                MESS_MG_DECATHLON_FEEL_TERRIFIC,
                MESS_MG_DECATHLON_FEEL_SOSO,
                MESS_MG_DECATHLON_FEEL_ALRIGHT,
                MESS_MG_DECATHLON_FEEL_UNCERTAIN,
                MESS_MG_DECATHLON_FEEL_EXCITED,
                MESS_MG_DECATHLON_FEEL_GIDDY,
                MESS_MG_DECATHLON_FEEL_NERVOUS
            };
            HuWinMesSet(winId, MESS_MG_DECATHLON_EMOTION_PLAYER_SPEAK);
            HuWinInsertMesSet(winId, charMessTbl[GwPlayerConf[DecaPlayerRankGet(frandmod(GW_PLAYER_MAX), NULL, NULL)].charNo], 0);
            HuWinInsertMesSet(winId, feelMes[frandmod(7)], 1);
        }
            
            break;
    }
    HuWinMesWait(winId);
    DecaMesFlagSet(DECA_MESSFLAG_EMOTION);
    HuPrcEnd();
}