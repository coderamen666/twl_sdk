/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/main.h>

#include <nitro/spi.h>
#include <nnsys/misc.h>
#include <nnsys/snd/capture.h>
#include <nnsys/snd/player.h>
#include <nnsys/snd/sndarc_stream.h>
#include <nnsys/snd/resource_mgr.h>
#include <nnsys/snd/config.h>

/******************************************************************************
	Static variables
 ******************************************************************************/

static PMSleepCallbackInfo sPreSleepCallback;
static PMSleepCallbackInfo sPostSleepCallback;

static SNDDriverInfo sDriverInfo[2] ATTRIBUTE_ALIGN( 32 );
static u32 sDriverInfoCommandTag;
static s8 sCurDriverInfo;
static BOOL sDriverInfoFirstFlag;
static BOOL sInitialized = FALSE;

/******************************************************************************
	Static Function Declarations
 ******************************************************************************/
static void BeginSleep( void* );
static void EndSleep( void* );

static const SNDDriverInfo* GetCurDriverInfo( void );

/******************************************************************************
	Inline functions
 ******************************************************************************/
static NNS_SND_INLINE const SNDDriverInfo* GetCurDriverInfo( void )
{
    if ( sCurDriverInfo < 0 ) return NULL;
    return &sDriverInfo[ sCurDriverInfo ];
}

