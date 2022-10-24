/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - anm
  File:     nsbtp.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1397 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#include <nnsys/g3d/anm/nsbtp.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>

#ifndef NNS_G3D_NSBTP_DISABLE
/*---------------------------------------------------------------------------*
    NNSi_G3dAnmObjInitNsBtp

    Initializes NNSG3dAnmObj for NSBTP resources.
    This is called from NNS_G3dInitAnmObj.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dAnmObjInitNsBtp(NNSG3dAnmObj*       pAnmObj,
                        void*               pResAnm,
                        const NNSG3dResMdl* pResMdl)
{
    u32 i;
    NNSG3dResTexPatAnm* tpAnm = (NNSG3dResTexPatAnm*)pResAnm;
    const NNSG3dResMat* mat = NNS_G3dGetMat(pResMdl);

    NNS_G3D_NULL_ASSERT(pAnmObj->resTex);

    pAnmObj->funcAnm    = (void*) NNS_G3dFuncAnmMatNsBtpDefault;
    pAnmObj->numMapData = pResMdl->info.numMat;
    pAnmObj->resAnm     = tpAnm;

    // Zero-clear the mapData first
    MI_CpuClear16(&pAnmObj->mapData[0], sizeof(u16) * pAnmObj->numMapData);

    for (i = 0; i < tpAnm->dict.numEntry; ++i)
    {
        const NNSG3dResName* name = NNS_G3dGetResNameByIdx(&tpAnm->dict, i);
        int idx = NNS_G3dGetMatIdxByName(mat, name);
        if (!(idx < 0))
        {
            // When a resource corresponding to resource ID 'i' exists, associate the material ID 'idx' with the resource ID 'i'
            // 
            pAnmObj->mapData[idx] = (u16)(i | NNS_G3D_ANMOBJ_MAPDATA_EXIST);
        }
    }
}


/*---------------------------------------------------------------------------*
    SetTexParamaters_

    Set the texture-related part of pResult.
 *---------------------------------------------------------------------------*/
static void SetTexParamaters_
( 
    const NNSG3dResTex*     pTex, 
    const NNSG3dResName*    pTexName, 
    NNSG3dMatAnmResult*     pResult 
)
{
    NNS_G3D_NULL_ASSERT( pTex );
    NNS_G3D_NULL_ASSERT( pTexName );
    NNS_G3D_NULL_ASSERT( pResult );
    NNS_G3D_ASSERTMSG((pTex->texInfo.vramKey != 0 || pTex->texInfo.sizeTex == 0),
                      "No texture key assigned");
    NNS_G3D_ASSERTMSG((pTex->tex4x4Info.vramKey != 0 || pTex->tex4x4Info.sizeTex == 0),
                      "No texture(4x4) key assigned");
    {
        //
        // From names, consult dictionary for texture data
        //
        const NNSG3dResDictTexData* pData = NNS_G3dGetTexDataByName( pTex, pTexName );        
        NNS_G3D_NULL_ASSERT( pData );
        
        {
            const u32 vramOffset = (( pData->texImageParam & REG_G3_TEXIMAGE_PARAM_TEXFMT_MASK) !=
                                                (GX_TEXFMT_COMP4x4 << REG_G3_TEXIMAGE_PARAM_TEXFMT_SHIFT)) ?
                                    NNS_GfdGetTexKeyAddr(pTex->texInfo.vramKey) >> NNS_GFD_TEXKEY_ADDR_SHIFT :    // Some texture other than 4x4
                                    NNS_GfdGetTexKeyAddr(pTex->tex4x4Info.vramKey) >> NNS_GFD_TEXKEY_ADDR_SHIFT;  // A 4x4 texture

            // Leave "flip," "repeat," and "texgen," and reset, then combine.
            // Leave the data that materials have.
            pResult->prmTexImage &= REG_G3_TEXIMAGE_PARAM_TGEN_MASK |
                                    REG_G3_TEXIMAGE_PARAM_FT_MASK | REG_G3_TEXIMAGE_PARAM_FS_MASK |
                                    REG_G3_TEXIMAGE_PARAM_RT_MASK | REG_G3_TEXIMAGE_PARAM_RS_MASK;
            pResult->prmTexImage |= pData->texImageParam + vramOffset;
            
            pResult->origWidth  = (u16)(( pData->extraParam & NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_MASK ) >> 
                                    NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_SHIFT);
            pResult->origHeight = (u16)(( pData->extraParam & NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_MASK ) >> 
                                    NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_SHIFT);
            
            //
            // Compute the horizontal and vertical scale ratios needed for calculating the texture matrix
            //
            {
                const s32 w = (s32)(((pData->extraParam) & NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_MASK) >> NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_SHIFT);
                const s32 h = (s32)(((pData->extraParam) & NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_MASK) >> NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_SHIFT);
                                
                pResult->magW = ( w != pResult->origWidth ) ?
                                FX_Div( w << FX32_SHIFT, pResult->origWidth << FX32_SHIFT) :
                                FX32_ONE;
                pResult->magH = ( h != pResult->origHeight ) ?
                                FX_Div( h << FX32_SHIFT, pResult->origHeight << FX32_SHIFT) :
                                FX32_ONE;
            }
        }
    }
}


