/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - anm
  File:     nsbma.c

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

#include <nnsys/g3d/anm/nsbma.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>

#ifndef NNS_G3D_NSBMA_DISABLE
//---------------------------------------------------------------------
static u16
GetMatColAnmValue_(const NNSG3dResMatCAnm* pAnm, u32 info, u32 frame)
{
    const u16* pDataHead;
    u32 last_interp;
    u32 idx;

    if (info & NNS_G3D_MATCANM_ELEM_CONST)
    {
        return (u16)(info & NNS_G3D_MATCANM_ELEM_OFFSET_CONSTANT_MASK);
    }

    pDataHead = (const u16*)((const u8*)pAnm + (info & NNS_G3D_MATCANM_ELEM_OFFSET_CONSTANT_MASK));
    
    if (!(info & NNS_G3D_MATCANM_ELEM_STEP_MASK))
    {
        // NNS_G3D_MATCANM_ELEM_STEP_1 is selected
        return *(pDataHead + frame);
    }
    
    //
    // When it is not a constant, but NNS_G3D_MATCANM_ELEM_STEP_2 or NNS_G3D_MATCANM_ELEM_STEP_4
    //
    
    // Data is present in each frame starting from last_interp
    // last_interp is either a multiple of 2 or 4
    last_interp = (NNS_G3D_MATCANM_ELEM_LAST_INTERP_MASK & info)
                          >> NNS_G3D_MATCANM_ELEM_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_MATCANM_ELEM_STEP_2)
    {
        // NNS_G3D_MATCANM_ELEM_STEP_2 is selected
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // Impossible if anything other than the last frame
                return *(pDataHead + (last_interp >> 1) + 1);
            }
            else
            {
                // Because the final frame is not on an odd number, 50:50 interpolation is needed
                idx = frame >> 1;
                goto MATC_INTERP_50_50;
            }
        }
        else
        {
            // Even frame, so no interpolation is needed
            return *(pDataHead + (frame >> 1));
        }
    }
    else
    {
        // NNS_G3D_MATCANM_ELEM_STEP_4 is selected
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                return *(pDataHead + (last_interp >> 2) + (frame & 3));
            }

            // With interpolation management
            if (frame & 1)
            {
                u32 idx_sub;
                u32 v, v_sub;
                u32 c, c_sub;
                u32 rval;
                const u32 cmask0 = GX_RGB_R_MASK | GX_RGB_B_MASK;
                const u32 cmask1 = GX_RGB_G_MASK;

                if (frame & 2)
                {
                    // Interpolate with 3:1 position
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // Interpolate with 1:3 position
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }

                // Interpolation for when 1:3 and 3:1
                v = *(pDataHead + idx);
                v_sub = *(pDataHead + idx_sub);

                c = v & cmask0;
                c_sub = v_sub & cmask0;
                rval = ((c + c + c + c_sub) >> 2) & cmask0;

                c = v & cmask1;
                c_sub = v_sub & cmask1;
                rval |= ((c + c + c + c_sub) >> 2) & cmask1;

                return (u16)rval;
            }
            else
            {
                // Becomes the 50:50 interpolation
                idx = frame >> 2;
                goto MATC_INTERP_50_50;
            }
        }
        else
        {
            // Frames are exactly multiples of 4
            return *(pDataHead + (frame >> 2));
        }
    }
    NNS_G3D_ASSERT(0);
MATC_INTERP_50_50:
    {
        u32 rval;
        u32 v0 = *(pDataHead + idx);
        u32 v1 = *(pDataHead + idx + 1);
        const u32 cmask0 = GX_RGB_R_MASK | GX_RGB_B_MASK;
        const u32 cmask1 = GX_RGB_G_MASK;

        // Red & blue
        rval = (((v0 & cmask0) + (v1 & cmask0)) >> 1) & cmask0;
        // Green
        rval |= (((v0 & cmask1) + (v1 & cmask1)) >> 1) & cmask1;

        return (u16) rval;
    }
}


