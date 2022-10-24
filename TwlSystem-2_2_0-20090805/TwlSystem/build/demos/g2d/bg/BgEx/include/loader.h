/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - bg - BgEx
  File:     loader.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef LOAD_H_
#define LOAD_H_

#include <nnsys/fnd.h>
#include <nnsys/g2d.h>

#ifdef __cplusplus
extern "C" {
#endif



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
void* LoadNCGR(
    NNSG2dCharacterData** ppCharData,
    const char* pFname,
    NNSFndAllocator* pAllocator
);
void* LoadNCLR(
    NNSG2dPaletteData** ppPltData,
    const char* pFname,
    NNSFndAllocator* pAllocator
);
void* LoadNSCR(
    NNSG2dScreenData** ppScrData,
    const char* pFname,
    NNSFndAllocator* pAllocator
);


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
);

/*---------------------------------------------------------------------------*
  Name:         LoadNCLREx

  Description:  Loads a partial character's G2D binary file into memory and constructs a run-time structure.
                

  Arguments:    ppCharData: Pointer to the buffer that stores the pointer to the palette data.
                            
                ppPosInfo:  Pointer to the buffer that stores the pointer to the palette compression data.
                            
                pFname:     Filename of the compressed palette G2D binary file.
                pAllocator: Pointer to the memory allocator.

  Returns:      Returns a pointer to the memory allocated by pAllocator when the function succeeds.
                
                Returns NULL if failed.
 *---------------------------------------------------------------------------*/
void* LoadNCLREx(
    NNSG2dPaletteData** ppPltData,
    NNSG2dPaletteCompressInfo** ppCmpInfo,
    const char* pFname,
    NNSFndAllocator* pAllocator
);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // LOAD_H_

