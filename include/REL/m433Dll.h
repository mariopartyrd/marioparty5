#ifndef _M433DLL_H
#define _M433DLL_H

#include "game/audio.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/sprite.h"

#include "datanum/m433.h"


#define M433_MODE_INIT 0
#define M433_MODE_GAMESTART 1
#define M433_MODE_GAMEWIN 2
#define M433_MODE_EXITAUTO 3
#define M433_MODE_EXITREQ 4
#define M433_MODE_EXIT 5

#define M433_GAMEMES_INIT 0
#define M433_GAMEMES_START 1
#define M433_GAMEMES_WAIT 2
#define M433_GAMEMES_FINISH 3
#define M433_GAMEMES_CLOSE 4

#define M433_WINMODE_NONE 0
#define M433_WINMODE_NORMAL 1
#define M433_WINMODE_NOSTOP 2

#define M433_WINTYPE_NORMAL 1
#define M433_WINTYPE_NONE 2

#define M433_CAMERAMODE_NORMAL 0
#define M433_CAMERAMODE_START 1
#define M433_CAMERAMODE_GAME 2

#define M433_GUIDEMODE_IDLE 0
#define M433_GUIDEMODE_READY 1
#define M433_GUIDEMODE_JUMP 2
#define M433_GUIDEMODE_RUN 3
#define M433_GUIDEMODE_FACE_CENTER 4
#define M433_GUIDEMODE_FACE_RIGHT 5
#define M433_GUIDEMODE_FACE_LEFT 6


#define M433StageRandF() (M433StageRandom()/65536.0f)
#define M433StageSRandF() ((M433StageRandom()-0x8000)/32768.0f)

extern int gameMaxScore;
extern int gameBallNumMax;
extern int gameType;

typedef struct M433GameWork_s {
    unsigned int mode;
    int modeTime;
    BOOL modeActive;
    int cameraMode;
    int startTime;
    int gamemesMode;
    int gamemesTimer;
    unsigned int second;
    unsigned int frame;
    int winMode;
    unsigned int winTimer;
    int winPlayer[2];
    unsigned int winType;
    unsigned int winFlag;
    unsigned int winFlag2;
} M433_GAMEWORK;

void M433StageInit(OMOBJMAN *om);
void M433StageClose(void);
OMOBJFUNC M433StartCameraCreate(OMOBJMAN *om, OMOBJ *obj);
int M433StageRandom(void);
void M433StageCameraFocusSet(float time, Vec *pos, Vec *rot, float zoom);
void M433StageCameraSpeedSet(float speed);
void M433StageCameraOfsSet(u8 no, u16 time, float radius, float speed, float rotSpeed);
void M433StageRotMtxCalc(Mtx matrix, float x, float y, float z);
void M433StageMtxRotGet(Mtx matrix, HuVecF *result);
void M433GuideModeSet(s16 mode);

void M433PlayerInit(OMOBJMAN *om);
void M433PlayerClose(void);
BOOL M433WinGroupCheck(int side);
unsigned int M433GameWinGet(int side);
unsigned int M433EventGet(int side);
BOOL M433ServeGroupCheck(int side);
unsigned int M433ServeCelGet(int side);
BOOL M433PointGroupCheck(int side);
float M433PlaneDistGet(HuVecF *pos1, HuVecF *point1, HuVecF *pos2, HuVecF *point2);
unsigned int M433GameEventGet(void);
unsigned int M433ServeSideGet(HuVecF *pos);
unsigned int M433ContactCountGet(void);
BOOL M433ContactCheck(unsigned int side, unsigned int cel);
BOOL M433ContactGroupCheck(unsigned int side);

BOOL M433BallCoordGet(HuVecF *pos);
unsigned int M433BallSideGet(HuVecF *pos);
float M433BallPointCheck(float radius, HuVecF *pos);
BOOL M433BallJumpCheck(HuVecF *pos);
unsigned int M433BallTimeGet(void);
unsigned int M433BallAirTimeGet(void);
void M433SandEffectAdd(HuVecF *pos, float speed);
void M433DustEffectAdd(HuVecF *pos);
float M433ServeTargetTimeGet(void);
void M433BallContactSet(HuVecF *pos, unsigned int type, unsigned int side, unsigned int cel);
void M433BallServeSet(float angle, float mag, unsigned int side, unsigned int scaleF, HuVecF *pos);
void M433BallTrailStart(void);
float M433BallTrailGet(void);
void M433StageCollide(HuVecF *pos, unsigned int side);
BOOL M433BallSlipCheck(float radius, HuVecF *pos);
void M433BallExplodeSet(void);

unsigned int M433GameTimerGet(void);

void M433ModeWatch(OMOBJ *obj);
void M433ModeUpdate(OMOBJ *obj);
OMOBJFUNC M433GameWinCreate(OMOBJMAN *om, OMOBJ *obj);
unsigned int M433GameModeGet(void);
unsigned int M433CameraModeGet(void);
unsigned int M433GamemesModeGet(void);
unsigned int M433GameWinModeGet(void);
void M433GameWinnerSet(int type, int player1, int player2);
int M433GameWinTypeGet(void);
void M433GameWinFlagSet(int playerNo);
void M433GameWinFlag2Set(int playerNo);
BOOL M433GameFinishEndCheck(void);
void M433CameraAimSetV(HuVecF *eye, HuVecF *pos);
void M433CameraAimSet(float eyeX, float eyeY, float eyeZ, float posX, float posY, float posZ);
float M433AngleLerp(float start, float end, float time);
float M433AngleSub(float start, float end);


#endif