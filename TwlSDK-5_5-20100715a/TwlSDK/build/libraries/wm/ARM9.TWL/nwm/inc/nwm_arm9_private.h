/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_arm9_private.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-26#$
  $Rev: 8688 $
  $Author: sato_masaki $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_NWM_ARM9_NWM_ARM9_PRIVATE_H__
#define LIBRARIES_NWM_ARM9_NWM_ARM9_PRIVATE_H__

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "nwm_common_private.h"

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    Macro Definitions
 *---------------------------------------------------------------------------*/

#define     NWM_CHECK_RESULT( res )      \
    if( (( res ) != NWM_RETCODE_SUCCESS) && (( res ) != NWM_RETCODE_OPERATING) ) \
    {                                   \
        return ( res );                 \
    }

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/

#ifdef NWM_SUPPORT_HWRESET
extern PMExitCallbackInfo hwResetCbInfo;
#endif

/*---------------------------------------------------------------------------*
    Private Function Prototypes
 *---------------------------------------------------------------------------*/

void NWMi_ReceiveFifo9(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err);
void NWMi_SetCallbackTable(NWMApiid id, NWMCallbackFunc callback);
void NWMi_SetReceiveCallbackTable(NWMFramePort port, NWMCallbackFunc callback);
NWMRetCode NWMi_SendCommand(NWMApiid id, u16 paramNum, ...);
NWMRetCode NWMi_SendCommandDirect(void *data, u32 length);
NWMRetCode NWMi_CheckState(s32 paramNum, ...);
NWMArm9Buf *NWMi_GetSystemWork(void);
NWMRetCode NWMi_CheckInitialized(void);
void NWMi_ClearFifoRecvFlag(void);
NWMRetCode NWMi_RegisterFirmware(void* addr, u32 size);

BOOL NWMi_CheckEnableFlag(void);

#ifdef NWM_SUPPORT_HWRESET
void NWMi_ForceExit(void *arg);
#endif

inline BOOL NWMi_IsAPIActive(u16 apiid)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    if (sys) {

        DC_InvalidateRange(&sys->status->apiActiveFlag[apiid], sizeof(u8));

        return (sys->status->apiActiveFlag[apiid] == 1) ? TRUE:FALSE;
    }
    return FALSE;
}

/*
  These functions can be called only by launcher.
 */

NWMRetCode NWMi_InstallFirmware(NWMCallbackFunc callback, void* addr, u32 size, BOOL isColdstart);
u32 NWMi_GetFirmImageOffset(void* addr, u32 fwType);
u32 NWMi_GetFirmImageLength(void* addr, u32 fwType);
u8* NWMi_GetFirmImageHashAddress(void* addr, u32 fwType);

BOOL NWMi_CheckFirmDataParamIntegrity(void);

/*
   Private API functions
 */

#ifdef NWM_INCLUDE_FIRM
NWMRetCode NWMi_LoadDeviceEx(NWMCallbackFunc callback); /* Transfers wireless firmware */
#endif

// for ARM9 WPA supplicant
NWMRetCode NWMi_SetWPAKey(NWMCallbackFunc callback, u8 cipherType, u8 macAddr[6], u8 keyIndex, u8 keyLen, u8* keyData, u8 keyRsc[8]);

// for ARM9 WPA supplicant
NWMRetCode NWMi_SetWPAParams(NWMCallbackFunc callback, u32 operation, u32 value);

NWMRetCode NWMi_CreateQoS(NWMCallbackFunc callback, NWMQoSParam *param);

/*===========================================================================*/



#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_NWM_ARM9_NWM_ARM9_PRIVATE_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
