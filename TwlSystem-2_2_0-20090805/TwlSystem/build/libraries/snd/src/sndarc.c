/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     sndarc.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1423 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/sndarc.h>
#include <nnsys/snd/config.h>

/* If include from other environment (for example, VC or BCB), */
/* please define NNS_FROM_TOOL                               */
#ifndef NNS_FROM_TOOL

#include <nitro/snd.h>
#include <nnsys/misc.h>
#include <nnsys/snd/player.h>

#else

#define NNS_ASSERT(exp)           ((void) 0)
#define NNS_NULL_ASSERT(exp)           ((void) 0)
#define NNS_ALIGN4_ASSERT(exp)           ((void) 0)
#define NNS_MAX_ASSERT(exp, max)           ((void) 0)

static NNS_SND_INLINE
void MI_CpuCopy32( register const void *srcp, register void *destp, register u32 size )
{
    const u32* sp = (const u32*)srcp;
    u32* dp = (u32*)destp;
    u32* dpe = (u32*)((char*)dp + size);
    
    while( dp < dpe ) {
        *dp++ = *sp++;
    }
}

#ifdef _MSC_VER
#pragma warning( disable : 4018 ) // Warning: Signed/unsigned mismatch
#pragma warning( disable : 4311 ) // Warning: Pointer truncation from 'type' to 'type'
#pragma warning( disable : 4312 ) // Warning: Conversion from 'type' to 'type' of greater size
#endif

#endif // NNS_FROM_TOOL

/******************************************************************************
    Static Variables
 ******************************************************************************/

static const char null_string = '\0';
static NNSSndArc* sCurrent = NULL;

/******************************************************************************
    Static Function Declarations
 ******************************************************************************/

static const char* GetSymbol(
    const NNSSndArcOffsetTable* table,
    int index,
    const void* base
);
static void* GetPtr( void* base, u32 offset );
static const void* GetPtrConst( const void* base, u32 offset );
static const NNSSndArcOffsetTable* GetOffsetTable( const NNSSndArcInfo* info, u32 offset );
static void InfoDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void FatDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void SymbolDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );

/******************************************************************************
    Inline functions
 ******************************************************************************/

static NNS_SND_INLINE
void* GetPtr( void* base, u32 offset )
{
    if ( offset == 0 ) return NULL;
    return (u8*)base + offset;
}

static NNS_SND_INLINE
const void* GetPtrConst( const void* base, u32 offset )
{
    if ( offset == 0 ) return NULL;
    return (const u8*)base + offset;
}

static NNS_SND_INLINE
const NNSSndArcOffsetTable* GetOffsetTable( const NNSSndArcInfo* info, u32 offset )
{
    return (const NNSSndArcOffsetTable*)GetPtrConst( info, offset );
}

/******************************************************************************
    Public Functions
 ******************************************************************************/

#ifndef SDK_SMALL_BUILD

