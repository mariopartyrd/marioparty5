#include "game/board/path.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/tutorial.h"
#include "game/board/com.h"
#include "game/board/audio.h"
#include "game/board/status.h"
#include "game/board/window.h"

#include "messnum/boardope.h"

#include "game/frand.h"

s8 MBPadStkXGet(int padNo);
s8 MBPadStkYGet(int padNo);
void MBScrollExec(int playerNo);

static OMOBJ *pathYajiObj[GW_PLAYER_MAX];

static u32 pathFlag;

void MBPathInit(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        pathYajiObj[i] = NULL;
    }
}

static int MasuGetPathLink(int masuId, s16 *linkTbl)
{
    int linkNum = 0;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    int i;
    for(i=0; i<masuP->linkNum; i++) {
        MASU *masuLinkP = MBMasuGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]);
        if((masuLinkP->flag & MASU_FLAG_PATHBLOCKL) == 0 && (masuLinkP->flag & MASU_FLAG_PATHBLOCKR) == 0 && (masuLinkP->flag & MBPathFlagGet()) == 0) {
            linkTbl[linkNum++] = masuP->linkTbl[i];
        }
    }
    return linkNum;
}

static s16 PathGetDefaultParty(int masuId)
{
    int choice;
    int len;
    int bestLen;
    MASU *masuP;
    int i;

    choice = -1;
    bestLen = 9999;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    for(i=0; i<masuP->linkNum; i++) {
        len = MBMasuPathTypeLenGet(masuP->linkTbl[i], MASU_TYPE_STAR);
        if(len < bestLen) {
            choice = i;
            bestLen = len;
        }
    }
    if(choice < 0) {
        return 0;
    } else {
        return choice;
    }
}

static s16 PathGetDefaultStoryCom(int masuId)
{
    int choice;
    int len;
    int bestLen;
    MASU *masuP;
    int i;
    
    choice = -1;
    bestLen = 9999;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    for(i=0; i<masuP->linkNum; i++) {
        len = MBMasuPathLenGet(masuP->linkTbl[i], GwPlayer[MBPlayerStoryPlayerGet()].masuId);
        if(len < bestLen) {
            choice = i;
            bestLen = len;
        }
    }
    if(choice < 0) {
        return 0;
    } else {
        return choice;
    }
}

static s16 PathGetDefaultStory(int masuId)
{
    int choice;
    int len;
    int bestLen;
    MASU *masuP;
    int i, j;
    
    choice = -1;
    bestLen = 9999;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    for(i=0; i<masuP->linkNum; i++) {
        
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(MBPlayerStoryComCheck(j) && MBPlayerAliveCheck(j)) {
                len = MBMasuPathLenGet(masuP->linkTbl[i], GwPlayer[j].masuId);
                if(len < bestLen) {
                    choice = i;
                    bestLen = len;
                }
            }
        }
    }
    if(choice < 0) {
        return 0;
    } else {
        return choice;
    }
}

typedef struct PathYajiWork_s {
    unsigned killF : 1;
    unsigned playerNo : 2;
    unsigned linkNum : 2;
    unsigned flag : 1;
    s8 choice;
    s16 angle;
    s16 mdlTemp;
    MBMODELID modelId[4];
} PATHYAJIWORK;

static void PathYajiUpdate(OMOBJ *obj);

static void PathYajiCreate(int playerNo, int masuId, s16 *linkTbl, int linkNum, s16 choiceStart)
{
    PATHYAJIWORK *work;
    OMOBJ *obj;
    HuVecF pos;
    HuVecF posLink;
    HuVecF dirLink;
    float angle;
    int i;
    if(pathYajiObj[playerNo] || linkNum == 1) {
        return;
    }
    pathYajiObj[playerNo] = obj = MBAddObj(256, 0, 0, PathYajiUpdate);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->scale.x = obj->scale.y = obj->scale.z = 3;
    work = omObjGetWork(obj, PATHYAJIWORK);
    work->linkNum = linkNum;
    work->killF = FALSE;
    work->flag = FALSE;
    work->playerNo = playerNo;
    work->angle = 0;
    work->choice = choiceStart;
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        if(GWPartyFGet() == TRUE || (GWPartyFGet() == FALSE && playerNo == MBPlayerStoryPlayerGet())) {
            MBWinCreateHelp(BOARDOPE_PAD_ENTER_NOEXIT);
        }
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    pos.y += 300;
    for(i=0; i<linkNum; i++) {
        work->modelId[i] = MBModelCreate(MBDATANUM(BOARD_HSF_yaji), NULL, TRUE);
        MBMasuPosGet(MASU_LAYER_DEFAULT, linkTbl[i], &posLink);
        VECSubtract(&posLink, &pos, &dirLink);
        angle = fmod(HuAtan(dirLink.x, dirLink.z), 360);
        MBModelRotSet(work->modelId[i], 0, angle, 0);
        MBModelLayerSet(work->modelId[i], 3);
        posLink.x = pos.x+(100*HuSin(angle));
        posLink.y = pos.y;
        posLink.z = pos.z+(100*HuCos(angle));
        MBModelPosSetV(work->modelId[i], &posLink);
        MBModelScaleSet(work->modelId[i], obj->scale.x, obj->scale.y, obj->scale.z);
    }
}

