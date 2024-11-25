#ifndef _BOARD_CAMERA_H
#define _BOARD_CAMERA_H

#include "game/hu3d.h"
#include "game/board/model.h"

#define MB_CAMERA_FOCUS_MAX 8

#define MB_CAMERA_CURVE_LINEAR 0
#define MB_CAMERA_CURVE_BEZIER 1

#define MB_CAMERA_FOCUS_MODEL 0
#define MB_CAMERA_FOCUS_MASU 1

#define MB_CAMERA_VIEW_ZOOMIN 0
#define MB_CAMERA_VIEW_ZOOMOUT 1
#define MB_CAMERA_VIEW_WALK 2
#define MB_CAMERA_VIEW_FOCUSALL 3

typedef struct mbCamera_s MBCAMERA;

typedef void (*MBCAMERAPOSHOOK)(MBCAMERA *cameraP);

typedef struct mbCameraViewKey_s {
	HuVecF rot;
	HuVecF pos;
	float zoom;
	HuVecF ofs;
	float fov;
} MBCAMERAVIEWKEY;

typedef struct mbCameraMotion_s {
	int viewNo;
	int curveType;
	s16 time;
	s16 maxTime;
	MBCAMERAVIEWKEY viewKey[3];
} MBCAMERAMOTION;

typedef struct mbCameraFocus_s {
	int type;
	int targetId;
} MBCAMERAFOCUS;

struct mbCamera_s {
	union {
		struct {
			u8 dispOn : 1;
			u8 skipF : 1;
			u8 quake : 1;
		};
		u8 attrAll;
	};
	u16 bit;
	float fov;
	float near;
	float far;
	float aspect;
	float viewportX;
    float viewportY;
    float viewportW;
    float viewportH;
    float viewportNear;
    float viewportFar;
	HuVecF pos;
	HuVecF up;
	HuVecF target;
	HuVecF offset;
	HuVecF rot;
	float zoom;
	float speed;
	int quakeTime;
	float quakePower;
	MBCAMERAPOSHOOK posHook;
	MBCAMERAMOTION motion;
	int focusNum;
	MBCAMERAFOCUS focus[MB_CAMERA_FOCUS_MAX];
};

typedef struct mbView_s {
    s16 rotX;
    s16 zoom;
    s16 fov;
} MBVIEW;


void MBCameraCreate(void);
void MBCameraZoomSet(float zoom);
float MBCameraZoomGet(void);
void MBCameraRotSetV(HuVecF *rot);
void MBCameraRotSet(float rotX, float rotY, float rotZ);
void MBCameraRotGet(HuVecF *rot);
void MBCameraPosTargetSetV(HuVecF *target);
void MBCameraPosTargetSet(float targetX, float targetY, float targetZ);
void MBCameraPosTargetGet(HuVecF *target);
void MBCameraPosSetV(HuVecF *pos);
void MBCameraPosSet(float posX, float posY, float posZ);
void MBCameraPosGet(HuVecF *pos);
void MBCameraOffsetSetV(HuVecF *offset);
void MBCameraOffsetSet(float offsetX, float offsetY, float offsetZ);
void MBCameraOffsetGet(HuVecF *offset);
void MBCameraDirGet(HuVecF *dir);
void MBCameraPosDirGet(HuVecF *pos, HuVecF *dir);
void MBCameraPosHookSet(MBCAMERAPOSHOOK posHook);
void MBCameraBitSet(u16 bit);
void MBCameraFovSet(float fov);
void MBCameraScissorSet(int x, int y, int w, int h);
void MBCameraNearFarSet(float near, float far);
void MBCameraNearFarGet(float *near, float *far);
void MBCameraMotionEnd(void);
MBCAMERA *MBCameraGet(void);
void MBCameraPush(void);
void MBCameraPop(void);
void MBCameraMaxTimeSet(s16 maxTime);
int MBCameraViewNoGet(void);
void MBCameraViewNoSet(int playerNo, int viewNo);
void MBCameraSkipSet(BOOL skipF);
void MBCameraSpeedSet(float speed);
void MBCameraFocusPlayerAdd(int playerNo);
void MBCameraFocusAdd(MBMODELID modelId);
void MBCameraFocusPlayerSet(int playerNo);
void MBCameraFocusSet(MBMODELID modelId);
void MBCameraFocusMasuAdd(int masuId);
void MBCameraFocusMasuSet(int masuId);
void MBCameraPosViewSet(HuVecF *pos, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime);

void MBCameraPlayerViewSet(s16 playerNo, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime);
void MBCameraModelViewSet(MBMODELID modelId, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime);

void MBCameraModelKeySet(MBMODELID modelId, MBCAMERAVIEWKEY *key1, MBCAMERAVIEWKEY *key2, s16 maxTime);
void MBCameraMasuViewSet(s16 masuId, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime);
void MBCameraQuakeSet(int maxTime, float power);
void MBCameraQuakeReset(void);
BOOL MBCameraCullCheck(HuVecF *pos, float radius);
BOOL MBCameraMotionCheck(void);
void MBCameraMotionWait(void);
void MBCameraViewGet(int viewNo, MBVIEW *viewP);
void MBCameraLookAtGet(Mtx lookAt);
void MBCameraLookAtGetInv(Mtx lookAtInv);
void MBCameraFocusPlayerAddAll(void);
void MBCameraPointFocusSet(HuVecF *rot, HuVecF *offset, float fov, s16 maxTime);

#endif