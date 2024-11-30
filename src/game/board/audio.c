#include "game/board/audio.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/hsfex.h"
#include "game/disp.h"

typedef struct MBAudFx_s {
    int seId;
    int seNo;
    int vol;
    BOOL emiterF;
    BOOL pauseF;
    BOOL lockF;
    Vec pos;
    int *fxRef;
} MBAUDFX;

typedef struct MBMus_s {
    int streamNo;
    s16 id;
    s16 closeTime;
    s16 stopTime;
    s16 stopF;
    HUPROCESS *proc;
} MBMUS;

static MBAUDFX audFxWork[MB_AUD_FX_MAX];
static MBMUS musWork[MB_MUS_CHAN_MAX];

static int dummyId = -1;

static BOOL audFxOnF;
static HUPROCESS *audFxProc;

static void MusMainProc(void);
static void MusMainProcKill(void);

static void AudFXMainProc(void);
static void AudFXMainProcKill(void);

void MBAudInit(void)
{
    int i;
    MBAUDFX *audFx = &audFxWork[0];
    for(i=0; i<MB_MUS_CHAN_MAX; i++) {
        musWork[i].streamNo = MSM_STREAMNO_NONE;
        musWork[i].id = MSM_STREAM_NONE;
        musWork[i].closeTime = 0;
        musWork[i].stopTime = 0;
        musWork[i].stopF = FALSE;
        musWork[i].proc = NULL;
    }
    dummyId = -1;
    memset(&audFxWork[0], 0, sizeof(audFxWork));
    for(i=0, audFx = &audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        audFx->seId = MSM_SE_NONE;
        audFx->seNo = MSM_SENO_NONE;
    }
    audFxProc = MBPrcCreate(AudFXMainProc, 8193, 8192);
    MBPrcDestructorSet(audFxProc, AudFXMainProcKill);
    if(_CheckFlag(FLAG_BOARD_TURN_NOSTART)) {
        audFxOnF = TRUE;
    } else {
        audFxOnF = FALSE;
    }
}

void MBMusMainPlay(void)
{
    static const s16 musTbl[MBNO_MAX] = {
        MSM_STREAM_W01,
        MSM_STREAM_W02,
        MSM_STREAM_W03,
        MSM_STREAM_W04,
        MSM_STREAM_W05,
        MSM_STREAM_W06,
        MSM_STREAM_W07,
        MSM_STREAM_W10,
        MSM_STREAM_MGCIRCUIT
    };
    MBMusPlay(MB_MUS_CHAN_BG, musTbl[MBBoardNoGet()], 127, 0);
}

void MBMusPlay(int chan, int id, s8 vol, u16 fadeSpeed)
{
    MBMUS *musP = &musWork[chan];
    int streamNo;
    int chanOther;
    MSM_STREAMPARAM streamParam;
    if(musicOffF || omSysExitReq) {
        return;
    }
    streamParam.flag = MSM_STREAMPARAM_CHAN|MSM_STREAMPARAM_VOL;
    chanOther = chan ^ 1;
    if(musWork[chan].streamNo != MSM_STREAMNO_NONE && id == musWork[chan].id) {
        if(MBMusStatGet(chan) == MSM_STREAM_PLAY || MBMusStatGet(chan) == MSM_STREAM_PAUSEOUT) {
            return;
        }
    }
    if(musWork[chan].streamNo != MSM_STREAMNO_NONE && id == musWork[chan].id && MBMusStopCheck(chan)) {
        MBMusPauseFadeOut(chan, FALSE, 0);
        return;
    }
    if(MBMusCheck(chan)) {
        if(MBMusEndCheck(chan) == FALSE) {
            MBMusFadeOutDefault(chan);
        }
        while(MBMusCheck(chanOther)) {
            HuPrcVSleep();
        }
    }
    if(fadeSpeed) {
        streamParam.flag |= MSM_STREAMPARAM_FADESPEED;
        streamParam.fadeSpeed = fadeSpeed;
    }
    streamParam.chan = chan*2;
    streamParam.vol = vol;
    streamNo = msmStreamPlay(id, &streamParam);
    if(streamNo >= 0) {
        int *chanP;
        musP->streamNo = streamNo;
        musP->id = id;
        musP->closeTime = 3;
        musP->stopTime = 60;
        musP->stopF = FALSE;
        musP->proc = MBPrcCreate(MusMainProc, 8193, 8192);
        musP->proc->property = chanP = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, sizeof(int), HU_MEMNUM_OVL);
        *chanP = chan;
        MBPrcDestructorSet(musP->proc, MusMainProcKill);
    }
}

