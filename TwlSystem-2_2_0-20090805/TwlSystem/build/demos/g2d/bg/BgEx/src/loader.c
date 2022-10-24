/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - bg - BgEx
  File:     loader.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "loader.h"


#define LoadXXXX(ppData, pFname, pAlloc, unpack, print) \
    void*   pFile = NULL;                               \
                                                        \
    SDK_NULL_ASSERT( ppData );                          \
    SDK_NULL_ASSERT( pFname );                          \
                                                        \
    pFile = LoadFileToMainMem_( pFname, pAlloc );       \
    if( pFile != NULL )                                 \
    {                                                   \
        if( unpack( pFile, ppData ) )                   \
        {                                               \
            print( *ppData );                           \
            return pFile;                               \
        }                                               \
                                                        \
        NNS_FndFreeToAllocator( pAlloc, pFile );        \
    }                                                   \
                                                        \
    return NULL;                                        \


/*---------------------------------------------------------------------------*
  Name:         LoadFileToMainMem_

  Description:  Loads the specified file into memory.

  Arguments:    pFname:     Name of the file to be loaded.
                pAllocator: Pointer to the memory allocator.

  Returns:      Returns a pointer to the memory allocated by pAllocator when the function succeeds.
                
                Returns NULL if failed.
 *---------------------------------------------------------------------------*/
static void* LoadFileToMainMem_( const char* pFname, NNSFndAllocator* pAllocator )
{
    FSFile  file;
    void*   pFile = NULL;
    BOOL    bSuccess;

    SDK_NULL_ASSERT( pFname );

    FS_InitFile( &file );
    bSuccess = FS_OpenFile( &file, pFname );

    if( bSuccess )
    {
        const u32 szFile    = FS_GetLength( &file );


        pFile = NNS_FndAllocFromAllocator( pAllocator, szFile );
        SDK_NULL_ASSERT( pFile );

        if( szFile != (u32)FS_ReadFile( &file, pFile, (s32)szFile ) )
        {
            NNS_FndFreeToAllocator( pAllocator, pFile );
            pFile = NULL;
        }

        bSuccess = FS_CloseFile( &file );
        SDK_ASSERT( bSuccess );
    }else{
        OS_Warning(" Can't find the file : %s ", pFname );
    }

    return pFile;
}


/*---------------------------------------------------------------------------*
  Name:         LoadN**R

  Description:  Loads a G2D binary file into memory and constructs a runtime structure.
                

  Arguments:    ppCharData: Pointer to the buffer that stores the pointer to the character data.
                            
                ppPltData:  Pointer to the buffer that stores the pointer to the palette data.
                            
                ppScrData:  Pointer to the buffer that stores the pointer to the screen data.
                            
                pFname:     Filename of the G2D binary file.
                pAllocator: Pointer to the memory allocator.

  Returns:      Returns a pointer to the memory allocated by pAllocator when the function succeeds.
                
                Returns NULL if failed.
 *---------------------------------------------------------------------------*/
void* LoadNSCR( NNSG2dScreenData** ppScrData, const char* pFname, NNSFndAllocator* pAllocator )
{
    LoadXXXX(ppScrData, pFname, pAllocator, NNS_G2dGetUnpackedScreenData, NNS_G2dPrintScreenData);
}

void* LoadNCLR( NNSG2dPaletteData** ppPltData, const char* pFname, NNSFndAllocator* pAllocator )
{
    LoadXXXX(ppPltData, pFname, pAllocator, NNS_G2dGetUnpackedPaletteData, NNS_G2dPrintPaletteData);
}

void* LoadNCGR( NNSG2dCharacterData** ppCharData, const char* pFname, NNSFndAllocator* pAllocator )
{
    LoadXXXX(ppCharData, pFname, pAllocator, NNS_G2dGetUnpackedBGCharacterData, NNS_G2dPrintCharacterData);
}



/*---------------------------------------------------------------------------*
  Name:         LoadNCGREx

  Description:  Loads a partial character's G2D binary file into memory and constructs a runtime structure.
                

  Arguments:    ppCharData: Pointer to the buffer that stores the pointer to the character data.
                            
                ppPosInfo:  Pointer to the buffer that stores the pointer to the partial character position information data.
                            
                pFname:     Filename of the partial character G2D binary file.
                pAllocator: Pointer to the memory allocator.

  Returns:      Returns a pointer to the memory allocated by pAllocator when the function succeeds.
                
                Returns NULL if failed.
 *---------------------------------------------------------------------------*/
void* LoadNCGREx(
    NNSG2dCharacterData** ppCharData,
    NNSG2dCharacterPosInfo** ppPosInfo,
    const char* pFname,
    NNSFndAllocator* pAllocator
)
{
    void* pFile;
    BOOL bSuccess;

    pFile = LoadNCGR(ppCharData, pFname, pAllocator);
    bSuccess = NNS_G2dGetUnpackedCharacterPosInfo(pFile, ppPosInfo);

    if( ! bSuccess )
    {
        *ppPosInfo = NULL;
    }

    return pFile;
}



/*---------------------------------------------------------------------------*
  Name:         LoadNCLREx

  Description:  Loads a partial character's G2D binary file into memory and constructs a runtime structure.
                

  Arguments:    ppCharData: Pointer to the buffer that stores the pointer to the palette data.
                            
                ppPosInfo:  Pointer to the buffer that stores the pointer to the palette compression data.
                            
                pFname:     File name of the compressed palette G2D binary file.
                pAllocator: Pointer to the memory allocator.

  Returns:      Returns a pointer to the memory allocated by pAllocator when the function succeeds.
                
                Returns a NULL if failed.
 *---------------------------------------------------------------------------*/
void* LoadNCLREx(
    NNSG2dPaletteData** ppPltData,
    NNSG2dPaletteCompressInfo** ppCmpInfo,
    const char* pFname,
    NNSFndAllocator* pAllocator
)
{
    void* pFile;
    BOOL bSuccess;

    pFile = LoadNCLR(ppPltData, pFname, pAllocator);
    bSuccess = NNS_G2dGetUnpackedPaletteCompressInfo(pFile, ppCmpInfo);

    if( ! bSuccess )
    {
        *ppCmpInfo = NULL;
    }

    return pFile;
}

