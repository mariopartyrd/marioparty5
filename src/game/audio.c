#include "game/audio.h"
#include "game/memory.h"
#include "game/armem.h"
#include "game/wipe.h"
#include "game/object.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"

#define HUMSMHEAP_SIZE 0xBFC00

#define HUAUD_STREAM_MAX 16

#define MSM_FILE_PATH "/sound/MP5_SND.msm"
#define PDT_FILE_PATH "/sound/MP5_Str.pdt"

static int HuSePlay(int seId, MSM_SEPARAM *param);

static BOOL charFxLoadF[13];
static s32 sndFxBuf[64][2];

static s16 Hu3DAudVol;
static s16 sndGroupBak;
static BOOL auxANoBak;
static BOOL auxBNoBak;
static s8 HuAuxAVol;
static s8 HuAuxBVol;
float Snd3DBackSurDisOffset;
float Snd3DFrontSurDisOffset;
float Snd3DStartDisOffset;
float Snd3DSpeedOffset;
float Snd3DDistOffset;
BOOL musicOffF;
u8 fadeStat;

void HuAudInit(void)
{
    MSM_INIT msmInit;
    MSM_ARAM msmAram;
    s32 result;
    s16 i;
    msmInit.heap = HuMemDirectMalloc(HEAP_SOUND, HUMSMHEAP_SIZE);
    msmInit.heapSize = HUMSMHEAP_SIZE;
    msmInit.msmPath = MSM_FILE_PATH;
    msmInit.pdtPath = PDT_FILE_PATH;
    msmInit.open = NULL;
    msmInit.read = NULL;
    msmInit.close = NULL;
    msmAram.skipARInit = TRUE;
    msmAram.aramEnd = HU_AMEM_BASE;
    result = msmSysInit(&msmInit, &msmAram);
    if(result < 0) {
        OSReport("MSM(Sound Manager) Error:Error Code %d\n", result);
        while(1);
    }
    if(OSGetSoundMode() == OS_SOUND_MODE_MONO) {
        msmSysSetOutputMode(SND_OUTPUTMODE_MONO);
    } else {
        msmSysSetOutputMode(SND_OUTPUTMODE_SURROUND);
    }
    for(i=0; i<64; i++) {
        sndFxBuf[i][0] = -1;
    }
    for(i=0; i<13; i++) {
        charFxLoadF[i] = FALSE;
    }
    sndGroupBak = MSM_GRP_NONE;
    auxANoBak = 0;
    auxBNoBak = 1;
    fadeStat = FALSE;
    musicOffF = FALSE;
}

s32 HuAudStreamPlay(char *name, BOOL flag)
{
    return 0;
}

void HuAudStreamVolSet(s16 vol)
{
    AISetStreamVolLeft(vol);
    AISetStreamVolRight(vol);
    Hu3DAudVol = vol;
}

void HuAudStreamPauseOn(void)
{
    AISetStreamPlayState(AI_STREAM_STOP);
}

void HuAudStreamPauseOff(void)
{
    AISetStreamPlayState(AI_STREAM_START);
}

void HuAudStreamFadeOut(s32 streamNo)
{
    
}

void HuAudAllStop(void)
{
    HuAudSeqAllStop();
    HuAudFXAllStop();
    HuAudSStreamAllStop();
}

void HuAudFadeOut(s32 speed)
{
    HuAudFXAllStop();
    HuAudSeqAllFadeOut(speed);
    HuAudSStreamAllFadeOut(speed);
}

static void dummyfloat(unsigned int i)
{
    float y = i;
}

int HuAudFXPlay(int seId)
{
    WIPEWORK *wipeP = &wipeData;

    if(omSysExitReq || (wipeData.mode == WIPE_MODE_OUT && wipeP->time / wipeP->maxTime > 0.5)) {
        return 0;
    }
    return HuAudFXPlayVolPan(seId, MSM_VOL_MAX, MSM_PAN_CENTER);
}

