/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     msJump.h

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-06-25#$
  $Rev: 6757 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_MACHINE_SETTING_JUMP_H_
#define TWL_OS_MACHINE_SETTING_JUMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_TWL
#ifdef SDK_ARM9
//----------------------------------------------------------------------------

#include <nitro.h>

/*---------------------------------------------------------------------------*
    Constants
 *---------------------------------------------------------------------------*/
typedef enum OSTWLMSJumpDestination
{
    OS_TWL_MACHINE_SETTING_PAGE_1             =  0,
    OS_TWL_MACHINE_SETTING_PAGE_2             =  2,
    OS_TWL_MACHINE_SETTING_PAGE_3             =  3,
    OS_TWL_MACHINE_SETTING_PAGE_4             =  4,
    OS_TWL_MACHINE_SETTING_APP_MANAGER        =  5,
    OS_TWL_MACHINE_SETTING_WIRELESS_SW        =  6,
    OS_TWL_MACHINE_SETTING_BRIGHTNESS         =  7,
    OS_TWL_MACHINE_SETTING_USER_INFO          =  8,
    OS_TWL_MACHINE_SETTING_DATE               =  9,
    OS_TWL_MACHINE_SETTING_TIME               = 10,
    OS_TWL_MACHINE_SETTING_ALARM              = 11,
    OS_TWL_MACHINE_SETTING_TP_CALIBRATION     = 12,
    OS_TWL_MACHINE_SETTING_LANGUAGE           = 13,
    OS_TWL_MACHINE_SETTING_PARENTAL_CONTROL   = 14,
    OS_TWL_MACHINE_SETTING_NETWORK_SETTING    = 15,
    OS_TWL_MACHINE_SETTING_NETWORK_EULA       = 16,
    OS_TWL_MACHINE_SETTING_NETWORK_OPTION     = 17,
    OS_TWL_MACHINE_SETTING_COUNTRY            = 18,
    OS_TWL_MACHINE_SETTING_SYSTEM_UPDATE      = 19,
    OS_TWL_MACHINE_SETTING_SYSTEM_INITIALIZE  = 20,
    OS_TWL_MACHINE_SETTING_MAX
} OSTWLMSJumpDestination;

/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToMachineSetting

  Description:  Makes an application jump to the specified TWL System Settings item.
  
  Arguments:    dest: A setting
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToMachineSetting(u8 dest);


/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToEulaDirect

  Description:  Makes an application jump to the TWL System Settings and brings up the User Agreement (under Internet).
                
  
  Arguments:    None.
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToEulaDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToApplicationManagerDirect

  Description:  Makes an application jump to the TWL System Settings and brings up the Data Management screen.
                
  
  Arguments:    None.
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToApplicationManagerDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToNetworkSettngDirect

  Description:  Makes an application jump to the TWL System Settings and brings up the Connection Settings screen (under Internet).
                
  
  Arguments:    None.
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToNetworkSettngDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToCountrySettingDirect

  Description:  Makes an application jump to the TWL System Settings and brings up the Country screen.
                
                NOTE: This setting does not exist on the Japanese version of the TWL system, so this will jump to the start of the System Settings.
  
  Arguments:    None.
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToCountrySettingDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToSystemUpdateDirect

  Description:  Makes an application jump to the TWL System Settings and brings up the System Update screen.
                
  
  Arguments:    None.
 
  Returns:      FALSE: The application jump failed for some reason
                NOTE: TRUE will not be returned when processing is successful because a reset will occur within this function.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToSystemUpdateDirect( void );

#endif /* SDK_ARM9 */
#endif /* SDK_TWL */


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEM_H_ */
#endif
