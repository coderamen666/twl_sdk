/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     ownerInfoEx_private.h

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-07-10#$
  $Rev: 11355 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_COMMON_OWNERINFO_PRIVATE_H_
#define TWL_OS_COMMON_OWNERINFO_PRIVATE_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    Declarations that general developers should not be aware of have been taken out of ownerInfoEx.h.
    Applications should not directly use the items defined here.
 *---------------------------------------------------------------------------*/

/*===========================================================================*/

#include    <twl/types.h>
#include    <twl/spec.h>
#ifndef SDK_TWL
#include    <nitro/hw/common/mmap_shared.h>
#else
#include    <twl/hw/common/mmap_shared.h>
#endif

#include <twl/os/common/ownerInfoEx.h>


/*---------------------------------------------------------------------------*
    Formats that will be saved in main memory
 *---------------------------------------------------------------------------*/


// Structure that is compatible with TWL timestamp data
typedef struct OSTWLDate{
    u8              month;                      // Month: 01 to 12
    u8              day;                        // Day: 01 to 31
} OSTWLDate;          // 2 bytes

// Structure that is compatible with TWL owner information
typedef struct OSTWLOwnerInfo
{
    u8              userColor : 4;              // Favorite color
    u8              rsv : 4;                    // Reserved
    u8              pad;                        // Padding
    OSTWLDate       birthday;                   // Birthdate
    u16             nickname[ OS_TWL_NICKNAME_LENGTH + 1 ];   // Nickname (with terminating character)
    u16             comment[ OS_TWL_COMMENT_LENGTH + 1 ];     // Comment (with terminating character)
} OSTWLOwnerInfo;     // 80 bytes


// Structure that is compatible with TWL settings data
typedef struct OSTWLSettingsData
{
    union {
        struct {
            u32     isFinishedInitialSetting : 1;       // Initial settings complete?
            u32     isFinishedInitialSetting_Launcher : 1;  // Initial launcher settings complete?
            u32     isSetLanguage : 1;                  // Language code set?
            u32     isAvailableWireless : 1;            // Enabling/disabling the wireless module's RF unit
            u32     rsv : 20;                           // Reserved
            u32     isAgreeEULAFlagList : 8;            // List of EULA acceptance flags
            // Wi-Fi settings are separate data; do not prepare the flags set here.
        };
        u32         raw;
    } flags;
    u8                      rsv[ 1 ];               // Reserved
    u8                      country;                // Country code
    u8                      language;               // Language (the 8-bit data size differs from NTR systems)
    u8                      rtcLastSetYear;         // The year previously set with the RTC
    s64                     rtcOffset;              // Offset value when configuring the RTC (each time the user changes the configuration of the RTC, this fluctuates in response to that value)j
                                                    //   The parameter size up to this point is 16 bytes
    u8                      agreeEulaVersion[ 8 ];  //    8 bytes: The accepted EULA version
    u8                      pad1[2];
    u8                      pad2[6];                //    6 bytes
    u8                      pad3[16];               //   16bytes
    u8                      pad4[20];               //   20 bytes
    OSTWLOwnerInfo          owner;                  //   80 bytes: Owner information
    OSTWLParentalControl    parentalControl;        //  148 bytes: Parental Controls information
} OSTWLSettingsData;  // 296 bytes


// Structure that is compatible with configuration data for normal TWL hardware information
typedef struct OSTWLHWNormalInfo
{
    u8              rtcAdjust;                                  // RTC adjustment value
    u8              rsv[ 3 ];
    u8              movableUniqueID[ OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN ]; // A unique ID that can be moved
} OSTWLHWNormalInfo;  // 20 bytes


// Structure that is compatible with configuration data for secure TWL hardware information
typedef struct OSTWLHWSecureInfo
{
    u32             validLanguageBitmap;                            // A bitmap of language codes valid on this system
    struct {
        u8          forceDisableWireless :1;
        u8          :7;
    }flags;
    u8              pad[ 3 ];
    u8              region;                                         // Region
    u8              serialNo[ OS_TWL_HWINFO_SERIALNO_LEN_MAX ];   // Serial number (an ASCII string with a terminating character)
} OSTWLHWSecureInfo;  // 24 bytes


typedef struct OSTWLWirelessFirmwareData
{
    u8              data;
    u8              rsv[ 3 ];
} OSTWLWirelessFirmwareData;

/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetWirelessWirmwareData

  Description:  Gets pointer to data for wireless firmware.

  Arguments:    None.
  Returns:      Returns a pointer to data for the wireless firmware.
                Return NULL when not running in TWL.
 *---------------------------------------------------------------------------*/
OSTWLWirelessFirmwareData *OS_GetWirelessFirmwareData(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetValidLanguageBitmap

  Description:  Gets the bitmap for the corresponding language

  Arguments:    None.
  Returns:      Returns the bitmap for the corresponding language
                Return 0 when not running in TWL
 *---------------------------------------------------------------------------*/
u32 OS_GetValidLanguageBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetSerialNo

  Description:  Gets the serial number.

  Arguments:    serialNo: Storage destination
                           Stores OS_TWL_HWINFO_SERIALNO_LEN_MAX bytes, including the terminating character.
                           When not running in TWL, always fill with 0

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetSerialNo(u8 *serialNo);

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictDSDownloadBoot

  Description:  Determines whether a Parental Controls setting has been applied to restrict DS Download Play from starting.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictDSDownloadBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.dsDownload;
}

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* TWL_OS_COMMON_OWNERINFO_EX_PRIVATE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