int HuAudFXPlayVol(int seId, s16 vol)
{
    if(omSysExitReq) {
        return 0;
    }
    return HuAudFXPlayVolPan(seId, vol, MSM_PAN_CENTER);
}

int HuAudFXPlayPan(int seId, s16 pan)
{
    MSM_SEPARAM seParam;
    if(omSysExitReq) {
        return 0;
    }
    seParam.flag = MSM_SEPARAM_PAN;
    seParam.pan = pan;
    return HuSePlay(seId, &seParam);
}

int HuAudFXPlayVolPan(int seId, s16 vol, s16 pan)
{
    MSM_SEPARAM seParam;
    if(omSysExitReq) {
        return 0;
    }
    seParam.flag = MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
    seParam.vol = vol;
    seParam.pan = pan;
    return HuSePlay(seId, &seParam);
}

void HuAudFXStop(int seNo)
{
    msmSeStop(seNo, 0);
}

void HuAudFXAllStop(void)
{
    msmSeStopAll(FALSE, 0);
}

void HuAudFXFadeOut(int seNo, s32 speed)
{
    msmSeStop(seNo, speed);
}

void HuAudFXPanning(int seNo, s16 pan)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return;
    }
    param.flag = MSM_SEPARAM_PAN;
    param.pan = pan;
    msmSeSetParam(seNo, &param);
}

void HuAudFXListnerSet(Vec *pos, Vec *heading, float sndDist, float sndSpeed)
{
    if(omSysExitReq) {
      return;
    }
    HuAudFXListnerSetEX(pos, heading,
        sndDist + Snd3DDistOffset,
        sndSpeed + Snd3DSpeedOffset,
        Snd3DStartDisOffset,
        Snd3DFrontSurDisOffset + (0.25 * sndDist + Snd3DStartDisOffset),
        Snd3DBackSurDisOffset + (0.25 * sndDist + Snd3DStartDisOffset));
}

void HuAudFXListnerSetEX(Vec *pos, Vec *heading, float sndDist, float sndSpeed, float startDis, float frontSurDis, float backSurDis)
{
    MSM_SELISTENER listener;
    if(omSysExitReq) {
      return;
    }
    listener.flag = MSM_LISTENER_STARTDIS|MSM_LISTENER_FRONTSURDIS|MSM_LISTENER_BACKSURDIS;
    listener.startDis = startDis + Snd3DStartDisOffset;
    listener.frontSurDis = frontSurDis + Snd3DFrontSurDisOffset;
    listener.backSurDis = backSurDis + Snd3DBackSurDisOffset;
    msmSeSetListener(pos, heading, sndDist + Snd3DDistOffset, sndSpeed + Snd3DSpeedOffset, &listener);
    OSReport("//////////////////////////////////\n");
    OSReport("sndDist %f\n", sndDist);
    OSReport("sndSpeed %f\n", sndSpeed);
    OSReport("startDis %f\n", listener.startDis);
    OSReport("frontSurDis %f\n", listener.frontSurDis);
    OSReport("backSurDis %f\n", listener.backSurDis);
    OSReport("//////////////////////////////////\n");
}

void HuAudFXListnerUpdate(Vec *pos, Vec *heading)
{
    if(omSysExitReq) {
      return;
    }
    msmSeUpdataListener(pos, heading);
}

int HuAudFXEmiterPlay(int seId, Vec *pos)
{
    MSM_SEPARAM seParam;
    if(omSysExitReq) {
      return 0;
    }
    seParam.flag = MSM_SEPARAM_POS;
    seParam.pos.x = pos->x;
    seParam.pos.y = pos->y;
    seParam.pos.z = pos->z;
    return HuSePlay(seId, &seParam);
}

void HuAudFXEmiterUpDate(int seNo, Vec *pos)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return;
    }
    param.flag = MSM_SEPARAM_POS;
    param.pos.x = pos->x;
    param.pos.y = pos->y;
    param.pos.z = pos->z;
    msmSeSetParam(seNo, &param);
}