/******************************************************************************
	Public Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndInit

  Description:  Initializes sound library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndInit( void )
{
    // double initialization check
    if ( sInitialized ) return;
    sInitialized = TRUE;
    
    // initialize sound driver
    SND_Init();
    
    // register sleep callback
    PM_SetSleepCallbackInfo( &sPreSleepCallback, BeginSleep, NULL );
    PM_SetSleepCallbackInfo( &sPostSleepCallback, EndSleep, NULL );
    
    PM_PrependPreSleepCallback( &sPreSleepCallback );
    PM_AppendPostSleepCallback( &sPostSleepCallback );
    
    // initialize each library
    NNSi_SndInitResourceMgr();
    NNSi_SndCaptureInit();
    NNSi_SndPlayerInit();
    
    // initialize driver information
    sCurDriverInfo = -1;
    sDriverInfoFirstFlag = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndShutdown

  Description:  Shuts down the sound library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndShutdown( void )
{
    if ( ! sInitialized ) return;

    // Stop all sound.
    NNS_SndStopSoundAll();
    NNS_SndStopChannelAll();
    
    // register sleep callback
    PM_DeletePreSleepCallback( &sPreSleepCallback );
    PM_DeletePostSleepCallback( &sPostSleepCallback );
    
    sInitialized = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndMain

  Description:  sound library framework

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndMain( void )
{
    // receive ARM7 response
    while ( SND_RecvCommandReply( SND_COMMAND_NOBLOCK ) != NULL ) {}
    
    // Framework for each library
    NNSi_SndPlayerMain();
    NNSi_SndCaptureMain();
#ifndef SDK_SMALL_BUILD    
    NNSi_SndArcStrmMain();
#endif /* SDK_SMALL_BUILD */
    
    // Issue ARM7 command
    (void)SND_FlushCommand( SND_COMMAND_NOBLOCK );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndSetMasterVolume

  Description:  set master volume

  Arguments:    volume - master volume

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndSetMasterVolume( int volume )
{
    NNS_MINMAX_ASSERT( volume, 0, SND_MASTER_VOLUME_MAX );
    
    SND_SetMasterVolume( volume );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndSetMonoFlag

  Description:  Changes the monaural flag.

  Arguments:    flag - whether to enable monaural

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndSetMonoFlag( BOOL flag )
{
    if ( flag ) SND_SetMasterPan( SND_CHANNEL_PAN_CENTER );
    else SND_ResetMasterPan();
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndStopSoundAll

  Description:  Immediately stops all sounds.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndStopSoundAll( void )
{
    u32 commandTag;
    
    NNS_SndPlayerStopSeqAll( 0 );
#ifndef SDK_SMALL_BUILD
    NNS_SndArcStrmStopAll( 0 );
#endif /* SDK_SMALL_BUILD */    
    NNSi_SndCaptureStop();
    
    SNDi_SetSurroundDecay( 0 );    
    SND_StopTimer( 0xffff, 0xffff, 0xffff, 0 );
    
    // wait for process done with ARM7
    commandTag = SND_GetCurrentCommandTag();
    (void)SND_FlushCommand( SND_COMMAND_BLOCK );
    SND_WaitForCommandProc( commandTag );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndStopChannelAll

  Description:  Stops all channels.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndStopChannelAll( void )
{
    SND_StopUnlockedChannel( 0xffff, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndUpdateDriverInfo

  Description:  Updates driver information.

  Arguments:    None.

  Returns:      whether the update was successful
 *---------------------------------------------------------------------------*/
BOOL NNS_SndUpdateDriverInfo( void )
{
    if ( ! sDriverInfoFirstFlag )
    {
        // receive ARM7 response
        while ( SND_RecvCommandReply( SND_COMMAND_NOBLOCK ) != NULL ) {}
        
        if ( ! SND_IsFinishedCommandTag( sDriverInfoCommandTag ) ) {
            // update incomplete
            return FALSE;
        }
        
        // update completed
        if ( sCurDriverInfo < 0 ) sCurDriverInfo = 1;
        
        SND_ReadDriverInfo( &sDriverInfo[ sCurDriverInfo ] );
        sDriverInfoCommandTag = SND_GetCurrentCommandTag();
        
        if ( sCurDriverInfo == 0 ) sCurDriverInfo = 1;
        else sCurDriverInfo = 0;
        
        DC_InvalidateRange( &sDriverInfo[ sCurDriverInfo ], sizeof( SNDDriverInfo ) );
        
        // Issue ARM7 command
        (void)SND_FlushCommand( SND_COMMAND_NOBLOCK );
        
        return TRUE;
    }
    else
    {
        // first update
        SND_ReadDriverInfo( &sDriverInfo[0] );
        sDriverInfoCommandTag = SND_GetCurrentCommandTag();
        sDriverInfoFirstFlag = FALSE;
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndReadDriverChannelInfo

  Description:  Gets channel information.

  Arguments:    chNo - channel number
                info - channel information structure that holds the obtained information

  Returns:      Whether successfully obtained
 *---------------------------------------------------------------------------*/
BOOL NNS_SndReadDriverChannelInfo( int chNo, SNDChannelInfo* info )
{
    const SNDDriverInfo* driverInfo;
    
    NNS_NULL_ASSERT( info );
    
    driverInfo = GetCurDriverInfo();
    if ( driverInfo == NULL ) return FALSE;
    
    return SND_ReadChannelInfo( driverInfo, chNo, info );
}

/******************************************************************************
	Private Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndReadDriverPlayerInfo

  Description:  Gets player information.

  Arguments:    playerNo - player number (physical number)
                info - player information structure that holds the obtained information

  Returns:      Whether successfully obtained
 *---------------------------------------------------------------------------*/
BOOL NNSi_SndReadDriverPlayerInfo( int playerNo, SNDPlayerInfo* info )
{
    const SNDDriverInfo* driverInfo;
    
    NNS_NULL_ASSERT( info );
    
    driverInfo = GetCurDriverInfo();
    if ( driverInfo == NULL ) return FALSE;

    return SND_ReadPlayerInfo( driverInfo, playerNo, info );
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndReadDriverTrackInfo

  Description:  Gets track information.

  Arguments:    playerNo - player number (physical number)
                trackNo - track number
                info - track information structure that holds the obtained information

  Returns:      Whether successfully obtained
 *---------------------------------------------------------------------------*/
BOOL NNSi_SndReadDriverTrackInfo( int playerNo, int trackNo, SNDTrackInfo* info )
{
    const SNDDriverInfo* driverInfo;
    
    NNS_NULL_ASSERT( info );
    
    driverInfo = GetCurDriverInfo();
    if ( driverInfo == NULL ) return FALSE;
    
    return SND_ReadTrackInfo( driverInfo, playerNo, trackNo, info );
}

/******************************************************************************
	Static Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         BeginSleep

  Description:  Prepares for sleep mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BeginSleep( void* )
{
    u32 commandTag;
    
    NNSi_SndCaptureBeginSleep();
    
    // Sends dummy commands to empty command buffer.
    SND_StopTimer( 0, 0, 0, 0 );
    
    // wait for process done with ARM7
    commandTag = SND_GetCurrentCommandTag();
    (void)SND_FlushCommand( SND_COMMAND_BLOCK );
    SND_WaitForCommandProc( commandTag );
}

/*---------------------------------------------------------------------------*
  Name:         EndSleep

  Description:  processes after sleep mode

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void EndSleep( void* )
{
    NNSi_SndCaptureEndSleep();
}

/*====== End of main.c ======*/

