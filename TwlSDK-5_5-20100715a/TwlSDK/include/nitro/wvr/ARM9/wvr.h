/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - include
  File:     wvr.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_WVR_ARM9_WVR_H_
#define NITRO_WVR_ARM9_WVR_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/gx/gx_vramcnt.h>

// Callback function type for asynchronous API
typedef void (*WVRCallbackFunc) (void *arg, WVRResult result);


/*---------------------------------------------------------------------------*
  Name:         WVR_StartUpAsync

  Description:  Starts operations of the wireless library.
                Until forcibly stopped, access to the specified VRAM (C or D) is prohibited.

  Arguments:    vram: Specifies VRAM bank allocated to ARM7.
                callback: Specifies the callback function to invoke when processing is complete.
                arg: Specifies arguments passed to the callback function.

  Returns:      Returns the processing result.
 *---------------------------------------------------------------------------*/
WVRResult WVR_StartUpAsync(GXVRamARM7 vram, WVRCallbackFunc callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         WVR_TerminateAsync

  Description:  Forcibly stops operations of the wireless library.
                Access to VRAM (C or D) is permitted after asynchronous processing is completed.

  Arguments:    callback: Specifies the callback function to invoke when processing is complete.
                arg: Specifies arguments passed to the callback function.

  Returns:      Returns the processing result.
 *---------------------------------------------------------------------------*/
WVRResult WVR_TerminateAsync(WVRCallbackFunc callback, void *arg);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_WVR_ARM9_WVR_H_ */

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
