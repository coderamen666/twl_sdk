/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     player.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/player.h>

#include <nitro/os.h>
#include <nitro/snd.h>
#include <nnsys/misc.h>
#include <nnsys/snd/main.h>

/******************************************************************************
    Macro Definitions
 ******************************************************************************/

#define FADER_SHIFT 8

/******************************************************************************
    Structure Definitions
 ******************************************************************************/

typedef struct NNSSndPlayerHeap
{
    NNSFndLink link;
    NNSSndHeapHandle handle;
    NNSSndSeqPlayer* player;
    int playerNo;
} NNSSndPlayerHeap;

/******************************************************************************
    Static variables
 ******************************************************************************/

static NNSSndSeqPlayer sSeqPlayer[ SND_PLAYER_NUM ];
static NNSSndPlayer sPlayer[ NNS_SND_PLAYER_NUM ];
static NNSFndList sPrioList;
static NNSFndList sFreeList;

/******************************************************************************
    static function declarations
 ******************************************************************************/

static void InitPlayer( NNSSndSeqPlayer* seqPlayer );
static void ShutdownPlayer( NNSSndSeqPlayer* seqPlayer );
static void ForceStopSeq( NNSSndSeqPlayer* seqPlayer );
static NNSSndSeqPlayer* AllocSeqPlayer( int prio );
static void InsertPlayerList( NNSSndPlayer* player, NNSSndSeqPlayer* seqPlayer );
static void InsertPrioList( NNSSndSeqPlayer* seqPlayer );
static void SetPlayerPriority( NNSSndSeqPlayer* seqPlayer, int priority );
static void PlayerHeapDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );

