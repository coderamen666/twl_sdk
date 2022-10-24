/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     ownerInfoEx.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-13#$
  $Rev: 9302 $
  $Author: hirose_kazuki $
 *---------------------------------------------------------------------------*/

#include    <nitro/os/common/ownerInfo.h>
#include    <nitro/os/common/systemWork.h>
#include    <nitro/os/common/emulator.h>
#include    <twl/os/common/ownerInfoEx.h>
#include    <twl/os/common/ownerInfoEx_private.h>
#include    <twl/hw/common/mmap_parameter.h>

#ifdef SDK_ARM9
#include    <twl/hw/ARM9/mmap_main.h>
#else //SDK_ARM7
#include    <twl/hw/ARM7/mmap_main.h>
#endif

/*---------------------------------------------------------------------------*
    Constants
 *---------------------------------------------------------------------------*/

//
// Each value of the owner information is read to a fixed address of the main memory by the launcher
#define  OS_ADDR_TWL_SETTINGSDATA   ( (OSTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA )
#define  OS_ADDR_TWL_HWNORMALINFO   ( (OSTWLHWNormalInfo *)HW_PARAM_TWL_HW_NORMAL_INFO )


/*---------------------------------------------------------------------------*
    function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         OS_GetOwnerInfoEx

  Description:  Gets the owner information. TWL version
                Information that also exists in NITRO is gotten from the same location as the NITRO version.
                The language code (language) can be different between TWL and NITRO, so get information from TWL.
                

  Arguments:    info:        Pointer to the buffer getting the owner information.
                            Data gets written to this buffer.
                            (*) TWL information can only be gotten in TWL mode
                               (When not running in TWL, this is always 0)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_GetOwnerInfoEx(OSOwnerInfoEx *info)
{
    NVRAMConfig *src;
    OSTWLSettingsData  *twlSettings;

    // Get the same information as as NITRO from the NITRO region (copy-paste sometimes)
    src = (NVRAMConfig *)(OS_GetSystemWork()->nvramUserInfo);
    //info->language = (u8)(src->ncd.option.language);          //The language code is designated for TWL mode and for NITRO mode
    info->favoriteColor = (u8)(src->ncd.owner.favoriteColor);
    info->birthday.month = (u8)(src->ncd.owner.birthday.month);
    info->birthday.day = (u8)(src->ncd.owner.birthday.day);
    info->nickNameLength = (u16)(src->ncd.owner.nickname.length);
    info->commentLength = (u16)(src->ncd.owner.comment.length);
    MI_CpuCopy16(src->ncd.owner.nickname.str,
                 info->nickName, OS_OWNERINFO_NICKNAME_MAX * sizeof(u16));
    MI_CpuCopy16(src->ncd.owner.comment.str, info->comment, OS_OWNERINFO_COMMENT_MAX * sizeof(u16));
    info->nickName[OS_OWNERINFO_NICKNAME_MAX] = 0;
    info->comment[OS_OWNERINFO_COMMENT_MAX] = 0;

    // Get the TWL information
    if( OS_IsRunOnTwl() )
    {
        twlSettings   = (OSTWLSettingsData*) OS_ADDR_TWL_SETTINGSDATA ;
        info->country = twlSettings->country;
        info->language = twlSettings->language;
    }
    else
    {
        info->country = 0;
        info->language = (u8)(src->ncd.option.language);
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsAvailableWireless

  Description:  Get information on the validity/invalidity of wireless module RF unit

  Arguments:    None.

  Returns:      TRUE if valid; FALSE if invalid
                When not running in TWL, this is always TRUE
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static BOOL OS_IsAvailableWireless_ltdmain(void);
static BOOL OS_IsAvailableWireless_ltdmain(void)
{
    OSTWLSettingsData *p;
    p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
    return (p->flags.isAvailableWireless == 0x1);
}
#include <twl/ltdmain_end.h>
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
BOOL OS_IsAvailableWireless(void)
{
    BOOL result;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        result = OS_IsAvailableWireless_ltdmain();
#endif
    }
    else
    {
        result = TRUE;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsAgreeEULA

  Description:  Gets whether EULA was agreed to.

  Arguments:    None.

  Returns:      BOOL: TRUE if EULA agreed to; FALSE if no agreement.
                When not running in TWL, this is always FALSE.
 *---------------------------------------------------------------------------*/
BOOL OS_IsAgreeEULA(void)
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return (p->flags.isAgreeEULAFlagList & 0x01) ? TRUE : FALSE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetAgreeEULAVersion

  Description:  Gets the agreed-to EULA version.

  Arguments:    None.

  Returns:      agreedEulaVersion: Agreed EULA version (0-255 characters).
 *---------------------------------------------------------------------------*/