static void PathYajiUpdate(OMOBJ *obj)
{
    PATHYAJIWORK *work = omObjGetWork(obj, PATHYAJIWORK);
    if(work->killF || MBKillCheck()) {
        int i;
        for(i=0; i<work->linkNum; i++) {
            MBModelKill(work->modelId[i]);
        }
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            if(GWPartyFGet() == TRUE || (GWPartyFGet() == FALSE && work->playerNo == MBPlayerStoryPlayerGet())) {
                MBTopWinKill();
            }
        }
        pathYajiObj[work->playerNo] = NULL;
        MBDelObj(obj);
        return;
    }
    work->angle += 8;
    if(work->angle > 180) {
        work->angle -= 180;
    }
    obj->scale.x = work->angle;
    obj->scale.x = obj->scale.y = obj->scale.z = HuSin((float)work->angle)+3;
    MBModelScaleSet(work->modelId[work->choice], obj->scale.x, obj->scale.y, obj->scale.z);
}

static void PathYajiChoiceSet(int playerNo, int choice)
{
    if(pathYajiObj[playerNo]) {
        PATHYAJIWORK *work = omObjGetWork(pathYajiObj[playerNo], PATHYAJIWORK);
        work->choice = choice;
    }
}

static void PathYajiKill(int playerNo)
{
    if(pathYajiObj[playerNo]) {
        PATHYAJIWORK *work = omObjGetWork(pathYajiObj[playerNo], PATHYAJIWORK);
        work->killF = TRUE;
        HuPrcSleep(10);
    }
}

