#include "dolphin.h"
#include "game/flag.h"
#include "game/dvd.h"
#include "game/pad.h"
#include "game/audio.h"
#include "game/thpmain.h"
#include "game/init.h"
#include "game/main.h"
#include "game/sreset.h"

#define SR_DVD_LOADING 0
#define SR_DVD_COVER_OPEN 1
#define SR_DVD_NO_DISK 2
#define SR_DVD_WRONG_DISK 3
#define SR_DVD_RETRY_ERROR 4
#define SR_DVD_FATAL_ERROR 5

#define PAD_BTN_SRESET (PAD_BUTTON_START|PAD_BUTTON_X|PAD_BUTTON_B)

static s32 SR_PreRstChk[4] = {};

#include "coveropen_en.inc"
#include "fatalerror_en.inc"
#include "loading_en.inc"
#include "nodisc_en.inc"
#include "retryerror_en.inc"
#include "wrongdisc_en.inc"

static s16 SR_PushTime[4] = {};
static s8 SR_ResetPad = -1;

static s16 XfbW;
static s16 XfbH;
static BOOL XfbProg;
static void *Xfb[2] = {};
static BOOL trychkBusyWait;
BOOL SR_ExecReset;
BOOL SR_ExecResetMenu;
static BOOL SR_RestartChk;
static BOOL H_ResetReady;

static void HuSoftResetPostProc(void);

BOOL HuSoftResetButtonCheck(void)
{
	if(SR_ExecReset) {
		HuRestartSystem();
	}
	return (SR_ExecReset) ? TRUE : FALSE;
}

static OSMessageQueue ToeMessageQueue;

void HuDvdErrDispIntFunc(u32 retraceCount)
{
	OSWakeupThread(&ToeMessageQueue.queueSend);
}

static void HuPreRstChk(void)
{
	static PADStatus padStat[4];
	int i;
	PADRead(padStat);
	for(i=0; i<4; i++) {
		PADStatus *status = &padStat[i];
		if(status->err != 0) { 
			continue;
		}
		if(status->button == PAD_BTN_SRESET) {
			SR_PreRstChk[i] = 1;
		} else {
			SR_PreRstChk[i] = 0;
		}
	}
}

static OSMessage ToeMessageArray[16];
static OSThread ToeThread;
static u8 ToeThreadStack[4096];

static void *ToeThreadFunc(void *param);
static void ToeDispCheck(void);

void HuDvdErrDispInit(GXRenderModeObj *rmode, void *xfb1, void *xfb2)
{
	BOOL intrOld;
	HuSRDisableF = FALSE;
	SR_ResetPad = -1;
	SR_ExecReset = H_ResetReady = 0;
	SR_RestartChk = 0;
	SR_PushTime[0] = SR_PushTime[1] = SR_PushTime[2] = SR_PushTime[3] = 0;
	VIWaitForRetrace();
	VIWaitForRetrace();
	VIWaitForRetrace();
	HuPreRstChk();
	HuDvdErrWait = FALSE;
	Xfb[0] = xfb1;
	Xfb[1] = xfb2;
	if(rmode) {
		XfbW = (u16)(((u16)rmode->fbWidth+15) & ~0xF);
		XfbH = rmode->xfbHeight;
	} else {
		XfbW = 640;
		XfbH = 480;
	}
	if((u16)rmode->xFBmode == VI_XFBMODE_SF) {
		XfbProg = FALSE;
	} else {
		XfbProg = TRUE;
	}
	trychkBusyWait = FALSE;
	OSInitMessageQueue(&ToeMessageQueue, ToeMessageArray, 16);
	OSCreateThread(&ToeThread, ToeThreadFunc, NULL, &ToeThreadStack[4096], 4096, 8, OS_THREAD_ATTR_DETACH);
	OSResumeThread(&ToeThread);
	intrOld = OSDisableInterrupts();
	VISetPreRetraceCallback(HuDvdErrDispIntFunc);
	OSRestoreInterrupts(intrOld);
}

static void *ToeThreadFunc(void *param)
{
    BOOL execPost = FALSE;
	while(1) {
		BOOL checkDisp;
        BOOL reset;
		OSSleepThread(&ToeMessageQueue.queueSend);
		if(!HuSoftResetCheck()) {
            if(SR_ExecReset) {
                reset = TRUE;
            } else {
                if(H_ResetReady == TRUE && OSGetResetButtonState() != TRUE) {
                    reset = TRUE;
                } else {
                    if(H_ResetReady != TRUE && OSGetResetButtonState() == TRUE) {
                        H_ResetReady = TRUE;
                    }
                    reset = FALSE;
                }
            }
            if(reset || SR_ExecResetMenu) {
                proc_reset:
                execPost = TRUE;
            }
        }  else {
            goto proc_reset;
        }
        if(execPost && HuSRDisableF == FALSE) {
            HuSoftResetPostProc();
        }
		if(SR_ExecReset) {
			HuRestartSystem();
		}
		if(SR_ExecReset) {
			checkDisp = TRUE;
		} else {
			checkDisp = FALSE;
		}
		if(!checkDisp) {
			ToeDispCheck();
		}
	}
}