void HuAudFXListnerKill(void)
{
    msmSeDelListener();
}

void HuAudFXPauseAll(BOOL pauseF)
{
    msmSePauseAll(pauseF, 100);
}

s32 HuAudFXStatusGet(int seNo)
{
    return msmSeGetStatus(seNo);
}

s32 HuAudFXPitchSet(int seNo, s16 pitch)
{
    MSM_SEPARAM param;

    if(omSysExitReq) {
        return 0;
    }
    param.flag = MSM_SEPARAM_PITCH;
    param.pitch = pitch;
    return msmSeSetParam(seNo, &param);
}

s32 HuAudFXVolSet(int seNo, s16 vol)
{
    MSM_SEPARAM param;

    if(omSysExitReq) {
        return 0;
    }
    param.flag = MSM_SEPARAM_VOL;
    param.vol = vol;
    return msmSeSetParam(seNo, &param);
}

int HuAudSeqPlay(s16 musId)
{
    int musNo;
    if(musicOffF || omSysExitReq) {
        return 0;
    }
    musNo = msmMusPlay(musId, NULL);
    return musNo;
}

void HuAudSeqStop(int musNo)
{
    if(musicOffF || omSysExitReq) {
        return;
    }
    msmMusStop(musNo, 0);
}

void HuAudSeqFadeOut(int musNo, s32 speed)
{
    if(musicOffF) {
        return;
    }
    msmMusStop(musNo, speed);
}

void HuAudSeqAllFadeOut(s32 speed)
{
    s16 i;
    for(i=0; i<MSM_MUS_MAX; i++) {
        if(msmMusGetStatus(i) == MSM_MUS_PLAY) {
            msmMusStop(i, speed);
        }
    }
}

void HuAudSeqAllStop(void)
{
    msmMusStopAll(FALSE, 0);
}

void HuAudSeqPauseAll(BOOL pause)
{
    msmMusPauseAll(pause, 100);
}

s32 HuAudSeqMidiCtrlGet(int musNo, s8 channel, s8 ctrl)
{
    if(musicOffF || omSysExitReq) {
        return 0;
    }
    return msmMusGetMidiCtrl(musNo, channel, ctrl);
}

static void SStreamStartProc(void);

typedef struct sStreamWork_s {
    int chanNo;
    int streamId;
} SSTREAMWORK;

static u8 streamVol[HUAUD_STREAM_MAX];

int HuAudSStreamChanPlay(s16 streamId, s16 chanNo)
{
    if(musicOffF || omSysExitReq) {
        return MSM_STREAMNO_NONE;
    }
    if(msmStreamGetStatus(chanNo) != MSM_STREAM_DONE) {
        HUPROCESS *process;
        SSTREAMWORK *work;
        msmStreamStop(chanNo, 0);
        process = HuPrcCreate(SStreamStartProc, MSM_AUXB_DEFAULT, 2304, 0);
        process->property = work = HuMemDirectMalloc(HEAP_HEAP, sizeof(SSTREAMWORK));
        work->chanNo = chanNo;
        work->streamId = streamId;
        return chanNo;
    } else {
        MSM_STREAMPARAM param;
        int streamNo;
        param.flag = MSM_STREAMPARAM_CHAN;
        param.chan = chanNo;
        streamNo = msmStreamPlay(streamId, &param);
        streamVol[chanNo] = MSM_VOL_MAX;
        return streamNo;
    }
}

static void SStreamStartProc(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    SSTREAMWORK *work = process->property;
    MSM_STREAMPARAM param;
    while(msmStreamGetStatus(work->chanNo) != MSM_STREAM_DONE) {
        HuPrcVSleep();
    }
    param.flag = MSM_STREAMPARAM_CHAN;
    param.chan = work->chanNo;
    msmStreamPlay(work->streamId, &param);
    streamVol[work->chanNo] = MSM_VOL_MAX;
    HuMemDirectFree(work);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

int HuAudSStreamPlay(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 0);
}

