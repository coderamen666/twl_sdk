/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - anm
  File:     nsbva.c

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

#include <nnsys/g3d/anm/nsbva.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>

#ifndef NNS_G3D_NSBVA_DISABLE
/*---------------------------------------------------------------------------*
    NNSi_G3dAnmObjInitNsBva

    Initializes NNSG3dAnmObj for NSBVA resources.
    This is called from NNS_G3dInitAnmObj.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dAnmObjInitNsBva(NNSG3dAnmObj* pAnmObj,
                        void* pResAnm,
                        const NNSG3dResMdl* pResMdl)
{
    u32 i;
    NNSG3dResVisAnm* visAnm;
    const NNSG3dResNodeInfo* jnt;

    visAnm = (NNSG3dResVisAnm*)pResAnm;
    jnt = NNS_G3dGetNodeInfo(pResMdl);
    pAnmObj->funcAnm = (void*) NNS_G3dFuncAnmVisNsBvaDefault;
    pAnmObj->numMapData = pResMdl->info.numNode;
   
    pAnmObj->resAnm = (void*)visAnm;
    
    //
    // Defined for all nodes for visibility animations
    //
    for (i = 0; i < pAnmObj->numMapData; ++i)
    {
        pAnmObj->mapData[i] = (u16)(i | NNS_G3D_ANMOBJ_MAPDATA_EXIST);
    }
}


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmCalcNsBva

    pResult: Stores the result of the visibility animation
    pAnmObj:
    dataIdx: Index that indicates the storage location for the data in the resource

 *---------------------------------------------------------------------------*/
void NNSi_G3dAnmCalcNsBva(NNSG3dVisAnmResult* pResult,
                          const NNSG3dAnmObj* pAnmObj,
                          u32 dataIdx)
{
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pAnmObj->resAnm);

    {
        fx32  frameRnd;
        u32   frame;
        u32   pos;
        u32   n;
        u32   mask;
        const NNSG3dResVisAnm* pVisAnm = pAnmObj->resAnm;

        // Round off the frames that are out of range
        if (pAnmObj->frame >= (pVisAnm->numFrame << FX32_SHIFT))
            frameRnd = (pVisAnm->numFrame << FX32_SHIFT) - 1;
        else if (pAnmObj->frame < 0)
            frameRnd = 0;
        else
            frameRnd = pAnmObj->frame;

        frame = (u32)FX_Whole( frameRnd );
        pos = frame * pVisAnm->numNode + dataIdx;
        n = pos >> 5;
        mask = 1U << (pos & 0x1f);

        pResult->isVisible = (BOOL)(pVisAnm->visData[n] & mask);
    }
}

#endif // NNS_G3D_NSBVA_DISABLE


