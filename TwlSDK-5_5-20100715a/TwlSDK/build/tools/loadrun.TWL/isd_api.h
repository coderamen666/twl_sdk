/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - loadrun
  File:     isd_api.h

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
#ifndef TWL_LOADRUN_ISD_API_H_
#define TWL_LOADRUN_ISD_API_H_

#include "ISTWLDLL.h"

#ifdef __cplusplus
extern "C" {
#endif

//---- For versions 0.63 and earlier
#ifndef ISD_FUNC_MODEHYBRID
#define	ISD_FUNC_MODEHYBRID	 "ISDSetHybridMode"
typedef enum { ISDHYBRID_TWL, ISDHYBRID_NTR } ISDHYBRIDMODE;
typedef BOOL (_cdecl *ISD_SETHYBRIDMODE)( ISDDEVICEHANDLE hDevice, ISDHYBRIDMODE eMode );
#endif

// Public functions
extern ISD_STREAMRECVCB ISTD_StreamRecvCB;
extern ISD_PRINTFRECVCB ISTD_PrintfRecvCB;
extern ISD_ENUMDEVICECB ISTD_EnumDeviceCB;
extern ISD_GETVERSION   ISTD_GetVersion;
extern ISD_FINDOPEN     ISTD_FindOpen;
extern ISD_FINDNEXT     ISTD_FindNext;
extern ISD_FINDCLOSE    ISTD_FindClose;
extern ISD_DEVSELECTOR  ISTD_DevSelector;
extern ISD_OPEN         ISTD_Open;
extern ISD_OPENEX       ISTD_OpenEx;
extern ISD_CLOSE        ISTD_Close;
extern ISD_GETDEVICEID  ISTD_GetDeviceID;
extern ISD_CHECKCONNECT ISTD_CheckConnect;
extern ISD_SETHYBRIDMODE ISTD_SetHybridMode;
extern ISD_DOWNLOADGO   ISTD_DownloadGo;
extern ISD_RESET        ISTD_Reset;
extern ISD_WRITEROM     ISTD_WriteROM;
extern ISD_READROM      ISTD_ReadROM;
extern ISD_ROMSIZE      ISTD_RomSize;
//extern ISD_STREAMSETCB  ISTD_StreamSetCB;
extern ISD_PRINTFSETCB  ISTD_PrintfSetCB;
extern ISD_GETLASTERROR ISTD_GetLastError;
extern ISD_GETERRORMESSAGE ISTD_GetErrorMessage;

//---- Device type
#define ISTD_DEVICE_NONE                   0
#define ISTD_DEVICE_IS_TWL_EMULATOR        1
#define ISTD_DEVICE_IS_TWL_DEBUGGER        2
#define ISTD_DEVICE_UNKNOWN                3


typedef struct ISTDDevice ISTDDevice;
struct ISTDDevice
{
    ISDDEVICEID id;
    int         type;
    int         host;
    int         serial;
};

typedef enum enumTWLArch {
    TWLArchARM9   = ISDCPUArchARM9,
    TWLArchARM7   = ISDCPUArchARM7,

    TWLArchNone   = -1
} TWLArch;

/*---------------------------------------------------------------------------*
  Name:         ISTD_InitDll

  Description:  Initializes DLL.

  Arguments:    None.

  Returns:      TRUE   if success
 *---------------------------------------------------------------------------*/
extern BOOL ISTD_InitDll(void);

/*---------------------------------------------------------------------------*
  Name:         ISTD_FreeDll

  Description:  Frees DLL module.

  Arguments:    None.

  Returns:      TRUE   if success
 *---------------------------------------------------------------------------*/
extern BOOL ISTD_FreeDll(void);


/*---------------------------------------------------------------------------*
  Name:         ISTD_GetDeviceList

  Description:  Gets connecting device list.

  Arguments:    deviceList: Pointer of list
                deviceMax: Max number of list

  Returns:      -1 if FAIL
                >=0 value is num of device which is connected
 *---------------------------------------------------------------------------*/
extern int ISTD_GetDeviceList(ISTDDevice * deviceList, int deviceMax);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_LOADRUN_ISD_API_H_ */
#endif