#ifndef NNS_FROM_TOOL
/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcInit

  Description:  Initializes the Sound Archive structure.

  Arguments:    arc: Pointer to the Sound Archive structure
                filePath: Sound archive file path
                heap: Sound heap
                symbolLoadFlag: Flag indicating whether to load the symbol data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndArcInit( NNSSndArc* arc, const char* filePath, NNSSndHeapHandle heap, BOOL symbolLoadFlag )
{
    BOOL result;
    
    NNS_ASSERT( FS_IsAvailable() );
    NNS_NULL_ASSERT( arc );
    
    /* Initialization of the NNSSndArc structure */ 
    arc->info = NULL;
    arc->fat  = NULL;
    arc->symbol = NULL;
    arc->loadBlockSize = 0;

#ifndef NNS_FROM_TOOL
    arc->filePath = NULL;
    arc->seekCache = NULL;
    arc->seekCacheSize = 0;
#endif /* NNS_FROM_TOOL */

    /* Get the file ID */
    result = FS_ConvertPathToFileID( & arc->fileId, filePath );

    /* Open the file */
    if ( result )
    {
        FS_InitFile( & arc->file );
        result = FS_OpenFileFast( & arc->file, arc->fileId );
        NNS_ASSERTMSG( result, "Cannot open file %s\n", filePath );
        if ( ! result ) return;
    }
    else
    {
        // For SD Cards and other times that FSFileID cannot be obtained
        arc->fileId.arc = NULL;             /* struct FSArchive* */
        arc->fileId.file_id = 0xFFFFFFFF;   /* u32 */

        result = FS_OpenFileEx( & arc->file, filePath, FS_FILEMODE_R );
        NNS_ASSERTMSG( result, "Cannot open file %s\n", filePath );
        if ( ! result ) return;
    }

    arc->file_open = TRUE;

    /* Set up */
    result = NNS_SndArcSetup( arc, heap, symbolLoadFlag );
    NNS_ASSERT( result );
    if ( ! result ) return;

    /* Set as the current archive */
    sCurrent = arc;    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcInitWithResult

  Description:  Initializes the sound archive structure (with a return value).

  Arguments:    arc: Pointer to the Sound Archive structure
                filePath: Sound archive file path
                heap: Sound heap
                symbolLoadFlag: Flag indicating whether to load the symbol data

  Returns:      Whether it was successful.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcInitWithResult( NNSSndArc* arc, const char* filePath, NNSSndHeapHandle heap, BOOL symbolLoadFlag )
{
    BOOL result;
    
    NNS_ASSERT( FS_IsAvailable() );
    NNS_NULL_ASSERT( arc );
    
    /* Initialization of the NNSSndArc structure */ 
    arc->info = NULL;
    arc->fat  = NULL;
    arc->symbol = NULL;    
    arc->loadBlockSize = 0;

    /* Get the file ID */
    result = FS_ConvertPathToFileID( & arc->fileId, filePath );
    if ( ! result ) return FALSE;
    
    /* Open the file */
    FS_InitFile( & arc->file );
    result = FS_OpenFileFast( & arc->file, arc->fileId );
    if ( ! result ) return FALSE;
    
    arc->file_open = TRUE;
    
    /* Set up */
    result = NNS_SndArcSetup( arc, heap, symbolLoadFlag );
    if ( ! result ) return FALSE;
    
    /* Set as the current archive */
    sCurrent = arc;
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcSetup

  Description:  Sets up the Sound Archive structure.

  Arguments:    arc: Pointer to the Sound Archive structure
                heap: Sound heap
                symbolLoadFlag: Flag indicating whether to load the symbol data

  Returns:      Whether it was successful.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcSetup( NNSSndArc* arc, NNSSndHeapHandle heap, BOOL symbolLoadFlag )
{
    BOOL result;
    s32 readSize;
    
    NNS_NULL_ASSERT( arc );
    NNS_ASSERT( arc->file_open );
    
    /* Load the header */
    result = FS_SeekFile( & arc->file, 0, FS_SEEK_SET );
    if ( ! result ) return FALSE;
    
    readSize = FS_ReadFile(
        & arc->file,
        & arc->header,
        sizeof( arc->header )
    );
    if ( readSize != sizeof( arc->header ) ) return FALSE;
    NNS_ASSERT(
        arc->header.fileHeader.signature[0] == 'S' &&
        arc->header.fileHeader.signature[1] == 'D' &&
        arc->header.fileHeader.signature[2] == 'A' &&
        arc->header.fileHeader.signature[3] == 'T' 
    );
    NNS_ASSERT( arc->header.fileHeader.version >= NNS_SND_ARC_SUPPORTED_FILE_VERSION );
    
    if ( heap != NNS_SND_HEAP_INVALID_HANDLE )
    {
        /* Load sound information table */
        arc->info = (NNSSndArcInfo*)NNS_SndHeapAlloc( heap, arc->header.infoSize, InfoDisposeCallback, (u32)arc, 0 );
        if ( arc->info == NULL ) return FALSE;
        result = FS_SeekFile( & arc->file, (s32)( arc->header.infoOffset ), FS_SEEK_SET );
        if ( ! result ) return FALSE;
        readSize = FS_ReadFile( & arc->file, arc->info, (s32)( arc->header.infoSize ) );
        if ( readSize != arc->header.infoSize ) return FALSE;
        NNS_ASSERT( arc->info->blockHeader.kind == NNS_SND_ARC_SIGNATURE_INFO );
        
        /* Load file allocation table */
        arc->fat = (NNSSndArcFat*)NNS_SndHeapAlloc( heap, arc->header.fatSize, FatDisposeCallback, (u32)arc, 0 );
        if ( arc->fat == NULL ) return FALSE;
        result = FS_SeekFile( & arc->file, (s32)( arc->header.fatOffset ), FS_SEEK_SET );
        if ( ! result ) return FALSE;
        readSize = FS_ReadFile( & arc->file, arc->fat, (s32)( arc->header.fatSize ) );
        if ( readSize != arc->header.fatSize ) return FALSE;
        NNS_ASSERT( arc->fat->blockHeader.kind == NNS_SND_ARC_SIGNATURE_FAT );
        
        /* Load the symbol table */
        if ( symbolLoadFlag && arc->header.symbolDataSize > 0 )
        {
            arc->symbol = (NNSSndArcSymbol*)NNS_SndHeapAlloc( heap, arc->header.symbolDataSize, SymbolDisposeCallback, (u32)arc, 0 );
            if ( arc->symbol == NULL ) return FALSE;
            result = FS_SeekFile( & arc->file, (s32)( arc->header.symbolDataOffset ), FS_SEEK_SET );
            if ( ! result ) return FALSE;
            
            readSize = FS_ReadFile( & arc->file, arc->symbol, (s32)( arc->header.symbolDataSize ) );
            if ( readSize != arc->header.symbolDataSize ) return FALSE;
            NNS_ASSERT( arc->symbol->blockHeader.kind == NNS_SND_ARC_SIGNATURE_SYMB );
        }
    }
    
    return TRUE;
}

#endif // NNS_FROM_TOOL

#endif /* SDK_SMALL_BUILD */

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcInitOnMemory

  Description:  Uses sound archive data in memory to initialize a sound archive structure.
                

  Arguments:    arc: Pointer to the Sound Archive structure
                data: Pointer to sound archive data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndArcInitOnMemory( NNSSndArc* arc, void* data )
{
    NNSSndArcHeader* header = (NNSSndArcHeader*)data;
    NNSSndArcFileInfo* file;
    int fileId;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( data );
    NNS_ALIGN4_ASSERT( data );
    
    MI_CpuCopy32( header, &arc->header, sizeof( arc->header ) );

    NNS_ASSERT(
        arc->header.fileHeader.signature[0] == 'S' &&
        arc->header.fileHeader.signature[1] == 'D' &&
        arc->header.fileHeader.signature[2] == 'A' &&
        arc->header.fileHeader.signature[3] == 'T' 
    );
    NNS_ASSERT( arc->header.fileHeader.version >= NNS_SND_ARC_SUPPORTED_FILE_VERSION );
    
    arc->info   = (NNSSndArcInfo*)GetPtr( header, arc->header.infoOffset );
    arc->fat    = (NNSSndArcFat*)GetPtr( header, arc->header.fatOffset );
    arc->symbol = (NNSSndArcSymbol*)GetPtr( header, arc->header.symbolDataOffset );
    arc->loadBlockSize = 0;
    
    NNS_NULL_ASSERT( arc->info );
    NNS_NULL_ASSERT( arc->fat );
    
    NNS_ASSERT( arc->info->blockHeader.kind == NNS_SND_ARC_SIGNATURE_INFO );
    NNS_ASSERT( arc->fat->blockHeader.kind  == NNS_SND_ARC_SIGNATURE_FAT );
    NNS_ASSERT( arc->symbol == NULL || arc->symbol->blockHeader.kind == NNS_SND_ARC_SIGNATURE_SYMB );
    
    for( fileId = 0; fileId < arc->fat->count ; fileId ++ )
    {
        file = & arc->fat->files[ fileId ];
        
        file->mem = GetPtr( header, file->offset );
        NNS_NULL_ASSERT( file->mem );
    }
    
    arc->file_open = FALSE;
    
    /* Set as the current archive */
    sCurrent = arc;    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcSetCurrent

  Description:  Sets up current sound archive.

  Arguments:    arc: Sound archive to be set

  Returns:      Current sound archive before setting.
 *---------------------------------------------------------------------------*/
NNSSndArc* NNS_SndArcSetCurrent( NNSSndArc* arc )
{
    NNSSndArc* oldArc = sCurrent;
    sCurrent = arc;
    return oldArc;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetCurrent

  Description:  Gets current sound archive.

  Arguments:    None.

  Returns:      Current sound archive.
 *---------------------------------------------------------------------------*/
NNSSndArc* NNS_SndArcGetCurrent( void )
{
    return sCurrent;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqParam

  Description:  Gets sequence parameter.

  Arguments:    seqNo: Sequence number

  Returns:      Sequence parameter structure.
 *---------------------------------------------------------------------------*/
const NNSSndSeqParam* NNS_SndArcGetSeqParam( int seqNo )
{
    const NNSSndArcSeqInfo* info;

    info = NNS_SndArcGetSeqInfo( seqNo );
    if ( info == NULL ) return NULL;
    
    return & info->param;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcParam

  Description:  Gets sequence archive parameter.

  Arguments:    seqArcNo: Sequence archive number
                index: Sequence archive index

  Returns:      Sequence parameter structure.
 *---------------------------------------------------------------------------*/
const NNSSndSeqParam* NNS_SndArcGetSeqArcParam( int seqArcNo, int index )
{
    const NNSSndArcSeqArcInfo* info;
    const NNSSndSeqArc* seqArc;
    const NNSSndSeqArcSeqInfo* sound;
    
    info = NNS_SndArcGetSeqArcInfo( seqArcNo );
    if ( info == NULL ) return NULL;
    
    seqArc = (const NNSSndSeqArc*)NNS_SndArcGetFileAddress( info->fileId );
    if ( seqArc == NULL ) return NULL;
    
    sound = NNSi_SndSeqArcGetSeqInfo( seqArc, index );
    if ( sound == NULL ) return NULL;

    return & sound->param;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqInfo

  Description:  Gets sequence information.

  Arguments:    seqNo: Sequence number

  Returns:      Pointer to the sequence information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcSeqInfo* NNS_SndArcGetSeqInfo( int seqNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->seqOffset );
    if ( table == NULL ) return NULL;
    
    if ( seqNo < 0 ) return NULL;
    if ( seqNo >= table->count ) return NULL;
    
    return (const NNSSndArcSeqInfo*)GetPtrConst( arc->info, table->offset[ seqNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcInfo

  Description:  Gets sequence archive information.

  Arguments:    seqArcNo: Sequence archive number

  Returns:      Pointer to the sequence archive information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcSeqArcInfo* NNS_SndArcGetSeqArcInfo( int seqArcNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->seqArcOffset );
    if ( table == NULL ) return NULL;
    
    if ( seqArcNo < 0 ) return NULL;
    if ( seqArcNo >= table->count ) return NULL;
    
    return (const NNSSndArcSeqArcInfo*)GetPtrConst( arc->info, table->offset[ seqArcNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetBankInfo

  Description:  Gets bank information.

  Arguments:    bankNo: Bank number

  Returns:      Pointer to the bank information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcBankInfo* NNS_SndArcGetBankInfo( int bankNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->bankOffset );
    if ( table == NULL ) return NULL;
    
    if ( bankNo < 0 ) return NULL;
    if ( bankNo >= table->count ) return NULL;
    
    return (const NNSSndArcBankInfo*)GetPtrConst( arc->info, table->offset[ bankNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetWaveArcInfo

  Description:  Gets the waveform archive information

  Arguments:    waveArcNo: Waveform archive number

  Returns:      Pointer to the waveform archive information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcWaveArcInfo* NNS_SndArcGetWaveArcInfo( int waveArcNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->waveArcOffset );
    if ( table == NULL ) return NULL;
    
    if ( waveArcNo < 0 ) return NULL;
    if ( waveArcNo >= table->count ) return NULL;
    
    return (const NNSSndArcWaveArcInfo*)GetPtrConst( arc->info, table->offset[ waveArcNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetStrmInfo

  Description:  Gets stream information.

  Arguments:    strmNo: Stream number

  Returns:      Pointer to stream information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcStrmInfo* NNS_SndArcGetStrmInfo( int strmNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->strmOffset );
    if ( table == NULL ) return NULL;
    
    if ( strmNo < 0 ) return NULL;
    if ( strmNo >= table->count ) return NULL;
    
    return (const NNSSndArcStrmInfo*)GetPtrConst( arc->info, table->offset[ strmNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetPlayerInfo

  Description:  Gets player information.

  Arguments:    playerNo: Player number

  Returns:      Pointer to the player information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcPlayerInfo* NNS_SndArcGetPlayerInfo( int playerNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->playerInfoOffset );
    if ( table == NULL ) return NULL;
    
    if ( playerNo < 0 ) return NULL;
    if ( playerNo >= table->count ) return NULL;
    
    return (const NNSSndArcPlayerInfo*)GetPtrConst( arc->info, table->offset[ playerNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetStrmPlayerInfo

  Description:  Gets stream player information.

  Arguments:    playerNo: Player number

  Returns:      Pointer to stream player information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcStrmPlayerInfo* NNS_SndArcGetStrmPlayerInfo( int playerNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->strmPlayerInfoOffset );
    if ( table == NULL ) return NULL;
    
    if ( playerNo < 0 ) return NULL;
    if ( playerNo >= table->count ) return NULL;
    
    return (const NNSSndArcStrmPlayerInfo*)GetPtrConst( arc->info, table->offset[ playerNo ] );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetGroupInfo

  Description:  Gets group information.

  Arguments:    groupNo: Group number

  Returns:      Pointer to the group information structure.
                NULL if process fails.
 *---------------------------------------------------------------------------*/
const NNSSndArcGroupInfo* NNS_SndArcGetGroupInfo( int groupNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->groupInfoOffset );
    if ( table == NULL ) return NULL;
    
    if ( groupNo < 0 ) return NULL;
    if ( groupNo >= table->count ) return NULL;
    
    return (const NNSSndArcGroupInfo*)GetPtrConst( arc->info, table->offset[ groupNo ] );
}


/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqCount

  Description:  Gets the sequence data count.

  Arguments:    None.

  Returns:      Sequence data count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetSeqCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->seqOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcCount

  Description:  Gets the sequence archive data count.

  Arguments:    None.

  Returns:      Sequence archive data count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetSeqArcCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->seqArcOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetBankCount

  Description:  Gets the bank data count.

  Arguments:    None.

  Returns:      Bank data count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetBankCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->bankOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetWaveArcCount

  Description:  Gets the wave archive data count.

  Arguments:    None.

  Returns:      Wave archive data count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetWaveArcCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->waveArcOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetStrmCount

  Description:  Gets the stream data count.

  Arguments:    None.

  Returns:      Stream data count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetStrmCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->strmOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetGroupCount

  Description:  Gets the group information count.

  Arguments:    None.

  Returns:      Group information count.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetGroupCount( void )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->info );
    
    table = GetOffsetTable( arc->info, arc->info->groupInfoOffset );
    if ( table == NULL ) return 0;
    
    return table->count ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcSeqCount

  Description:  Gets the number of sequences being sequence archived.

  Arguments:    seqArcNo: Sequence archive number

  Returns:      Number of sequences.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetSeqArcSeqCount( int seqArcNo )
{
    const NNSSndArcSeqArcInfo* info;
    const NNSSndSeqArc* seqArc;
    
    info = NNS_SndArcGetSeqArcInfo( seqArcNo );
    if ( info == NULL ) return 0;
    
    seqArc = (const NNSSndSeqArc*)NNS_SndArcGetFileAddress( info->fileId );
    if ( seqArc == NULL ) return 0;
    
    return NNSi_SndSeqArcGetSeqCount( seqArc );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetFileOffset

  Description:  Gets the file offset from file ID.

  Arguments:    fileId: File ID

  Returns:      File offset.
                Returns 0 if it fails.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetFileOffset( u32 fileId )
{
    NNSSndArc* arc = sCurrent;
    NNS_NULL_ASSERT( arc );    
    NNS_NULL_ASSERT( arc->fat );
    
    if ( fileId >= arc->fat->count ) return 0;
    return arc->fat->files[ fileId ].offset;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetFileSize

  Description:  Gets the file size from the file ID.

  Arguments:    fileId: File ID

  Returns:      File size.
                Returns 0 if it fails.
 *---------------------------------------------------------------------------*/
u32 NNS_SndArcGetFileSize( u32 fileId )
{
    NNSSndArc* arc = sCurrent;
    NNS_NULL_ASSERT( arc );    
    NNS_NULL_ASSERT( arc->fat );
    
    if ( fileId >= arc->fat->count ) return 0;
    return arc->fat->files[ fileId ].size;
}

/* If include from other environment (for example, VC or BCB), */
/* please define NNS_FROM_TOOL                               */
#ifndef NNS_FROM_TOOL
/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcReadFile

  Description:  Reads file into memory.

  Arguments:    fileId: File ID
                buffer: Starting address of buffer to load
                size: Size of buffer to load
                offset: Read offset

  Returns:      Size that was read.
                -1 if process fails.
 *---------------------------------------------------------------------------*/
s32 NNS_SndArcReadFile( u32 fileId, void* buffer, s32 size, s32 offset )
{
#ifndef SDK_SMALL_BUILD
    
    NNSSndArc* arc = sCurrent;
    const NNSSndArcFileInfo* file;
    s32 totalReadSize;
    s32 readSize;
    s32 blockSize;
    s32 currentOffset;
    s32 requestSize;
    u8* destAddress;
    
    NNS_ASSERT( FS_IsAvailable() );
    NNS_NULL_ASSERT( arc );    
    NNS_NULL_ASSERT( arc->fat );
    NNS_NULL_ASSERT( buffer );
    NNS_ASSERTMSG( arc->file_open, "Cannot use this function for the Sound Archive initialized by NNS_SndArcInitOnMemory\n" );
    
    if ( fileId >= arc->fat->count ) return -1;
    file = & arc->fat->files[ fileId ];
    
    currentOffset = offset;
    
    blockSize = arc->loadBlockSize;
    if ( blockSize == 0 ) {
        // Do not load in segments when the block size is 0
        blockSize = size;
    }
    totalReadSize = 0;
    destAddress = (u8*)buffer;
    
    while( totalReadSize < size )
    {
        // Calculate the load size
        requestSize = size - totalReadSize;
        if ( requestSize > blockSize ) requestSize = blockSize;
        if ( requestSize > file->size - currentOffset ) {
            requestSize = (s32)( file->size - currentOffset );
        }
        if ( requestSize == 0 ) {
            // Attempted to load more than the file size
            break;
        }

        // Loads file
        if ( ! FS_SeekFile( & arc->file, (s32)( file->offset + currentOffset ), FS_SEEK_SET ) ) {
            return -1;
        }
        readSize = FS_ReadFile( & arc->file, destAddress, requestSize );
        if ( readSize < 0 ) return readSize;

        // Update the state
        totalReadSize += readSize;
        currentOffset += readSize;
        destAddress += readSize;
    }
    
    return totalReadSize;
    
#else  /* SDK_SMALL_BUILD */
    
#pragma unused( fileId, buffer, size, offset )
    return -1;
    
#endif /* SDK_SMALL_BUILD */
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetFileID

  Description:  Gets the sound archive file ID.

  Arguments:    None.

  Returns:      File ID.
 *---------------------------------------------------------------------------*/
FSFileID NNS_SndArcGetFileID( void )
{
    NNSSndArc* arc = sCurrent;
    
    NNS_NULL_ASSERT( arc );    
    NNS_NULL_ASSERT( arc->fat );
    NNS_ASSERTMSG( arc->file_open, "Cannot use this function for the Sound Archive initialized by NNS_SndArcInitOnMemory\n" );
    
    return arc->fileId;
}

#endif // NNS_FROM_TOOL


/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetFileAddress

  Description:  Gets the stored file address.

  Arguments:    fileId: File ID

  Returns:      File address.
 *---------------------------------------------------------------------------*/
void* NNS_SndArcGetFileAddress( u32 fileId )
{
    NNSSndArc* arc = sCurrent;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->fat );
    
    if ( fileId >= arc->fat->count ) return NULL;
    return arc->fat->files[ fileId ].mem;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcSetFileAddress

  Description:  Stores the file address.

  Arguments:    fileId: File ID
                address: File address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndArcSetFileAddress( u32 fileId, void* address )
{
    NNSSndArc* arc = sCurrent;

    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->fat );    
    NNS_MAX_ASSERT( fileId, arc->fat->count - 1 );
    
    arc->fat->files[ fileId ].mem = address;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetLoadBlockSize

  Description:  Gets the load block size.

  Arguments:    None.

  Returns:      Load block size.
 *---------------------------------------------------------------------------*/
s32 NNS_SndArcGetLoadBlockSize()
{
    NNSSndArc* arc = sCurrent;
    
    NNS_NULL_ASSERT( arc );
    return arc->loadBlockSize;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcSetLoadBlockSize

  Description:  Sets the load block size.

  Arguments:    loadBlockSize: Load block size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndArcSetLoadBlockSize( s32 loadBlockSize )
{
    NNSSndArc* arc = sCurrent;

    NNS_NULL_ASSERT( arc );
    arc->loadBlockSize = loadBlockSize;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqSymbol

  Description:  Gets the sequence symbol.

  Arguments:    seqNo: Sequence number

  Returns:      Sequence symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetSeqSymbol( int seqNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;
    
    table = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->seqOffset );
    if ( table == NULL ) return & null_string;
    
    return GetSymbol( table, seqNo, arc->symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcSymbol

  Description:  Gets the sequence archive symbol.

  Arguments:    seqArcNo: Sequence archive number

  Returns:      Sequence archive symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetSeqArcSymbol( int seqArcNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcSeqArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;
    
    table = (const NNSSndArcSeqArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->seqArcOffset );
    if ( table == NULL ) return & null_string;
    
    if ( seqArcNo < 0 ) return & null_string;
    if ( seqArcNo >= table->count ) return & null_string;
    
    if ( table->offset[ seqArcNo ].symbol == 0 ) return & null_string;
    
    return (const char*)GetPtrConst( arc->symbol, table->offset[ seqArcNo ].symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetSeqArcIdxSymbol

  Description:  Gets the sequence archive index symbol.

  Arguments:    seqArcNo: Sequence archive number
                index: Index number

  Returns:      Sequence archive index symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetSeqArcIdxSymbol( int seqArcNo, int index )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcSeqArcOffsetTable* table;
    const NNSSndArcOffsetTable* symbolTable;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;
    
    table = (const NNSSndArcSeqArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->seqArcOffset );
    if ( table == NULL ) return & null_string;
    
    if ( seqArcNo < 0 ) return & null_string;
    if ( seqArcNo >= table->count ) return & null_string;
    
    symbolTable = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, table->offset[ seqArcNo ].table );
    if ( symbolTable == NULL ) return & null_string;
    
    return GetSymbol( symbolTable, index, arc->symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetBankSymbol

  Description:  Gets the bank symbol.

  Arguments:    bankNo: Bank number

  Returns:      Bank symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetBankSymbol( int bankNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;

    table = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->bankOffset );
    if ( table == NULL ) return & null_string;
    
    return GetSymbol( table, bankNo, arc->symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetWaveArcSymbol

  Description:  Gets the waveform archive symbol.

  Arguments:    waveArcNo: Waveform archive number

  Returns:      Waveform archive symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetWaveArcSymbol( int waveArcNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;

    table = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->waveArcOffset );
    if ( table == NULL ) return & null_string;
    
    return GetSymbol( table, waveArcNo, arc->symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetStrmSymbol

  Description:  Gets the stream symbol.

  Arguments:    strmNo: Stream number

  Returns:      Stream symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetStrmSymbol( int strmNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;
    
    table = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->strmOffset );
    if ( table == NULL ) return & null_string;
    
    return GetSymbol( table, strmNo, arc->symbol );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcGetGroupSymbol

  Description:  Gets the group symbol.

  Arguments:    groupNo: Group number

  Returns:      Waveform archive symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
const char* NNS_SndArcGetGroupSymbol( int groupNo )
{
    NNSSndArc* arc = sCurrent;
    const NNSSndArcOffsetTable* table;
    
    NNS_NULL_ASSERT( arc );
    
    if ( arc->symbol == NULL ) return & null_string;
    
    table = (const NNSSndArcOffsetTable*)GetPtrConst( arc->symbol, arc->symbol->groupOffset );
    if ( table == NULL ) return & null_string;
    
    return GetSymbol( table, groupNo, arc->symbol );
}

/******************************************************************************
    Static function
 ******************************************************************************/

#ifndef NNS_FROM_TOOL
/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcSetFilePath

  Description:  Sets a sound archive file path.
                Only the path string address will be recorded.

  Arguments:    arc: Sound archive to be set
                filePath: Sound archive file path to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndArcSetFilePath( NNSSndArc* arc, const char* filePath )
{
    NNS_NULL_ASSERT( arc );
    arc->filePath = filePath;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcGetFilePath

  Description:  Gets the sound archive file path.

  Arguments:    None.

  Returns:      File path string.
 *---------------------------------------------------------------------------*/
const char* NNSi_SndArcGetFilePath( void )
{
    NNSSndArc* arc = sCurrent;

    NNS_NULL_ASSERT( arc );
    return arc->filePath;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcSetSeekCacheInfo

  Description:  Specifies the cache memory size for the FS_SetSeekCache function.
                This is now necessary with streaming playback in sound archives on the SD Card.

  Arguments:    arc: Sound archive to be set
                cache: Address of the cache memory to set
                size: Size of the cache memory to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndArcSetSeekCacheInfo( NNSSndArc* arc, void* cache, u32 size )
{
    NNS_NULL_ASSERT( arc );
    arc->seekCache = cache;
    arc->seekCacheSize = size;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcGetSeekCache

  Description:  Gets the cache memory for the FS_SetSeekCache function.

  Arguments:    None.

  Returns:      Cache memory address
 *---------------------------------------------------------------------------*/
void* NNSi_SndArcGetSeekCache()
{
    NNSSndArc* arc = sCurrent;
    NNS_NULL_ASSERT( arc );
    return arc->seekCache;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcGetSeekCacheSize

  Description:  Gets the cache memory size for the FS_SetSeekCache function.

  Arguments:    None.

  Returns:      Cache memory size
 *---------------------------------------------------------------------------*/
u32 NNSi_SndArcGetSeekCacheSize()
{
    NNSSndArc* arc = sCurrent;
    NNS_NULL_ASSERT( arc );
    return arc->seekCacheSize;
}
#endif // NNS_FROM_TOOL

/******************************************************************************
    Static Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         GetSymbol

  Description:  Gets the symbol from the symbol offset table.

  Arguments:    table: Pointer to symbol offset table
                index: Index number
                base: Offset base address

  Returns:      Symbol string.
                Empty string on failure.
 *---------------------------------------------------------------------------*/
static const char* GetSymbol( const NNSSndArcOffsetTable* table, int index, const void* base )
{
    if ( index < 0 ) return & null_string;
    if ( index >= table->count ) return & null_string;
    
    if ( table->offset[ index ] == 0 ) return & null_string;
    
    return (const char*)GetPtrConst( base, table->offset[ index ] );
}

/*---------------------------------------------------------------------------*
  Name:         InfoDisposeCallback

  Description:  Callback function that is called when sound information table is discarded.

  Arguments:    mem: Starting address of memory block (not used)
                size: Size of memory block (not used)
                data1: User data (pointer to sound archive)
                data2: User data (not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InfoDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    NNSSndArc* arc = (NNSSndArc*)data1;
    
    (void)mem;
    (void)size;
    (void)data2;
    
    NNS_NULL_ASSERT( arc );
    
    arc->info = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         FatDisposeCallback

  Description:  Callback function that is called when file allocation table is discarded.

  Arguments:    mem: Starting address of memory block (not used)
                size: Size of memory block (not used)
                data1: User data (pointer to sound archive)
                data2: User data (not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FatDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    NNSSndArc* arc = (NNSSndArc*)data1;
    int i;
    
    (void)mem;
    (void)size;
    (void)data2;
    
    NNS_NULL_ASSERT( arc );
    NNS_NULL_ASSERT( arc->fat );
    
    for( i = 0; i < arc->fat->count ; i++ ) {
#ifndef NNS_FROM_TOOL
        NNS_ASSERTMSG(
            arc->fat->files[ i ].mem == NULL,
            "Cannot clear sndarc FAT block, because some file is on memory."
        );
#endif // NNS_FROM_TOOL
    }
    
    arc->fat = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         SymbolDisposeCallback

  Description:  Callback function that is called when symbol data is discarded.

  Arguments:    mem: Starting address of memory block (not used)
                size: Size of memory block (not used)
                data1: User data (pointer to sound archive)
                data2: User data (not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SymbolDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    NNSSndArc* arc = (NNSSndArc*)data1;

    (void)mem;
    (void)size;
    (void)data2;
    
    NNS_NULL_ASSERT( arc );

    arc->symbol = NULL;
}
    
/*====== End of sndarc.c ======*/

