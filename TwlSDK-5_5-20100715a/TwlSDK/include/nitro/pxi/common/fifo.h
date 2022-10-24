/*---------------------------------------------------------------------------*
  Project:  TwlSDK - -include - PXI
  File:     pxi_fifo.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_PXI_COMMON_FIFO_H_
#define NITRO_PXI_COMMON_FIFO_H_

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/pxi/common/regname.h>


#ifdef __cplusplus
extern "C" {
#endif

/* TAG ID definition */
typedef enum
{
    PXI_FIFO_TAG_EX = 0,               // Extension format
    PXI_FIFO_TAG_USER_0,               // for application programmer, use it in free
    PXI_FIFO_TAG_USER_1,               // for application programmer, use it in free
    PXI_FIFO_TAG_SYSTEM,               // SDK inner usage
    PXI_FIFO_TAG_NVRAM,                // NVRAM
    PXI_FIFO_TAG_RTC,                  // RTC
    PXI_FIFO_TAG_TOUCHPANEL,           // Touch Panel
    PXI_FIFO_TAG_SOUND,                // Sound
    PXI_FIFO_TAG_PM,                   // Power Management
    PXI_FIFO_TAG_MIC,                  // Microphone
    PXI_FIFO_TAG_WM,                   // Wireless Manager
    PXI_FIFO_TAG_FS,                   // File System
    PXI_FIFO_TAG_OS,                   // OS
    PXI_FIFO_TAG_CTRDG,                // Cartridge
    PXI_FIFO_TAG_CARD,                 // Card
    PXI_FIFO_TAG_WVR,                  // Control driving wireless library
    PXI_FIFO_TAG_CTRDG_Ex,             // Cartridge Ex
    PXI_FIFO_TAG_CTRDG_PHI,            // Cartridge PHI
#ifdef SDK_TWL
    PXI_FIFO_TAG_MI,                   // MI
    PXI_FIFO_TAG_AES,                  // AES
    PXI_FIFO_TAG_FATFS,                // FATFS
    PXI_FIFO_TAG_CAMERA,               // CAMERA
    PXI_FIFO_TAG_WMW,                  // WM
    PXI_FIFO_TAG_SCFG,                 // SCFG
    PXI_FIFO_TAG_SNDEX,                // SNDEX

    PXI_FIFO_TAG_SEA = 31,             // SEA
#endif

    PXI_MAX_FIFO_TAG = 32              // MAX FIFO TAG
}
PXIFifoTag;


/* for Compatibility */
#define	PXI_FIFO_DEVICE_TEST		PXI_FIFO_TAG_USR_0
#define	PXI_FIFO_DEVICE_FLASH		PXI_FIFO_TAG_NVRAM
#define	PXI_FIFO_DEVICE_RTC		PXI_FIFO_TAG_RTC
#define	PXI_FIFO_DEVICE_TOUCHPANEL	PXI_FIFO_TAG_TOUCHPANEL
#define PXI_MAX_DEVICES			PXI_MAX_FIFO_TAG


/* PXI_FIFO return code */
typedef enum
{
    PXI_FIFO_SUCCESS = 0,
    PXI_FIFO_FAIL_SEND_ERR = -1,
    PXI_FIFO_FAIL_SEND_FULL = -2,
    PXI_FIFO_FAIL_RECV_ERR = -3,
    PXI_FIFO_FAIL_RECV_EMPTY = -4,
    PXI_FIFO_NO_CALLBACK_ENTRY = -5
}
PXIFifoStatus;


/* type definition */
#define	PXI_FIFOMESSAGE_BITSZ_TAG	5
#define	PXI_FIFOMESSAGE_BITSZ_ERR	1
#define	PXI_FIFOMESSAGE_BITSZ_DATA	26
typedef union
{
    struct
    {
        u32     tag:PXI_FIFOMESSAGE_BITSZ_TAG;
        u32     err:PXI_FIFOMESSAGE_BITSZ_ERR;
        u32     data:PXI_FIFOMESSAGE_BITSZ_DATA;
    }
    e;
    u32     raw;

}
PXIFifoMessage;


// type definition
typedef void (*PXIFifoCallback) (PXIFifoTag tag, u32 data, BOOL err);
typedef void (*PXIFifoEmtpyCallback) (void);


/*---------------------------------------------------------------------------*
  Name:         PXI_IsFifoError

  Description:  Check if error on fifo?

  Arguments:    status  Status 

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline BOOL PXI_IsFifoError(PXIFifoStatus status)
{
    return PXI_FIFO_SUCCESS == status;
}


/*---------------------------------------------------------------------------*
  Name:         PXI_InitFifo

  Description:  initialize FIFO system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PXI_InitFifo(void);


/*---------------------------------------------------------------------------*
  Name:         PXI_SetFifoRecvCallback

  Description:  set callback function when data arrive via FIFO

  Arguments:    device_no    DEVICE NO.
                callback     callback function to be called

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PXI_SetFifoRecvCallback(int fifotag, PXIFifoCallback callback);


/*---------------------------------------------------------------------------*
  Name:         PXI_IsCallbackReady
                PXI_IsArm7CallbackReady
                PXI_IsArm9CallbackReady

  Description:  check if callback is ready

  Arguments:    fifotag    fifo tag NO (0-31)
                proc       processor name PXI_PROC_ARM9 or PXI_PROC_ARM7
  
  Returns:      TRUE if callback is ready
 *---------------------------------------------------------------------------*/
BOOL    PXI_IsCallbackReady(int fifotag, PXIProc proc);

static inline BOOL PXI_IsArm7CallbackReady(int fifotag)
{
    return PXI_IsCallbackReady(fifotag, PXI_PROC_ARM7);
}

static inline BOOL PXI_IsArm9CallbackReady(int fifotag)
{
    return PXI_IsCallbackReady(fifotag, PXI_PROC_ARM9);
}


/*---------------------------------------------------------------------------*
  Name:         PXI_SetFifoSendCallback

  Description:  set callback function when data is sent via FIFO

  Arguments:    callback   callback function to be called

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PXI_SetFifoSendCallback(PXIFifoEmtpyCallback callback);


/*---------------------------------------------------------------------------*
  Name:         PXI_SendWordByFifo

  Description:  Send 32bit-word to anothre CPU via FIFO

  Arguments:    device_no    DEVICE NO.
                data         data(26-bit) whichi is sent

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
int     PXI_SendWordByFifo(int fifotag, u32 data, BOOL err);


//======================================================================
//              Interrupt handler called when RECV FIFO not empty
//======================================================================
void    PXIi_HandlerRecvFifoNotEmpty(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_PXI_COMMON_FIFO_H_ */
