#include "REL/decathlonDll.h"

typedef struct DecaScoreMg_s {
    HUSPRGRPID grpId;
    int winId[4];
} DECASCOREMG;

typedef struct DecaScorePlayer_s {
    HUSPRGRPID grpId;
    int winId[GW_PLAYER_MAX][4];
} DECASCOREPLAYER;

typedef struct DecaScoreWork_s {
    s16 mode;
    DECASCOREMG scoreMg;
    DECASCOREPLAYER scorePlayer;
} DECASCOREWORK;

static DECASCOREWORK decaScoreWork;

static int charSprFileTbl[15] = {
    DECATHLON_ANM_char_mario,
    DECATHLON_ANM_char_luigi,
    DECATHLON_ANM_char_peach,
    DECATHLON_ANM_char_yoshi,
    DECATHLON_ANM_char_wario,
    DECATHLON_ANM_char_daisy,
    DECATHLON_ANM_char_waluigi,
    DECATHLON_ANM_char_kinopio,
    DECATHLON_ANM_char_teresa,
    DECATHLON_ANM_char_minikoopa,
    DECATHLON_ANM_char_minikoopa,
    DECATHLON_ANM_char_minikoopa,
    DECATHLON_ANM_char_minikoopa,
    DECATHLON_ANM_char_minikoopa,
    DECATHLON_ANM_char_minikoopa,
};

static u32 unitMesTbl[6] = {
    MESS_MG_DECATHLON_SCORE_VALUE_TIME,
    MESS_MG_DECATHLON_SCORE_VALUE_DIST,
    MESS_MG_DECATHLON_SCORE_VALUE_DIST_FRAC,
    MESS_MG_DECATHLON_SCORE_VALUE_DIST_FOOT,
    0,
    MESS_MG_DECATHLON_SCORE_VALUE_NUMCAP
};

static HuVec2F scoreMgPos = { 162, 202 };
static HuVec2F scorePlayerPos = { 106, 112 };
static HuVec2F lbl_1_data_45C = { 106, 912 };
static HuVec2F scoreCharIconPos[GW_PLAYER_MAX] = {
    { 93, 20 },
    { 93, 60 },
    { 93, 100 },
    { 93, 140 },
};

static HuVec2F scoreArrowPos[GW_PLAYER_MAX] = {
    { 232, 20 },
    { 232, 60 },
    { 232, 100 },
    { 232, 140 },
};

static HuVec2F scoreWinPos[GW_PLAYER_MAX] = {
    { 0, 0 },
    { 102, 0 },
    { 105, 0 },
    { 262, 0 },
};

static int scoreWinSize[GW_PLAYER_MAX] = {
    80,
    253,
    112,
    96
};

static void MakeScoreMes(s8 *out, int digitMax, unsigned int score, BOOL leadZero)
{
    int outPos;
    int digit;
    int ofs;
    digit = score;
    outPos = 1;
    while(1) {
        digit = digit/10;
        if(digit == 0) {
            break;
        }
        outPos++;
    }
    if(outPos >= digitMax) {
        out[outPos] = 0;
        digit = score;
        while(outPos--) {
            out[outPos] = (digit%10)+'0';
            digit /= 10;
        }
    } else {
        ofs = digitMax-outPos;
        out[digitMax] = 0;
        digit = score;
        while(outPos--) {
            out[ofs+outPos] = (digit%10)+'0';
            digit /= 10;
        }
        if(leadZero) {
            while(ofs--) {
                out[ofs] = '0';
            }
        } else {
            while(ofs--) {
                out[ofs] = ' ';
            }
        }
    }
}

void DecaScoreModeSet(int mode)
{
    decaScoreWork.mode = mode;
}

BOOL DecaScoreIdleCheck(void)
{
    return decaScoreWork.mode == DECA_SCOREMODE_WAIT;
}

void DecaScoreInit(void)
{
    int i, j;
    decaScoreWork.mode = DECA_SCOREMODE_INIT;
    decaScoreWork.scoreMg.grpId = HUSPR_GRP_NONE;
    decaScoreWork.scorePlayer.grpId = HUSPR_GRP_NONE;
    for(i=4; i--;) {
        decaScoreWork.scoreMg.winId[i] = HUWIN_NONE;
    }
    for(i=GW_PLAYER_MAX; i--;) {
        for(j=4; j--;) {
            decaScoreWork.scorePlayer.winId[i][j] = HUWIN_NONE;
        }
    }
}

