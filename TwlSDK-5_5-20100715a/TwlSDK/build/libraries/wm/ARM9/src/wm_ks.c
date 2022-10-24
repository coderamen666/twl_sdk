/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_ks.c

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

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
  Name:         WM_StartKeySharing

  Description:  Enables key-sharing features.
                By performing MP communication after enabling the feature, key sharing communications will be carried out by piggybacking on the MP communications.
                

  Arguments:    buf:            Pointer to the buffer that stores the key information.
                                Its actual body is a pointer to a WMDataSharingInfo structure.
                port:        Number of port to use.

  Returns:      WMErrCode:      Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartKeySharing(WMKeySetBuf *buf, u16 port)
{
    return WM_StartDataSharing(buf, port, 0xffff, WM_KEYDATA_SIZE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndKeySharing

  Description:  Disables the key sharing feature.

  Arguments:    buf:            Pointer to the buffer that stores the key information.
                                Its actual body is a pointer to a WMDataSharingInfo structure.

  Returns:      WMErrCode:      Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndKeySharing(WMKeySetBuf *buf)
{
    return WM_EndDataSharing(buf);
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetKeySet

  Description:  Loads one key set data that has been key shared.

  Arguments:    buf:            Pointer to the buffer that stores the key information.
                                Its actual body is a pointer to a WMDataSharingInfo structure.
                keySet:         Pointer to the buffer that reads out the key set.
                                Specify a buffer other than the one given with WM_StartKeySharing.
                                

  Returns:      MWErrCode:      Returns process results.
 *---------------------------------------------------------------------------*/
WMErrCode WM_GetKeySet(WMKeySetBuf *buf, WMKeySet *keySet)
{
    WMErrCode result;
    u16     sendData[WM_KEYDATA_SIZE / sizeof(u16)];
    WMDataSet ds;
    WMArm9Buf *p = WMi_GetSystemWork();

    {
        sendData[0] = (u16)PAD_Read();
        result = WM_StepDataSharing(buf, sendData, &ds);
        if (result == WM_ERRCODE_SUCCESS)
        {
            keySet->seqNum = buf->currentSeqNum;

            {
                u16     iAid;
                for (iAid = 0; iAid < 16; iAid++)
                {
                    u16    *keyData;
                    keyData = WM_GetSharedDataAddress(buf, &ds, iAid);
                    if (keyData != NULL)
                    {
                        keySet->key[iAid] = keyData[0];
                    }
                    else
                    {
                        // Other party that failed to receive is 0
                        keySet->key[iAid] = 0;
                    }
                }
            }
            return WM_ERRCODE_SUCCESS; // Successful completion
        }
        else
        {
            return result;
        }
    }
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