void MBMusFadeOutDefault(int chan)
{
    MBMUS *musP = &musWork[chan];
    MBMusFadeOut(chan, 1000);
}

void MBMusFadeOut(int chan, u16 speed)
{
    MBMUS *musP = &musWork[chan];
    if(musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    if(MBMusEndCheck(chan)) {
        return;
    }
    HuAudSStreamFadeOut(musP->streamNo, speed);
    musP->stopTime = (int)((speed/1000.0f)*60.0f)+10;
    musP->stopF = TRUE;
}

void MBMusStop(int chan)
{
    MBMUS *musP = &musWork[chan];
    HuAudSStreamStop(musP->streamNo);
}

void MBMusStub(void)
{
    
}

void MBMusParamSet(int chan, s8 vol, u16 fadeSpeed)
{
    MBMUS *musP = &musWork[chan];
    MSM_STREAMPARAM streamParam;
    if(musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    streamParam.flag = MSM_STREAMPARAM_CHAN|MSM_STREAMPARAM_VOL;
    if(fadeSpeed) {
        streamParam.flag |= MSM_STREAMPARAM_FADESPEED;
        streamParam.fadeSpeed = fadeSpeed;
    }
    if(vol < 0) {
        vol = MSM_VOL_MAX;
    }
    streamParam.vol = vol;
    msmStreamSetParam(musP->streamNo, &streamParam);
}

void MBMusPauseFadeOut(int chan, BOOL pauseF, int speed)
{
    MBMUS *musP = &musWork[chan];
    if(musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    if(pauseF) {
        if(MBMusStopCheck(chan)) {
            return;
        }
    } else {
        if(MBMusStopCheck(chan) == FALSE) {
            return;
        }
    }
    HuAudSStreamPauseFadeOut(musP->streamNo, pauseF, 1000);
}

s32 MBMusStatGet(int chan)
{
    MBMUS *musP = &musWork[chan];
    if(musP->streamNo == MSM_STREAMNO_NONE) {
        return MSM_STREAM_DONE;
    }
    return HuAudSStreamStatGet(musP->streamNo);
}

void MBAudClose(void)
{
    int i;
    for(i=0; i<MB_MUS_CHAN_MAX; i++) {
        MBMUS *musP = &musWork[i];
        MBMusFadeOut(i, 1000);
    }
    MBAudFXClose();
}

BOOL MBMusCheck(int chan)
{
    if(musWork[chan].streamNo != MSM_STREAMNO_NONE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MBMusStopCheck(int chan)
{
    if(MBMusCheck(chan) == FALSE) {
        return FALSE;
    } else if(MBMusStatGet(chan) == MSM_STREAM_PAUSEIN) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MBMusEndCheck(int chan)
{
    if(MBMusCheck(chan) == FALSE) {
        return TRUE;
    } else if(MBMusStatGet(chan) == MSM_STREAM_STOP || MBMusStatGet(chan) == MSM_STREAM_DONE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int MBMusPlayJingle(s16 id)
{
    int streamNo = HuAudSStreamChanPlay(id, MB_MUS_CHAN_FG*2);
    return streamNo;
}

s32 MBMusJingleStatGet(int streamNo)
{
    if(streamNo < 0) {
        return MSM_STREAM_DONE;
    }
    return msmStreamGetStatus(streamNo);
}

static void MusMainProc(void)
{
    MBMUS *musP;
    do {
        HUPROCESS *proc = HuPrcCurrentGet();
        int *chanP = proc->property;
        musP = &musWork[*chanP];
        if(MBKillCheck()) {
            if(MBMusEndCheck(*chanP) == FALSE) {
                MBMusStop(*chanP);
            }
            musP->streamNo = MSM_STREAMNO_NONE;
            musP->id = MSM_STREAM_NONE;
            musP->closeTime = 0;
            musP->stopTime = 0;
            musP->stopF = FALSE;
            musP->proc = NULL;
            break;
        } else {
            if(musP->stopF) {
                if(--musP->stopTime < 0) {
                    MBMusStop(*chanP);
                    musP->streamNo = MSM_STREAMNO_NONE;
                    musP->id = MSM_STREAM_NONE;
                    musP->closeTime = 0;
                    musP->stopTime = 0;
                    musP->stopF = FALSE;
                    musP->proc = NULL;
                    break;
                }
            }
            switch(MBMusStatGet(*chanP)) {
                case MSM_STREAM_DVDERROR:
                    MBMusStop(*chanP);
                    musP->closeTime = 0;
                    
                case MSM_STREAM_DONE:
                    if(--musP->closeTime <= 0) {
                        musP->streamNo = MSM_STREAMNO_NONE;
                        musP->id = MSM_STREAM_NONE;
                        musP->closeTime = 0;
                        musP->stopTime = 0;
                        musP->stopF = FALSE;
                        musP->proc = NULL;
                    }
                    break;
            }
            HuPrcVSleep();
            if(musP == NULL) {
                break;
            }
        }
    } while(musP->streamNo != MSM_STREAMNO_NONE);
    HuPrcEnd();
}

static void MusMainProcKill(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();
    if(proc->property) {
        HuMemDirectFree(proc->property);
        proc->property = NULL;
    }
}

void MBAudStub1(void)
{
    
}

void MBAudStub2(void)
{
    
}

int MBAudFXSet(int seId)
{
    return MBAudFXCreate(seId, TRUE);
}

int MBAudFXCreate(int seId, BOOL multiF)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    if(multiF == FALSE) {
        for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
            if(audFx->seId == seId) {
                return MSM_SENO_NONE;
            }
        }
    }
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seId == MSM_SENO_NONE) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return MSM_SENO_NONE;
    }
    if(audFxOnF) {
        audFx->seNo = MBAUD_FXNO_OFF;
        audFx->seId = seId;
        audFx->vol = MSM_VOL_MAX;
        audFx->pauseF = FALSE;
        audFx->emiterF = FALSE;
        audFx->lockF = TRUE;
        audFx->fxRef = NULL;
        return MBAUD_FXNO_OFF;
    } else {
        audFx->seNo = HuAudFXPlay(seId);
        if(audFx->seNo < 0) {
            audFx->seNo = audFx->seId = MSM_SE_NONE;
            return MSM_SENO_NONE;
        } else {
            audFx->seId = seId;
            audFx->vol = MSM_VOL_MAX;
            audFx->pauseF = FALSE;
            audFx->emiterF = FALSE;
            audFx->lockF = FALSE;
            audFx->fxRef = NULL;
            return audFx->seNo;
        }
    }
}

void MBAudFXVolSet(int seNo, s16 vol)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seNo == seNo) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    audFx->vol = vol;
    HuAudFXVolSet(audFx->seNo, audFx->vol);
}

void MBAudFXKill(int seNo)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seNo == seNo) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    audFx->seId = MSM_SE_NONE;
    audFx->seNo = MSM_SENO_NONE;
    HuAudFXFadeOut(seNo, 1000);
}