static BOOL PathExec(int playerNo, s16 *masuId, BOOL debugF)
{
    float anglePath[MASU_LINK_MAX*2];
    s16 pathLinkTbl[MASU_LINK_MAX*2];
    HuVecF dir;
    HuVecF dir2D;
    HuVecF pos;
    HuVecF pos2D;
    HuVecF posLink;
    HuVecF posLink2D;
    int i;
    int linkNum;
    int choice;
    int choiceTime;
    s16 masuPlayer;
    int choiceAuto;
    int padNo;
    static const s8 chanceTbl[] = {
        30, 20, 10, 0
    };
    
    choiceTime = -1;
    masuPlayer = GwPlayer[playerNo].masuId;
    linkNum = MasuGetPathLink(masuPlayer, pathLinkTbl);
    if(!debugF && linkNum <= 1) {
        *masuId = pathLinkTbl[0];
        return FALSE;
    }
    if(GWPartyFGet() != FALSE) {
        choice = PathGetDefaultParty(masuPlayer);
    } else if(MBPlayerStoryComCheck(playerNo)) {
        choice = PathGetDefaultStoryCom(masuPlayer);
    } else {
        choice = PathGetDefaultStory(masuPlayer);
    }
    PathYajiCreate(playerNo, masuPlayer, pathLinkTbl, linkNum, choice);
    if(debugF) {
        linkNum += MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuPlayer, &pathLinkTbl[linkNum]);
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuPlayer, &pos);
    MB3Dto2D(&pos, &pos2D);
    for(i=0; i<linkNum; i++) {
        MBMasuPosGet(MASU_LAYER_DEFAULT, pathLinkTbl[i], &posLink);
        MB3Dto2D(&posLink, &posLink2D);
        VECSubtract(&posLink2D, &pos2D, &dir2D);
        anglePath[i] = fmod(HuAtan(-dir2D.y, dir2D.x), 360);
    }
    if(!debugF) {
        MBPlayerMotIdleSet(playerNo);
        HuPrcSleep(10);
    }
    padNo = GwPlayer[playerNo].padNo;
    while(1) {
        u16 btnDown;
        if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            if(choiceTime < 0) {
                choiceAuto = MBTutorialExec(TUTORIAL_INST_PATH_CHOICE);
                dir.x = 64*HuCos(anglePath[choiceAuto]);
                dir.y = 64*HuSin(anglePath[choiceAuto]);
                choiceTime = 30;
            } else {
                choiceTime--;
                if(choiceTime == 0) {
                    btnDown = PAD_BUTTON_A;
                }
            }
        } else if(GwPlayer[playerNo].comF && !debugF) {
            if(choiceTime < 0) {
                if(GWPartyFGet() != FALSE) {
                    choiceAuto = MBComMasuPathNoGet(playerNo, linkNum, pathLinkTbl); 
                } else {
                    if(frandmod(100) < chanceTbl[GwPlayer[playerNo].dif]) {
                        choiceAuto = pathLinkTbl[frandmod(linkNum)];
                    } else {
                        choiceAuto = choice;
                    }
                }
                dir.x = 64*HuCos(anglePath[choiceAuto]);
                dir.y = 64*HuSin(anglePath[choiceAuto]);
                btnDown = 0;
                choiceTime = 6;
            } else {
                choiceTime--;
                if(choiceTime == 0) {
                    btnDown = PAD_BUTTON_A;
                }
            }
        } else {
            dir.x = MBPadStkXGet(padNo);
            dir.y = MBPadStkYGet(padNo);
            btnDown = HuPadBtnDown[padNo];
        }
        if(btnDown == PAD_BUTTON_A) {
            *masuId = pathLinkTbl[choice];
            MBAudFXPlay(MSM_SE_CMN_03);
            PathYajiKill(playerNo);
            break;
        }
        if(btnDown == PAD_BUTTON_Y) {
            MBAudFXPlay(MSM_SE_CMN_02);
            MBWalkNumDispSet(playerNo, FALSE);
            PathYajiKill(playerNo);
            MBPlayerMotIdleSet(playerNo);
            MBScrollExec(playerNo);
            MBStatusDispSetAll(TRUE);
            while(!MBStatusOffCheckAll()) {
                HuPrcVSleep();
            }
            MBWalkNumDispSet(playerNo, TRUE);
            PathYajiCreate(playerNo, masuPlayer, pathLinkTbl, linkNum, choice);
            continue;
        }
        if(debugF && (HuPadBtnDown[padNo] == PAD_BUTTON_X)) {
            PathYajiKill(playerNo);
            return TRUE;
        }
        if(dir.x != 0.0f || dir.y != 0.0f) {
            float angle = HuAtan(dir.y, dir.x);
            int dir = -1;
            float bestDiff = 9999;
            for(i=0; i<linkNum; i++) {
                float angleDiff = fmod(anglePath[i]-angle, 360);
                if(angleDiff < -180) {
                    angleDiff += 360;
                } else if(angleDiff > 180) {
                    angleDiff -= 360;
                }
                if(fabs(angleDiff) < bestDiff) {
                    dir = i;
                    bestDiff = fabs(angleDiff);
                }
            }
            if(bestDiff < 45) {
                if(choice != dir) {
                    MBAudFXPlay(MSM_SE_CMN_01);
                    choice = dir;
                    PathYajiChoiceSet(playerNo, choice);
                }
                if(debugF) {
                    *masuId = pathLinkTbl[choice];
                    PathYajiKill(playerNo);
                    break;
                }
            }
        } else {
            MBPlayerMotIdleSet(playerNo);
        }
        HuPrcVSleep();
    }
    return FALSE;
}


BOOL MBPathExec(int playerNo, s16 *masuId)
{
    return PathExec(playerNo, masuId, FALSE);
}

BOOL MBPathExecDebug(int playerNo, s16 *masuId)
{
    return PathExec(playerNo, masuId, TRUE);
}

void MBPathFlagSet(u32 flag)
{
    pathFlag |= flag;
}

void MBPathFlagReset(u32 flag)
{
    pathFlag &= ~flag;
}


void MBPathFlagInit(void)
{
    pathFlag = MASU_FLAG_NONE;
}

u32 MBPathFlagGet(void)
{
    return pathFlag;
}