int HuAudSStreamPlayFront(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 0);
}

int HuAudSStreamPlayBack(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 2);
}

void HuAudSStreamStop(int streamNo)
{
    if(musicOffF) {
        return;
    }
    msmStreamStop(streamNo, 0);
}

void HuAudSStreamFadeOut(int streamNo, s32 speed)
{
    if(musicOffF) {
        return;
    }
    msmStreamStop(streamNo, speed);
}

void HuAudSStreamAllFadeOut(s32 speed)
{
    msmStreamStopAll(speed);
}

void HuAudSStreamPause(s16 streamNo, BOOL pause)
{
    msmStreamPause(streamNo, (pause) ? TRUE : FALSE, 5);
}

void HuAudSStreamPauseFadeOut(s16 streamNo, BOOL pause, s32 speed)
{
    msmStreamPause(streamNo, (pause) ? TRUE : FALSE, speed);
}

void HuAudSStreamPauseAll(BOOL pause)
{
    msmStreamPauseAll((pause) ? TRUE : FALSE, 5);
}

void HuAudSStreamAllStop(void)
{
    msmStreamStopAll(0);
}

s32 HuAudSStreamStatGet(int streamNo)
{
    return msmStreamGetStatus(streamNo);
}

typedef struct sStreamFadeWork_s {
    u8 volStart;
    u8 volEnd;
    u32 speed;
    int streamNo;
} SSTREAMFADEWORK;

static void SStreamFade(void);

void HuAudSStreamVolSet(int streamNo, u8 vol, u32 speed)
{
    if(musicOffF) {
        return;
    }
    if(speed <= 16) {
        MSM_STREAMPARAM param;
        param.flag = MSM_STREAMPARAM_VOL;
        param.vol = vol;
        msmStreamSetParam(streamNo, &param);
        streamVol[streamNo] = vol;
    } else {
        HUPROCESS *process = HuPrcCreate(SStreamFade, MSM_AUXB_DEFAULT, 2304, 0);
        SSTREAMFADEWORK *work = HuMemDirectMalloc(HEAP_HEAP, sizeof(SSTREAMFADEWORK));
        process->property = work;
        work->speed =  speed;
        work->streamNo = streamNo;
        work->volStart = streamVol[streamNo];
        work->volEnd = vol;
    }
}

static void SStreamFade(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    SSTREAMFADEWORK *work = process->property;
    MSM_STREAMPARAM param;
    float maxTime;
    s16 i;
    param.flag = MSM_STREAMPARAM_VOL;
    maxTime = work->speed/16.666668f;
    for(i=1; i<=maxTime; i++) {
        float vol;
        float time;
        time = i/maxTime;
        vol = (work->volEnd*time)+(work->volStart*(1.0-time));
        param.vol = vol;
        msmStreamSetParam(work->streamNo, &param);
        streamVol[work->streamNo] = vol;
        HuPrcVSleep();
    }
    param.vol = work->volEnd;
    msmStreamSetParam(work->streamNo, &param);
    streamVol[work->streamNo] = work->volEnd;
    HuMemDirectFree(work);
    HuPrcEnd();
    HuPrcSleep(10);
}