/*---------------------------------------------------------------------------*
    SetPlttParamaters_

    Sets the palette-related part of pResult.
 *---------------------------------------------------------------------------*/
static void SetPlttParamaters_
( 
    const NNSG3dResTex*     pTex, 
    const NNSG3dResName*    pPlttName, 
    NNSG3dMatAnmResult*     pResult 
)
{
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_NULL_ASSERT(pPlttName);
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_ASSERTMSG((pTex->plttInfo.vramKey != 0 || pTex->plttInfo.sizePltt == 0),
         "No palette key assigned");
    
    {
        // Get the data field corresponding to the palette name from the Texture block
        const NNSG3dResDictPlttData* pPlttData = NNS_G3dGetPlttDataByName(pTex, pPlttName);
        u16 plttBase    = pPlttData->offset;
        u16 vramOffset  = (u16)(NNS_GfdGetTexKeyAddr(pTex->plttInfo.vramKey) >> NNS_GFD_TEXKEY_ADDR_SHIFT);
        
        NNS_G3D_NULL_ASSERT(pPlttData);    
        
        
        // If 4 colors, this bit is set
        if (!(pPlttData->flag & 1))
        {
            // Four-bit shift if other than 4colors
            // If 4 colors, shift is 3-bit shift, so it's left as is
            plttBase >>= 1;
            vramOffset >>= 1;
        }
        
        //
        // Set address
        //
        pResult->prmTexPltt = (u32)(plttBase + vramOffset);
    }
}


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmCalcNsBtp

    pResult: Stores the material animation result
    pAnmObj:
    dataIdx: Index that indicates the storage location for the data in the resource
 *---------------------------------------------------------------------------*/
void NNSi_G3dAnmCalcNsBtp(NNSG3dMatAnmResult* pResult,
                          const NNSG3dAnmObj* pAnmObj,
                          u32 dataIdx)
{
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pAnmObj->resTex);
    
    {
        fx32 frame;
        //
        // Gets the texture pattern FV that corresponds to the current frame
        //
        const NNSG3dResTexPatAnm*     pPatAnm 
            = (const NNSG3dResTexPatAnm*)pAnmObj->resAnm;
        const NNSG3dResTexPatAnmFV*   pTexFV;
        
        // Round off the frames that are out of range
        if (pAnmObj->frame >= (pPatAnm->numFrame << FX32_SHIFT))
            frame = (pPatAnm->numFrame << FX32_SHIFT) - 1;
        else if (pAnmObj->frame < 0)
            frame = 0;
        else
            frame = pAnmObj->frame;
            
        pTexFV = NNSi_G3dGetTexPatAnmFV( pPatAnm, 
                                        (u16)dataIdx, 
                                        (u16)FX_Whole( frame ) );
        NNS_G3D_NULL_ASSERT(pTexFV);
        
        //
        // Set the texture-related part of pResult
        //
        SetTexParamaters_( pAnmObj->resTex, 
                           NNSi_G3dGetTexPatAnmTexNameByIdx( pPatAnm, pTexFV->idTex ),
                           pResult ); 
        //                   
        // Sets the palette-related part of pResult
        //
        
        //
        // In the current implementation of the converter, 255 is used for pTexFV->idPltt when referencing direct texture formats.
        // When 255 is set for pTexFV->idPltt, the library must skip palette setup processing.
        // 
        //
        if( pTexFV->idPltt != 255 )
        {
            SetPlttParamaters_( pAnmObj->resTex, 
                                NNSi_G3dGetTexPatAnmPlttNameByIdx( pPatAnm, pTexFV->idPltt ),
                                pResult ); 
        }                                     
    }
}

#endif // NNS_G3D_NSBTP_DISABLE