u8 OS_GetAgreedEULAVersion( void )
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return p->agreeEulaVersion[ 0 ];
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetROMHeaderEULAVersion

  Description:  Gets version of EULA registered in ROM header of application.

  Arguments:    None.

  Returns:      cardEulaVersion: EULA version registered in ROM header of application (0-255 characters).
 *---------------------------------------------------------------------------*/
u8 OS_GetROMHeaderEULAVersion( void )
{
    if( OS_IsRunOnTwl() )
    {
        // Hard coding for the agree_EULA version in the ROM header.
        return *(u8 *)( HW_TWL_ROM_HEADER_BUF + 0x020e );
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetParentalControlInfoPtr

  Description:  Gets the pointer to parental controls information

  Arguments:    None.

  Returns:      Return the pointer.
                Return NULL when not running in TWL.
 *---------------------------------------------------------------------------*/
const OSTWLParentalControl* OS_GetParentalControlInfoPtr(void)
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return &(p->parentalControl);
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsParentalControledApp

  Description:  Determines whether parental controls should be applied when starting the application.
                

  Arguments:    appRatingInfo: Offset within ROM header of application
                                    Specify pointer to 16-byte rating information embedded in the 0x2f0 position
                                    

  Returns:      Return TRUE to apply parental controls and prompt password input. For applications that can be started without restriction, return FALSE.
                
                
 *---------------------------------------------------------------------------*/
/* The following code is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_begin.h>
#endif
BOOL OSi_IsParentalControledAppCore(u8* appRatingInfo);

BOOL
OSi_IsParentalControledAppCore(u8* appRatingInfo)
{
    OSTWLParentalControl*   p   =   &(((OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA)->parentalControl);
    
    if (p->flags.isSetParentalControl)
    {
        if (p->ogn < OS_TWL_PCTL_OGN_MAX)
        {
            if (appRatingInfo == NULL)
            {
                /* Rating information for the application to be investigated is unknown */
                OS_TWarning("Invalid pointer to Application rating information.\n");
                return TRUE;
            }
            else
            {
                if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_ENABLE_MASK) == 0)
                {
                    /* Rating information of the research group corresponding to the application is undefined */
                    OS_TWarning("Application doesn't have rating information for the organization.\n");
                    return TRUE;
                }
                else
                {
                    if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_ALWAYS_MASK) != 0)
                    {
                        return TRUE;
                    }
                    else
                    {
                        if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_AGE_MASK) > p->ratingAge)
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }
        else
        {
            /* Research group specification in the system configuration was unexpected */
            OS_TWarning("Invalid rating organization index (%d) in LCFG.\n", p->ogn);
        }
    }
    return FALSE;
}
/* The code above is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_end.h>
#endif

BOOL
OS_IsParentalControledApp(u8* appRatingInfo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return OSi_IsParentalControledAppCore(appRatingInfo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMovableUniqueID

  Description:  Gets a unique ID (included in the HW information) that can be moved between systems.

  Arguments:    pUniqueID: Storage destination (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN bytes are stored)
                         (When not running in TWL, always fill with 0)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetMovableUniqueID(u8 *pUniqueID)
{
    OSTWLHWNormalInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWNormalInfo*)OS_ADDR_TWL_HWNORMALINFO;
        MI_CpuCopy8( p->movableUniqueID, pUniqueID, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*sizeof(u8) );
    }
    else
    {
        MI_CpuFill8( pUniqueID, 0, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*sizeof(u8) );
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetValidLanguageBitmap

  Description:  Gets the bit map of the corresponding language

  Arguments:    None.
  Returns:      Returns the bit map of the corresponding language
                Return 0 when not running in TWL
 *---------------------------------------------------------------------------*/