static void DecaScoreMain(void);

void DecaScoreStart(void)
{
    int i;
    DECASCOREMG *scoreMg;
    DECASCOREPLAYER *scorePlayer;
    HUSPRGRPID grpId;
    int j;
    int memberNo;
    HUWINID winId;
    ANIMDATA *animP;
    scoreMg = &decaScoreWork.scoreMg;
    scorePlayer = &decaScoreWork.scorePlayer;
    if(scoreMg->grpId < 0) {
        scoreMg->grpId = grpId = HuSprGrpCreate(10);
        for(i=10; i--;) {
            animP = HuSprAnimDataRead(decaMgTbl[i].mgIcon);
            HuSprGrpMemberSet(grpId, i, HuSprCreate(animP, 100, 0));
            HuSprTPLvlSet(grpId, i, 0);
        }
        for(i=0; i<4; i++) {
            if(scoreMg->winId[i] >= 0) {
                HuWinDispOff(scoreMg->winId[i]);
            } else {
                scoreMg->winId[i] = winId = HuWinExCreateFrame(214, (i*40)+128, 400, 32, HUWIN_SPEAKER_NONE, 1);
                HuWinMesSpeedSet(winId, 0);
                HuWinBGTPLvlSet(winId, 0);
            }
        }
        HuSprGrpPosSet(grpId, scoreMgPos.x, scoreMgPos.y);
    }
    if(scorePlayer->grpId < 0) {
        scorePlayer->grpId = grpId = HuSprGrpCreate(GW_PLAYER_MAX*2);
        for(i=GW_PLAYER_MAX; i--;) {
            animP = HuSprAnimDataRead(charSprFileTbl[GwPlayerConf[i].charNo]);
            memberNo = i;
            HuSprGrpMemberSet(grpId, memberNo, HuSprCreate(animP, 50, 0));
            HuSprPosSet(grpId, memberNo, scoreCharIconPos[i].x, scoreCharIconPos[i].y);
            HuSprTPLvlSet(grpId, memberNo, 0);
        }
        for(i=GW_PLAYER_MAX; i--;) {
            animP = HuSprAnimDataRead(DECATHLON_ANM_arrow);
            memberNo = i+GW_PLAYER_MAX;
            HuSprGrpMemberSet(grpId, memberNo, HuSprCreate(animP, 50, 0));
            HuSprPosSet(grpId, memberNo, scoreArrowPos[i].x, scoreArrowPos[i].y);
            HuSprTPLvlSet(grpId, memberNo, 0);
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            for(j=0; j<4; j++) {
                if(scorePlayer->winId[i][j] >= 0) {
                    HuWinDispOff(scoreMg->winId[i]);
                } else {
                    scorePlayer->winId[i][j] = winId = HuWinExCreateFrame(106+scoreWinPos[j].x, 112+scoreWinPos[j].y+(i*40), scoreWinSize[j], 32, HUWIN_SPEAKER_NONE, 1);
                    HuWinMesSpeedSet(winId, 0);
                    HuWinBGTPLvlSet(winId, 0);
                    HuWinDispOff(winId);
                }
            }
            HuWinAttrSet(scorePlayer->winId[i][1], HUWIN_ATTR_ALIGN_RIGHT);
            HuWinAttrSet(scorePlayer->winId[i][2], HUWIN_ATTR_ALIGN_RIGHT);
            HuWinAttrSet(scorePlayer->winId[i][3], HUWIN_ATTR_ALIGN_RIGHT);
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            HuWinMesSet(scorePlayer->winId[i][0], MESS_MG_DECATHLON_RANK_1+i);
        }
        HuSprGrpPosSet(grpId, scorePlayerPos.x, scorePlayerPos.y);
    }
    if(!DecaFlagGet(DECAFLAG_SCOREON)) {
        
        DecaFlagSet(DECAFLAG_SCOREON);
        HuPrcChildCreate(DecaScoreMain, 200, 4096, 0, HuPrcCurrentGet());
    }
}

