#include "game/board/com.h"
#include "game/board/window.h"
#include "game/board/masu.h"
#include "game/frand.h"

static void SetChoiceComKey(int playerNo, BOOL moveF, BOOL horiF);

static void SetComChoiceLeft(void)
{
    SetChoiceComKey(GwSystem.turnPlayerNo, FALSE, TRUE);
}

void MBComChoiceSetLeft(void)
{
    MBTopWinComKeyHookSet(SetComChoiceLeft);
}

static void SetComChoiceRight(void)
{
    SetChoiceComKey(GwSystem.turnPlayerNo, TRUE, TRUE);
}

void MBComChoiceSetRight(void)
{
    MBTopWinComKeyHookSet(SetComChoiceRight);
}

static void SetComChoiceUp(void)
{
    SetChoiceComKey(GwSystem.turnPlayerNo, FALSE, FALSE);
}

void MBComChoiceSetUp(void)
{
    MBTopWinComKeyHookSet(SetComChoiceUp);
}

static void SetComChoiceDown(void)
{
    SetChoiceComKey(GwSystem.turnPlayerNo, TRUE, FALSE);
}

void MBComChoiceSetDown(void)
{
    MBTopWinComKeyHookSet(SetComChoiceDown);
}

static void SetChoiceComKey(int playerNo, BOOL moveF, BOOL horiF)
{
    s32 key[GW_PLAYER_MAX];
    int padNo;
    s16 delay;
    s32 keyDir;
    key[0] = key[1] = key[2] = key[3] = 0;
    padNo = GwPlayer[playerNo].padNo;
    delay = GWComKeyDelayGet();
    if(horiF == FALSE) {
        keyDir = PAD_BUTTON_DOWN;
    } else {
        keyDir = PAD_BUTTON_RIGHT;
    }
    if(moveF == TRUE) {
        key[padNo] = keyDir;
        HuWinComKeyWait(key[0], key[1], key[2], key[3], delay);
    }
    key[padNo] = PAD_BUTTON_A;
    HuWinComKeyWait(key[0], key[1], key[2], key[3], delay);
}

int MBComMasuPathNoGet(int playerNo, int linkNum, s16 *linkTbl)
{
    int i;
    int choice;
    int pathLenMin;
    static const s8 chanceTbl[] = {
        30, 20, 10, 0
    };
    choice = -1;
    pathLenMin = 9999;
    for(i=0; i<linkNum; i++) {
        int pathLen = MBMasuPathTypeLenGet(linkTbl[i], MASU_TYPE_STAR);
        if(pathLen < pathLenMin) {
            choice = i;
            pathLenMin = pathLen;
        }
    }
    if(choice < 0 || (pathLenMin > 20 && frandmod(100) < chanceTbl[GwPlayer[playerNo].dif])) {
        choice = frandmod(linkNum);
    }
    return choice;
}