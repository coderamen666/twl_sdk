/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     seqdata.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/seqdata.h>

/* if include from Other Environment for example VC or BCB, */
/* please define NNS_FROM_TOOL                               */
#ifndef NNS_FROM_TOOL

#include <nnsys/misc.h>

#else

#define NNS_NULL_ASSERT(exp)           ((void) 0)

#ifdef _MSC_VER
#pragma warning( disable : 4018 ) // warning: signed/unsigned mismatch
#pragma warning( disable : 4311 ) // warning: pointer truncation from 'type' to 'type'
#pragma warning( disable : 4312 ) // warning: conversion from 'type' to 'type' of greater size
#endif

#endif // NNS_FROM_TOOL

/******************************************************************************
	Private Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndSeqArcGetSeqCount

  Description:  Gets the number of sequences inside the sequence archive.

  Arguments:    seqArc - the sequence archive

  Returns:      the number of sequences
 *---------------------------------------------------------------------------*/
u32 NNSi_SndSeqArcGetSeqCount( const NNSSndSeqArc* seqArc )
{
    NNS_NULL_ASSERT( seqArc );
    return seqArc->count;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndSeqArcGetSeqInfo

  Description:  Gets the sequence information inside the sequence archive.

  Arguments:    seqArc - the sequence archive
                seqNo  - the index number

  Returns:      the sequence information structure
 *---------------------------------------------------------------------------*/
const NNSSndSeqArcSeqInfo* NNSi_SndSeqArcGetSeqInfo(
    const NNSSndSeqArc* seqArc,
    int index
)
{
    NNS_NULL_ASSERT( seqArc );
    
	if ( index < 0 ) return NULL;
	if ( index >= seqArc->count ) return NULL;
    if ( seqArc->info[ index ].offset == NNS_SND_SEQ_ARC_INVALID_OFFSET ) return NULL;
	
	return & seqArc->info[ index ];
}

/*====== End of seqarc.c ======*/

