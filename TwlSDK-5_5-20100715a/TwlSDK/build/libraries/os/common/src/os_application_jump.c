/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_application_jump.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <twl/os.h>
#include <application_jump_private.h>
#include <twl/spi.h>
#include <twl/code32.h>
#include <twl/mcu.h>
#include <twl/hw/common/mmap_shared.h>
#include <twl/hw/common/mmap_parameter.h>

// Define data-------------------------------------------------------
#define OSi_LAUNCHER_PARAM_MAGIC_CODE           "TLNC"
#define OSi_LAUNCHER_PARAM_MAGIC_CODE_LEN       4
#define OSi_TITLE_ID_MEDIA_FLAG_MASK            ( 0x0001ULL << 34 )
#define OSi_TITLE_ID_DATA_ONLY_FLAG_MASK        ( 0x0001ULL << 35 )
#define OSi_TMP_APP_SIZE_MAX                    ( 16 * 1024 * 1024 )

#define OSi_TMP_APP_PATH_RAW                    "nand:/tmp/jump.app"
#define OSi_TMP_APP_PATH_RAW_LENGTH             18

typedef struct NandFirmResetParameter {
    u8      isHotStart :1;
    u8      isResetSW :1;
    u8      rsv :5;
    u8      isValid :1;
}NandFirmResetParameter;

// NAND firmware parameter address
#define OSi_GetNandFirmResetParam()         ( (NandFirmResetParameter *)HW_NAND_FIRM_HOTSTART_FLAG )

// Launcher parameter address
#define OSi_GetLauncherParam()              ( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )


// Extern data-------------------------------------------------------

// Function's prototype----------------------------------------------
static void OSi_DoHardwareReset( void );
static BOOL OSi_IsNandApp( OSTitleId id );
static BOOL OSi_IsSameMakerCode(OSTitleId id);
static OSTitleId OSi_SearchTitleIdFromList(OSTitleId id, u64 mask);

// Global variables--------------------------------------------------

// Static variables--------------------------------------------------
#include  <twl/ltdmain_begin.h>
static OSTitleId s_prevTitleId = 0;
#include  <twl/ltdmain_end.h>
// Const data--------------------------------------------------------

#include <twl/ltdmain_begin.h>
static BOOL OSi_IsNandApp( OSTitleId id )
{
    if( id & OSi_TITLE_ID_MEDIA_FLAG_MASK )
    {
        return TRUE;
    }
    return FALSE;
}

static BOOL OSi_IsSameMakerCode(OSTitleId id)
{
    const OSTitleIDList *list = (const OSTitleIDList *)HW_OS_TITLE_ID_LIST;
    const int   total = MATH_MIN( list->num, OS_TITLEIDLIST_MAX );
    int i;
    for (i = 0; i < total; ++i)
    {
        if (list->TitleID[i] == id)
        {
            int pos = (i >> 3);
            int bit = (0x01 << (i & 7));
            if (( list->sameMakerFlag[pos] & bit) != 0)
            {
                /* The maker code is the same */
                return TRUE;
            }
            else
            {
                /* The maker code is different */
                return FALSE;
            }
        }
    }
    /* The target TitleID is not in the list */
    return FALSE;
}

static OSTitleId OSi_SearchTitleIdFromList( OSTitleId id, u64 mask )
{
    const OSTitleIDList *list = (const OSTitleIDList *)HW_OS_TITLE_ID_LIST;
    const int OS_TITLE_ID_LIST_MAX = sizeof(list->TitleID) / sizeof(*list->TitleID);
    const int num = MATH_MIN(list->num, OS_TITLE_ID_LIST_MAX);
    int i;

    /* Under the assumption that game codes are unique, extract the relevant TitleID from the list */
    for (i = 0; i < num; ++i)
    {
        if ( (id & mask) == (list->TitleID[i] & mask)                       /* Do the game codes match? */
            && !(list->TitleID[i] & OSi_TITLE_ID_DATA_ONLY_FLAG_MASK) )     /* Is this not a data-only title? */
        {
            return list->TitleID[i];
        }
    }
    return 0;
}

#include  <twl/ltdmain_end.h>

#ifdef SDK_ARM9

