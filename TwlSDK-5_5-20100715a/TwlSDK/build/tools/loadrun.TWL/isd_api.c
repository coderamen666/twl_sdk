/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - loadrun
  File:     isd_api.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <windows.h>
#include "isd_api.h"

//---- IS-dll functions
ISD_STREAMRECVCB ISTD_StreamRecvCB;
ISD_PRINTFRECVCB ISTD_PrintfRecvCB;
ISD_ENUMDEVICECB ISTD_EnumDeviceCB;
ISD_GETVERSION   ISTD_GetVersion;
//NITROTOOLAPI_FINDOPEN ISNTD_FindOpen;
ISD_FINDOPEN     ISTD_FindOpen;
//NITROTOOLAPI_FINDNEXT ISNTD_FindNext;
ISD_FINDNEXT     ISTD_FindNext;
//NITROTOOLAPI_FINDCLOSE ISNTD_FindClose;
ISD_FINDCLOSE    ISTD_FindClose;
//NITROTOOLAPI_DEVSELECTOR ISNTD_Selector;
ISD_DEVSELECTOR  ISTD_DevSelector;
//NITROTOOLAPI_OPEN ISNTD_DeviceOpen;
ISD_OPEN         ISTD_Open;
ISD_OPENEX       ISTD_OpenEx;
//NITROTOOLAPI_CHECKCONNECT ISNTD_CheckConnect;
//NITROTOOLAPI_CLOSE ISNTD_DeviceClose;
ISD_CLOSE        ISTD_Close;
ISD_GETDEVICEID  ISTD_GetDeviceID;
ISD_CHECKCONNECT ISTD_CheckConnect;
ISD_SETHYBRIDMODE ISTD_SetHybridMode;
ISD_DOWNLOADGO   ISTD_DownloadGo;
//NITROTOOLAPI_RESET ISNTD_Reset;
ISD_RESET        ISTD_Reset;
//NITROTOOLAPI_WRITEROM ISNTD_WriteROM;
ISD_WRITEROM     ISTD_WriteROM;
//NITROTOOLAPI_READROM ISNTD_ReadROM;
ISD_READROM      ISTD_ReadROM;
//NITROTOOLAPI_ROMSIZE ISNTD_GetROMSize;
ISD_ROMSIZE      ISTD_RomSize;
//NITROTOOLAPI_STREAMSETCB ISNTD_SetReceiveStreamCallBackFunction;
//ISD_STREAMSETCB  ISTD_StreamSetCB;
ISD_PRINTFSETCB  ISTD_PrintfSetCB;
//NITROTOOLAPI_GETLASTERROR ISNTD_GetLastError;
ISD_GETLASTERROR ISTD_GetLastError;
//NITROTOOLAPI_GETERRORMESSAGE ISNTD_GetLastErrorMessage;
ISD_GETERRORMESSAGE ISTD_GetErrorMessage;
//NITROTOOLAPI_CHECKPOWER ISNTD_GetPowerStatus;
//ISD_CHECKPOWER not implemented
//NITROTOOLAPI_STREAMWRITE ISNTD_StreamWrite;
//ISD_STREAMWRITE Not implemented
//NITROTOOLAPI_STREAMGETWRITABLELEN ISNTD_StreamGetWritableLength;
//IISD_STREAMGETWRITABLELEN Not implemented
//NITROTOOLAPI_CARDSLOTPOWER ISNTD_CardSlotPower;
//ISD_CARDSLOTPOWER Not implemented

//No corresponding functions below
//NITROTOOLAPI_SYNCWRITE ISNTD_SyncWriteROM;
//NITROTOOLAPI_SYNCREAD ISNTD_SyncReadROM;
//NITROTOOLAPI_STREAMCHKRECV ISNTD_StreamCheckStreamReceive;
//NITROTOOLAPI_SETSYNCTIMEOUT ISNTD_SetSyncTimeOut;
//NITROTOOLAPI_GETDEBUGPRINT ISNTD_GetDebugPrint;
//NITROTOOLAPI_CARTRIDGESLOTPOWER ISNTD_CartridgeSlotPower;

//---- flag of initialized already
static BOOL ISTDi_IsInitialized = FALSE;

//---- dll instance
HINSTANCE ISTD_DllInstance;


#ifdef _declDllFunc
#undef _declDllFunc
#endif
#define _declDllFunc(type,name) (ISD_##type)GetProcAddress(ISTD_DllInstance, ISD_FUNC_##name)

/*---------------------------------------------------------------------------*
  Name:         ISTD_InitDll

  Description:  initialize IS-dll functions

  Arguments:    None.

  Returns:      TRUE if success. FALSE if called already.
 *---------------------------------------------------------------------------*/