static void ScoreWinUpdate(int winId, int value, s8 *textBuf1, s8 *textBuf2, BOOL padF)
{
    int mgNo = DecaMgNoGet();
    switch(decaMgTbl[mgNo].scoreType) {
        case DECA_SCORE_TIME:
            if(value == 0) {
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_2, 0);
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_2, 1);
            } else {
                if(padF) {
                    MakeScoreMes(textBuf1, 2, value/60, FALSE);
                    MakeScoreMes(textBuf2, 2, ((value%60)/60.0f)*100.0f, TRUE);
                } else {
                    MakeScoreMes(textBuf1, 0, value/60, FALSE);
                    MakeScoreMes(textBuf2, 2, ((value%60)/60.0f)*100.0f, TRUE);
                }
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf1), 0);
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf2), 1);
            }
            HuWinMesSet(winId, unitMesTbl[decaMgTbl[mgNo].scoreType]);
            break;
        
        case DECA_SCORE_DIST:
            if(padF) {
                MakeScoreMes(textBuf1, 3, value, FALSE);
            } else {
                MakeScoreMes(textBuf1, 0, value, FALSE);
            }
            HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf1), 0);
            HuWinMesSet(winId, unitMesTbl[decaMgTbl[mgNo].scoreType]);
            break;
        
        case DECA_SCORE_DIST_FRAC:
            if(value == 0) {
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_2, 0);
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_1, 1);
            } else {
                int num = (((value%1000)-(value%100))/100);
                if(!textBuf1 && num == 0){
                    num = 1;
                }
                if(padF) {
                    MakeScoreMes(textBuf1, 2, value/1000, FALSE);
                    MakeScoreMes(textBuf2, 1, num, FALSE);
                } else {
                    MakeScoreMes(textBuf1, 0, value/1000, FALSE);
                    MakeScoreMes(textBuf2, 0, num, FALSE);
                }
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf1), 0);
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf2), 1);
            }
            HuWinMesSet(winId, unitMesTbl[decaMgTbl[mgNo].scoreType]);
            break;
        
        case DECA_SCORE_DIST_FOOT:
            if(value == 0) {
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_2, 0);
                HuWinInsertMesSet(winId, MESS_MG_DECATHLON_NULL_2, 1);
            } else {
                if(GWLanguageGet() == HUWIN_LANG_ENGLISH) {
                    if(padF) {
                        MakeScoreMes(textBuf1, 3, value/100, FALSE);
                        MakeScoreMes(textBuf2, 2, value%100, TRUE);
                    } else {
                        MakeScoreMes(textBuf1, 0, value/100, FALSE);
                        MakeScoreMes(textBuf2, 2, value%100, TRUE);
                    }
                } else {
                    if(padF) {
                        MakeScoreMes(textBuf1, 2, value/100, FALSE);
                        MakeScoreMes(textBuf2, 2, value%100, TRUE);
                    } else {
                        MakeScoreMes(textBuf1, 0, value/100, FALSE);
                        MakeScoreMes(textBuf2, 2, value%100, TRUE);
                    }
                }
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf1), 0);
                HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf2), 1);
            }
            HuWinMesSet(winId, unitMesTbl[decaMgTbl[mgNo].scoreType]);
            break;
        
        case DECA_SCORE_NUMBER:
            if(padF) {
                MakeScoreMes(textBuf1, 2, value, FALSE);
            } else {
                MakeScoreMes(textBuf1, 0, value, FALSE);
            }
            HuWinMesSet(winId, MESSNUM_PTR(textBuf1));
            break;
        
        case DECA_SCORE_NUMBER_CAP:
            if(padF) {
                MakeScoreMes(textBuf1, 2, value, FALSE);
            } else {
                MakeScoreMes(textBuf1, 0, value, FALSE);
            }
            HuWinInsertMesSet(winId, MESSNUM_PTR(textBuf1), 0);
            HuWinMesSet(winId, unitMesTbl[decaMgTbl[mgNo].scoreType]);
            break;
    }
}