void OSi_InitPrevTitleId( void )
{
    OSDeliverArgInfo deliverArgInfo;
    OS_InitDeliverArgInfo( &deliverArgInfo, 0 );
    if ( OS_DELIVER_ARG_SUCCESS == OS_DecodeDeliverArg() )
    {
        s_prevTitleId = OS_GetTitleIdFromDeliverArg();
    }
    if ( (s_prevTitleId != 0 && !OSi_SearchTitleIdFromList(s_prevTitleId, 0xffffffffffffffff)) ||
          (char)(s_prevTitleId >>24) == '0' ||
          (char)(s_prevTitleId >>24) == '1' ||
          (char)(s_prevTitleId >>24) == '2' ||
          (char)(s_prevTitleId >>24) == '3' ||
          (char)(s_prevTitleId >>24) == '4'  )
    {
        /* Make it look like this was started from the launcher and delete DeliverArg as well */
        s_prevTitleId = 0;
        OS_InitDeliverArgInfo( &deliverArgInfo, 0 );
        (void)OS_EncodeDeliverArg();
    }
    else
    {
        OS_SetDeliverArgStateInvalid();
    }
}

// For safety purposes, locate this function in itcm
#include <twl/itcm_begin.h>
static void OSi_DoHardwareReset( void )
{
    // [TODO:] Lastly, change to HW reset via a microcomputer
    // Issue reset command
    PM_ForceToResetHardware();
    OS_Terminate();
}

#include <twl/itcm_end.h>