SNDGRPTBL sndGrpTable[] = {
    { DLL_w01dll, MSM_GRP_BD1, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w02dll, MSM_GRP_BD2, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w03dll, MSM_GRP_BD3, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w04dll, MSM_GRP_BD4, 6, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w05dll, MSM_GRP_BD5, 5, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w06dll, MSM_GRP_BD6, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w07dll, MSM_GRP_BD7, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w01sdll, MSM_GRP_BD1, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w02sdll, MSM_GRP_BD2, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w03sdll, MSM_GRP_BD2, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w04sdll, MSM_GRP_BD4, 6, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w05sdll, MSM_GRP_BD5, 5, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w06sdll, MSM_GRP_BD6, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_w07sdll, MSM_GRP_BD7, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mgmatchdll, MSM_GRP_DEFAULT, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mgtourdll, MSM_GRP_DEFAULT, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mgwarsdll, MSM_GRP_DEFAULT, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mgfreedll, MSM_GRP_DEFAULT, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_decathlondll, MSM_GRP_DEFAULT, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m501dll, MSM_GRP_MG501, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m502dll, MSM_GRP_MG502, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m503dll, MSM_GRP_MG503, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m504dll, MSM_GRP_MG504, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m505dll, MSM_GRP_MG505, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m506dll, MSM_GRP_MG506, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m507dll, MSM_GRP_MG507, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m508dll, MSM_GRP_MG508, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m509dll, MSM_GRP_MG509, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m510dll, MSM_GRP_MG510, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m511dll, MSM_GRP_MG511, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m512dll, MSM_GRP_MG512, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m513dll, MSM_GRP_MG513, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m514dll, MSM_GRP_MG514, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m515dll, MSM_GRP_MG515, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m516dll, MSM_GRP_MG516, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m517dll, MSM_GRP_MG517, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m518dll, MSM_GRP_MG518, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m519dll, MSM_GRP_MG519, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m520dll, MSM_GRP_MG520, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m521dll, MSM_GRP_MG521, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m522dll, MSM_GRP_MG522, 5, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m523dll, MSM_GRP_MG523, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m524dll, MSM_GRP_MG524, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m525dll, MSM_GRP_MG525, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m526dll, MSM_GRP_MG526, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m527dll, MSM_GRP_MG527, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m528dll, MSM_GRP_MG528, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m529dll, MSM_GRP_MG529, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m530dll, MSM_GRP_MG530, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m531dll, MSM_GRP_MG531, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m532dll, MSM_GRP_MG532, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m533dll, MSM_GRP_MG533, 6, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m534dll, MSM_GRP_MG534, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m535dll, MSM_GRP_MG535, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m536dll, MSM_GRP_MG536, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m537dll, MSM_GRP_MG537, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m538dll, MSM_GRP_MG538, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m539dll, MSM_GRP_MG539, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m540dll, MSM_GRP_MG540, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m541dll, MSM_GRP_MG541, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m542dll, MSM_GRP_MG542, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m543dll, MSM_GRP_MG543, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m544dll, MSM_GRP_MG544, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m545dll, MSM_GRP_MG545, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m546dll, MSM_GRP_MG546, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m547dll, MSM_GRP_MG547, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m548dll, MSM_GRP_MG548, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m549dll, MSM_GRP_MG549, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m550dll, MSM_GRP_MG550, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m551dll, MSM_GRP_MG551, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m552dll, MSM_GRP_MG552, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m553dll, MSM_GRP_MG553, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m554dll, MSM_GRP_MG554, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m555dll, MSM_GRP_MG555, 7, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m562dll, MSM_GRP_MG562, 5, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m563dll, MSM_GRP_MG563, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m564dll, MSM_GRP_MG564, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m565dll, MSM_GRP_MG565, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m566dll, MSM_GRP_MG566, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m567dll, MSM_GRP_MG567, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m568dll, MSM_GRP_MG568, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m569dll, MSM_GRP_MG569, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m570dll, MSM_GRP_MG570, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m571dll, MSM_GRP_MGDONKEY, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m572dll, MSM_GRP_MGDONKEY, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m573dll, MSM_GRP_MGDONKEY, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m574dll, MSM_GRP_MG574, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m575dll, MSM_GRP_MG575, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m576dll, MSM_GRP_MG576, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m577dll, MSM_GRP_MG577, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m433dll, MSM_GRP_MG433, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m579dll, MSM_GRP_MG579, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m580dll, MSM_GRP_MG580, 2, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m559dll, MSM_GRP_MGKOOPA, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m560dll, MSM_GRP_MGKOOPA, 3, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_m561dll, MSM_GRP_MGKOOPA, 4, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdseldll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_systemdll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdminidll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdstorydll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdcarddll, MSM_GRP_CARD, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_rescarddll, MSM_GRP_CARD, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdomakedll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdbeachdll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdpartydll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_mdoptiondll, MSM_GRP_OPTION, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_partyresultdll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_storymodedll, MSM_GRP_STORY, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_storyresultdll, MSM_GRP_STORY, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_carddll, MSM_GRP_CARD, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_sd00dll, MSM_GRP_SD, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_sdroomdll, MSM_GRP_SD, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_fileseldll, MSM_GRP_MENU, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_instdll, MSM_GRP_NONE, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_resultdll, MSM_GRP_NONE, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_bootdll, MSM_GRP_NONE, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, -1, -1 },
    { DLL_NONE, MSM_GRP_NONE, MSM_AUXA_DEFAULT, MSM_AUXB_DEFAULT, 0, 0 }
};