static void _HuDvdErrDispXFB(s32 error);

static void ToeDispCheck(void)
{
	s32 status;
	if(SR_ResetPad != -1 || SR_ExecReset != FALSE || SR_RestartChk != 0) {
		return;
	}
	status = DVDGetDriveStatus();
	switch(status) {
		case DVD_STATE_FATAL_ERROR:
			status = SR_DVD_FATAL_ERROR;
			trychkBusyWait = TRUE;
            HuSRDisableF = TRUE;
			break;
			
		case DVD_STATE_END:
			HuDvdErrWait = FALSE;
			trychkBusyWait = FALSE;
			return;
			
		case DVD_STATE_COVER_OPEN:
			status = SR_DVD_COVER_OPEN;
			trychkBusyWait = TRUE;
			break;
			
		case DVD_STATE_BUSY:
		case DVD_STATE_COVER_CLOSED:
			if(!trychkBusyWait) {
				return;
			}
			status = SR_DVD_LOADING;
			break;
			
		case DVD_STATE_NO_DISK:
			status = SR_DVD_NO_DISK;
			trychkBusyWait = TRUE;
			break;
			
		case DVD_STATE_WRONG_DISK:
			status = SR_DVD_WRONG_DISK;
			trychkBusyWait = TRUE;
			break;
			
		case DVD_STATE_RETRY:
			status = SR_DVD_RETRY_ERROR;
			trychkBusyWait = TRUE;
			break;
			
		default:
			return;
	}
	HuDvdErrWait = TRUE;
	HuPadRumbleAllStop();
	VISetBlack(FALSE);
	VIFlush();

	_HuDvdErrDispXFB(status);
}

static void DvdErrDispXFB(void *data);

static void _HuDvdErrDispXFB(s32 error)
{
	static void *bmpMes[][6] = {
		loading_en, coveropen_en, nodisc_en, wrongdisc_en, retryerror_en, fatalerror_en
	};
    s16 i;
    s32 driveStatus;
    PADStatus padStat[4];
    OSTick resetTick[4];
    u8 resetCancelF[4];
    u32 *xfb1;
    u32 *xfb2;
	BOOL resetPadF = FALSE;
    s8 languageNo = 0;
    DvdErrDispXFB(bmpMes[languageNo][error]);
    for(i=0; i<4; i++) {
        resetCancelF[i] = FALSE;
    }
    resetPadF = FALSE;
    driveStatus = DVDGetDriveStatus();
    while(driveStatus != 0) {
        
        if(driveStatus != DVDGetDriveStatus()) {
            break;
        }
        PADRead(padStat);
        for(i=0; i<4; i++) {
            PADStatus *stat = &padStat[i];
            if(stat->err != 0) {
                continue;
            }
            if(stat->button != PAD_BTN_SRESET) {
                resetCancelF[i] = FALSE;
                continue;
            } else if(resetCancelF[i] == FALSE) {
                resetTick[i] = OSGetTick();
            } else {
                if(OSTicksToMilliseconds(OSGetTick()-resetTick[i]) > 500) {
                    resetPadF = TRUE;
                }
            }
            resetCancelF[i] = TRUE;
        }
        if(HuSRDisableF == FALSE) {
            BOOL resetF;
            if(SR_ExecReset) {
                resetF = TRUE;
            } else {
                if(H_ResetReady == TRUE && OSGetResetButtonState() != TRUE) {
                    resetF = TRUE;
                } else {
                    if(H_ResetReady != TRUE && OSGetResetButtonState() == TRUE) {
                        H_ResetReady = TRUE;
                    }
                    resetF = FALSE;
                }
            }
            if(resetF || resetPadF) {
                if(msmSysCheckInit()) {
                    msmStreamSetMasterVolume(0);
                    msmSeSetMasterVolume(0);
                    msmMusSetMasterVolume(0);
                }
                HuRestartSystem();
            }
        }
        xfb1 = Xfb[0];
        xfb2 = Xfb[1];
        DCInvalidateRange(&xfb1[(640/2)*200], sizeof(u16)*640);
        DCInvalidateRange(&xfb2[(640/2)*200], sizeof(u16)*640);
        if(xfb1[((640/2)*200)+32] != 0x800080 || xfb2[((640/2)*200)+32] != 0x800080) {
            DvdErrDispXFB(bmpMes[languageNo][error]);
        }
        VISetNextFrameBuffer(DemoCurrentBuffer);
        VIFlush();
        OSYieldThread();
    }
}