/* The following code is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_begin.h>
#endif
// Hardware reset by specifying LauncherParam
void OS_SetLauncherParamAndResetHardware( OSTitleId id, LauncherBootFlags *flag )
{
    // Copy the manufacturer and game codes
    u32 *maker_code_dest_addr = (u32 *)((u32)(OSi_GetLauncherParam()) + HW_PARAM_DELIVER_ARG_MAKERCODE_OFS);
    u32 *game_code_dest_addr = (u32 *)((u32)(OSi_GetLauncherParam()) + HW_PARAM_DELIVER_ARG_GAMECODE_OFS);
    u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
    u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
    *maker_code_dest_addr = (u32)*maker_code_src_addr;
    *game_code_dest_addr = *game_code_src_addr;

    // Set the launcher parameter
    OSi_GetLauncherParam()->body.v1.prevTitleID = *(OSTitleId *)(HW_TWL_ROM_HEADER_BUF + 0x230);
    OSi_GetLauncherParam()->body.v1.bootTitleID = id;
    OSi_GetLauncherParam()->body.v1.flags = *flag;
    MI_CpuCopyFast( OSi_LAUNCHER_PARAM_MAGIC_CODE, (char *)&OSi_GetLauncherParam()->header.magicCode, OSi_LAUNCHER_PARAM_MAGIC_CODE_LEN);
    OSi_GetLauncherParam()->header.bodyLength = sizeof(LauncherParamBody);
    OSi_GetLauncherParam()->header.crc16 = SVC_GetCRC16( 65535, &OSi_GetLauncherParam()->body, OSi_GetLauncherParam()->header.bodyLength );
    OSi_GetLauncherParam()->header.version = 1;

    if( id == 0 || OS_IsDeliverArgEncoded() != TRUE || OS_GetTitleIdLastEncoded() != OSi_GetLauncherParam()->body.v1.prevTitleID )
    {
        // If the DeliverArg is not encoded with the current application, force initialization of DeliverArg and encode
        OSDeliverArgInfo deliverArgInfo;
        OS_InitDeliverArgInfo( &deliverArgInfo, 0 );
        (void)OS_EncodeDeliverArg();
    }

    DC_FlushRange((const void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE + HW_PARAM_LAUNCH_PARAM_SIZE );
    DC_WaitWriteBufferEmpty();

    OSi_DoHardwareReset();
}

// Function that returns from the application after the application jump to the original application
// If there is no original application, FALSE
BOOL OS_ReturnToPrevApplication( void )
{
    if( s_prevTitleId != 0)
    {
        return OS_DoApplicationJump( s_prevTitleId, OS_APP_JUMP_NORMAL ); //Never return
    }
    return FALSE;
}

static inline BOOL IsSameTitleID( OSTitleId titleID, OSTitleId listedTitleID )
{
    if ((char)titleID == 'A')
    {
        // Corresponding to language code A option
        return (listedTitleID >> 8) == (titleID >> 8);
    }
    else
    {
        return listedTitleID == titleID;
    }
}

// Get the ID that can actually be started for the given title ID
// If a startable ID exists, set that ID to the *bootableTitleID and return TRUE
// If a startable ID does not exist, return FALSE
static BOOL OS_GetBootableTitleID( OSTitleId titleID, OSTitleId *bootableTitleID )
{
    const OSTitleIDList *list = (const OSTitleIDList *)HW_OS_TITLE_ID_LIST;
    const int OS_TITLE_ID_LIST_MAX = sizeof(list->TitleID) / sizeof(*list->TitleID);
    const int num = MATH_MIN(list->num, OS_TITLE_ID_LIST_MAX);
    int i;
    for (i = 0; i < num; ++i)
    {
        if (IsSameTitleID(titleID, list->TitleID[i]) && (list->appJumpFlag[i / 8] & (u8)(0x1 << (i % 8))))
        {
            *bootableTitleID = list->TitleID[i];
            return TRUE;
        }
    }
    return FALSE;
}

// Whether an application has been installed that corresponds to that TitleID
BOOL OSi_CanApplicationJumpTo( OSTitleId titleID )
{
    return OS_GetBootableTitleID(titleID, &titleID);
}

// Wrapper for OS_SetLauncherParamAndResetHardware
BOOL OS_DoApplicationJump( OSTitleId id, OSAppJumpType jumpType )
{
    FSFile  file[1];
    LauncherBootFlags flag;
    u8 platform_code;
    u8 bit_field;
    
    switch( jumpType )
    {
        case OS_APP_JUMP_NORMAL:
            if( id & OSi_TITLE_ID_DATA_ONLY_FLAG_MASK )
            {
                // Set so that it is not possible to jump to DataOnly items
                return FALSE;
            }
            if( id != 0 )
            {
                if (!OS_GetBootableTitleID(id, &id))
                {
                    return FALSE;
                }
            }
            // Set the bootType and jump
            if( id & OSi_TITLE_ID_MEDIA_FLAG_MASK )
            {
                flag.bootType = LAUNCHER_BOOTTYPE_NAND;
            }else
            {
                flag.bootType = LAUNCHER_BOOTTYPE_ROM;
            }
            break;
        case OS_APP_JUMP_TMP:
            // When you yourself are a TMP application, TMP jump is not possible
            if( OS_IsTemporaryApplication() )
            {
                OS_TPrintf("OS_DoApplicationJump error : tmp app can't jump to tmp app!\n");
                return FALSE;
            }
            // Get the TitleID from the file of OS_TMP_APP_PATH and check the allowed TMP jump bits
            FS_InitFile( file );
            if( !FS_OpenFileEx(file, OS_TMP_APP_PATH, FS_FILEMODE_R) )
            {
                OS_TPrintf("OS_DoApplicationJump error : tmp app open error!\n");
                return FALSE;
            }
            // File size check
            if( FS_GetFileLength( file ) > OSi_TMP_APP_SIZE_MAX )
            {
                OS_TPrintf("OS_DoApplicationJump error : too large tmp app size!\n");
                return FALSE;
            }
            
            if( !FS_SeekFile( file, 0x12, FS_SEEK_SET ) ||
                ( sizeof(platform_code) != FS_ReadFile( file, &platform_code, sizeof(platform_code) ) ) ||
                !FS_SeekFile( file, 0x1d, FS_SEEK_SET ) ||
                ( sizeof(bit_field) != FS_ReadFile( file, &bit_field, sizeof(bit_field) ) )
            )
            {
                OS_TPrintf("OS_DoApplicationJump error : tmp app read error!\n");
                (void)FS_CloseFile(file);
                return FALSE;
            }
            
            if( !( bit_field & 0x2 ) )
            {
                OS_TPrintf("OS_DoApplicationJump error : tmp jump bit is not enabled!\n");
                (void)FS_CloseFile(file);
                return FALSE;
            }
            flag.bootType = LAUNCHER_BOOTTYPE_TEMP;
            // The TWL application gets the title ID from the header and enters it. For NTR applications, for the time being enter TitleIDs other than 0
            if( platform_code & 0x2 )
            {
                if( !FS_SeekFile( file, 0x0230, FS_SEEK_SET ) ||
                    ( sizeof(id) != FS_ReadFile( file, &id, sizeof(id) ) )
                )
                {
                    OS_TPrintf("OS_DoApplicationJump error : tmp app read error!\n");
                    (void)FS_CloseFile(file);
                    return FALSE;
                }
            }else
            {
                id = 0x1;
            }
            (void)FS_CloseFile(file);
            break;
        default:
            return FALSE;
    }

    if ( PMi_TryLockForReset() == FALSE )
    {
        return FALSE;
    }
    
    // Shared configurations
    flag.isValid = TRUE;
    flag.isLogoSkip = TRUE;
    flag.isInitialShortcutSkip = FALSE;
    flag.isAppLoadCompleted = FALSE;
    flag.isAppRelocate = FALSE;
    flag.rsv = 0;
    
    OS_SetLauncherParamAndResetHardware( id, &flag ); // never return.
    return TRUE;
}

/* The code above is located in the TWL extended memory region */
#ifdef    SDK_TWL
#include  <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToSystemMenu

  Description:  Runs a hardware reset and jumps to the system menu.
  
  Arguments:    None.
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToSystemMenu( void )
{
    BOOL result = FALSE;
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        // ---- Application Jump with id==0
        //      means to jump to system menu
        result = OS_DoApplicationJump( 0, OS_APP_JUMP_NORMAL );
    }
    else
