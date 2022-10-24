/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemWork.h

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_COMMON_SYSTEMWORK_H_
#define TWL_OS_COMMON_SYSTEMWORK_H_


/* If include from other environment (for example, VC or BCB), */
/* please define SDK_FROM_TOOL.                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))
//
//--------------------------------------------------------------------
#ifndef SDK_ASM

#include        <twl/types.h>
#include        <twl/hw/common/mmap_shared.h>
#ifdef SDK_TWL
#ifdef SDK_ARM9
#include        <twl/hw/ARM9/mmap_global.h>
#else //SDK_ARM7
#include        <twl/hw/ARM7/mmap_global.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_TWL

// Mount devices
typedef enum OSMountDevice {
    OS_MOUNT_DEVICE_SD   = 0,
    OS_MOUNT_DEVICE_NAND = 1,
    OS_MOUNT_DEVICE_MAX  = 2
}OSMountDevice;


// Mount targets
typedef enum OSMountTarget {
    OS_MOUNT_TGT_ROOT = 0,
    OS_MOUNT_TGT_FILE = 1,
    OS_MOUNT_TGT_DIR  = 2,
    OS_MOUNT_TGT_MAX  = 3
}OSMountTarget;


// Permissions
typedef enum OSMountPermission {
    OS_MOUNT_USR_X = 0x01,
    OS_MOUNT_USR_W = 0x02,
    OS_MOUNT_USR_R = 0x04
}OSMountPermission;


// Resource placement
typedef enum OSMountResource {
    OS_MOUNT_RSC_MMEM = 0,
    OS_MOUNT_RSC_WRAM = 1
}OSMountResource;


#define OS_MOUNT_PARTITION_MAX_NUM      3           // The maximum number of mountable partitions
#define OS_MOUNT_DRIVE_START            'A'         // The first drive name (you can specify only uppercase letters from 'A' to 'Z')
#define OS_MOUNT_DRIVE_END              'Z'         // The last drive name
#define OS_MOUNT_ARCHIVE_NAME_LEN       16          // The maximum length of an archive name
#define OS_MOUNT_PATH_LEN               64          // The maximum length of a path
#define OS_MOUNT_INFO_MAX               (size_t)((HW_TWL_FS_BOOT_SRL_PATH_BUF - HW_TWL_FS_MOUNT_INFO_BUF) / sizeof(OSMountInfo))

#define OS_TITLEIDLIST_MAX              118         // The maximum number of items retained by the title ID list


// Structure with mount information for archives
typedef struct OSMountInfo {
    u8      drive[ 1 ];
    u8      device : 3;
    u8      target : 2;
    u8      partitionIndex : 2;
    u8      resource : 1;
    u8      userPermission : 3;                         // Specifies whether this is readable and writable by the user
    u8      rsv_A : 5;
    u8      rsv_B;
    char    archiveName[ OS_MOUNT_ARCHIVE_NAME_LEN ];   // Size with a terminating '\0'
    char    path[ OS_MOUNT_PATH_LEN ];                  // Size with a terminating '\0'
}OSMountInfo;   // 84 bytes


// Title ID list structure
typedef struct OSTitleIDList {
	u8		num;
	u8		rsv[ 15 ];
	u8		publicFlag [ 16 ];	// Flag indicating whether there is public save data
	u8		privateFlag[ 16 ];	// Flag indicating whether there is private save data
	u8		appJumpFlag[ 16 ];	// Flag indicating whether an application jump can be performed
	u8		sameMakerFlag[ 16 ];	// Flag indicating whether the manufacturer is the same
	u64		TitleID[ OS_TITLEIDLIST_MAX ];
}OSTitleIDList; // 1024 bytes

typedef struct OSHotBootStatus {
	u8		isDisable :1;
	u8		rsv :7;
}OSHotBootStatus;

/*---------------------------------------------------------------------------*
  Name:         OS_GetMountInfo

  Description:  Gets mount information.

  Arguments:    None.

  Returns:      Returns a pointer to the start of the mount information list.
 *---------------------------------------------------------------------------*/
static inline const OSMountInfo *OS_GetMountInfo( void )
{
    return (const OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetBootSRLPath

  Description:  Gets the SRL path information to this program.

  Arguments:    None.

  Returns:      Returns a pointer to the SRL path to this program.
 *---------------------------------------------------------------------------*/
static inline const char *OS_GetBootSRLPath( void )
{
    return (const char *)HW_TWL_FS_BOOT_SRL_PATH_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleId

  Description:  Gets the application's title ID.

  Arguments:    None.

  Returns:      The title ID as a u64 value.
 *---------------------------------------------------------------------------*/
static inline u64 OS_GetTitleId( void )
{
    return *(u64 *)(HW_TWL_ROM_HEADER_BUF + 0x230);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMakerCode

  Description:  Gets the application's MakerCode.

  Arguments:    None.

  Returns:      The manufacturer code as a u16 value.
 *---------------------------------------------------------------------------*/
static inline u16 OS_GetMakerCode( void )
{
    return *(u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSystemMenuVersionInfoContentID

  Description:  Gets the contentID information for the System Menu version.

  Arguments:    None.

  Returns:      Returns a pointer to the start of the contentID. (This includes a null terminator.)
 *---------------------------------------------------------------------------*/
static inline const u8 *OSi_GetSystemMenuVersionInfoContentID( void )
{
    return (const u8 *)HW_SYSM_VER_INFO_CONTENT_ID;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSystemMenuVersionLastGameCode

  Description:  Gets the last byte of the System Menu version's game code.

  Arguments:    None.

  Returns:      Returns a pointer to the last byte of the game code.
 *---------------------------------------------------------------------------*/
static inline u8 OSi_GetSystemMenuVersionInfoLastGameCode( void )
{
    return *(u8 *)HW_SYSM_VER_INFO_CONTENT_LAST_INITIAL_CODE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IsEnableHotBoot

  Description:  Can the HotBoot flag be set?

  Arguments:    None.

  Returns:      TRUE: HotBoot cannot be set; FALSE: HotBoot can be set.
 *---------------------------------------------------------------------------*/
static inline BOOL OSi_IsEnableHotBoot( void )
{
	return ( (OSHotBootStatus *)HW_SYSM_DISABLE_SET_HOTBOOT )->isDisable ? 0 : 1;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetEnableHotBoot

  Description:  Controls whether the HotBoot flag can be set.

  Arguments:    isEnable -> TRUE: HotBoot can be set; FALSE: HotBoot cannot be set

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void OSi_SetEnableHotBoot( BOOL isEnable )
{
	( (OSHotBootStatus *)HW_SYSM_DISABLE_SET_HOTBOOT )->isDisable = isEnable ? 0 : 1;
}


#endif // SDK_TWL

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // SDK_ASM

#endif // SDK_FROM_TOOL

/* TWL_OS_COMMON_SYSTEMWORK_H_ */
#endif
