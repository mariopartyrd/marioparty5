#ifndef _GAME_AUDIO_H
#define _GAME_AUDIO_H

#include "msm.h"
#include "msm_grp.h"
#include "msm_se.h"
#include "msm_stream.h"

typedef struct sndGrpTbl_s {
    s16 ovl;
    s16 grpSet;
    s32 auxANo;
    s32 auxBNo;
    s8 auxAVol;
    s8 auxBVol;
} SNDGRPTBL;

void HuAudInit(void);
void HuAudAllStop(void);
void HuAudFadeOut(s32 speed);

int HuAudFXPlay(int seId);
int HuAudFXPlayVol(int seId, s16 vol);
int HuAudFXPlayPan(int seId, s16 pan);
int HuAudFXPlayVolPan(int seId, s16 vol, s16 pan);
void HuAudFXStop(int seNo);
void HuAudFXAllStop(void);
void HuAudFXFadeOut(int seNo, s32 speed);
void HuAudFXPanning(int seNo, s16 pan);
void HuAudFXListnerSet(Vec *pos, Vec *heading, float sndDist, float sndSpeed);
void HuAudFXListnerSetEX(Vec *pos, Vec *heading, float sndDist, float sndSpeed, float startDis, float frontSurDis, float backSurDis);
void HuAudFXListnerUpdate(Vec *pos, Vec *heading);
int HuAudFXEmiterPlay(int seId, Vec *pos);
void HuAudFXEmiterUpDate(int seNo, Vec *pos);
void HuAudFXListnerKill(void);
void HuAudFXPauseAll(BOOL pauseF);
s32 HuAudFXStatusGet(int seNo);
s32 HuAudFXPitchSet(int seNo, s16 pitch);
s32 HuAudFXVolSet(int seNo, s16 vol);

int HuAudSeqPlay(s16 musId);
void HuAudSeqStop(int musNo);
void HuAudSeqFadeOut(int musNo, s32 speed);
void HuAudSeqAllFadeOut(s32 speed);
void HuAudSeqAllStop(void);
void HuAudSeqPauseAll(BOOL pause);
s32 HuAudSeqMidiCtrlGet(int musNo, s8 channel, s8 ctrl);


int HuAudSStreamChanPlay(s16 streamId, s16 chanNo);
int HuAudSStreamPlay(s16 streamId);
int HuAudBGMPlay(s16 streamId);
int HuAudJinglePlay(s16 streamId);
void HuAudSStreamStop(int streamNo);
void HuAudSStreamFadeOut(int streamNo, s32 speed);
void HuAudSStreamAllFadeOut(s32 speed);
void HuAudSStreamPause(s16 streamNo, BOOL pause);
void HuAudSStreamPauseFadeOut(s16 streamNo, BOOL pause, s32 speed);
void HuAudSStreamPauseAll(BOOL pause);
void HuAudSStreamAllStop(void);
s32 HuAudSStreamStatGet(int streamNo);
void HuAudSStreamVolSet(int streamNo, u8 vol, u32 speed);


void HuAudDllSndGrpSet(u16 ovl);
void HuAudSndGrpSetSet(s16 grpSet);
void HuAudSndGrpSet(s16 grp);
void HuAudAUXSet(s32 auxA, s32 auxB);
void HuAudAUXVolSet(s8 volA, s8 volB);

s32 PlayerFXPlay(s16 playerNo, s16 seId);
s32 PlayerFXPlayPos(s16 playerNo, s16 seId, Vec *pos);
void PlayerFXStop(s16 playerNo, s16 seId);

s32 CharFXPlayVolPan(s16 charNo, s16 seId, s16 vol, s16 pan);
s32 CharFXPlay(s16 charNo, s16 seId);
s32 CharFXPlayVol(s16 charNo, s16 seId, s16 vol);
s32 CharFXPlayPan(s16 charNo, s16 seId, s16 pan);
s32 CharFXPlayPos(s16 charNo, s16 seId, Vec *pos);
void CharFXStop(s16 charNo, s16 seId);

extern SNDGRPTBL sndGrpTable[];

extern float Snd3DBackSurDisOffset;
extern float Snd3DFrontSurDisOffset;
extern float Snd3DStartDisOffset;
extern float Snd3DSpeedOffset;
extern float Snd3DDistOffset;
extern BOOL musicOffF;
extern u8 fadeStat;

#endif