/******************************************************************************
    Public Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetPlayerVolume

  Description:  Volume setting for each player,

  Arguments:    playerNo - player number
                volume - Volume.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetPlayerVolume( int playerNo, int volume )
{
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( volume, 0, SND_CALC_DECIBEL_SCALE_MAX );
    
    sPlayer[ playerNo ].volume = (u8)volume;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetPlayableSeqCount

  Description:  Sets the maximum number of sequences that can be played back simultaneously.

  Arguments:    playerNo - player number
                seqCount - maximum number of sequences that can be played back simultaneously

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetPlayableSeqCount( int playerNo, int seqCount )
{
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( seqCount, 0, SND_PLAYER_NUM );
    
    sPlayer[ playerNo ].playableSeqCount = (u16)seqCount;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetAllocatableChannel

  Description:  Sets the allocatable channels.

  Arguments:    playerNo - player number
                chBitFlag - bit flag for the allocatable channel
                            Specifying 0 makes them all allocatable.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetAllocatableChannel( int playerNo, u32 chBitFlag )
{
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( chBitFlag, 0, 0xffff );
    
    sPlayer[ playerNo ].allocChBitFlag = chBitFlag;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerCreateHeap

  Description:  Creates the player heap.

  Arguments:    playerNo - player number
                heap - sound heap
                size - size of the player heap

  Returns:      whether the player heap was created successfully
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerCreateHeap( int playerNo, NNSSndHeapHandle heap, u32 size )
{
    NNSSndHeapHandle playerHeapHandle;
    NNSSndPlayerHeap* playerHeap;    
    void* buffer;
    
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    
    /* Allocates a buffer for use as the player heap. */
    buffer = NNS_SndHeapAlloc( heap, sizeof( NNSSndPlayerHeap ) + size, PlayerHeapDisposeCallback, 0, 0 );
    if ( buffer == NULL ) {
        return FALSE;
    }
    
    /* initialize the player heap structure */
    playerHeap = (NNSSndPlayerHeap*)buffer;
    
    playerHeap->player = NULL;
    playerHeap->playerNo = playerNo;
    playerHeap->handle = NNS_SND_HEAP_INVALID_HANDLE;
    
    /* build player heap */
    playerHeapHandle = NNS_SndHeapCreate(
        (u8*)buffer + sizeof( NNSSndPlayerHeap ),
        size
    );
    if ( playerHeapHandle == NNS_SND_HEAP_INVALID_HANDLE ) {
        return FALSE;
    }
    
    playerHeap->handle = playerHeapHandle;    
    NNS_FndAppendListObject( &sPlayer[ playerNo ].heapList, playerHeap );
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeq

  Description:  Stops the sequence.

  Arguments:    handle - sound handle
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeq( NNSSndHandle* handle, int fadeFrame )
{
    NNS_NULL_ASSERT( handle );
    
    NNSi_SndPlayerStopSeq( handle->player, fadeFrame );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeqByPlayerNo

  Description:  Stops the sequence.

  Arguments:    playerNo - player number
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeqByPlayerNo( int playerNo, int fadeFrame )
{
    NNSSndSeqPlayer* seqPlayer;
    int i;
    
    NNS_MINMAX_ASSERT( playerNo , 0, NNS_SND_PLAYER_NO_MAX );
    
    for( i = 0; i < SND_PLAYER_NUM ; i++ )
    {
        seqPlayer = & sSeqPlayer[ i ];
        
        if ( seqPlayer->status != NNS_SND_SEQ_PLAYER_STATUS_STOP &&
             seqPlayer->player == & sPlayer[ playerNo ] )
        {
            NNSi_SndPlayerStopSeq( seqPlayer, fadeFrame );            
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeqBySeqNo

  Description:  Stops the sequence.

  Arguments:    seqNo      - sequence number
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeqBySeqNo( int seqNo, int fadeFrame )
{
    NNSSndSeqPlayer* seqPlayer;
    int i;
    
    for( i = 0; i < SND_PLAYER_NUM ; i++ )
    {
        seqPlayer = & sSeqPlayer[ i ];
        
        if ( seqPlayer->status != NNS_SND_SEQ_PLAYER_STATUS_STOP &&
             seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQ &&
             seqPlayer->seqNo == seqNo )
        {
            NNSi_SndPlayerStopSeq( seqPlayer ,fadeFrame );
        }
    }    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeqBySeqArcNo

  Description:  Stops the sequence.

  Arguments:    seqArcNo   - sequence archive number
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeqBySeqArcNo( int seqArcNo, int fadeFrame )
{
    NNSSndSeqPlayer* seqPlayer;
    int i;
    
    for( i = 0; i < SND_PLAYER_NUM ; i++ )
    {
        seqPlayer = & sSeqPlayer[ i ];
        
        if ( seqPlayer->status != NNS_SND_SEQ_PLAYER_STATUS_STOP &&
             seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQARC &&
             seqPlayer->seqNo == seqArcNo )
        {
            NNSi_SndPlayerStopSeq( seqPlayer ,fadeFrame );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeqBySeqArcIdx

  Description:  Stops the sequence.

  Arguments:    seqArcNo   - sequence archive number
                index      - index number
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeqBySeqArcIdx( int seqArcNo, int index, int fadeFrame )
{
    NNSSndSeqPlayer* seqPlayer;
    int i;
    
    for( i = 0; i < SND_PLAYER_NUM ; i++ )
    {
        seqPlayer = & sSeqPlayer[ i ];
        
        if ( seqPlayer->status != NNS_SND_SEQ_PLAYER_STATUS_STOP &&
             seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQARC &&
             seqPlayer->seqNo == seqArcNo &&
             seqPlayer->seqArcIndex == index )
        {
            NNSi_SndPlayerStopSeq( seqPlayer ,fadeFrame );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerStopSeqAll

  Description:  Stops all sequences.

  Arguments:    fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerStopSeqAll( int fadeFrame )
{
    NNSSndSeqPlayer* seqPlayer;
    int i;
    
    for( i = 0; i < SND_PLAYER_NUM ; i++ )
    {
        seqPlayer = & sSeqPlayer[ i ];
        
        if ( seqPlayer->status != NNS_SND_SEQ_PLAYER_STATUS_STOP )
        {
            NNSi_SndPlayerStopSeq( seqPlayer, fadeFrame );
        }
    }    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerPause

  Description:  Pauses or restarts a sequence.

  Arguments:    handle - sound handle
                flag     - pause or resume

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerPause( NNSSndHandle* handle, BOOL flag )
{
    NNS_NULL_ASSERT( handle );

    NNSi_SndPlayerPause( handle->player, flag );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerPauseByPlayerNo

  Description:  Pauses or restarts a sequence.

  Arguments:    playerNo - player number
                flag     - pause or resume

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerPauseByPlayerNo( int playerNo, BOOL flag )
{
    NNSSndSeqPlayer* seqPlayer;
    NNSSndSeqPlayer* next;
    
    NNS_MINMAX_ASSERT( playerNo , 0, NNS_SND_PLAYER_NO_MAX );
    
    for( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPlayer[ playerNo ].playerList, NULL );
         seqPlayer != NULL ; seqPlayer = next )
    {
        next = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPlayer[ playerNo ].playerList, seqPlayer );
        
        NNSi_SndPlayerPause( seqPlayer, flag );
    }    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerPauseAll

  Description:  Pauses or restarts a sequence.

  Arguments:    flag     - pause or resume

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerPauseAll( BOOL flag )
{
    NNSSndSeqPlayer* seqPlayer;
    NNSSndSeqPlayer* next;
    
    for( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, NULL );
         seqPlayer != NULL ; seqPlayer = next )
    {
        next = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, seqPlayer );
        
        NNSi_SndPlayerPause( seqPlayer, flag );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHandleInit

  Description:  Initializes the sound handle.

  Arguments:    handle - sound handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndHandleInit( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );
    
    handle->player = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHandleReleaseSeq

  Description:  Release the sequence from a sound handle.

  Arguments:    handle - sound handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndHandleReleaseSeq( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    NNS_ASSERT( handle == handle->player->handle );
    
    handle->player->handle = NULL;  
    handle->player = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerCountPlayingSeqByPlayerNo

  Description:  Gets the number of sequences being played back.

  Arguments:    playerNo - player number

  Returns:      number of sequences being played back
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerCountPlayingSeqByPlayerNo( int playerNo )
{
    NNS_MINMAX_ASSERT( playerNo , 0, NNS_SND_PLAYER_NO_MAX );
    
    return sPlayer[ playerNo ].playerList.numObjects;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerCountPlayingSeqBySeqNo

  Description:  Gets the number of sequences being played back.

  Arguments:    seqNo      - sequence number

  Returns:      number of sequences being played back
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerCountPlayingSeqBySeqNo( int seqNo )
{
    int count = 0;
    
    NNSSndSeqPlayer* seqPlayer = NULL;
    while ( ( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, seqPlayer ) ) != NULL )
    {
        if ( seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQ &&
             seqPlayer->seqNo == seqNo )
        {
            count++;
        }
    }
    
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerCountPlayingSeqBySeqArcNo

  Description:  Gets the number of sequences being played back.

  Arguments:    seqArcNo   - sequence archive number

  Returns:      number of sequences being played back
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerCountPlayingSeqBySeqArcNo( int seqArcNo )
{
    int count = 0;
    
    NNSSndSeqPlayer* seqPlayer = NULL;
    while ( ( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, seqPlayer ) ) != NULL )
    {
        if ( seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQARC &&
             seqPlayer->seqNo == seqArcNo )
        {
            count++;
        }
    }
    
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerCountPlayingSeqBySeqArcIdx

  Description:  Gets the number of sequences being played back.

  Arguments:    seqArcNo   - sequence archive number
                index      - index number

  Returns:      number of sequences being played back
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerCountPlayingSeqBySeqArcIdx( int seqArcNo, int index )
{
    int count = 0;
    
    NNSSndSeqPlayer* seqPlayer = NULL;
    while ( ( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, seqPlayer ) ) != NULL )
    {
        if ( seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQARC &&
             seqPlayer->seqNo == seqArcNo &&
             seqPlayer->seqArcIndex == index )
        {
            count++;
        }
    }
    
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetVolume

  Description:  Updates sequence volume.

  Arguments:    handle - sound handle
                volume - Volume.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetVolume( NNSSndHandle* handle, int volume )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( volume, 0, SND_CALC_DECIBEL_SCALE_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    handle->player->extVolume = (u8)volume;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetInitialVolume

  Description:  Sets the initial sequence volume.

  Arguments:    handle - sound handle
                volume - Volume.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetInitialVolume( NNSSndHandle* handle, int volume )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( volume, 0, SND_CALC_DECIBEL_SCALE_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    handle->player->initVolume = (u8)volume;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerMoveVolume

  Description:  Changes sequence volume over a period of time.

  Arguments:    handle - sound handle
                targetVolume - target volume
                frames - Number of frames whose volume is to be changed.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerMoveVolume( NNSSndHandle* handle, int targetVolume, int frames )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( targetVolume, 0, SND_CALC_DECIBEL_SCALE_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    // prohibited during fadeout    
    if ( handle->player->status == NNS_SND_SEQ_PLAYER_STATUS_FADEOUT ) return;
    
    NNSi_SndFaderSet( & handle->player->fader, targetVolume << FADER_SHIFT, frames );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetPlayerPriority

  Description:  Change the player priority.

  Arguments:    handle - sound handle
                priority - Player priority.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetPlayerPriority( NNSSndHandle* handle, int priority )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( priority, 0, NNS_SND_PLAYER_PRIO_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SetPlayerPriority( handle->player, priority );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetChannelPriority

  Description:  Changes voicing priority.

  Arguments:    handle - sound handle
                priority - voicing priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetChannelPriority( NNSSndHandle* handle, int priority )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( priority, 0, 127 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetPlayerChannelPriority( handle->player->playerNo, priority );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackMute

  Description:  Mutes tracks.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                flag         - mute flag

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackMute( NNSSndHandle* handle, u16 trackBitMask, BOOL flag )
{
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackMute(
        handle->player->playerNo,
        trackBitMask,
        flag
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackMuteEx

  Description:  Mutes tracks.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                mute         - mute settings

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if SDK_CURRENT_VERSION_NUMBER >= SDK_VERSION_NUMBER(3,1,0)
void NNS_SndPlayerSetTrackMuteEx( NNSSndHandle* handle, u16 trackBitMask, NNSSndSeqMute mute )
{
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackMuteEx(
        handle->player->playerNo,
        trackBitMask,
        (SNDSeqMute)mute
    );
}
void SND_SetTrackMuteEx(int playerNo, u32 trackBitMask, SNDSeqMute mute);
SDK_WEAK_SYMBOL void SND_SetTrackMuteEx(int playerNo, u32 trackBitMask, SNDSeqMute mute)
{
    (void)playerNo;
    (void)trackBitMask;
    (void)mute;
    
    NNS_WARNING( FALSE, "SND_SetTrackMuteEx is not supported." );
}
#endif

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackVolume

  Description:  Change track volume.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                volume - Volume.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackVolume( NNSSndHandle* handle, u16 trackBitMask, int volume )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( volume, 0, SND_CALC_DECIBEL_SCALE_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackVolume(
        handle->player->playerNo,
        trackBitMask,
        SND_CalcDecibel( volume )
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackPitch

  Description:  Change track pitch.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                pitch        - pitch

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackPitch( NNSSndHandle* handle, u16 trackBitMask, int pitch )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( pitch, NNS_SND_PLAYER_TRACK_PITCH_MIN, NNS_SND_PLAYER_TRACK_PITCH_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackPitch( handle->player->playerNo, trackBitMask, pitch );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackPan

  Description:  Change track pan.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                pan - Pan.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackPan( NNSSndHandle* handle, u16 trackBitMask, int pan )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( pan, NNS_SND_PLAYER_TRACK_PAN_MIN, NNS_SND_PLAYER_TRACK_PAN_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackPan( handle->player->playerNo, trackBitMask, pan );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackPanRange

  Description:  Changes the track pan range.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                panRange    - pan range

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackPanRange( NNSSndHandle* handle, u16 trackBitMask, int panRange )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( panRange, NNS_SND_PLAYER_TRACK_PAN_RANGE_MIN, NNS_SND_PLAYER_TRACK_PAN_RANGE_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackPanRange( handle->player->playerNo, trackBitMask, panRange );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackModDepth

  Description:  Changes the modulation depth.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                depth        - modulation depth

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackModDepth( NNSSndHandle* handle, u16 trackBitMask, int depth )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( depth, 0, 255 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackModDepth( handle->player->playerNo, trackBitMask, depth );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackModSpeed

  Description:  Changes the modulation speed.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                speed        - modulation speed

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackModSpeed( NNSSndHandle* handle, u16 trackBitMask, int speed )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( speed, 0, 255 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackModSpeed( handle->player->playerNo, trackBitMask, speed );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTrackAllocatableChannel

  Description:  Changes the channel which can be allocated.

  Arguments:    handle - sound handle
                trackBitMask - track bit mask
                chBitFlag    - bit flag for the allocatable channel
                               Specifying 0 makes them all allocatable.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTrackAllocatableChannel( NNSSndHandle* handle, u16 trackBitMask, u32 chBitFlag )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( chBitFlag, 0, 0xffff );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetTrackAllocatableChannel( handle->player->playerNo, trackBitMask, chBitFlag );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetTempoRatio

  Description:  Change the tempo ratio.

  Arguments:    handle - sound handle
                ratio  - tempo ratio

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetTempoRatio( NNSSndHandle* handle, int ratio )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( ratio, 0, 65535 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    SND_SetPlayerTempoRatio( handle->player->playerNo, ratio );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetSeqNo

  Description:  Sets the sequence number.

  Arguments:    handle - sound handle
                seqNo      - sequence number

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetSeqNo( NNSSndHandle* handle, int seqNo )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( seqNo, 0, NNS_SND_PLAYER_SEQ_NO_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    handle->player->seqType = NNS_SND_PLAYER_SEQ_TYPE_SEQ;
    handle->player->seqNo = (u16)seqNo;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerSetSeqArcNo

  Description:  Sets the sequence archive number.

  Arguments:    handle - sound handle
                seqArcNo   - sequence archive number
                index      - index number

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndPlayerSetSeqArcNo( NNSSndHandle* handle, int seqArcNo, int index )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( seqArcNo, 0, NNS_SND_PLAYER_SEQ_NO_MAX );
    NNS_MINMAX_ASSERT( index, 0, NNS_SND_PLAYER_SEQARC_INDEX_MAX );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return;
    
    handle->player->seqType = NNS_SND_PLAYER_SEQ_TYPE_SEQARC;
    handle->player->seqNo       = (u16)seqArcNo;
    handle->player->seqArcIndex = (u16)index;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerGetSeqType

  Description:  Gets the sequence type.

  Arguments:    handle - sound handle

  Returns:      sequence type
 *---------------------------------------------------------------------------*/
NNSSndPlayerSeqType NNS_SndPlayerGetSeqType( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return NNS_SND_PLAYER_SEQ_TYPE_INVALID;
    
    return (NNSSndPlayerSeqType)( handle->player->seqType );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerGetSeqNo

  Description:  Gets the sequence number.

  Arguments:    handle - sound handle

  Returns:      the sequence number or -1
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerGetSeqNo( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );

    if ( ! NNS_SndHandleIsValid( handle ) ) return -1;
    
    if ( handle->player->seqType != NNS_SND_PLAYER_SEQ_TYPE_SEQ ) return -1;

    return handle->player->seqNo;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerGetSeqArcNo

  Description:  Gets the sequence archive number.

  Arguments:    handle - sound handle

  Returns:      the sequence archive number or -1
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerGetSeqArcNo( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );

    if ( ! NNS_SndHandleIsValid( handle ) ) return -1;
    
    if ( handle->player->seqType != NNS_SND_PLAYER_SEQ_TYPE_SEQARC ) return -1;

    return handle->player->seqNo;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerGetSeqArcIdx

  Description:  Gets the sequence archive index.

  Arguments:    handle - sound handle

  Returns:      the sequence archive index or -1
 *---------------------------------------------------------------------------*/
int NNS_SndPlayerGetSeqArcIdx( NNSSndHandle* handle )
{
    NNS_NULL_ASSERT( handle );

    if ( ! NNS_SndHandleIsValid( handle ) ) return -1;
    
    if ( handle->player->seqType != NNS_SND_PLAYER_SEQ_TYPE_SEQARC ) return -1;
    
    return handle->player->seqArcIndex;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerReadVariable

  Description:  Read the sequence local variables.

  Arguments:    handle - sound handle
                varNo   - variable number
                var    - address the variable is to be loaded into

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerReadVariable( NNSSndHandle* handle, int varNo, s16* var )
{
    NNSSndSeqPlayer* seqPlayer;
    
    NNS_NULL_ASSERT( handle );
    NNS_NULL_ASSERT( var );
    NNS_MINMAX_ASSERT( varNo, 0, SND_PLAYER_VARIABLE_NUM-1 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return FALSE;
    
    seqPlayer = handle->player;
    
    if ( ! seqPlayer->startFlag ) {
        *var = SND_DEFAULT_VARIABLE;
        return TRUE;
    }
    
    *var = SND_GetPlayerLocalVariable( seqPlayer->playerNo, varNo );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerReadGlobalVariable

  Description:  Read the sequence global variables.

  Arguments:    varNo   - variable number
                var    - address the variable is to be loaded into

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerReadGlobalVariable( int varNo, s16* var )
{
    NNS_NULL_ASSERT( var );
    NNS_MINMAX_ASSERT( varNo, 0, SND_GLOBAL_VARIABLE_NUM-1 );
    
    *var = SND_GetPlayerGlobalVariable( varNo );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerWriteVariable

  Description:  Write the sequence local variables.

  Arguments:    handle - sound handle
                varNo   - variable number
                var    - value to be written

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerWriteVariable( NNSSndHandle* handle, int varNo, s16 var )
{
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( varNo, 0, SND_PLAYER_VARIABLE_NUM-1 );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return FALSE;
    
    SND_SetPlayerLocalVariable( handle->player->playerNo, varNo, var );

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerWriteGlobalVariable

  Description:  Write the sequence global variables.

  Arguments:    varNo   - variable number
                var    - value to be written

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerWriteGlobalVariable( int varNo, s16 var )
{
    NNS_MINMAX_ASSERT( varNo, 0, SND_GLOBAL_VARIABLE_NUM-1 );
    
    SND_SetPlayerGlobalVariable( varNo, var );

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerGetTick

  Description:  Get the number of ticks since playback began.

  Arguments:    handle - sound handle

  Returns:      number of ticks
 *---------------------------------------------------------------------------*/
u32 NNS_SndPlayerGetTick( NNSSndHandle* handle )
{
    NNSSndSeqPlayer* seqPlayer;
    
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return 0;
    
    seqPlayer = handle->player;
    
    if ( ! seqPlayer->startFlag ) {
        // pre start
        return 0;
    }
    
    return SND_GetPlayerTickCounter( seqPlayer->playerNo );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerReadDriverPlayerInfo

  Description:  Gets player information for the driver.

  Arguments:    handle - sound handle
                info - player information structure that holds the obtained player information

  Returns:      whether the information was retrieved successfully
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerReadDriverPlayerInfo( NNSSndHandle* handle, SNDPlayerInfo* info )
{
    NNSSndSeqPlayer* seqPlayer;
    
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return FALSE;
    
    seqPlayer = handle->player;
    NNS_NULL_ASSERT( seqPlayer );
    
    return NNSi_SndReadDriverPlayerInfo( seqPlayer->playerNo, info );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndPlayerReadDriverTrackInfo

  Description:  Gets track information for the driver.

  Arguments:    handle - sound handle
                trackNo - track number
                info - track information structure that holds the obtained player information

  Returns:      whether the information was retrieved successfully
 *---------------------------------------------------------------------------*/
BOOL NNS_SndPlayerReadDriverTrackInfo( NNSSndHandle* handle, int trackNo, SNDTrackInfo* info )
{
    NNSSndSeqPlayer* seqPlayer;
    
    NNS_NULL_ASSERT( handle );
    
    if ( ! NNS_SndHandleIsValid( handle ) ) return FALSE;
    
    seqPlayer = handle->player;
    NNS_NULL_ASSERT( seqPlayer );
    
    return NNSi_SndReadDriverTrackInfo( seqPlayer->playerNo, trackNo, info );
}

/******************************************************************************
    Private Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerInit

  Description:  Initializes the player library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerInit( void )
{
    NNSSndPlayer* player;
    int playerNo;
    
    NNS_FND_INIT_LIST( &sPrioList, NNSSndSeqPlayer, prioLink );
    NNS_FND_INIT_LIST( &sFreeList, NNSSndSeqPlayer, prioLink );
    
    for( playerNo = 0; playerNo < SND_PLAYER_NUM ; playerNo++ )
    {
        sSeqPlayer[ playerNo ].status = NNS_SND_SEQ_PLAYER_STATUS_STOP;
        sSeqPlayer[ playerNo ].playerNo = (u8)playerNo;
        NNS_FndAppendListObject( & sFreeList, & sSeqPlayer[ playerNo ] );
    }
    
    for( playerNo = 0; playerNo < NNS_SND_PLAYER_NUM ; playerNo++ )
    {
        player = &sPlayer[ playerNo ];
        
        NNS_FND_INIT_LIST( & player->playerList, NNSSndSeqPlayer, playerLink );
        NNS_FND_INIT_LIST( & player->heapList, NNSSndPlayerHeap, link );
        player->volume = 127;
        player->playableSeqCount = 1;
        player->allocChBitFlag = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerMain

  Description:  Framework for the player library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerMain( void )
{
    NNSSndSeqPlayer* seqPlayer;
    NNSSndSeqPlayer* next;
    u32 status;
    int fader;
    
    // Gets the player's status
    status = SND_GetPlayerStatus();
    
    for( seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, NULL );
         seqPlayer != NULL ; seqPlayer = next )
    {
        next = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, seqPlayer );
        
        // check start flag
        if ( ! seqPlayer->startFlag ) {
            if ( SND_IsFinishedCommandTag( seqPlayer->commandTag ) )
            {
                seqPlayer->startFlag = TRUE;
            }
        }
        
        // stop check from ARM7 side
        if ( seqPlayer->startFlag )
        {
            if ( ( status & ( 1 << seqPlayer->playerNo ) ) == 0 ) {
                ShutdownPlayer( seqPlayer );
                continue;
            }
        }
        
        // update fader
        NNSi_SndFaderUpdate( & seqPlayer->fader );
        
        // Update parameter.
        fader
            = SND_CalcDecibel( seqPlayer->initVolume )
            + SND_CalcDecibel( seqPlayer->extVolume )
            + SND_CalcDecibel( seqPlayer->player->volume )
            + SND_CalcDecibel( NNSi_SndFaderGet( & seqPlayer->fader ) >> FADER_SHIFT )
            ;
        if ( fader < -32768 ) fader = -32768;
        else if ( fader > 32767 ) fader = 32767;
        
        if ( fader != seqPlayer->volume )
        {
            SND_SetPlayerVolume( seqPlayer->playerNo, fader );
            seqPlayer->volume = (s16)fader;
        }
        
        // Fadeout completion check.
        if ( seqPlayer->status == NNS_SND_SEQ_PLAYER_STATUS_FADEOUT )
        {
            if ( NNSi_SndFaderIsFinished( & seqPlayer->fader ) )
            {
                ForceStopSeq( seqPlayer );
            }
        }
        
        // prepare complete flag
        if ( seqPlayer->prepareFlag ) {
            SND_StartPreparedSeq( seqPlayer->playerNo );
            seqPlayer->prepareFlag = FALSE;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerAllocSeqPlayer

  Description:  Allocates a sequence player.

  Arguments:    handle - sound handle
                playerNo - player number
                prio     - player priority

  Returns:      allocated sequence player
                NULL on failure
 *---------------------------------------------------------------------------*/
NNSSndSeqPlayer* NNSi_SndPlayerAllocSeqPlayer( NNSSndHandle* handle, int playerNo, int prio )
{
    NNSSndSeqPlayer* seqPlayer;
    NNSSndPlayer* player;

    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( prio, 0, NNS_SND_PLAYER_PRIO_MAX );
    
    player = & sPlayer[ playerNo ];
    
    // disconnect sound handle
    if ( NNS_SndHandleIsValid( handle ) ) {
        NNS_SndHandleReleaseSeq( handle );
    }
    
    // check the maximum number of sounds playable simultaneously for each player
    if ( player->playerList.numObjects >= player->playableSeqCount )
    {
        // get the lowest priority object
        seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & player->playerList, NULL );
        if ( seqPlayer == NULL ) return NULL;
        if ( prio < seqPlayer->prio ) return NULL;
        
        ForceStopSeq( seqPlayer );
    }
    
    // allocation processing
    seqPlayer = AllocSeqPlayer( prio );
    if ( seqPlayer == NULL ) return NULL;
    
    // Initialization
    InsertPlayerList( player, seqPlayer );
    
    // connect to sound handle
    seqPlayer->handle = handle;
    handle->player = seqPlayer;
    
    return seqPlayer;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerFreeSeqPlayer

  Description:  Deallocates the sequence player.

  Arguments:    seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerFreeSeqPlayer( NNSSndSeqPlayer* seqPlayer )
{
    NNS_NULL_ASSERT( seqPlayer );

    ShutdownPlayer( seqPlayer );
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerStartSeq

  Description:  Starts a sequence.

  Arguments:    seqPlayer  - sequence player
                seqDataBase   - base address of sequence data
                seqDataOffset - sequence data offset
                bank          - bank data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerStartSeq(
    NNSSndSeqPlayer* seqPlayer,
    const void* seqDataBase,
    u32 seqDataOffset,
    const SNDBankData* bank
)
{
    NNSSndPlayer* player;
    
    NNS_NULL_ASSERT( seqPlayer );
    NNS_NULL_ASSERT( seqDataBase );
    NNS_NULL_ASSERT( bank );
    
    player = seqPlayer->player;
    NNS_NULL_ASSERT( player );
    
    SND_PrepareSeq(
        seqPlayer->playerNo,
        seqDataBase,
        seqDataOffset,
        bank
    );
    if ( player->allocChBitFlag ) {
        SND_SetTrackAllocatableChannel(
            seqPlayer->playerNo,
            0xffff,
            player->allocChBitFlag
        );
    }
    
    // Initialization process
    InitPlayer( seqPlayer );
    seqPlayer->commandTag = SND_GetCurrentCommandTag();
    seqPlayer->prepareFlag = TRUE;
    seqPlayer->status = NNS_SND_SEQ_PLAYER_STATUS_PLAY;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerStopSeq

  Description:  Stops the sequence.

  Arguments:    seqPlayer  - sequence player
                fadeFrame - fade-out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerStopSeq( NNSSndSeqPlayer* seqPlayer, int fadeFrame )
{
    if ( seqPlayer == NULL ) return;
    if ( seqPlayer->status == NNS_SND_SEQ_PLAYER_STATUS_STOP ) return;
    
    if ( fadeFrame == 0 )
    {
        ForceStopSeq( seqPlayer );
        return;
    }
    
    NNSi_SndFaderSet( & seqPlayer->fader, 0, fadeFrame );
    
    SetPlayerPriority( seqPlayer, 0 );
    
    seqPlayer->status = NNS_SND_SEQ_PLAYER_STATUS_FADEOUT;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerPause

  Description:  Pauses or restarts a sequence.

  Arguments:    seqPlayer  - sequence player
                flag      - Pause or restart.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndPlayerPause( NNSSndSeqPlayer* seqPlayer, BOOL flag )
{
    if ( seqPlayer == NULL ) return;
    
    if ( flag != seqPlayer->pauseFlag )
    {
        SND_PauseSeq( seqPlayer->playerNo, flag );        
        seqPlayer->pauseFlag = (u8)flag;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndPlayerAllocHeap

  Description:  Allocates a player heap.

  Arguments:    playerNo - player number
                seqPlayer - sequence player that will use the heap

  Returns:      allocated heap
 *---------------------------------------------------------------------------*/
NNSSndHeapHandle NNSi_SndPlayerAllocHeap( int playerNo, NNSSndSeqPlayer* seqPlayer )
{
    NNSSndPlayer* player;
    NNSSndPlayerHeap* heap;
    
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    
    player = & sPlayer[ playerNo ];
    
    heap = (NNSSndPlayerHeap*)NNS_FndGetNextListObject( & player->heapList, NULL );
    if ( heap == NULL ) return NULL;
    
    NNS_FndRemoveListObject( & player->heapList, heap );
    
    heap->player = seqPlayer;
    seqPlayer->heap = heap;
    
    NNS_SndHeapClear( heap->handle );
    
    return heap->handle;
}

/******************************************************************************
    Static Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         InitPlayer

  Description:  Initializes the sequence player.

  Arguments:    seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitPlayer( NNSSndSeqPlayer* seqPlayer )
{
    NNS_NULL_ASSERT( seqPlayer );
    
    seqPlayer->pauseFlag = FALSE;
    seqPlayer->startFlag = FALSE;
    seqPlayer->prepareFlag = FALSE;
    
    seqPlayer->seqType = NNS_SND_PLAYER_SEQ_TYPE_INVALID;
    
    seqPlayer->volume = 0;
    
    seqPlayer->initVolume = 127;
    seqPlayer->extVolume = 127;
    
    NNSi_SndFaderInit( & seqPlayer->fader );
    NNSi_SndFaderSet( & seqPlayer->fader, 127 << FADER_SHIFT, 1 );
}

/*---------------------------------------------------------------------------*
  Name:         InsertPlayerList

  Description:  Adds players to the player list.

  Arguments:    player      - player
                seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InsertPlayerList( NNSSndPlayer* player, NNSSndSeqPlayer* seqPlayer )
{
    NNSSndSeqPlayer* next = NULL;
    while ( ( next = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & player->playerList, next ) ) != NULL )
    {
        if ( seqPlayer->prio < next->prio ) break;
    }
    
    NNS_FndInsertListObject( & player->playerList, next, seqPlayer );
    
    seqPlayer->player = player;
}

/*---------------------------------------------------------------------------*
  Name:         InsertPrioList

  Description:  Adds to the priority list.

  Arguments:    seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InsertPrioList( NNSSndSeqPlayer* seqPlayer )
{
    NNSSndSeqPlayer* next = NULL;
    while ( ( next = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, next ) ) != NULL )
    {
        if ( seqPlayer->prio < next->prio ) break;
    }

    NNS_FndInsertListObject( & sPrioList, next, seqPlayer );
}

/*---------------------------------------------------------------------------*
  Name:         ForceStopSeq

  Description:  Forcibly stops a sequence.

  Arguments:    seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ForceStopSeq( NNSSndSeqPlayer* seqPlayer )
{
    if ( seqPlayer->status == NNS_SND_SEQ_PLAYER_STATUS_FADEOUT ) {
        SND_SetPlayerVolume( seqPlayer->playerNo, SND_VOLUME_DB_MIN );
    }
    SND_StopSeq( seqPlayer->playerNo );
    ShutdownPlayer( seqPlayer );
}

/*---------------------------------------------------------------------------*
  Name:         AllocSeqPlayer

  Description:  Allocates a sequence player.

  Arguments:    prio     - player priority

  Returns:      allocated sequence player
                NULL on failure
 *---------------------------------------------------------------------------*/
static NNSSndSeqPlayer* AllocSeqPlayer( int prio )
{
    NNSSndSeqPlayer* seqPlayer;
    
    NNS_MINMAX_ASSERT( prio, 0, NNS_SND_PLAYER_PRIO_MAX );
    
    seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sFreeList, NULL );
    if ( seqPlayer == NULL )
    {
        // get lowest priority player
        seqPlayer = (NNSSndSeqPlayer*)NNS_FndGetNextListObject( & sPrioList, NULL );        
        NNS_NULL_ASSERT( seqPlayer );
        
        if ( prio < seqPlayer->prio ) return NULL;
        
        ForceStopSeq( seqPlayer );
    }
    NNS_FndRemoveListObject( & sFreeList, seqPlayer );
    
    seqPlayer->prio = (u8)prio;
    
    InsertPrioList( seqPlayer );
    
    return seqPlayer;
}

/*---------------------------------------------------------------------------*
  Name:         ShutdownPlayer

  Description:  Shuts down player after sequence stops.

  Arguments:    seqPlayer  - sequence player

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShutdownPlayer( NNSSndSeqPlayer* seqPlayer )
{
    NNSSndPlayer* player;
    
    NNS_NULL_ASSERT( seqPlayer );
    
    // disconnect sound handle
    if ( seqPlayer->handle != NULL ) {
        NNS_ASSERT( seqPlayer == seqPlayer->handle->player );
        seqPlayer->handle->player = NULL;
        seqPlayer->handle = NULL;
    }
    
    // Disconnect player.
    player = seqPlayer->player;
    NNS_NULL_ASSERT( player );
    NNS_FndRemoveListObject( & player->playerList, seqPlayer );        
    seqPlayer->player = NULL;
    
    // Free heap.
    if ( seqPlayer->heap != NULL ) {
        NNS_FndAppendListObject( & player->heapList, seqPlayer->heap );
        seqPlayer->heap->player = NULL;
        seqPlayer->heap = NULL;
    }
    
    // move to free list
    NNS_FndRemoveListObject( &sPrioList, seqPlayer );
    NNS_FndAppendListObject( &sFreeList, seqPlayer );
    
    seqPlayer->status = NNS_SND_SEQ_PLAYER_STATUS_STOP;
}

/*---------------------------------------------------------------------------*
  Name:         PlayerHeapDisposeCallback

  Description:  Performs processing when the player heap is disposed of.

  Arguments:    mem   - start address of the memory block
                size  - The size of the memory block
                data1 - user data (not used)
                data2 - user data (not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PlayerHeapDisposeCallback( void* mem, u32 /*size*/, u32 /*data1*/, u32 /*data2*/ )
{
    NNSSndPlayerHeap* heap = (NNSSndPlayerHeap*)mem;
    NNSSndSeqPlayer* seqPlayer;
    
    if ( heap->handle == NNS_SND_HEAP_INVALID_HANDLE ) return;
    
    // clear heap
    NNS_SndHeapDestroy( heap->handle );
    
    seqPlayer = heap->player;
    if ( seqPlayer != NULL ) {
        // disconnect player during use
        seqPlayer->heap = NULL;
    }
    else {
        // delete from heap list when not being used
        NNS_FndRemoveListObject( &sPlayer[ heap->playerNo ].heapList, heap );
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetPlayerPriority

  Description:  Change the player priority.

  Arguments:    seqPlayer  - sequence player
                priority - Player priority.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetPlayerPriority( NNSSndSeqPlayer* seqPlayer, int priority )
{
    NNSSndPlayer* player;
    
    NNS_NULL_ASSERT( seqPlayer );
    NNS_MINMAX_ASSERT( priority, 0, NNS_SND_PLAYER_PRIO_MAX );
    
    player = seqPlayer->player;
    if ( player != NULL ) {
        NNS_FndRemoveListObject( & player->playerList, seqPlayer );
        seqPlayer->player = NULL;
    }
    NNS_FndRemoveListObject( & sPrioList, seqPlayer );
    
    seqPlayer->prio = (u8)priority;
    
    if ( player != NULL ) {
        InsertPlayerList( player, seqPlayer );
    }
    InsertPrioList( seqPlayer );
}


/*====== End of player.c ======*/

