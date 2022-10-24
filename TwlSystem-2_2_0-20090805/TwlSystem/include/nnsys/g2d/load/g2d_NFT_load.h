/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d - load
  File:     g2d_NFT_load.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_G2D_NFT_LOAD_H_
#define NNS_G2D_NFT_LOAD_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_config.h>
#include <nnsys/g2d/g2d_Data.h>
#include <nnsys/g2d/g2d_Font.h>

#ifdef __cplusplus
extern "C" {
#endif



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetUnpackedFont

  Description:  Expands the NFTR file to a NITRO font.

  Arguments:    pNftrFile:  Pointer to the NFTR file data.
                ppFont:     Pointer to the buffer that stores the pointer to the NITRO font.
                            

  Returns:      Returns true if the expansion was a success.
 *---------------------------------------------------------------------------*/
BOOL NNSi_G2dGetUnpackedFont( void* pNftrFile, NNSG2dFontInformation** ppFont );



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dUnpackNFT

  Description:  Expands the NFTR file data into a format that can be used by the library.
                This only needs to be executed once on each file data before that data is used in the library.
                

  Arguments:    pHeader:    Pointer to the binary file header of the NFTR file.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dUnpackNFT(NNSG2dBinaryFileHeader* pHeader);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dPrintFont

  Description:  Outputs the NITRO font information to debug output.

  Arguments:    pFont:  Pointer to the NITRO font.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_FINALROM
    NNS_G2D_INLINE void NNS_G2dPrintFont( const NNSG2dFont* /*pFont*/ )
    {
    }
#else // SDK_FINALROM
    void NNS_G2dPrintFont( const NNSG2dFont* pFont );
#endif // SDK_FINALROM



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_NFT_LOAD_H_