void MBAudFXPauseSet(int seNo, BOOL pauseF)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seNo == seNo) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    if(pauseF == FALSE) {
        if(audFx->pauseF != FALSE) {
            (void)i;
        } else {
            (void)i;
            return;
        }
        audFx->pauseF = FALSE;
        msmSePause(audFx->seNo, FALSE, 1000);
    } else {
        if(audFx->pauseF != FALSE) {
            (void)audFx;
        } else {
            audFx->pauseF = TRUE;
            msmSePause(audFx->seNo, TRUE, 1000);
            return;
        }
    }
}

void MBAudFXClose(void)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seNo != MSM_SENO_NONE) {
            MBAudFXKill(audFx->seNo);
        }
    }
}

int MBAudFXEmiterSet(int seId, Vec *pos)
{
    return MBAudFXEmiterCreate(seId, pos, TRUE);
}

int MBAudFXEmiterCreate(int seId, Vec *pos, BOOL multiF)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    if(multiF == FALSE) {
        for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
            if(audFx->seId == seId) {
                return MSM_SENO_NONE;
            }
        }
    }
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seId == MSM_SENO_NONE) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return MSM_SENO_NONE;
    }
    if(audFxOnF) {
        audFx->seNo = MBAUD_FXNO_OFF;
        audFx->seId = seId;
        audFx->vol = MSM_VOL_MAX;
        audFx->pauseF = FALSE;
        audFx->emiterF = TRUE;
        audFx->lockF = TRUE;
        audFx->fxRef = NULL;
        return MBAUD_FXNO_OFF;
    } else {
        audFx->seNo = HuAudFXEmiterPlay(seId, pos);
        if(audFx->seNo < 0) {
            audFx->seNo = MSM_SENO_NONE;
            return MSM_SENO_NONE;
        } else {
            audFx->seId = seId;
            audFx->vol = MSM_VOL_MAX;
            audFx->pauseF = FALSE;
            audFx->emiterF = TRUE;
            audFx->pos = *pos;
            audFx->lockF = FALSE;
            audFx->fxRef = NULL;
            return audFx->seNo;
        }
    }
}

