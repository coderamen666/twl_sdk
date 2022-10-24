/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     sndarc_player.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/sndarc_player.h>

#include <nnsys/misc.h>
#include <nnsys/snd/sndarc.h>
#include <nnsys/snd/player.h>
#include <nnsys/snd/sndarc_loader.h>

/******************************************************************************
	static function declarations
 ******************************************************************************/

static BOOL StartSeq(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    const NNSSndArcSeqInfo* info,
    int seqNo
);
static BOOL StartSeqArc(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    const NNSSndSeqArcSeqInfo* sound,
    const NNSSndSeqArc* seqArc,
    int seqArcNo,
    int index
);

/******************************************************************************
	public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcPlayerSetup

  Description:  Set up the player using player information in the sound archive
                

  Arguments:    heap - sound heap for creating the player heap

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcPlayerSetup( NNSSndHeapHandle heap )
{
    NNSSndArc* arc = NNS_SndArcGetCurrent();
    int playerNo;
    const NNSSndArcPlayerInfo* playerInfo;
    
    NNS_NULL_ASSERT( arc );
    
    for( playerNo = 0; playerNo < NNS_SND_PLAYER_NUM ; ++playerNo )
    {
        playerInfo = NNS_SndArcGetPlayerInfo( playerNo );
        if ( playerInfo == NULL ) continue;
        
        NNS_SndPlayerSetPlayableSeqCount( playerNo, playerInfo->seqMax );
        NNS_SndPlayerSetAllocatableChannel( playerNo, playerInfo->allocChBitFlag );
        
        if ( playerInfo->heapSize > 0 && heap != NNS_SND_HEAP_INVALID_HANDLE )
        {
            int i;
            
            for( i = 0; i < playerInfo->seqMax ; i++ )
            {
                if ( ! NNS_SndPlayerCreateHeap( playerNo, heap, playerInfo->heapSize ) ) {
                    return FALSE;
                }
            }
        }
    }
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcPlayerStartSeq

  Description:  Plays sequence.

  Arguments:    handle - sound handle
                seqNo      - sequence number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcPlayerStartSeq( NNSSndHandle* handle, int seqNo )
{
    const NNSSndArcSeqInfo* info;
    
    NNS_NULL_ASSERT( handle );
    
    info = NNS_SndArcGetSeqInfo( seqNo );
    if ( info == NULL ) return FALSE;    
    
    return StartSeq(
        handle,
        info->param.playerNo,
        info->param.bankNo,
        info->param.playerPrio,
        info,
        seqNo
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcPlayerStartSeqEx

  Description:  Sets a number of parameters and plays sequence.

  Arguments:    handle - sound handle
                playerNo   - player number (-1 is entered if player number not specified)
                bankNo     - bank number (-1 is entered if not specified)
                playerPrio - player priority (-1 is entered if not specified)
                seqNo      - sequence number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcPlayerStartSeqEx(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    int seqNo
)
{
    const NNSSndArcSeqInfo* info;
   
    NNS_NULL_ASSERT( handle );
    NNS_MAX_ASSERT( playerNo,   NNS_SND_PLAYER_NUM-1 );
    NNS_MAX_ASSERT( playerPrio, NNS_SND_PLAYER_PRIO_MAX );

    info = NNS_SndArcGetSeqInfo( seqNo );
    if ( info == NULL ) return FALSE;    
    
    return StartSeq(
        handle,
        playerNo   >= 0 ? playerNo   : info->param.playerNo,
        bankNo     >= 0 ? bankNo     : info->param.bankNo,
        playerPrio >= 0 ? playerPrio : info->param.playerPrio,
        info,
        seqNo
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcPlayerStartSeqArc

  Description:  Plays sequence archive.

  Arguments:    handle - sound handle
                seqArcNo   - sequence archive number
                index      - index number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcPlayerStartSeqArc( NNSSndHandle* handle, int seqArcNo, int index )
{
    const NNSSndArcSeqArcInfo* info;
    const NNSSndSeqArcSeqInfo* sound;
    const NNSSndSeqArc* seqArc;
    
    NNS_NULL_ASSERT( handle );
    
    info = NNS_SndArcGetSeqArcInfo( seqArcNo );
    if ( info == NULL ) return FALSE;
    seqArc = (NNSSndSeqArc*)NNS_SndArcGetFileAddress( info->fileId );
    if ( seqArc == NULL ) return FALSE;
    sound = NNSi_SndSeqArcGetSeqInfo( seqArc, index );
    if ( sound == NULL ) return FALSE;
    
    return StartSeqArc(
        handle,
        sound->param.playerNo,
        sound->param.bankNo,
        sound->param.playerPrio,
        sound,
        seqArc,
        seqArcNo,
        index
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcPlayerStartSeqArcEx

  Description:  Sets a number of parameters and plays sequence archive.

  Arguments:    handle - sound handle
                playerNo   - player number (-1 is entered if player number not specified)
                bankNo     - bank number (-1 is entered if not specified)
                playerPrio - player priority (-1 is entered if not specified)
                seqArcNo   - sequence archive number
                index      - index number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcPlayerStartSeqArcEx(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    int seqArcNo,
    int index
)
{
    const NNSSndArcSeqArcInfo* info;
    const NNSSndSeqArc* seqArc;
    const NNSSndSeqArcSeqInfo* sound;
    
    NNS_NULL_ASSERT( handle );
    NNS_MAX_ASSERT( playerNo,   NNS_SND_PLAYER_NUM-1 );
    NNS_MAX_ASSERT( playerPrio, NNS_SND_PLAYER_PRIO_MAX );
    
    info = NNS_SndArcGetSeqArcInfo( seqArcNo );
    if ( info == NULL ) return FALSE;
    seqArc = (NNSSndSeqArc*)NNS_SndArcGetFileAddress( info->fileId );
    if ( seqArc == NULL ) return FALSE;
    sound = NNSi_SndSeqArcGetSeqInfo( seqArc, index );
    if ( sound == NULL ) return FALSE;
    
    return StartSeqArc(
        handle,
        playerNo   >= 0 ? playerNo   : sound->param.playerNo,
        bankNo     >= 0 ? bankNo     : sound->param.bankNo,
        playerPrio >= 0 ? playerPrio : sound->param.playerPrio,
        sound,
        seqArc,
        seqArcNo,
        index
    );
}

/******************************************************************************
	Static function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         StartSeq

  Description:  Plays sequence.

  Arguments:    handle - sound handle
                playerNo - player number
                bankNo    - bank number
                playerPrio - player priority
                info - pointer to sequence-information structure
                seqNo      - sequence number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
static BOOL StartSeq(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    const NNSSndArcSeqInfo* info,
    int seqNo
)
{
    NNSSndSeqPlayer* player;
    NNSSndHeapHandle heap;
    NNSSndSeqData* seq;
    SNDBankData* bank;
    NNSSndArcLoadResult result;
    
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( playerPrio, 0, NNS_SND_PLAYER_PRIO_MAX );
    NNS_NULL_ASSERT( info );
    
    /* allocate player */
    player = NNSi_SndPlayerAllocSeqPlayer( handle, playerNo, playerPrio );
    if ( player == NULL ) return FALSE;
    
    /* Allocates a player heap. */
    heap = NNSi_SndPlayerAllocHeap( playerNo, player );
    
    /* get or load bank and waveform data */
    result = NNSi_SndArcLoadBank( bankNo, NNS_SND_ARC_LOAD_BANK | NNS_SND_ARC_LOAD_WAVE, heap, FALSE, &bank );
    if ( result != NNS_SND_ARC_LOAD_SUCESS ) {
        NNSi_SndPlayerFreeSeqPlayer( player );
        return FALSE;
    }
    
    /* get or load sequence data */
    result = NNSi_SndArcLoadSeq( seqNo, NNS_SND_ARC_LOAD_SEQ, heap, FALSE, &seq );
    if ( result != NNS_SND_ARC_LOAD_SUCESS ) {
        NNSi_SndPlayerFreeSeqPlayer( player );
        return FALSE;
    }
    
    /* play sequence */
    NNSi_SndPlayerStartSeq(
        player,
        (u8*)seq + seq->baseOffset,
        0,
        bank
    );
    
    /* Set parameters */
    NNS_SndPlayerSetInitialVolume( handle, info->param.volume );
    NNS_SndPlayerSetChannelPriority( handle, info->param.channelPrio );
    NNS_SndPlayerSetSeqNo( handle, seqNo );    
    
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         StartSeqArc

  Description:  Plays sequence archive.

  Arguments:    handle - sound handle
                playerNo - player number
                bankNo    - bank number
                playerPrio - player priority
                sound      - sequence information structure in the sequence archive
                seqArc     - sequence archive pointer
                seqArcNo   - sequence archive number
                index      - index number

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
static BOOL StartSeqArc(
    NNSSndHandle* handle,
    int playerNo,
    int bankNo,
    int playerPrio,
    const NNSSndSeqArcSeqInfo* sound,
    const NNSSndSeqArc* seqArc,
    int seqArcNo,
    int index
)
{
    NNSSndSeqPlayer* player;
    NNSSndHeapHandle heap;
    SNDBankData* bank;
    NNSSndArcLoadResult result;
    
    NNS_NULL_ASSERT( handle );
    NNS_MINMAX_ASSERT( playerNo, 0, NNS_SND_PLAYER_NO_MAX );
    NNS_MINMAX_ASSERT( playerPrio, 0, NNS_SND_PLAYER_PRIO_MAX );
    NNS_NULL_ASSERT( sound );
    NNS_NULL_ASSERT( seqArc );
    
    /* allocate player */
    player = NNSi_SndPlayerAllocSeqPlayer( handle, playerNo, playerPrio );
    if ( player == NULL ) return FALSE;
    
    /* Allocates a player heap. */
    heap = NNSi_SndPlayerAllocHeap( playerNo, player );
    
    /* get or load bank and waveform data */
    result = NNSi_SndArcLoadBank( bankNo, NNS_SND_ARC_LOAD_BANK | NNS_SND_ARC_LOAD_WAVE, heap, FALSE, &bank );
    if ( result != NNS_SND_ARC_LOAD_SUCESS ) {
        NNSi_SndPlayerFreeSeqPlayer( player );
        return FALSE;
    }
    
    /* play sequence*/
    NNSi_SndPlayerStartSeq(
        player,
        (u8*)seqArc + seqArc->baseOffset,
        sound->offset,
        bank
    );
    
    /* Set parameters */    
    NNS_SndPlayerSetInitialVolume( handle, sound->param.volume );
    NNS_SndPlayerSetChannelPriority( handle, sound->param.channelPrio );    
    NNS_SndPlayerSetSeqArcNo( handle, seqArcNo, index );
    
    return TRUE;
}

/*====== End of sndarc_player.c ======*/

