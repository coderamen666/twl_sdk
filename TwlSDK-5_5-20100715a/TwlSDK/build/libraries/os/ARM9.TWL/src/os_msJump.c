/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_msJump.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
 
#include <nitro.h>
#include <twl/os/common/msJump.h>

/*---------------------------------------------------------------------------*
 Constant Definitions
 *---------------------------------------------------------------------------*/
#define TITLE_ID_MACHINE_SETTING 0x00030015484e4241 /* HNBA and location of destination do not matter */

//============================================================================

/* The following code is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_begin.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToMachineSetting

  Description:  Application jump to the item specified in the TWL system configuration
  
  Arguments:    dest: Setting
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToMachineSetting(u8 dest)
{
    switch( dest )
    {
        OSTWLRegion region = OS_GetRegion();

        case  OS_TWL_MACHINE_SETTING_COUNTRY:
            /* With a region that does not have a country setting in the system configuration, end with FALSE */
            switch ( OS_GetRegion() )
            {
                case OS_TWL_REGION_AMERICA:
                case OS_TWL_REGION_EUROPE:
                case OS_TWL_REGION_AUSTRALIA:

                    break;

                case OS_TWL_REGION_JAPAN:
//                case OS_TWL_REGION_CHINA:
//                case OS_TWL_REGION_KOREA:
                /* Regions other than the above end with FALSE */
                default:
                    OS_TWarning("Region Error.");
                    return FALSE;
            }
            break;

    case  OS_TWL_MACHINE_SETTING_LANGUAGE:
            /* With a region that does not have a language setting in the system configuration, end with FALSE */
            switch ( OS_GetRegion() )
            {
                case OS_TWL_REGION_AMERICA:
                case OS_TWL_REGION_EUROPE:

                    break;

                case OS_TWL_REGION_JAPAN:
                case OS_TWL_REGION_AUSTRALIA:
//                case OS_TWL_REGION_CHINA:
//                case OS_TWL_REGION_KOREA:
                /* Regions other than the above end with FALSE */
                default:
                    OS_TWarning("Region Error.");
                    return FALSE;
            }
            break;
        
        case  OS_TWL_MACHINE_SETTING_PAGE_1:
        case  OS_TWL_MACHINE_SETTING_PAGE_2:
        case  OS_TWL_MACHINE_SETTING_PAGE_3:
        case  OS_TWL_MACHINE_SETTING_PAGE_4:
        case  OS_TWL_MACHINE_SETTING_APP_MANAGER:
        case  OS_TWL_MACHINE_SETTING_WIRELESS_SW:
        case  OS_TWL_MACHINE_SETTING_BRIGHTNESS:
        case  OS_TWL_MACHINE_SETTING_USER_INFO:
        case  OS_TWL_MACHINE_SETTING_DATE:
        case  OS_TWL_MACHINE_SETTING_TIME:
        case  OS_TWL_MACHINE_SETTING_ALARM:
        case  OS_TWL_MACHINE_SETTING_TP_CALIBRATION:
        case  OS_TWL_MACHINE_SETTING_PARENTAL_CONTROL:
        case  OS_TWL_MACHINE_SETTING_NETWORK_SETTING:
        case  OS_TWL_MACHINE_SETTING_NETWORK_EULA:
        case  OS_TWL_MACHINE_SETTING_NETWORK_OPTION:
        case  OS_TWL_MACHINE_SETTING_SYSTEM_UPDATE:
        case  OS_TWL_MACHINE_SETTING_SYSTEM_INITIALIZE:
            break;
        default:
            OS_TWarning("Unknown Destination");
            return FALSE;
    }

    /* Set Deliver Argument */
    {
        OSDeliverArgInfo argInfo;
        int result;

        OS_InitDeliverArgInfo(&argInfo, 0);
        (void)OS_DecodeDeliverArg();   //There are cases where the DeliverArg is not set in advance, so continue processing regardless of whether it is right or wrong
        OSi_SetDeliverArgState( OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE );
        result = OS_SetSysParamToDeliverArg( (u16)dest );
        
        if(result != OS_DELIVER_ARG_SUCCESS )
        {
            OS_TWarning("Failed to Set DeliverArgument.");
            return FALSE;
        }
        result = OS_EncodeDeliverArg();
        if(result != OS_DELIVER_ARG_SUCCESS )
        {
            OS_TWarning("Failed to Encode DeliverArgument.");
            return FALSE;
        }
    }
    /* Do Application Jump */
    return OS_DoApplicationJump( TITLE_ID_MACHINE_SETTING, OS_APP_JUMP_NORMAL );


    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToEulaDirect

  Description:  Application jump in the TWL system settings to startup the "Internet - Usage Agreement."
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToEulaDirect( void )
{
    return OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_NETWORK_EULA );
}

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToApplicationManagerDirect

  Description:  Application jump in the TWL system settings to startup the "Software Managment."
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToApplicationManagerDirect( void )
{
    return OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_APP_MANAGER );
}

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToNetworkSettngDirect

  Description:  Application jump in the TWL system settings to startup the "Internet - Connection Settings."
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToNetworkSettngDirect( void )
{
    return OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_NETWORK_SETTING );
}

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToCountrySettingDirect

  Description:  Application jump in the TWL system settings to startup the "Country."
                
                * Because the same settings do not exist on TWL for the Japanese market, jump to the top of this setting.
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToCountrySettingDirect( void )
{
    return OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_COUNTRY );
}

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToSystemUpdateDirect

  Description:  Application jump in the TWL system settings to startup the "update system."
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToSystemUpdateDirect( void )
{
    return OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_SYSTEM_UPDATE );
}

/* The code above is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToInternetSetting

  Description:  Run a hardware reset and jump to the Internet connection settings of the TWL system settings.
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToInternetSetting(void)
{
    BOOL result = FALSE;
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        result = OSi_JumpToNetworkSettngDirect();
    }
    else
#endif
    {
        OS_TWarning("This Hardware don't support this funciton");
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToEULAViewer

  Description:  Run a hardware reset and jump to the Internet usage agreement of the TWL system settings.
                
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToEULAViewer(void)
{
    BOOL result = FALSE;
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        result = OSi_JumpToEulaDirect();
    }
    else
#endif
    {
        OS_TWarning("This Hardware don't support this funciton");
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToWirelessSetting

  Description:  Run a hardware reset and jump to the wireless communcations of the TWL system settings.
  
  Arguments:    None.
 
  Returns:      FALSE  Application jump failed for some reason   
                * If the process was successful, a reset occurred during this function so TRUE is not returned
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToWirelessSetting(void)
{
    BOOL result = FALSE;
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        result = OSi_JumpToMachineSetting( OS_TWL_MACHINE_SETTING_WIRELESS_SW );
    }
    else
#endif
    {
        OS_TWarning("This Hardware don't support this funciton");
    }
    return result;
}
/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