void HuAudDllSndGrpSet(u16 ovl)
{
    SNDGRPTBL *sndGrpP;
    s16 grpSet;
    if(ovl == DLL_bootdll) {
        return;
    }
    sndGrpP=&sndGrpTable[0];
    while(1) {
        if(sndGrpP->ovl == ovl) {
            grpSet = sndGrpP->grpSet;
            break;
        }
        if(sndGrpP->ovl == DLL_NONE) {
            grpSet = MSM_GRP_DEFAULT;
            break;
        }
        sndGrpP++;
    }
    OSReport("SOUND ##########################\n");
    if(grpSet != MSM_GRP_NONE) {
        HuAudSndGrpSetSet(grpSet);
    }
    if(sndGrpP->auxANo != auxANoBak || sndGrpP->auxBNo != auxBNoBak) {
        msmSysSetAux(sndGrpP->auxANo, sndGrpP->auxBNo);
        OSReport("Change AUX %d,%d\n", sndGrpP->auxANo, sndGrpP->auxBNo);
        auxANoBak = sndGrpP->auxANo;
        auxBNoBak = sndGrpP->auxBNo;
        HuPrcVSleep();
    }
    HuAudAUXVolSet(sndGrpP->auxAVol, sndGrpP->auxBVol);
    OSReport("##########################\n");
}

#define SNDGRP_TIMEOUT 500

#define SNDGRP_WAIT(tickStart) \
    while((msmMusGetNumPlay(TRUE) != 0 || msmSeGetNumPlay(TRUE) != 0) && OSTicksToMilliseconds(OSGetTick()-(tickStart)) < SNDGRP_TIMEOUT)

void HuAudSndGrpSetSet(s16 grpSet)
{
    void *buf;
    OSTick tickStart;
    s32 result;

    if(sndGroupBak != grpSet) {
        msmMusStopAll(TRUE, 0);
        msmSeStopAll(TRUE, 0);
        tickStart = OSGetTick();
        SNDGRP_WAIT(tickStart);
        if(OSTicksToMilliseconds(OSGetTick()-tickStart) >= SNDGRP_TIMEOUT) {
            OSReport("Timed Out! Mus %d:SE %d\n", msmMusGetNumPlay(TRUE), msmSeGetNumPlay(TRUE));
        }
        OSReport("GroupSet %d\n", grpSet);
        sndGroupBak = grpSet;
        result = msmSysDelGroupAll();
        buf = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(TRUE));
        result = msmSysLoadGroup(grpSet, buf, FALSE);
        if(result) {
            OSReport("***********GroupSet Error %d\n", result);
        }
        HuMemDirectFree(buf);
    }
}

void HuAudSndGrpSet(s16 grp)
{
    void *buf = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(grp));
    msmSysLoadGroup(grp, buf, FALSE);
    HuMemDirectFree(buf);
}