static void DvdErrDispXFB(void *data)
{
    s16 *bmpData;
    u8 *xfb1Ptr;
    u8 *xfb2Ptr;
    u32 i;
    u32 j;
    u32 bits;
    u32 row;
    
    u32 *xfb1;
    u32 *xfb2;
    u32 *dataPtr;
    s32 rowOffset;
    s32 rowPitch;
    u8 color2;
    u8 color1;
    xfb1 = Xfb[0];
    xfb2 = Xfb[1];
    for(i=0; i<153600; i++, xfb1++, xfb2++) {
        *xfb1 = *xfb2 = 0x800080;
    }
    DCFlushRangeNoSync(Xfb[0], 0x96000);
	DCFlushRangeNoSync(Xfb[1], 0x96000);
    bmpData = data;
    dataPtr = (u32 *)(&bmpData[2]);
    rowOffset = ((XfbW/2)-(bmpData[0]/2))*2;
	rowPitch = XfbW*2;
	color2 = color1 = 128;
    for(row=0; row<bmpData[1]; row++) {
        void *rowPtr[2];
        xfb1Ptr = ((u8 *)(Xfb[0])+((row+200)*rowPitch)+rowOffset);
		rowPtr[1] = xfb1Ptr;
		xfb2Ptr = ((u8 *)(Xfb[1])+((row+200)*rowPitch)+rowOffset);
		rowPtr[0] = xfb2Ptr;
        for(i=0; i<bmpData[0]; i += 32) {
            bits = *dataPtr++;
            for(j=0; j<32; j += 2, bits >>= 2, xfb1Ptr += 4, xfb2Ptr += 4) {
                if(bits & 0x3){
                    u8 y1;
                    u8 y2;
					if(bits & 0x1) {
						y1 = 0xEB;
					} else {
						y1 = 0x10;
					}
					if(bits & 0x2) {
						y2 = 0xEB;
					} else {
						y2 = 0x10;
					}
					xfb1Ptr[0] = y1;
					xfb1Ptr[1] = color2;
					xfb1Ptr[2] = y2;
					xfb1Ptr[3] = color1;
					xfb2Ptr[0] = y1;
					xfb2Ptr[1] = color2;
					xfb2Ptr[2] = y2;
					xfb2Ptr[3] = color1;
				}
            }
        }
        DCFlushRangeNoSync(rowPtr[1], bmpData[0]*2);
        DCFlushRangeNoSync(rowPtr[0], bmpData[0]*2);
    }
    PPCSync();
}

void HuRestartSystem(void)
{
	u32 retrace[2];
	BOOL prevInt;
	if(SR_RestartChk) {
		return;
	}
	SR_RestartChk = TRUE;
	PADRecalibrate(PAD_CHAN0_BIT|PAD_CHAN1_BIT|PAD_CHAN2_BIT|PAD_CHAN3_BIT);
	msmSysCheckInit();
	VISetBlack(TRUE);
	VIFlush();
	prevInt = OSDisableInterrupts();
	if(!prevInt) {
		OSReport("PrevInt=DISABLE!!\n");
	}
	OSEnableInterrupts();
	retrace[1] = VIGetRetraceCount();
	retrace[0] = 0;
	while(retrace[1] == VIGetRetraceCount()) {
		if(retrace[0]++ >= 1349800) {
			break;
		}
	}
	OSReport("Timeout Count=%d\n", retrace[0]);
	GXAbortFrame();
    if(SR_ExecResetMenu != 0) {
        OSResetSystem(TRUE, 0, TRUE);
    } else if(DVDGetDriveStatus() == DVD_STATE_WRONG_DISK) {
        OSResetSystem(TRUE, 0, FALSE);
    } else {
        OSResetSystem(FALSE, 0, FALSE);
    }
	
}

BOOL HuSoftResetCheck(void)
{
	int i;
	if(VCounter == 0) {
		return FALSE;
	}
	if(SR_ExecReset) {
		return TRUE;
	}
	if(SR_ResetPad != -1) {
		if(_PadBtn[SR_ResetPad] != PAD_BTN_SRESET) {
			return TRUE;
		}
	} else {
		for(i=0; i<4; i++) {
			if(SR_PreRstChk[i] && _PadBtn[i] != PAD_BTN_SRESET) {
				SR_PreRstChk[i] = FALSE;
			}
		}
	}
	
	if(HuSoftResetCountCheck()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL HuSoftResetCountCheck(void)
{
	int i;
	for(i=0; i<4; i++) {
		if(_PadBtn[i] != PAD_BTN_SRESET) {
			SR_PushTime[i] = 0;
		} else {
			if(!SR_PreRstChk[i]) {
				if(_PadBtn[i] & PAD_BUTTON_START) {
					if(SR_PushTime[i]++ >= 30) {
						SR_ResetPad = i;
						return TRUE;
					}
				} else {
					SR_PushTime[i] = 0;
				}
			}
		}
	}
	return FALSE;
}

static void HuSoftResetPostProc(void)
{
	if(!SR_ExecReset) {
		VISetBlack(TRUE);
		VIFlush();
		if(THPProc) {
			HuTHPStop();
			HuTHPClose();
		}
		if(msmSysCheckInit()) {
			msmStreamSetMasterVolume(0);
			msmSeSetMasterVolume(0);
			msmMusSetMasterVolume(0);
		}
		HuPadRumbleAllStop();
		SR_ExecReset = TRUE;
	}
}