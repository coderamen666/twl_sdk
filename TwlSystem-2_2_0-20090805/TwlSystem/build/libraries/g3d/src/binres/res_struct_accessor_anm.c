/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - binres
  File:     res_struct_accessor_anm.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#include <nnsys/g3d/binres/res_struct_accessor.h>

#include "res_print_internal.h"




#define NNS_G3D_SIZE_NNSG3DRESJNTANM    sizeof( NNSG3dResJntAnm )
#define NNS_G3D_SIZE_ROTMTX3            ( 3 * sizeof( u16 ) )
#define NNS_G3D_SIZE_ROTMTX5            ( 5 * sizeof( u16 ) )


BOOL NNSi_G3dIsValidAnmRes(const void* pRes)
{
    NNS_G3D_NULL_ASSERT(pRes);

    if (pRes)
    {
        const NNSG3dResAnmCommon* p = (const NNSG3dResAnmCommon*) pRes;

        if (p->anmHeader.category0 == 'M' ||
            p->anmHeader.category0 == 'J' ||
            p->anmHeader.category0 == 'V')
        {
            if (p->anmHeader.category1 == 'CA' ||
                p->anmHeader.category1 == 'VA' ||
                p->anmHeader.category1 == 'MA' ||
                p->anmHeader.category1 == 'TP' ||
                p->anmHeader.category1 == 'TA')
                return TRUE;
        }
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dGetAnmByIdx

    Gets idx level animation resource from animation resource file.
    The animation type is not required.
 *---------------------------------------------------------------------------*/
void*
NNS_G3dGetAnmByIdx(const void* pRes, u32 idx)
{
    NNS_G3D_NULL_ASSERT(pRes);

    if (pRes)
    {
        const NNSG3dResFileHeader* header;
        const NNSG3dResAnmSet* anmSet;
        const NNSG3dResDictAnmSetData* anmSetData;
        u32* blks;

        header = (const NNSG3dResFileHeader*) pRes;
        blks = (u32*)((u8*)header + header->headerSize);

        anmSet = (const NNSG3dResAnmSet*)((u8*)header + blks[0]);
        anmSetData = (const NNSG3dResDictAnmSetData*)NNS_G3dGetResDataByIdx(&anmSet->dict, idx);

        if (anmSetData)
        {
            return (void*)((u8*)anmSet + anmSetData->offset);
        }
    }

    return NULL;
}


/*---------------------------------------------------------------------------*
    NNS_G3dGetAnmByName

    Gets animation resource that can be referenced with *pName from the animation resource file.
    The animation type is not required.
 *---------------------------------------------------------------------------*/
void*
NNS_G3dGetAnmByName(const void* pRes, const NNSG3dResName* pName)
{
    NNS_G3D_NULL_ASSERT(pRes);
    
    if (pRes)
    {
        const NNSG3dResFileHeader* header;
        const NNSG3dResAnmSet* anmSet;
        const NNSG3dResDictAnmSetData* anmSetData;
        u32* blks;
        header = (const NNSG3dResFileHeader*) pRes;
        blks = (u32*)((u8*)header + header->headerSize);

        anmSet = (const NNSG3dResAnmSet*)((u8*)header + blks[0]);
        anmSetData = (const NNSG3dResDictAnmSetData*)NNS_G3dGetResDataByName(&anmSet->dict, pName);

        if (anmSetData)
        {
            return (void*)((u8*)anmSet + anmSetData->offset);
        }
    }

    return NULL;
}

//---------------------------------------------------------------------
// pointer that expresses the function that accesses the data array
typedef fx32  (*FrameIdxValueAccessFuncPtr)( const void* pDataHead, u32 frameIdx, u32 step, u32 offset );


/*---------------------------------------------------------------------------*
     common animation-related functions
 *---------------------------------------------------------------------------*/

const void* 
NNSi_G3dGetBinaryBlockFromFile
( 
    const u8*  pFileHead, 
    u32        fileSignature, 
    u32        blockSignature 
)
{
    NNS_G3D_NULL_ASSERT( pFileHead );
    if (pFileHead)
    {
        u32 i = 0;
        const NNSG3dResFileHeader*   pHeader
           = (const NNSG3dResFileHeader*)&pFileHead[0];

        if( pHeader->sigVal == fileSignature )
        {        
            //
            // all binary blocks
            //
            for( i = 0 ; i < pHeader->dataBlocks; i++ )
            {
               const NNSG3dResDataBlockHeader* blk 
                   = NNS_G3dGetDataBlockHeaderByIdx( pHeader, i );
               NNS_G3D_NULL_ASSERT( blk );

               if( blk->kind == blockSignature )
               {
                   return (const void*)blk;
               }
            }
        }
    }
    //
    // could not be found
    //
    return NULL;
}


//------------------------------------------------------------------------------
// Checks if the animation header has the specified category information.
static BOOL IsValidAnimHeader( const NNSG3dResAnmHeader* pAnmHeader, u8 cat0, u16 cat1 )
{
    NNS_G3D_NULL_ASSERT( pAnmHeader );
    return (BOOL)( pAnmHeader                    &&
                   pAnmHeader->category0 == cat0 && 
                   pAnmHeader->category1 == cat1 );
}





/*---------------------------------------------------------------------------*
    visibility animation-related functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetVisAnmByIdx

  Description:  This function gets the visibility animation of the specified number from the visibility animation set.
                
                If an invalid number was input, the ASSERT will fail.
                
  Arguments:    pAnmSet:    visibility animation set
                idx:        file number
                
  Returns:      Visibility animation
  
 *---------------------------------------------------------------------------*/
const NNSG3dResVisAnm*
NNS_G3dGetVisAnmByIdx( const NNSG3dResVisAnmSet* pAnmSet, u8 idx  )
{
    NNS_G3D_NULL_ASSERT( pAnmSet );
    NNS_G3D_ASSERT( idx < pAnmSet->dict.numEntry );
    
    if (pAnmSet)
    {
        const NNSG3dResDictVisAnmSetData* pSetData =
            (const NNSG3dResDictVisAnmSetData*)NNS_G3dGetResDataByIdx( &pAnmSet->dict, idx );
        
        if (pSetData)
            return (const NNSG3dResVisAnm*)((u8*)pAnmSet + pSetData->offset);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetVisAnmSet

  Description:  This function gets the visibility animation set.
                
  Arguments:    pFileHead:        pointer to the beginning of nsbva
                
                
  Returns:      visibility animation set
  
 *---------------------------------------------------------------------------*/
 
const NNSG3dResVisAnmSet*
NNS_G3dGetVisAnmSet( const u8* pFileHead )
{
    const void *pVisBlk 
        = NNSi_G3dGetBinaryBlockFromFile( pFileHead,
                                          NNS_G3D_SIGNATURE_NSBVA,
                                          NNS_G3D_DATABLK_VIS_ANM );
    NNS_G3D_NULL_ASSERT( pVisBlk );
    return (const NNSG3dResVisAnmSet*)pVisBlk;
}





/*---------------------------------------------------------------------------*
    texture pattern animation-related functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         NNSi_G3dGetTexNameByIdx

  Description:      This function gets the texture name.
                
  Arguments:    pAnm:     Texture pattern animation
                idx:      number of specified texture  (animation result: NNSG3dResTexPatAnmFV.idTex)
                
                
  Returns:      texture name
  
 *---------------------------------------------------------------------------*/
const NNSG3dResName* 
NNSi_G3dGetTexPatAnmTexNameByIdx( const NNSG3dResTexPatAnm* pPatAnm, u8 texIdx )
{
    NNS_G3D_NULL_ASSERT( pPatAnm );
    NNS_G3D_ASSERT( texIdx < pPatAnm->numTex );
    
    if (pPatAnm && texIdx < pPatAnm->numTex)
    {
        const NNSG3dResName* pNameArray 
            = (const NNSG3dResName*)((const u8*)pPatAnm + pPatAnm->ofsTexName);
        
        return &pNameArray[texIdx];
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_G3dGetPlttNameByIdx

  Description:      This function gets the palette name.
                
  Arguments:    pAnm:     Texture pattern animation
                idx:      number of specified texture  (animation result: NNSG3dResTexPatAnmFV.idPltt)
                
                
  Returns:      palette name
  
 *---------------------------------------------------------------------------*/
const NNSG3dResName* 
NNSi_G3dGetTexPatAnmPlttNameByIdx( const NNSG3dResTexPatAnm* pPatAnm, u8 plttIdx )
{
    NNS_G3D_NULL_ASSERT( pPatAnm );
    NNS_G3D_ASSERT( plttIdx < pPatAnm->numPltt );
    
    if (pPatAnm && plttIdx < pPatAnm->numPltt)
    {
        const NNSG3dResName* pNameArray 
            = (const NNSG3dResName*)((const u8*)pPatAnm + pPatAnm->ofsPlttName);
        
        return &pNameArray[plttIdx];
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_G3dGetTexPatAnmFVByFVIndex

  Description:  This function gets the animation result from the designated FV number of the specified number of the texture pattern animation.
                
                
  Arguments:    pAnm:     Texture pattern animation
                idx:      specified material number
                fvIdx:    FV number    
                
  Returns:      texture pattern animation, animation result
  
 *---------------------------------------------------------------------------*/
const NNSG3dResTexPatAnmFV* 
NNSi_G3dGetTexPatAnmFVByFVIndex( const NNSG3dResTexPatAnm* pPatAnm, u32 idx, u32 fvIdx )
{
    NNS_G3D_NULL_ASSERT( pPatAnm );
    NNS_G3D_ASSERT( IsValidAnimHeader( &pPatAnm->anmHeader, 'M', 'TP' ) );
    
    
    {
        // gets texture pattern animation
        const NNSG3dResDictTexPatAnmData* pAnmData = 
           NNSi_G3dGetTexPatAnmDataByIdx( pPatAnm, idx );
        
        NNS_G3D_ASSERT( fvIdx < pAnmData->numFV );
        {
           // frame-> gets (texture, palette) data array beginning
           const NNSG3dResTexPatAnmFV* pfvArray 
               = (const NNSG3dResTexPatAnmFV*)((u8*)pPatAnm + pAnmData->offset);

           return &pfvArray[fvIdx];
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_G3dGetTexPatAnmFV

  Description:  This function gets the animation result from the designated frame of the specified number of the texture pattern animation.
                
                
  Arguments:    pAnm:      Texture pattern animation
                idx:       specified material number
                frame:     specified frame    
                
  Returns:      texture pattern animation, animation result
  
 *---------------------------------------------------------------------------*/
const NNSG3dResTexPatAnmFV* 
NNSi_G3dGetTexPatAnmFV( const NNSG3dResTexPatAnm* pPatAnm, u32 idx, u32 frame )
{
    NNS_G3D_ASSERT( IsValidAnimHeader( &pPatAnm->anmHeader, 'M', 'TP' ) );
    {
        // gets texture pattern animation
        const NNSG3dResDictTexPatAnmData* pAnmData = 
           NNSi_G3dGetTexPatAnmDataByIdx( pPatAnm, idx );
        {
            //
            // frame-> gets (texture, palette) data array beginning
            //
            const NNSG3dResTexPatAnmFV* pfvArray 
                = (const NNSG3dResTexPatAnmFV*)((u8*)pPatAnm + pAnmData->offset);
           
            //
            // Search for the FV index
            // Requests initial search value of index based on hint.
            // Begin searching from the initial search value and get the original index value
            //                
            const u32 fvIdx = (u32)( (fx32)pAnmData->ratioDataFrame * frame >> FX32_SHIFT);
            NNS_G3D_ASSERT( fvIdx < pAnmData->numFV );
            //
            // Begin searching from the initial search value and get the original index value
            //
            {
                u32 realFvIdx = fvIdx;
                
                // until the frame value of NNSG3dResTexPatAnmFV becomes smaller than the current frame
                while( realFvIdx > 0 && pfvArray[ realFvIdx ].idxFrame >= frame )
                {
                    realFvIdx--;
                }
                
                // until the frame of the next FV after the current NNSG3dResTexPatAnmFV becomes larger than the current frame
                while( realFvIdx + 1 < pAnmData->numFV && pfvArray[ realFvIdx + 1 ].idxFrame <= frame )
                {
                    realFvIdx++;
                }    
                NNS_G3D_ASSERT( realFvIdx < pAnmData->numFV );

                
                return &pfvArray[realFvIdx];
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_G3dGetTexPatAnmDataByIdx

  Description:  Gets the texture pattern animation data's specified number and the material's texture pattern animation structure.
                
                
  Arguments:    pPatAnm:      Texture pattern animation
                idx:          specified material number

                
  Returns:      texture pattern animation data
  
 *---------------------------------------------------------------------------*/
const NNSG3dResDictTexPatAnmData* 
NNSi_G3dGetTexPatAnmDataByIdx( const NNSG3dResTexPatAnm* pPatAnm, u32 idx )
{
    NNS_G3D_ASSERT( IsValidAnimHeader( &pPatAnm->anmHeader, 'M', 'TP' ) );
    return (const NNSG3dResDictTexPatAnmData*)NNS_G3dGetResDataByIdx( &pPatAnm->dict, idx );
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetTexPatAnmByIdx

  Description:  This function gets the texture pattern animation of the specified number from the texture pattern animation set.
                
                If an invalid number was input, the ASSERT will fail.
                
  Arguments:    pAnmSet:        texture pattern animation set
                idx:            file number
                
  Returns:      Texture pattern animation
  
 *---------------------------------------------------------------------------*/
const NNSG3dResTexPatAnm*
NNS_G3dGetTexPatAnmByIdx( const NNSG3dResTexPatAnmSet* pAnmSet, u8 idx )
{
    NNS_G3D_NULL_ASSERT( pAnmSet );
    NNS_G3D_ASSERT( idx < pAnmSet->dict.numEntry );

    if (pAnmSet)
    {
        const NNSG3dResDictTexPatAnmSetData* pSetData =
            (const NNSG3dResDictTexPatAnmSetData*)NNS_G3dGetResDataByIdx( &pAnmSet->dict, idx );

        if (pSetData)
            return (const NNSG3dResTexPatAnm*)((u8*)pAnmSet + pSetData->offset);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetTexPatAnmSet

  Description:  This function gets the texture pattern animation set.
                
  Arguments:    pFileHead:        pointer to the beginning of nsbtp
                
                
  Returns:      texture pattern animation set
  
 *---------------------------------------------------------------------------*/
 
const NNSG3dResTexPatAnmSet*
NNS_G3dGetTexPatAnmSet( const u8* pFileHead )
{
    const void *pVisBlk 
        = NNSi_G3dGetBinaryBlockFromFile( pFileHead,
                                          NNS_G3D_SIGNATURE_NSBTP,
                                          NNS_G3D_DATABLK_TEXPAT_ANM );
    NNS_G3D_NULL_ASSERT( pVisBlk );
    return (const NNSG3dResTexPatAnmSet*)pVisBlk;
}



/*---------------------------------------------------------------------------*
    texture SRT animation-related functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetTexSRTAnmByIdx

  Description:  This function gets the texture SRT animation of the specified number from the texture SRT animation set.
                If an invalid number was input, the ASSERT will fail.
                
  Arguments:    pTexSRTSet:        texture SRT animation set
                idx:        file number
                
  Returns:      Texture SRT Animation
  
 *---------------------------------------------------------------------------*/
const NNSG3dResTexSRTAnm*
NNS_G3dGetTexSRTAnmByIdx( const NNSG3dResTexSRTAnmSet* pTexSRTSet, u8 idx )
{
    NNS_G3D_NULL_ASSERT( pTexSRTSet );
    NNS_G3D_ASSERT( idx < pTexSRTSet->dict.numEntry );

    if (pTexSRTSet)
    {
        const NNSG3dResDictTexSRTAnmSetData* pSetData = 
            (const NNSG3dResDictTexSRTAnmSetData*)NNS_G3dGetResDataByIdx( &pTexSRTSet->dict, idx );
        
        if (pSetData)
            return (const NNSG3dResTexSRTAnm*)((u8*)pTexSRTSet + pSetData->offset );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetTexSRTAnmSet

  Description:  This function gets the binary block that expresses the texture SRT animation set.
                
  Arguments:    pFileHead:        pointer to the beginning of nsbta
                
                
  Returns:      texture SRT animation set
  
 *---------------------------------------------------------------------------*/
 
const NNSG3dResTexSRTAnmSet*
NNS_G3dGetTexSRTAnmSet( const u8* pFileHead )
{
    const void *pVisBlk 
        = NNSi_G3dGetBinaryBlockFromFile( pFileHead,
                                          NNS_G3D_SIGNATURE_NSBTA,
                                          NNS_G3D_DATABLK_TEXSRT_ANM );
    NNS_G3D_NULL_ASSERT( pVisBlk );
    return (const NNSG3dResTexSRTAnmSet*)pVisBlk;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetMatCAnmByIdx

  Description:  This function gets the material color animation of the specified number from the material color animation set.
                If an invalid number was input, the ASSERT will fail.
                
  Arguments:    pAnmSet:        material color animation set
                idx:        file number
                
  Returns:      material color animation
  
 *---------------------------------------------------------------------------*/
const NNSG3dResMatCAnm*
NNS_G3dGetMatCAnmByIdx( const NNSG3dResMatCAnmSet* pAnmSet, u8 idx )
{
    NNS_G3D_NULL_ASSERT( pAnmSet );
    NNS_G3D_ASSERT( idx < pAnmSet->dict.numEntry );
    
    if (pAnmSet)
    {
        const NNSG3dResDictMatCAnmSetData* pSetData = 
            (const NNSG3dResDictMatCAnmSetData*)NNS_G3dGetResDataByIdx( &pAnmSet->dict, idx );
        
        if (pSetData)
            return (const NNSG3dResMatCAnm*)((u8*)pAnmSet + pSetData->offset );
    }
    return NULL;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetMatCAnmSet

  Description:  This function gets the binary block that expresses the material color animation set.
                
  Arguments:    pFileHead:        pointer to the beginning of nsbma
                
                
  Returns:      material color animation set
  
 *---------------------------------------------------------------------------*/
 
const NNSG3dResMatCAnmSet*
NNS_G3dGetMatCAnmSet(const u8* pFileHead)
{
    const void *pVisBlk 
        = NNSi_G3dGetBinaryBlockFromFile( pFileHead,
                                          NNS_G3D_SIGNATURE_NSBMA,
                                          NNS_G3D_DATABLK_MATC_ANM );
    NNS_G3D_NULL_ASSERT( pVisBlk );
    return (const NNSG3dResMatCAnmSet*)pVisBlk;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetJntAnmSRTTag

  Description:  Gets the specified node number's NNSG3dResJntAnmSRTTag (which describes the properties of the SRT data).
                
                
                
  Arguments:    pJntAnm:           Joint animation
                nodeIdx:           node number
                
                  
  Returns:      NNSG3dResJntAnmSRTTag of specified node number 
  
 *---------------------------------------------------------------------------*/
const NNSG3dResJntAnmSRTTag*
NNS_G3dGetJntAnmSRTTag(const NNSG3dResJntAnm* pJntAnm, 
                       int                    nodeIdx)
{
    NNS_G3D_NULL_ASSERT( pJntAnm );
    NNS_G3D_ASSERT( nodeIdx < pJntAnm->numNode );
    {
        const u16*    pTagOffset 
            = (const u16*)((u8*)pJntAnm + NNS_G3D_SIZE_NNSG3DRESJNTANM);
        const u16     tagOffset  
            = pTagOffset[nodeIdx];

        return (const NNSG3dResJntAnmSRTTag*)((u8*)pJntAnm + tagOffset );
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetJntAnmByIdx

  Description:  This function gets the joint animation of the specified number from the joint animation set.
                If an invalid number was input, the ASSERT will fail.
                
  Arguments:    pJntAnmSet:           joint animation set
                idx:           joint animation number
                
  Returns:      Joint animation
  
 *---------------------------------------------------------------------------*/
const NNSG3dResJntAnm*
NNS_G3dGetJntAnmByIdx
( 
    const NNSG3dResJntAnmSet*    pJntAnmSet, 
    int                          idx 
)
{
    NNS_G3D_NULL_ASSERT( pJntAnmSet );
    NNS_G3D_ASSERT( idx < pJntAnmSet->dict.numEntry );

    if (pJntAnmSet)
    {
        const NNSG3dResDictJntAnmSetData* pJntAnmData = 
            (const NNSG3dResDictJntAnmSetData*)NNS_G3dGetResDataByIdx( &pJntAnmSet->dict, (u32)idx );

        if (pJntAnmData)
            return (const NNSG3dResJntAnm*)((u8*)pJntAnmSet + pJntAnmData->offset);
    }

    return NULL;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G3dGetJntAnmSet

  Description:  This function gets the binary block that expresses the joint animation set.
                
  Arguments:    pFileHead:        pointer to the beginning of nsbca
                
                
  Returns:      joint animation set
  
 *---------------------------------------------------------------------------*/
 
const NNSG3dResJntAnmSet*
NNS_G3dGetJntAnmSet( const u8* pFileHead )
{
    const void *pVisBlk 
        = NNSi_G3dGetBinaryBlockFromFile( pFileHead,
                                          NNS_G3D_SIGNATURE_NSBCA,
                                          NNS_G3D_DATABLK_JNT_ANM );
    NNS_G3D_NULL_ASSERT( pVisBlk );
    return (const NNSG3dResJntAnmSet*)pVisBlk;
}