void HuAudSndCommonGrpSet(s16 grp, BOOL delGrpF) 
{
    OSTick tickStart;
    s16 result;
    void *buf;
    s16 i;
    
    for(i=0; i<13; i++) {
        charFxLoadF[i] = 0;
    }
    msmMusStopAll(TRUE, 0);
    msmSeStopAll(TRUE, 0);
    tickStart = OSGetTick();
    SNDGRP_WAIT(tickStart);
    OSReport("CommonGrpSet %d\n", grp);
    if(delGrpF) {
        result = msmSysDelGroupBase(0);
        if(result < 0) {
            OSReport("Del Group Error %d\n", result);
        }
    }
    buf = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(grp));
    msmSysLoadGroupBase(grp, buf);
    HuMemDirectFree(buf);
    sndGroupBak = MSM_GRP_NONE;
}

void HuAudAUXSet(s32 auxA, s32 auxB)
{
    if(auxA == MSM_AUX_NONE) {
        auxA = MSM_AUXA_DEFAULT;
    }
    if(auxB == MSM_AUX_NONE) {
        auxB = MSM_AUXB_DEFAULT;
    }
    msmSysSetAux(auxA, auxB);
}

void HuAudAUXVolSet(s8 volA, s8 volB)
{
    HuAuxAVol = volA;
    HuAuxBVol = volB;
}

static s32 charSndGrpTable[13] = {
    10,
    11,
    12,
    13,
    14,
    16,
    17,
    15,
    10,
    10,
    10,
    10,
    10
};

void HuAudCharGrpSet(s16 ovl)
{
    SNDGRPTBL *sndGrpP;
    OSTick tickStart;
    s16 grpSet;
    s16 result;
    s16 charNo;
    s16 loadNum;
    void *buf;
    s16 i;

    if(ovl != DLL_NONE) {
        sndGrpP = sndGrpTable;
        while(1) {
            if(sndGrpP->ovl == ovl && sndGrpP->grpSet == MSM_GRP_NONE) {
                return;
            }
            if(sndGrpP->ovl == DLL_NONE) {
                break;
            }
            sndGrpP++;
        }
    }
    for(i=loadNum=0; i<GW_PLAYER_MAX; i++) {
        charNo = GwPlayerConf[i].charNo;
        if(charNo < 0 || charNo >= 13 || charNo == 0xFF || charFxLoadF[charNo]) {
            loadNum++;
        }
    }
    if(loadNum < GW_PLAYER_MAX) {
        for(i=0; i<13; i++) {
            charFxLoadF[i] = FALSE;
        }
        msmMusStopAll(TRUE, 0);
        msmSeStopAll(TRUE, 0);
        tickStart = OSGetTick();
        SNDGRP_WAIT(tickStart);
        OSReport("############CharGrpSet\n");
        result = msmSysDelGroupBase(0);
        if(result < 0) {
            OSReport("Del Group Error %d\n", result);
        } else {
            OSReport("Del Group OK\n");
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            charNo = GwPlayerConf[i].charNo;
            if(charNo >= 0 && charNo < 8 && charNo != 0xFF) {
                charFxLoadF[charNo] = TRUE;
                grpSet = charSndGrpTable[charNo];
                buf = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(grpSet));
                msmSysLoadGroupBase(grpSet, buf);
                HuMemDirectFree(buf);
            }
        }
        sndGroupBak = MSM_GRP_NONE;
    }
}

s32 HuAudPlayerFXPlay(s16 playerNo, s16 seId)
{
    s16 charNo = GwPlayerConf[playerNo].charNo;
    return HuAudCharFXPlay(charNo, seId);
}

s32 HuAudPlayerFXPlayPos(s16 playerNo, s16 seId, Vec *pos)
{
    s16 charNo = GwPlayerConf[playerNo].charNo;
    return HuAudCharFXPlayPos(charNo, seId, pos);
}

void HuAudPlayerFXStop(s16 playerNo, s16 seId)
{
    s16 charNo = GwPlayerConf[playerNo].charNo;
    HuAudCharFXStop(charNo, seId);
}