static void DecaScoreMain(void)
{
    int i;
    DECASCOREPLAYER *scorePlayer;
    int temp;
    DECASCOREMG *scoreMg;
    int timer;
    s16 soundTimer;
    s16 seNo;
    int rank;
    
    s8 scoreBuf1[GW_PLAYER_MAX][10];
    s8 scoreBuf2[GW_PLAYER_MAX][10];
    
    s8 scoreMes[GW_PLAYER_MAX][10];
    s8 scoreValueMes[GW_PLAYER_MAX][10];
    
    char mgNoMes[10];
    int startScore[GW_PLAYER_MAX];
    int mgScore[GW_PLAYER_MAX];
    int rankTbl[GW_PLAYER_MAX];
    int score[GW_PLAYER_MAX];
    scoreMg = &decaScoreWork.scoreMg;
    scorePlayer = &decaScoreWork.scorePlayer;
    while(DecaFlagGet(DECAFLAG_SCOREON)) {
        switch(decaScoreWork.mode) {
            case DECA_SCOREMODE_INIT:
                decaScoreWork.mode = DECA_SCOREMODE_WAIT;
                break;
            
            case DECA_SCOREMODE_RECORD:
            {
                int mgNo;
                int mgScore;
                
                mgNo = DecaMgNoGet();
                mgScore = DecaMgHighScoreGet();
                for(i=0; i<4; i++) { 
                    HuWinDispOn(scoreMg->winId[i]);
                }
                sprintf(mgNoMes, "%d", mgNo+1);
                HuWinInsertMesSet(scoreMg->winId[0], MESSNUM_PTR(mgNoMes), 0);
                HuWinInsertMesSet(scoreMg->winId[1], DecaMgNameMesGet(), 0);
                ScoreWinUpdate(scoreMg->winId[3], mgScore, &scoreBuf1[0][0], &scoreBuf2[0][0], FALSE);
                HuSprTPLvlSet(scoreMg->grpId, DecaMgNoGet(), 1);
                HuWinMesSet(scoreMg->winId[0], MESS_MG_DECATHLON_SCORE_MGNO);
                HuWinMesSet(scoreMg->winId[1], MESS_MG_DECATHLON_SCORE_MGNAME);
                HuWinMesSet(scoreMg->winId[2], MESS_MG_DECATHLON_SCORE_MGRECORD);
                decaScoreWork.mode = DECA_SCOREMODE_WAIT;
            }
                break;
            
            case DECA_SCOREMODE_MGSCORE:
            {
                int no;
                int mgNo;
                int rank;
                
                mgNo = DecaMgNoGet();
                for(i=GW_PLAYER_MAX; i--;) {
                    HuWinDispOn(scorePlayer->winId[i][0]);
                    HuWinDispOn(scorePlayer->winId[i][1]);
                    HuWinDispOn(scorePlayer->winId[i][2]);
                    HuWinDispOff(scorePlayer->winId[i][3]);
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    no = DecaPlayerRankGet(i, &rank, &score[i]);
                    HuSprPosSet(scorePlayer->grpId, no, scoreCharIconPos[i].x, scoreCharIconPos[i].y);
                    HuSprTPLvlSet(scorePlayer->grpId, no, 1);
                    HuWinMesSet(scorePlayer->winId[i][0], MESS_MG_DECATHLON_RANK_1+rank);
                    MakeScoreMes(&scoreValueMes[i][0], 5, score[i], FALSE);
                    HuWinMesSet(scorePlayer->winId[i][2], MESSNUM_PTR(&scoreValueMes[i][0]));
                    ScoreWinUpdate(scorePlayer->winId[i][1], DecaMgScoreGet(no), &scoreBuf1[i][0], &scoreBuf2[i][0], TRUE);
                    HuSprTPLvlSet(scorePlayer->grpId, i+GW_PLAYER_MAX, 0);
                }
                if(!DecaFlagGet(DECAFLAG_FADE_ENDING)) {
                    HuAudFXPlay(MSM_SE_DEFAULT_79);
                }
                decaScoreWork.mode = DECA_SCOREMODE_WAIT;
            }
                break;
            
            case DECA_SCOREMODE_SCORE:
            {
                int no;
                int rank;
                for(i=GW_PLAYER_MAX; i--;) {
                    HuWinDispOn(scorePlayer->winId[i][0]);
                    HuWinDispOff(scorePlayer->winId[i][1]);
                    HuWinDispOn(scorePlayer->winId[i][2]);
                    HuWinDispOn(scorePlayer->winId[i][3]);
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    no = DecaPlayerRankGet(i, &rank, &score[i]);
                    HuSprPosSet(scorePlayer->grpId, no, scoreCharIconPos[i].x, scoreCharIconPos[i].y);
                    HuSprTPLvlSet(scorePlayer->grpId, no, 1);
                    HuWinMesSet(scorePlayer->winId[i][0], MESS_MG_DECATHLON_RANK_1+rank);
                    MakeScoreMes(&scoreValueMes[i][0], 5, score[i], FALSE);
                    MakeScoreMes(&scoreMes[i][0], 4, DecaScoreGet(no), FALSE);
                    HuWinMesSet(scorePlayer->winId[i][2], MESSNUM_PTR(&scoreValueMes[i][0]));
                    HuWinMesSet(scorePlayer->winId[i][3], MESSNUM_PTR(&scoreMes[i][0]));
                    HuSprTPLvlSet(scorePlayer->grpId, i+GW_PLAYER_MAX, 0);
                }
                HuAudFXPlay(MSM_SE_DEFAULT_79);
                decaScoreWork.mode = DECA_SCOREMODE_WAIT;
            }
                break;
            
            case DECA_SCOREMODE_ADD_BEGIN:
            {
                int no;
                int doneNum;
                int rank;
                for(i=GW_PLAYER_MAX; i--;) {
                    HuWinDispOn(scorePlayer->winId[i][0]);
                    HuWinDispOff(scorePlayer->winId[i][1]);
                    HuWinDispOn(scorePlayer->winId[i][2]);
                    HuWinDispOn(scorePlayer->winId[i][3]);
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    rankTbl[i] = no = DecaPlayerRankGet(i, &rank, &startScore[i]);
                    mgScore[i] = DecaScoreGet(no);
                    score[i] = startScore[i]+mgScore[i];
                    if(temp < mgScore[i]) {
                        temp = mgScore[i];
                    }
                    HuWinMesSet(scorePlayer->winId[i][0], MESS_MG_DECATHLON_RANK_1+rank);
                    MakeScoreMes(&scoreValueMes[i][0], 5, startScore[i], FALSE);
                    MakeScoreMes(&scoreMes[i][0], 4, mgScore[i], FALSE);
                    HuSprPosSet(scorePlayer->grpId, no, scoreCharIconPos[i].x, scoreCharIconPos[i].y);
                    HuSprTPLvlSet(scorePlayer->grpId, no, 1);
                    HuSprTPLvlSet(scorePlayer->grpId, i+GW_PLAYER_MAX, 1);
                }
                temp = 1+(temp/60.0f);
                timer = 60;
                soundTimer = 0;
                seNo = HuAudFXPlay(MSM_SE_DEFAULT_79);
                decaScoreWork.mode = DECA_SCOREMODE_ADD;
                
            case DECA_SCOREMODE_ADD:
                doneNum = 0;
                if(timer) {
                    if(--timer == 0) {
                        for(i=0; i<GW_PLAYER_MAX; i++) {
                            int vibTime = mgScore[i]/temp;
                            if(vibTime) {
                                omVibrate(GwPlayerConf[rankTbl[i]].padNo, vibTime, 4, 2);
                            }
                            HuAudFXPlay(MSM_SE_DEFAULT_79);
                        }
                    }
                    break;
                } else {
                    for(i=GW_PLAYER_MAX; i--;) {
                        soundTimer++;
                        if(soundTimer == 6) {
                            HuAudFXStop((int)seNo);
                            seNo = HuAudFXPlay(MSM_SE_DEFAULT_79);
                            soundTimer = 0;
                        }
                        if(startScore[i] <= score[i]-temp) {
                            startScore[i] += temp;
                            mgScore[i] -= temp;
                        } else {
                            startScore[i] = score[i];
                            mgScore[i] = 0;
                            doneNum++;
                        }
                        MakeScoreMes(&scoreValueMes[i][0], 5, startScore[i], FALSE);
                        MakeScoreMes(&scoreMes[i][0], 4, mgScore[i], FALSE);
                        HuWinMesSet(scorePlayer->winId[i][2], MESSNUM_PTR(&scoreValueMes[i][0]));
                        HuWinMesSet(scorePlayer->winId[i][3], MESSNUM_PTR(&scoreMes[i][0]));
                    }
                    if(doneNum == 4) {
                        HuAudFXStop(seNo);
                        seNo = HuAudFXPlay(MSM_SE_DEFAULT_79);
                        decaScoreWork.mode = DECA_SCOREMODE_WAIT;
                    }
                }
            }
            break;
            
            case DECA_SCOREMODE_NEWRANK_BEGIN:
            {
                int no;
                int rank;
                
                for(i=GW_PLAYER_MAX; i--;) {
                    HuWinDispOff(scorePlayer->winId[i][0]);
                    HuWinDispOff(scorePlayer->winId[i][1]);
                    HuWinDispOff(scorePlayer->winId[i][2]);
                    HuWinDispOff(scorePlayer->winId[i][3]);
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    no = DecaPlayerRankGet(i, &rank, &score[i]);
                    MakeScoreMes(&scoreValueMes[i][0], 5, score[i], FALSE);
                    HuWinMesSet(scorePlayer->winId[i][0], MESS_MG_DECATHLON_RANK_1+rank);
                    HuSprPosSet(scorePlayer->grpId, no, scoreCharIconPos[i].x, scoreCharIconPos[i].y);
                    HuSprTPLvlSet(scorePlayer->grpId, no, 0);
                    HuSprTPLvlSet(scorePlayer->grpId, i+GW_PLAYER_MAX, 0);
                }
                temp = GW_PLAYER_MAX;
                timer = 75;
                decaScoreWork.mode = DECA_SCOREMODE_NEWRANK;
            }
            
            case DECA_SCOREMODE_NEWRANK:
            {
                int no;
                int rank;
                if(timer) {
                    timer--;
                } else {
                    timer = 45;
                    HuAudFXPlay(MSM_SE_DEFAULT_78);
                    temp--;
                    HuWinDispOn(scorePlayer->winId[temp][0]);
                    HuWinDispOff(scorePlayer->winId[temp][1]);
                    HuWinDispOn(scorePlayer->winId[temp][2]);
                    HuWinDispOff(scorePlayer->winId[temp][3]);
                    no = DecaPlayerRankGet(temp, &rank, &score[temp]);
                    omVibrate(no, 15, 4, 2);
                    HuWinMesSet(scorePlayer->winId[temp][0], MESS_MG_DECATHLON_RANK_1+rank);
                    MakeScoreMes(&scoreValueMes[temp][0], 5, score[temp], FALSE);
                    HuWinMesSet(scorePlayer->winId[temp][2], MESSNUM_PTR(&scoreValueMes[temp][0]));
                    HuSprPosSet(scorePlayer->grpId, no, scoreCharIconPos[temp].x, scoreCharIconPos[temp].y);
                    HuSprTPLvlSet(scorePlayer->grpId, no, 1);
                    HuSprTPLvlSet(scorePlayer->grpId, temp+GW_PLAYER_MAX, 0);
                    if(temp == 0) {
                        decaScoreWork.mode = DECA_SCOREMODE_WAIT;
                    }
                }
            }
                break;
            
            case DECA_SCOREMODE_STOP:
                decaScoreWork.mode = DECA_SCOREMODE_STOP_IDLE;
                break;
            
            case DECA_SCOREMODE_HIDE:
                for(i=GW_PLAYER_MAX; i--;) {
                    HuWinDispOff(scorePlayer->winId[i][0]);
                    HuWinDispOff(scorePlayer->winId[i][1]);
                    HuWinDispOff(scorePlayer->winId[i][2]);
                    HuWinDispOff(scorePlayer->winId[i][3]);
                }
                for(i=GW_PLAYER_MAX; i--;) {
                    HuSprTPLvlSet(scorePlayer->grpId, i, 0);
                    HuSprTPLvlSet(scorePlayer->grpId, i+GW_PLAYER_MAX, 0);
                }
                for(i=0; i<4; i++) {
                    HuWinDispOff(scoreMg->winId[i]);
                }
                for(i=0; i<10; i++) {
                    HuSprTPLvlSet(scoreMg->grpId, i, 0);
                }
                break;
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}