/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     application_jump.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _APPLICATION_JUMP_H_
#define _APPLICATION_JUMP_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// Define data-------------------------------------------

#define OS_MCU_RESET_VALUE_BUF_HOTBT_MASK		0x00000001
#define OS_MCU_RESET_VALUE_OFS					0

typedef enum OSAppJumpType{
    OS_APP_JUMP_NORMAL = 0,
    OS_APP_JUMP_TMP = 1
}OSAppJumpType;

// TMP application path
#define OS_TMP_APP_PATH			"nand:/<tmpjump>"


// Function's prototype------------------------------------
#ifdef SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         OS_JumpToSystemMenu

  Description:  Runs a hardware reset and jumps to the System Menu.
  
  Arguments:    None.
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToSystemMenu( void );

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToInternetSetting

  Description:  Runs a hardware reset and jumps to the Internet connection settings of the TWL System Settings.
                
  
  Arguments:    None.
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToInternetSetting(void);

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToEULAViewer

  Description:  Runs a hardware reset and jumps to the Internet usage agreement of the TWL System Settings.
                
  
  Arguments:    None.
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToEULAViewer(void);

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToWirelessSetting

  Description:  Runs a hardware reset and jumps to the wireless communications of the TWL System Settings.
  
  Arguments:    None.
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToWirelessSetting(void);

/*---------------------------------------------------------------------------*
  Name:         OS_RebootSystem

  Description:  Resets the hardware and starts this program.
  
  Arguments:    None.
 
  Returns:      FALSE:  Run on NITRO, or failure in restarting.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_RebootSystem( void );

/*---------------------------------------------------------------------------*
  Name:         OS_IsBootFromSystemMenu

  Description:  Determines whether this application was started from the System Menu.
  
  Arguments:    None.
 
  Returns:      TRUE: This application was started from the System Menu.
                FALSE: This application was started from another application.
 *---------------------------------------------------------------------------*/
BOOL OS_IsBootFromSystemMenu(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsTemporaryApplication

  Description:  Checks whether self is a TMP application.
  
  Arguments:    None.
 
  Returns:      TRUE: TMP application.
                FALSE: Other than TMP applications.
 *---------------------------------------------------------------------------*/
BOOL OS_IsTemporaryApplication(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsRebooted

  Description:  Checks whether a restart was applied using OS_RebootSystem.
  
  Arguments:    None.
 
  Returns:      TRUE: Restart was applied more than one time.
                FALSE: First startup.
 *---------------------------------------------------------------------------*/
BOOL OS_IsRebooted( void );

// ---- These are internal functions, so do not use for applications
BOOL OS_ReturnToPrevApplication( void );
BOOL OSi_CanApplicationJumpTo( OSTitleId titleID );
OSTitleId OSi_GetPrevTitleId( void );
BOOL OS_DoApplicationJump( OSTitleId id, OSAppJumpType jumpType );
#endif

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _APPLICATION_JUMP_H_