u32 OS_GetValidLanguageBitmap( void )
{
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        return p->validLanguageBitmap;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetUniqueIDPtr

  Description:  Gets a pointer to the unique ID (included in the HW information) that can be moved between systems.

  Arguments:    None.

  Returns:      Return the pointer.
                Return NULL when not running in TWL.
 *---------------------------------------------------------------------------*/
const u8* OS_GetMovableUniqueIDPtr(void)
{
    OSTWLHWNormalInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWNormalInfo*)OS_ADDR_TWL_HWNORMALINFO;
        return (p->movableUniqueID);
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsForceDisableWireless

  Description:  Information on whether to forcefully invalidate wireless.
                Gets (including HW secure information)

  Arguments:    None.

  Returns:      TRUE if forcefully invalidated; FALSE when not invalidating.
                When not running in TWL, this is always FALSE.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static BOOL OS_IsForceDisableWireless_ltdmain(void);
static BOOL OS_IsForceDisableWireless_ltdmain(void)
{
    OSTWLHWSecureInfo *p;
    p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
    return (p->flags.forceDisableWireless == 0x1);
}
#include <twl/ltdmain_end.h>
#endif

BOOL OS_IsForceDisableWireless(void)
{
    BOOL  result;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        result = OS_IsForceDisableWireless_ltdmain();
#endif
    }
    else
    {
        result = FALSE;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegion

  Description:  Gets the region information.

  Arguments:    None.

  Returns:      Returns the region number.
                Always return 0 when not running in TWL .
 *---------------------------------------------------------------------------*/
OSTWLRegion OS_GetRegion(void)
{
    OSTWLRegion result;
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        result = (OSTWLRegion)(p->region);
    }
    else
    {
        result = (OSTWLRegion)0;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetSerialNo

  Description:  Gets the serial number.

  Arguments:    serialNo: Storage destination
                           OS_TWL_HWINFO_SERIALNO_LEN_MAX bytes, including the trailing end characters are stored.
                           When not running in TWL, always fill with 0

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_GetSerialNo(u8 *serialNo)
{
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        MI_CpuCopy8( p->serialNo, serialNo, OS_TWL_HWINFO_SERIALNO_LEN_MAX*sizeof(u8) );
    }
    else
    {
        MI_CpuFill8( serialNo, 0, OS_TWL_HWINFO_SERIALNO_LEN_MAX*sizeof(u8) );
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetWirelessFirmwareData

  Description:  Gets pointer to data for wireless firmware.

  Arguments:    None.
  Returns:      Pointer to data for wireless firmware.
 *---------------------------------------------------------------------------*/
OSTWLWirelessFirmwareData *OS_GetWirelessFirmwareData(void)
{
    if( OS_IsRunOnTwl() )
    {
        return (OSTWLWirelessFirmwareData*)HW_PARAM_WIRELESS_FIRMWARE_DATA;
    }
    else
    {
        return NULL;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetRegionCodeA3

  Description:  Gets the string that corresponds to the region code used by EC/NUP.

  Arguments:    region: Region code

  Returns:      Returns pointer to string that corresponds to the region code.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static const char* OSi_GetRegionCodeA3_ltdmain(OSTWLRegion region);
static const char* OSi_GetRegionCodeA3_ltdmain(OSTWLRegion region)
{
    const char* regionList[] =
    {
        "JPN",  // OS_TWL_REGION_JAPAN     = 0,  // NCL
        "USA",  // OS_TWL_REGION_AMERICA   = 1,  // NOA
        "EUR",  // OS_TWL_REGION_EUROPE    = 2,  // NOE
        "AUS",  // OS_TWL_REGION_AUSTRALIA = 3,  // NAL
        "CHN",  // OS_TWL_REGION_CHINA     = 4,  // IQue
        "KOR"   // OS_TWL_REGION_KOREA     = 5,  // NOK
    };
    
    if (region >= sizeof(regionList)/sizeof(regionList[0]))
    {
        OS_TWarning("Invalide region code.(%d)", region);
        return NULL;
    }
    return regionList[region];
}
#include <twl/ltdmain_end.h>
#endif /* SDK_TWL */

const char* OS_GetRegionCodeA3(OSTWLRegion region)
{
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        return OSi_GetRegionCodeA3_ltdmain(region);
#else /* SDK_TWL */
        return NULL;
#endif /* SDK_TWL */
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetISOCountryCodeA2

  Description:  Convert TWO independent countries and region code numbers to ISO 3166-1 alpha-2

  Arguments:    twlCountry: Country and region code

  Returns:      Returns pointer to string that corresponds to the country and region codes.
                Returns NULL when not running in TWL.
 *---------------------------------------------------------------------------*/
extern const int    OSi_CountryCodeListNumEntries;
extern const char*  OSi_CountryCodeList[];

const char* OS_GetISOCountryCodeA2(u8 twlCountry)
{
    const char* cca2 = NULL;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        if (twlCountry < OSi_CountryCodeListNumEntries)
        {
            cca2 = OSi_CountryCodeList[twlCountry];
        }
#ifndef SDK_FINALROM
        if (!cca2)
        {
            OS_TWarning("Invalid country code(%d)\n", twlCountry);
        }
#endif /* SDK_FINALROM */
#endif /* SDK_TWL */
        return cca2;
    }
    else  // When not running in TWL
    {
        return NULL;
    }
}

#undef OS_TWL_COUNTRY_NAME_MAX

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
