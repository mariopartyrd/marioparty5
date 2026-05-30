#ifndef _M506DLL_H
#define _M506DLL_H

#include "game/object.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/wipe.h"
#include "game/audio.h"

#include "game/mg/timer.h"

#include "datanum/m506.h"

#define M506_JUMP_MAX 60
#define M506_PLAYER_MAX GW_PLAYER_MAX
#define M506_VINE_MAX ((M506_JUMP_MAX/2)+1)
#define M506_DECOR_MAX (M506_JUMP_MAX/6)

typedef struct M506Work_s {
    HU3DMODELID stageMdlId; //0x00
    HU3DMODELID bgMdlId[2]; //0x02
    HU3DMODELID mameMdlId[M506_PLAYER_MAX][M506_JUMP_MAX]; //0x06
    HU3DMODELID mameVineMdlId[M506_PLAYER_MAX][M506_VINE_MAX]; //0x1E6
    HU3DMODELID mameSmallMdlId[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x2DE
    HU3DMODELID mameSpiralMdlId[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x32E
    HU3DMODELID kumoMdlId[4]; //0x37E
    HU3DMODELID auroraMdlId; //0x386
    HU3DMODELID seedMdlId; //0x388
    HU3DMODELID moundMdlId; //0x38A
    HU3DMODELID windmillMdlId; //0x38C
    HU3DMODELID starMdlId[2]; //0x38E
    HU3DMODELID rocketMdlId; //0x392
    HU3DMODELID moonMdlId; //0x394
    HU3DMODELID noko2MdlId; //0x396
    HU3DMODELID starfighterMdlId[M506_PLAYER_MAX]; //0x398
    HU3DMODELID stageBorderMdlId; //0x3A0
    HU3DMODELID sunMdlId; //0x3A2
    HU3DMODELID kumo2MdlId[M506_PLAYER_MAX]; //0x3A4
    HU3DMODELID smokeMdlId[3]; //0x3AC
    HU3DMODELID mameShadowMdlId[M506_PLAYER_MAX]; //0x3B2
    HU3DMODELID glowShadowMdlId; //0x3BA
    HU3DMODELID mameVineShadowMdlId; //0x3BC
    HU3DMODELID unk3BE;
    HU3DMOTID moundMotId; //0x3C0
    HU3DMOTID windmillMotId; //0x3C2
    HU3DTEXSCRID mameShadowTexScrId[M506_PLAYER_MAX]; //0x3C4
    u8 mameSide[M506_PLAYER_MAX][M506_JUMP_MAX]; //0x3CC
    u8 mameSmallFlag[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x4BC
    u8 mameSpiralFlag[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x4E4
    float mameVinePosY[M506_PLAYER_MAX]; //0x50C
    float mameVineRotY[M506_PLAYER_MAX]; //0x51C
    s16 mameVineTime[M506_PLAYER_MAX]; //0x52C
    s16 mameEnterTime[M506_PLAYER_MAX]; //0x534
    s16 unk53C[M506_PLAYER_MAX]; //0x534
    s16 mameMissTime[M506_PLAYER_MAX]; //0x544
    float mameSpiralPosY[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x54C
    float mameSmallPosY[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x5EC
    s16 mameSpiralTime[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x68C
    s16 mameSmallTime[M506_PLAYER_MAX][M506_DECOR_MAX]; //0x6DC
    float kumoDir[4]; //0x72C
    float kumoPosX[4]; //0x73C
    float kumoAngle[4]; //0x74C
    s16 starTime[2]; //0x75C
    float noko2Time; //0x760
    float noko2Angle; //0x764
    u16 noko2CamBit; //0x768
    u16 rocketCamBit; //0x76A
    u32 recordIdx; //0x76C
    MGTIMER *timer; //0x770
    GAMEMESID gameMesId; //0x774
    u32 recordVal; //0x778
    int gameMesDelay; //0x77C
} M506_WORK;

typedef struct M506PlayerWork_s {
    int no; //0x00
    int charNo; //0x04
    int padNo; //0x08
    int unkC;
    int unk10;
    int newHeight; //0x14
    int height; //0x18
    int prevHeight; //0x1C
    HuVecF pos; //0x20
    int jumpTicks; //0x2C
    s16 jumpF; //0x30
    s16 upJumpF; //0x32
    s16 nextJumpF; //0x34
    s16 curDir; //0x36
    s16 prevDir; //0x38
    s16 fallTime; //0x3A
    s16 prevFallTime; //0x3C
    s16 fallLevel; //0x3E
    s16 mode; //0x40
    s16 modeTimer; //0x42
    s16 jumpTimer; //0x44
    s16 motId; //0x46
    float motTime; //0x48
    float motMaxTime; //0x4C
    s16 playerType; //0x50
    s16 comDif; //0x52
    s16 comMode; //0x54
    s16 comDelayMax; //0x56
    s16 comDelay; //0x58
    s16 comRecoverF; //0x5A
    s16 comMoveChance; //0x5C
    s16 comMoveChanceMax; //0x5E
} M506_PLAYER_WORK;

typedef struct M506YCamera_s {
    int bit; //0x00
    HuVecF center; //0x04
    HuVecF target; //0x10
    HuVecF upDir; //0x1C
    HuVecF up; //0x28
    HuVecF dir; //0x34
    HuVecF hvr; //0x40
    float unk4C;
    HuVecF pos; //0x50
    float aspect; //0x5C
    float viewportX; //0x60
    float viewportW; //0x64
    float scissorX; //0x68
    float scissorW; //0x6C
    float fov; //0x70
} M506_YCAMERA;

typedef struct M506YCameraWork_s {
    M506_YCAMERA camera[M506_PLAYER_MAX]; //0x00
    float splitTime; //0x1D0
    BOOL unk1D4;
    s16 splitSprId[M506_PLAYER_MAX]; //0x1D8
} M506_YCAMERA_WORK;

typedef struct M506YLight_s {
    HU3DLIGHTID lightId; //0x00
    HuVecF hvr; //0x04
    HuVecF unk10; 
    HuVecF target; //0x1C
    HuVecF center; //0x28
    HuVecF pos; //0x34
    HuVecF dir; //0x40
    GXColor color; //0x4C
} M506_YLIGHT;

typedef struct M506YShadow_s {
    int bit; //0x00
    float tpLvl; //0x04
    HuVecF hvr; //0x08
    HuVecF center; //0x14
    HuVecF pos; //0x20
    HuVecF target; //0x2C
    HuVecF upDir; //0x38
    HuVecF up; //0x44
    HuVecF forward; //0x50
} M506_YSHADOW;

typedef struct M506YShadowWork_s {
    M506_YSHADOW shadow[M506_PLAYER_MAX]; //0x00
    u8 unk170[0x58];
} M506_YSHADOW_WORK;

extern OMOBJMAN *M506ObjMan;
extern OMOBJ *M506MainObj;
extern OMOBJ **M506PlayerObj;
extern OMOBJ *M506UpdateObj;
extern const s16 M506CameraBit[M506_PLAYER_MAX];
extern M506_YCAMERA_WORK *M506YCamera;
extern M506_YLIGHT *M506YLight;
extern M506_YSHADOW_WORK *M506YShadow;

void M506YCameraInit(void);
void M506YLightInit(void);
void M506YShadowInit(void);
void M506YCameraClose(void);
void M506YLightClose(void);
void M506YShadowClose(void);
void M506YCameraUpdate(void);
void M506YShadowUpdate(void);

void M506RocketUpdate(void);
void M506StarfighterUpdate(void);

void M506MainCreate(OMOBJ *obj);
void M506PlayerCreate(OMOBJ *obj);
void M506Exit(OMOBJ *obj);
BOOL M506ExitCheck(OMOBJ *obj);
void M506ModelUpdate(M506_WORK *m506Work);
void M506Create(M506_WORK *m506Work);
BOOL M506FlowUpdate(M506_WORK *work);
void M506MameVineScrollUp(M506_PLAYER_WORK *player);
void M506MameVineRotate(M506_PLAYER_WORK *player);
void M506MameSpiralIn(M506_PLAYER_WORK *player);
void M506MameSmallIn(M506_PLAYER_WORK *player);
void M506KumoUpdate(M506_WORK *work);
void M506StarUpdate(M506_WORK *work);
void M506Noko2Update(M506_WORK *work);
void M506MameUpdate(M506_PLAYER_WORK *player);
void M506MameModeEnter(M506_PLAYER_WORK *player);
void M506MameModeMiss(M506_PLAYER_WORK *player);
void M506MameModeJump(M506_PLAYER_WORK *player);

#endif