void MBAudFXEmiterUpdate(int seNo, Vec *pos)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seNo == seNo && audFx->emiterF) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    audFx->pos = *pos;
    HuAudFXEmiterUpDate(audFx->seNo, &audFx->pos);
}

void MBAudFXEmiterPosSet(int seId, Vec *pos)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seId == seId && audFx->emiterF && audFx->lockF) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    audFx->pos = *pos;
    
}

void MBAudFXRefSet(int seId, int *fxRef)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
        if(audFx->seId == seId && audFx->lockF) {
            break;
        }
    }
    if(i >= MB_AUD_FX_MAX) {
        return;
    }
    audFx->fxRef = fxRef;
}


static void AudFXMainProc(void)
{
    MBAUDFX *audFx = &audFxWork[0];
    int i;
    do {
        for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
            if(audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE && audFx->lockF == FALSE && HuAudFXStatusGet(audFx->seNo) == MSM_SE_DONE) {
                audFx->seId = MSM_SE_NONE;
                audFx->seNo = MSM_SENO_NONE;
            }
        }
        if(!_CheckFlag(FLAG_BOARD_TURN_NOSTART)) {
            if(audFxOnF) {
                for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
                    if(audFx->lockF && audFx->seNo == MBAUD_FXNO_OFF) {
                        if(audFx->emiterF) {
                            audFx->seNo = HuAudFXEmiterPlay(audFx->seId, &audFx->pos);
                            if(audFx->seNo < 0) {
                                audFx->seNo = MSM_SENO_NONE;
                                audFx->seId = MSM_SE_NONE;
                            }
                        } else {
                            audFx->seNo = HuAudFXPlay(audFx->seId);
                            if(audFx->seNo < 0) {
                                audFx->seNo = MSM_SENO_NONE;
                                audFx->seId = MSM_SE_NONE;
                            }
                        }
                        if(audFx->fxRef != NULL) {
                            *audFx->fxRef = audFx->seNo;
                        }
                        audFx->lockF = FALSE;
                    }
                    if(audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE) {
                        MBAudFXPauseSet(audFx->seNo, FALSE);
                    }
                }
            }
            audFxOnF = FALSE;
        } else {
            if(audFxOnF == FALSE) {
                for(i=0, audFx=&audFxWork[0]; i<MB_AUD_FX_MAX; i++, audFx++) {
                    if(audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE) {
                        MBAudFXPauseSet(audFx->seNo, TRUE);
                    }
                }
            }
            audFxOnF = TRUE;
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i)) {
                Vec pos;
                int pan;
                MBPlayerPosGet(i, &pos);
                pan = MBAudFXPosPanGet(&pos);
                CharModelVoicePanSet(GwPlayer[i].charNo, pan);
            }
        }
        HuPrcVSleep();
    } while(!MBKillCheck());
    HuPrcEnd();
}

static void AudFXMainProcKill(void)
{
    MBAudFXClose();
    audFxProc = NULL;
}

int MBAudFXPlay(s16 seId)
{
    return HuAudFXPlay(seId);
}

void MBAudFXStop(int seNo)
{
    HuAudFXStop(seNo);
}

int MBAudFXPosPlay(s16 seNo, Vec *pos)
{
    int pan = MBAudFXPosPanGet(pos);
    HuAudFXPlayPan(seNo, pan);
}

u8 MBAudFXPosPanGet(Vec *pos)
{
    HuVecF pos2D;
    int pan;
    Hu3D3Dto2D(pos, HU3D_CAM0, &pos2D);
    pan = (int)((pos2D.x*(1.0f/HU_DISP_WIDTH))*(MSM_PAN_RIGHT-MSM_PAN_LEFT))+MSM_PAN_LEFT;
    if(pan < MSM_PAN_LEFT) {
        pan = MSM_PAN_LEFT;
    } else if(pan > MSM_PAN_RIGHT) {
        pan = MSM_PAN_RIGHT;
    }
    return pan;
}