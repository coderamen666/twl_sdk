/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - demolib
  File:     loader.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include "g2d_demolib.h"
#include <nnsys/g2d/g2d_Softsprite.h>
#include <nnsys/g2d/g2d_Load.h>


/*---------------------------------------------------------------------------*
  Name:         LoadFileToMainMem_

  Description:  Allocates a buffer and loads a file.

  Arguments:    pFname:        Name of the file to load

  Returns:      If data is read successfully, this function returns a pointer to the buffer storing the contents of the file.
                When this buffer is no longer needed, it must be deallocated with G2DDemo_Free.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
static void* LoadFileToMainMem_( const char* pFname )
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


        pFile = G2DDemo_Alloc( szFile );
        SDK_NULL_ASSERT( pFile );

        if( szFile != (u32)FS_ReadFile( &file, pFile, (s32)szFile ) )
        {
            G2DDemo_Free( pFile );
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
  Name:         G2DDemo_LoadNCER

  Description:  Internally allocates a buffer and loads a cell data bank from an NCER file.
                
                After successfully completing, when the cell data bank is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppCellBank: Pointer that takes a pointer to the buffer storing the cell data bank.
                            
                pFname:     Name of the NCER file where the cell data bank is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNCER( NNSG2dCellDataBank** ppCellBank, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppCellBank );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCellBank( pFile, ppCellBank ) )
        {
            NNS_G2dPrintCellBank( *ppCellBank );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNANR

  Description:  Internally allocates a buffer and loads an animation data bank from an NANR file.
                
                After successfully completing, when the animation data bank is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppAnimBank: Pointer that takes a pointer to the buffer storing the animation data bank.
                            
                pFname:     Name of the NANR file where the animation data bank is stored.
                            

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNANR( NNSG2dAnimBankData** ppAnimBank, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppAnimBank );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedAnimBank( pFile, ppAnimBank ) )
        {
            NNS_G2dPrintAnimBank( *ppAnimBank );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}
/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNMCR

  Description:  Internally allocates a buffer and loads a multicell data bank from an NMCR file.
                
                After successfully completing, when the multicell data bank is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppMCBank:   Pointer that takes a pointer to the buffer storing the multicell data bank.
                            
                pFname:     Name of the NMCR file where the multicell data bank is stored.
                            

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNMCR( NNSG2dMultiCellDataBank** ppMCBank, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppMCBank );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedMultiCellBank( pFile, ppMCBank ) )
        {
            NNS_G2dPrintMultiCellBank ( *ppMCBank );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNMAR

  Description:  Internally allocates a buffer and loads a multicell animation data bank from an NMAR file.
                
                After successfully completing, when the multicell animation data bank is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppMCABank:  Pointer that takes a pointer to the buffer storing the multicell animation data bank.
                            
                pFname:     Name of the NMAR file where the multicell animation data bank is stored.
                            

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNMAR( NNSG2dMultiCellAnimBankData** ppMCABank, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppMCABank );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedMCAnimBank( pFile, ppMCABank ) )
        {
            NNS_G2dPrintAnimBank( *ppMCABank );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNCGR

  Description:  Internally allocates a buffer and loads character-formatted character data from an NCGR file.
                
                After successfully completing, when the character data is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppCharData: Pointer that takes a pointer to the buffer storing the character data.
                            
                pFname:     Name of NCGR file where the character data is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNCGR( NNSG2dCharacterData** ppCharData, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppCharData );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCharacterData( pFile, ppCharData ) )
        {
            NNS_G2dPrintCharacterData( *ppCharData );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNCGR

  Description:  Internally allocates a buffer and loads character-formatted character data from an NCGR file.
                
                After successfully completing, when the character data is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppCharData:     Pointer that takes a pointer to the buffer storing the character data.
                                
                ppCharPosInfo:  Pointer that takes a pointer to the buffer storing the character load position data.
                                
                
                pFname:     Name of NCGR file where the character data is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNCGREx
( 
    NNSG2dCharacterData**       ppCharData, 
    NNSG2dCharacterPosInfo**    ppCharPosInfo,
    const char*                 pFname 
)
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppCharData );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCharacterData( pFile, ppCharData ) )
        {
            NNS_G2dPrintCharacterData( *ppCharData );
            
            if( NNS_G2dGetUnpackedCharacterPosInfo( pFile, ppCharPosInfo ) )
            {
                NNS_G2dPrintCharacterPosInfo( *ppCharPosInfo );   
                return pFile;
            }
        }    
    }
    
    G2DDemo_Free( pFile );
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNCLR

  Description:  Internally allocates a buffer and loads palette data from an NCLR file.
                
                After successfully completing, when the palette data is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppPltData: Pointer that takes a pointer to the buffer storing the palette data.
                            
                pFname:     Name of NCLR file where the palette data is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNCLR( NNSG2dPaletteData** ppPltData, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppPltData );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedPaletteData( pFile, ppPltData ) )
        {
            NNS_G2dPrintPaletteData( *ppPltData );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNENR

  Description:  Internally allocates a buffer and loads an entity data bank from an NENR file.
                
                After successfully completing, when the entity data bank is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppEntityBank: Pointer that takes a pointer to the buffer storing the entity data bank.
                            
                pFname:     Name of the NENR file where the entity data bank is stored.
                            

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNENR( NNSG2dEntityDataBank** ppEntityBank, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppEntityBank );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedEntityBank( pFile, ppEntityBank ) )
        {
            NNS_G2dPrintEntityDataBank( *ppEntityBank );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNCBR

  Description:  Internally allocates a buffer and loads bitmap-formatted character data from an NCBR file.
                
                After successfully completing, when the character data is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppCharData: Pointer that takes a pointer to the buffer storing the character data.
                            
                pFname:     Name of NCBR file where the character data is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNCBR( NNSG2dCharacterData** ppCharData, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppCharData );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCharacterData( pFile, ppCharData ) )
        {
            NNS_G2dPrintCharacterData( *ppCharData );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_LoadNSCR

  Description:  Internally allocates a buffer and loads bitmap-formatted character data from an NCBR file.
                
                After successfully completing, when the character data is no longer needed, the buffer must be deallocated with the G2DDemo_Free( return value ) function.
                

  Arguments:    ppCharData: Pointer that takes a pointer to the buffer storing the character data.
                            
                pFname:     Name of NCBR file where the character data is stored.

  Returns:      After a successful load, returns a pointer to the internally allocated buffer that stores the file data.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
void* G2DDemo_LoadNSCR( NNSG2dScreenData** ppScreenData, const char* pFname )
{
    void*   pFile = NULL;

    SDK_NULL_ASSERT( ppScreenData );
    SDK_NULL_ASSERT( pFname );

    pFile = LoadFileToMainMem_( pFname );
    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedScreenData( pFile, ppScreenData ) )
        {
            NNS_G2dPrintScreenData( *ppScreenData );
            return pFile;
        }

        G2DDemo_Free( pFile );
    }

    return NULL;
}
