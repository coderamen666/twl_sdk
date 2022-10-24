/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     application_jump_private.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _APPLICATION_JUMP_PRIVATE_H_
#define _APPLICATION_JUMP_PRIVATE_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// Define data-------------------------------------------

// Determine between a hot and a cold boot
#define OSi_GetMCUFreeRegisterValue()			( *(vu32*)HW_RESET_PARAMETER_BUF )

#define OS_MCU_RESET_VALUE_BUF_ENABLE_MASK		0x80000000
#define OS_MCU_RESET_VALUE_LEN					1

// Location to place the ROM header when preloading
#define OS_TWL_HEADER_PRELOAD_MMEM			0x23e0000
#define OS_TWL_HEADER_PRELOAD_MMEM_END		(0x23e0000 + 0x4000)

// Boot type to set as the launcher parameter
typedef enum LauncherBootType {
    LAUNCHER_BOOTTYPE_ILLEGAL = 0,          // Illegal status
    LAUNCHER_BOOTTYPE_ROM,                  // Boot from ROM
    LAUNCHER_BOOTTYPE_TEMP,                 // Start an application in the TMP folder in NAND
    LAUNCHER_BOOTTYPE_NAND,                 // Start an application in NAND
    LAUNCHER_BOOTTYPE_MEMORY,               // Start an application in memory

	LAUNCHER_BOOTTYPE_MAX
}LauncherBootType;

// Launcher parameter flag
typedef struct LauncherBootFlags {
	u16			isValid : 1;				// TRUE:valid, FALSE:invalid
	u16			bootType : 3;				// Use the LauncherBootType value
	u16			isLogoSkip : 1;				// Request that the logo demo be skipped
	u16			isInitialShortcutSkip : 1;	// Request that the initial startup sequence be skipped
	u16			isAppLoadCompleted : 1;		// Indicates that the application has been loaded
	u16			isAppRelocate : 1;			// Request that the application be relocated
	u16			rsv : 8;
}LauncherBootFlags;


// Launcher parameter header
typedef struct LauncherParamHeader {
	u32			magicCode;					// SYSM_LAUNCHER_PARAM_MAGIC_CODE goes here
	u8			version;					// Determine the body based on the type
	u8			bodyLength;					// Length of the body
	u16			crc16;						// CRC16 checksum for the body
}LauncherParamHeader;


// Launcher parameter body
typedef union LauncherParamBody {
	struct {								// Note: For the moment, make the TitleProperty match the format at first
		OSTitleId			prevTitleID;	// Title ID prior to a reset
		OSTitleId			bootTitleID;	// Title ID to boot directly after a reset
		LauncherBootFlags	flags;			// Launcher operation flags during a reset
		u8					rsv[ 6 ];		// Reserved
	}v1;
}LauncherParamBody;


// Launcher parameters
typedef struct LauncherParam {
	LauncherParamHeader	header;
	LauncherParamBody	body;
}LauncherParam;

// Function's prototype------------------------------------

#ifdef SDK_ARM9
void OSi_InitPrevTitleId( void );
void OS_SetLauncherParamAndResetHardware( OSTitleId id, LauncherBootFlags *flag );


BOOL OSi_IsJumpFromSameMaker(void);
u32 OSi_GetInitialCode(void);
u32 OSi_GetPrevInitialCode(void);
BOOL OSi_CanArbitraryJumpTo(u32 initialCode);
BOOL OSi_JumpToArbitraryApplication(u32 initialCode);
OSTitleId OSi_GetPrevTitleId(void);

#else
BOOL OS_ReadLauncherParameter( LauncherParam *buf, BOOL *isHotstart );
#endif

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _APPLICATION_JUMP_PRIVATE_H_