//------------------------------------------------------------------------------
static u16
GetMatColAnmuAlphaValue_(const NNSG3dResMatCAnm* pAnm, 
                         u32 info, 
                         u32 frame)
{
    const u8* pDataHead;
    u32 last_interp;
    
    if( info & NNS_G3D_MATCANM_ELEM_CONST )
    {
        return (u16)(info & NNS_G3D_MATCANM_ELEM_OFFSET_CONSTANT_MASK);
    }

    pDataHead = (const u8*)pAnm + (info & NNS_G3D_MATCANM_ELEM_OFFSET_CONSTANT_MASK);

    if (!(info & NNS_G3D_MATCANM_ELEM_STEP_MASK))
    {
        // NNS_G3D_MATCANM_ELEM_STEP_1 is selected
        return *(pDataHead + frame);
    }

    //
    // When it is not a constant, but NNS_G3D_MATCANM_ELEM_STEP_2 or NNS_G3D_MATCANM_ELEM_STEP_4
    //
    
    // Data is present in each frame starting from last_interp
    // last_interp is either a multiple of 2 or 4
    last_interp = (NNS_G3D_MATCANM_ELEM_LAST_INTERP_MASK & info)
                          >> NNS_G3D_MATCANM_ELEM_LAST_INTERP_SHIFT;
    if (info & NNS_G3D_MATCANM_ELEM_STEP_2)
    {
        // NNS_G3D_MATCANM_ELEM_STEP_2 is selected
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // Impossible if anything other than the last frame
                return *(pDataHead + (last_interp >> 1) + 1);
            }
            else
            {
                // Because the final frame is not on an odd number, 50:50 interpolation is needed
                return (u16)((*(pDataHead + (frame >> 1)) + *(pDataHead + (frame >> 1) + 1)) >> 1);
            }
        }
        else
        {
            // Even frame, so no interpolation is needed
            return *(pDataHead + (frame >> 1));
        }
    }
    else
    {
        // NNS_G3D_MATCANM_ELEM_STEP_4 is selected
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                return *(pDataHead + (last_interp >> 2) + (frame & 3));
            }

            // With interpolation management
            if (frame & 1)
            {
                u32 idx, idx_sub;
                u32 v, v_sub;

                if (frame & 2)
                {
                    // Interpolate with 3:1 position
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // Interpolate with 1:3 position
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }

                // Interpolation for when 1:3 and 3:1
                v = *(pDataHead + idx);
                v_sub = *(pDataHead + idx_sub);

                return (u16)((v + v + v + v_sub) >> 2);
            }
            else
            {
                // Becomes the 50:50 interpolation
                return (u16)((*(pDataHead + (frame >> 2)) + *(pDataHead + (frame >> 2) + 1)) >> 1);
            }
        }
        else
        {
            // Frames are exactly multiples of 4.
            return *(pDataHead + (frame >> 2));
        }
    }
    NNS_G3D_ASSERT(0);
}


/*---------------------------------------------------------------------------*
  Name:         GetMatColAnm_

  Description:  Gets the animation result from the designated frame of the specified number of the material color animation.
                
                
  Arguments:    pAnm: Material color animation
                idx: Specified material number
                frame: Specified frame    
                pResult: Animation result
                
  Returns:      Material color animation.
  
 *---------------------------------------------------------------------------*/