#endif
    {
        OS_TWarning("This Hardware doesn't support this function");
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         OS_RebootSystem

  Description:  Runs a hardware reset and restarts itself.
  
  Arguments:    None.
 
  Returns:      FALSE: Run on NITRO, or failure in restarting.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OS_RebootSystem( void )
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        if( OS_IsTemporaryApplication() )
        {
            OS_TPrintf("OS_RebootSystem error : tmp app can't execute this function\n");
            return FALSE;
        }
        // Application jump to self
        return OS_DoApplicationJump( OS_GetTitleId(), OS_APP_JUMP_NORMAL );
    }
    else
#endif
    {
        OS_TWarning("This Hardware doesn't support this function");
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRebooted

  Description:  Checks whether a restart was applied using OS_RebootSystem.
  
  Arguments:    None.
 
  Returns:      TRUE: Restart was applied more than one time.
                FALSE: First startup.
 *---------------------------------------------------------------------------*/
BOOL OS_IsRebooted( void )
{
    BOOL result = FALSE;
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        if( OS_GetTitleId() == s_prevTitleId )
        {
            result = TRUE;
        }
    }
#endif
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_CanArbitraryJumpTo

  Description:  Checks whether a jump to the specified (arbitrary) application is possible.
  
  Arguments:    Game code of the application to jump to
  
  Returns:      TRUE: Execution can jump to the target application.
                FALSE: Execution cannot jump to the target application.
 *---------------------------------------------------------------------------*/
 
BOOL OSi_CanArbitraryJumpTo(u32 initialCode)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        OSTitleId id = OSi_SearchTitleIdFromList( (OSTitleId)initialCode, 0x00000000ffffffff );
        /* Non-NAND application are exceptions */
        if( OSi_IsNandApp(id) )
        {
            return OSi_CanApplicationJumpTo( id ) && OSi_IsSameMakerCode( id );
        }
    }
#endif
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToArbitraryApplication

  Description:  Jumps to the specified (arbitrary) application.
  
  Arguments:    Game code of the application to jump to
 
  Returns:      FALSE: Application jump failed for some reason.
                * If the process was successful, a reset occurred during this function, so TRUE is not returned.
                   
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToArbitraryApplication(u32 initialCode)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        OSTitleId id = OSi_SearchTitleIdFromList( (OSTitleId)initialCode, 0x00000000ffffffff );
        if( OSi_CanArbitraryJumpTo( initialCode ) )
        {
            return OS_DoApplicationJump( id, OS_APP_JUMP_NORMAL );
        }
    }
#endif
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetPrevTitleId

  Description:  Gets the Title ID of the application that used the OS_JumpToArbitraryApplication function to jump here.
                
  
  Arguments:    None.
 
  Returns:      The Title ID of the application that used the OS_JumpToArbitraryApplication function to jump here.
                Returns 0 if this application was started from the System Menu or debugger, or if it was run on NITRO.
                
                
 *---------------------------------------------------------------------------*/
OSTitleId OSi_GetPrevTitleId(void)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        const OSTitleIDList *list = (const OSTitleIDList *)HW_OS_TITLE_ID_LIST;
        const int   total = MATH_MIN( list->num, OS_TITLEIDLIST_MAX );
        int i;
        for (i = 0; i < total; ++i)
        {
            if (list->TitleID[i] == s_prevTitleId )
            {
                return s_prevTitleId;
            }
        }
    }
#endif
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsBootFromSystemMenu

  Description:  Determines whether this application was started from the System Menu.
  
  Arguments:    None.
 
  Returns:      TRUE: This application was started from the System Menu.
                FALSE: This application was started from another application.
 *---------------------------------------------------------------------------*/
BOOL OS_IsBootFromSystemMenu( void )
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        if( OSi_GetPrevTitleId() == 0 )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IsJumpFromSameMaker

  Description:  Determines whether this application was started from an application with the same maker code.
  
  Arguments:    None.
 
  Returns:      TRUE: This application was started from an application with the same maker code.
                FALSE: This application was started from another application or the System Menu.
 *---------------------------------------------------------------------------*/
BOOL OSi_IsJumpFromSameMaker( void )
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        if( OS_IsBootFromSystemMenu() )
        {
            /* Exclude this application in advance if it was started from the System Menu */
            return FALSE;
        }
    return OSi_IsSameMakerCode( s_prevTitleId );
    }