BOOL ISTD_InitDll(void)
{
    //---- check initialized already
    if (ISTDi_IsInitialized)
    {
        return FALSE;
    }
    ISTDi_IsInitialized = TRUE;

    //---- load dll
    ISTD_DllInstance = LoadLibrary(ISDDLL_NAME);

//	ISTD_StreamRecvCB    = _declDllFunc(STREAMRECVCB, STREAMRECVCB);
//	ISTD_PrintfRecvCB    = _declDllFunc(PRINTFRECVCB, PRINTFRECVCB);
//	ISTD_EnumDeviceCB    = _declDllFunc(ENUMDEVIVECV, ENUMDEVIVECV);
	ISTD_GetVersion      = _declDllFunc(GETVERSION, GETVERSION);
    ISTD_FindOpen        = _declDllFunc(FINDOPEN, FINDOPEN);
    ISTD_FindNext        = _declDllFunc(FINDNEXT, FINDNEXT);
	ISTD_FindClose       = _declDllFunc(FINDCLOSE, FINDCLOSE);
//	ISTD_DevSelector     = _declDllFunc(DEVSELECTOR, DEVSELECTOR);
	ISTD_Open            = _declDllFunc(OPEN, OPEN);
	ISTD_OpenEx          = _declDllFunc(OPENEX, OPENEX);
	ISTD_Close           = _declDllFunc(CLOSE, CLOSE);
	ISTD_GetDeviceID     = _declDllFunc(GETDEVICEID, GETDEVICEID);
	ISTD_CheckConnect    = _declDllFunc(CHECKCONNECT, CHECKCONNECT);
	ISTD_DownloadGo      = _declDllFunc(DOWNLOADGO, DOWNLOADGO);
	ISTD_SetHybridMode   = _declDllFunc(SETHYBRIDMODE, MODEHYBRID);
	ISTD_Reset           = _declDllFunc(RESET, RESET);
    ISTD_WriteROM        = _declDllFunc(WRITEROM, WRITEROM);
	ISTD_ReadROM         = _declDllFunc(READROM, READROM);
	ISTD_RomSize         = _declDllFunc(ROMSIZE, ROMSIZE);
//	ISTD_StreamSetCB     = _declDllFunc(STREAMSETCB, STREAMSETCB);
	ISTD_PrintfSetCB     = _declDllFunc(PRINTFSETCB, PRINTFSETCB);
	ISTD_GetLastError    = _declDllFunc(GETLASTERROR, GETLASTERROR);
	ISTD_GetErrorMessage = _declDllFunc(GETERRORMESSAGE, GETERRORMESSAGE);

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ISTD_FreeDll

  Description:  free dll module

  Arguments:    None.

  Returns:      TRUE   if success
 *---------------------------------------------------------------------------*/
BOOL ISTD_FreeDll(void)
{
    //---- check initialized already
    if (!ISTDi_IsInitialized)
    {
        return FALSE;
    }

    FreeLibrary(ISTD_DllInstance);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ISTD_GetDeviceList

  Description:  get connecting device list

  Arguments:    deviceList : pointer of list
                deviceMax  : max number of list

  Returns:      -1 if FAIL
                >=0 value is num of device which is connected
 *---------------------------------------------------------------------------*/
int ISTD_GetDeviceList(ISTDDevice * deviceList, int deviceMax)
{
    ISDFINDHANDLE finder;
    int     count = 0;
    ISDDEVICEID id;
    ISTDDevice *p = deviceList;

    //---- check device list
    if (!deviceList)
    {
        return -1;
    }

    //---- search devices
    finder = ISTD_FindOpen();
    if (!finder)
    {
        return -1;
    }

    //---- clear
    {
        int     n;
        for (n = 0; n < deviceMax; n++)
        {
            deviceList[n].type = ISTD_DEVICE_NONE;
        }
    }

    //---- store devices id to list
    while (ISTD_FindNext(finder, &id) && count < deviceMax)
    {
        p->id = id;
		switch( p->id.eType )
		{
			case ISDDEV_ISTWLEMU:
				p->type = ISTD_DEVICE_IS_TWL_EMULATOR;
				p->serial = id.nSerial;
				break;
			case ISDDEV_ISTWLDBG:
				p->type = ISTD_DEVICE_IS_TWL_DEBUGGER;
				p->serial = id.nSerial;
				break;
			default:
				p->type = ISTD_DEVICE_UNKNOWN;
				p->serial = 0;
				break;
		}

        p++;
        count++;
    }

    return count;
}