static NNS_G3D_INLINE void
GetMatColAnm_(const NNSG3dResMatCAnm* pAnm, 
              u16 idx, 
              u32 frame, 
              NNSG3dMatAnmResult* pResult)
{
    NNS_G3D_NULL_ASSERT( pAnm );
    NNS_G3D_ASSERT( idx < pAnm->dict.numEntry );
    NNS_G3D_ASSERT( frame < pAnm->numFrame );
    NNS_G3D_NULL_ASSERT( pResult );
    NNS_G3D_ASSERT( (pAnm->anmHeader.category0 == 'M') && (pAnm->anmHeader.category1 == 'MA') );
    
    {
        u16 diffuse, ambient, emission, specular, polygon_alpha;
        const NNSG3dResDictMatCAnmData*  pAnmData = 
           (const NNSG3dResDictMatCAnmData*)NNS_G3dGetResDataByIdx( &pAnm->dict, idx );

        // Diffuse ambient
        diffuse = GetMatColAnmValue_(pAnm, pAnmData->diffuse, frame);
        ambient = GetMatColAnmValue_(pAnm, pAnmData->ambient, frame);

        pResult->prmMatColor0 = GX_PACK_DIFFAMB_PARAM(diffuse, ambient,
                                                      (pResult->prmMatColor0 & REG_G3_DIF_AMB_C_MASK));
                                                      
        // Specular emission  
        emission = GetMatColAnmValue_(pAnm, pAnmData->emission, frame);
        specular = GetMatColAnmValue_(pAnm, pAnmData->specular, frame);
        pResult->prmMatColor1 = GX_PACK_SPECEMI_PARAM(specular, emission,
                                                      (pResult->prmMatColor1 & REG_G3_SPE_EMI_S_MASK));

        // Alpha
        polygon_alpha = GetMatColAnmuAlphaValue_( pAnm, pAnmData->polygon_alpha, frame );
        pResult->prmPolygonAttr = (pResult->prmPolygonAttr & ~REG_G3_POLYGON_ATTR_ALPHA_MASK) |
                                  (polygon_alpha << REG_G3_POLYGON_ATTR_ALPHA_SHIFT);
    }
}


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmObjInitNsBma

    Initializes NNSG3dAnmObj for NSBMA resources.
    This is called from NNS_G3dInitAnmObj.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dAnmObjInitNsBma(NNSG3dAnmObj* pAnmObj,
                        void* pResAnm,
                        const NNSG3dResMdl* pResMdl)
{
    u32 i;
    NNSG3dResMatCAnm* matCAnm;
    const NNSG3dResMat* mat;

    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pResAnm);
    NNS_G3D_NULL_ASSERT(pResMdl);

    matCAnm = (NNSG3dResMatCAnm*)pResAnm;
    mat = NNS_G3dGetMat(pResMdl);
    pAnmObj->funcAnm = (void*) NNS_G3dFuncAnmMatNsBmaDefault;
    pAnmObj->numMapData = pResMdl->info.numMat;

    // Zero-clear the mapData first
    MI_CpuClear16(&pAnmObj->mapData[0], sizeof(u16) * pAnmObj->numMapData);

    for (i = 0; i < matCAnm->dict.numEntry; ++i)
    {
        const NNSG3dResName* name = NNS_G3dGetResNameByIdx(&matCAnm->dict, i);
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
    NNSi_G3dAnmCalcNsBma

    pResult: Stores the material animation result
    pAnmObj:
    dataIdx: Index that indicates the storage location for the data in the resource
 *---------------------------------------------------------------------------*/
void NNSi_G3dAnmCalcNsBma(NNSG3dMatAnmResult* pResult,
                          const NNSG3dAnmObj* pAnmObj,
                          u32 dataIdx)
{
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_NULL_ASSERT(pAnmObj);
    {
        fx32 frame;
        const NNSG3dResMatCAnm*     pMatAnm 
                = (const NNSG3dResMatCAnm*)pAnmObj->resAnm;

        // Round off the frames that are out of range
        if (pAnmObj->frame >= (pMatAnm->numFrame << FX32_SHIFT))
            frame = (pMatAnm->numFrame << FX32_SHIFT) - 1;
        else if (pAnmObj->frame < 0)
            frame = 0;
        else
            frame = pAnmObj->frame;

        //
        // Get the animation result
        //
        GetMatColAnm_( pMatAnm,
                       (u16)dataIdx,                    // TODO: Argument type needs to match
                       (u32)FX_Whole( frame ),
                       pResult );
    }
}

#endif // NNS_G3D_NSBMA_DISABLE