static int charSeIdTable[13] = {
    MSM_SE_MARIO,
    MSM_SE_LUIGI,
    MSM_SE_PEACH,
    MSM_SE_YOSHI,
    MSM_SE_WARIO,
    MSM_SE_DAISY,
    MSM_SE_WALUIGI,
    MSM_SE_KINOPIO,
    MSM_SE_TERESA,
    MSM_SE_MINIKOOPA,
    MSM_SE_MINIKOOPA,
    MSM_SE_MINIKOOPAG,
    MSM_SE_MINIKOOPAB
};

static int charVoiceSeIdTable[13] = {
    MSM_SE_VOICE_MARIO,
    MSM_SE_VOICE_LUIGI,
    MSM_SE_VOICE_PEACH,
    MSM_SE_VOICE_YOSHI,
    MSM_SE_VOICE_WARIO,
    MSM_SE_VOICE_DAISY,
    MSM_SE_VOICE_WALUIGI,
    MSM_SE_VOICE_KINOPIO,
    MSM_SE_VOICE_TERESA,
    MSM_SE_VOICE_MINIKOOPA,
    MSM_SE_VOICE_MINIKOOPA,
    MSM_SE_VOICE_MINIKOOPAG,
    MSM_SE_VOICE_MINIKOOPAB
};

s32 HuAudCharFXPlayVolPan(s16 charNo, s16 seId, s16 vol, s16 pan)
{
    MSM_SEPARAM param;
    if(omSysExitReq || WipeCheck()) {
        return 0;
    }
    if(seId < MSM_SE_VOICE_MARIO) {
        seId -= MSM_SE_MARIO;
        seId += charSeIdTable[charNo];
    } else {
        seId -= MSM_SE_VOICE_MARIO;
        seId += charVoiceSeIdTable[charNo];
    }
    param.flag = MSM_SEPARAM_NONE;
    if(HuAuxAVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLA;
    }
    if(HuAuxBVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLB;
    }
    param.auxAVol = HuAuxAVol;
    param.auxBVol = HuAuxBVol;
    param.flag |= MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
    param.vol = vol;
    param.pan = pan;
    return HuSePlay(seId, &param);
}

s32 HuAudCharFXPlay(s16 charNo, s16 seId)
{
    return HuAudCharFXPlayVolPan(charNo, seId, MSM_VOL_MAX, MSM_PAN_CENTER);
}

s32 HuAudCharFXPlayVol(s16 charNo, s16 seId, s16 vol)
{
    return HuAudCharFXPlayVolPan(charNo, seId, vol, MSM_PAN_CENTER);
}

s32 HuAudCharFXPlayPan(s16 charNo, s16 seId, s16 pan)
{
    return HuAudCharFXPlayVolPan(charNo, seId, MSM_VOL_MAX, pan);
}

s32 HuAudCharFXPlayPos(s16 charNo, s16 seId, Vec *pos)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return 0;
    }
    seId += charSeIdTable[charNo];
    param.flag = MSM_SEPARAM_POS;
    if(HuAuxAVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLA;
    }
    if(HuAuxBVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLB;
    }
    param.auxAVol = HuAuxAVol;
    param.auxBVol = HuAuxBVol;
    param.pos.x = pos->x;
    param.pos.y = pos->y;
    param.pos.z = pos->z;
    return HuSePlay(seId, &param);
}

void HuAudCharFXStop(s16 charNo, s16 seId)
{
    int entrySeNo[MSM_ENTRY_SENO_MAX];
    u16 entryNum;
    u16 i;
    seId += charSeIdTable[charNo];
    entryNum = msmSeGetEntryID(seId, entrySeNo);
    for(i=0; i<entryNum; i++) {
        msmSeStop(entrySeNo[i], 0);
    }
}

static int HuSePlay(int seId, MSM_SEPARAM *param)
{
    int result = msmSePlay(seId, param);
    if(result < 0) {
        OSReport("#########SE Entry Error<SE %d:ErrorNo %d>\n", seId, result);
    }
    return result;
}
