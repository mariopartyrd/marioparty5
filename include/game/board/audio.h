#ifndef _BOARD_AUDIO_H
#define _BOARD_AUDIO_H

#include "game/audio.h"

#define MB_AUD_FX_MAX 16
#define MBAUD_FXNO_OFF -99

#define MB_MUS_CHAN_BG 0
#define MB_MUS_CHAN_FG 1
#define MB_MUS_CHAN_MAX 2

void MBAudInit(void);
void MBMusMainPlay(void);
void MBMusPlay(int chan, int id, s8 vol, u16 fadeSpeed);
s32 MBMusStatGet(int chan);
void MBMusFadeOutDefault(int chan);
void MBMusFadeOut(int chan, u16 speed);
void MBMusStop(int chan);
void MBMusParamSet(int chan, s8 vol, u16 fadeSpeed);
void MBMusPauseFadeOut(int chan, BOOL pauseF, int speed);
void MBAudClose(void);

BOOL MBMusCheck(int chan);
BOOL MBMusStopCheck(int chan);
BOOL MBMusEndCheck(int chan);
int MBMusPlayJingle(s16 id);
s32 MBMusJingleStatGet(int streamNo);
int MBAudFXSet(int seId);
int MBAudFXCreate(int seId, BOOL multiF);
void MBAudFXVolSet(int seNo, s16 vol);
void MBAudFXKill(int seNo);
void MBAudFXClose(void);
int MBAudFXEmiterSet(int seId, Vec *pos);
int MBAudFXEmiterCreate(int seId, Vec *pos, BOOL multiF);
void MBAudFXEmiterUpdate(int seNo, Vec *pos);
void MBAudFXEmiterPosSet(int seId, Vec *pos);
void MBAudFXRefSet(int seId, int *fxRef);

int MBAudFXPlay(s16 seId);
void MBAudFXStop(int seNo);
int MBAudFXPosPlay(s16 seNo, Vec *pos);
u8 MBAudFXPosPanGet(Vec *pos);

#endif