#endif
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetInitialCode

  Description:  Gets the game code of the running application.
  
  Arguments:    None.
 
  Returns:      The game code.
                This cannot be obtained when run on NITRO, so 0 will be returned.
 *---------------------------------------------------------------------------*/
u32 OSi_GetInitialCode(void)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        return (u32)( OS_GetTitleId() );
    }
#endif
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetPrevInitialCode

  Description:  Gets the game code of the application that used the OS_JumpToArbitraryApplication function to jump here.
                
  
  Arguments:    None.
 
  Returns:      The game code.
                Returns 0 if this application was started from the System Menu or debugger, or if it was run on NITRO because the value cannot be obtained.
                
 *---------------------------------------------------------------------------*/
u32 OSi_GetPrevInitialCode(void)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        return (u32)( OSi_GetPrevTitleId() );
    }
#endif
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsTemporaryApplication

  Description:  Checks whether self is a TMP application.
  
  Arguments:    None.
 
  Returns:      TRUE: TMP application.
                FALSE: Other than TMP applications.
 *---------------------------------------------------------------------------*/
BOOL OS_IsTemporaryApplication(void)
{
#ifdef SDK_TWL
    static BOOL isChecked = FALSE, isTmpApp = FALSE;
    if( OS_IsRunOnTwl() )
    {
        if( isChecked == FALSE ) /* If already checked, flow previous results */
        {
            if( 0 == STD_CompareNString( OS_GetBootSRLPath(), OSi_TMP_APP_PATH_RAW, OSi_TMP_APP_PATH_RAW_LENGTH ) )
            {
                isTmpApp = TRUE;
            }
            else
            {
                isTmpApp = FALSE;
            }
            isChecked = TRUE;
        }
        return isTmpApp;
    }
#endif
    return FALSE;
}

#else // SDK_ARM9
    //----------------------------------------------------------------
    // For ARM7

static BOOL OSi_IsValidLauncherParam( void )
{
    return ( STD_StrNCmp( (const char *)&OSi_GetLauncherParam()->header.magicCode,
                             OSi_LAUNCHER_PARAM_MAGIC_CODE,
                             OSi_LAUNCHER_PARAM_MAGIC_CODE_LEN ) == 0 ) &&
    ( OSi_GetLauncherParam()->header.bodyLength > 0 ) &&
    ( OSi_GetLauncherParam()->header.crc16 == SVC_GetCRC16( 65535, &OSi_GetLauncherParam()->body, OSi_GetLauncherParam()->header.bodyLength ) );
}

static BOOL OSi_IsEnableJTAG( void )
{
    // If SCFG register is invalid, the SCFG register value becomes "0". Check the value retracted to WRAM.
    u8 value = (u8)(( reg_SCFG_EXT & REG_SCFG_EXT_CFG_MASK ) ?
                    ( reg_SCFG_JTAG & REG_SCFG_JTAG_CPUJE_MASK ) :
                    ( *(u8 *)HWi_WSYS09_ADDR & HWi_WSYS09_JTAG_CPUJE_MASK ));
    return value ? TRUE : FALSE;
}

// Launcher parameter lead and determine hot/cold boot
// Return TRUE if launcher parameter is valid; FALSE if invalid
// If the launcher parameter in LauncherParam is valid, that launcher parameter is stored
// TRUE if hot boot in isHotstart; False is stored if a cold boot.
BOOL OS_ReadLauncherParameter( LauncherParam *buf, BOOL *isHotstart )
{
    if( !OSi_GetNandFirmResetParam()->isValid ) {
        *(u8 *)OSi_GetNandFirmResetParam() = (u8)MCU_GetFreeRegister( OS_MCU_RESET_VALUE_OFS );
        OSi_GetNandFirmResetParam()->isValid = 1;
    }
    
    // Hot/cold boot determination
    // If "HOTBT flag of a microcomputer free register = 0," cold boot
    if( !OSi_GetNandFirmResetParam()->isHotStart ) {
        *isHotstart = FALSE;
    }else {
        *isHotstart = TRUE;
        // Determine launcher parameter validity
        if(  OSi_IsValidLauncherParam() &&
            !OSi_GetNandFirmResetParam()->isResetSW
        ) {
            // If launcher parameter is valid, copy to buf
            MI_CpuCopy32 ( OSi_GetLauncherParam(), buf, sizeof(LauncherParam) );
            return TRUE;
        }else {
            // Else clear the transfer destination buffer
            MI_CpuClear32 ( buf, sizeof(LauncherParam) );
        }
    }
    return FALSE;
}

#endif // SDK_ARM9
