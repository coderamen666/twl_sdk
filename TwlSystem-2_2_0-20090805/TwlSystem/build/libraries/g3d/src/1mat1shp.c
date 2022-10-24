/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     1mat1shp.c

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

#include <nnsys/g3d/1mat1shp.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>
#include <nnsys/g3d/gecom.h>
#include <nnsys/g3d/cgtool.h>


/*---------------------------------------------------------------------------*
    NNS_G3dDraw1Mat1Shp

    Renders after matID and shpID are specified. This function is for rendering simple models.
    If sendMat is FALSE, no material will be sent.
    See the 1mat1shp sample for details on how to use this function.
 *---------------------------------------------------------------------------*/
void
NNS_G3dDraw1Mat1Shp(const NNSG3dResMdl* pResMdl,
                    u32 matID,
                    u32 shpID,
                    BOOL sendMat)
{
    NNS_G3D_NULL_ASSERT(pResMdl);

    if (pResMdl->info.posScale != FX32_ONE)
    {
        // when scale correction by pos_scale is needed
        NNS_G3dGeScale(pResMdl->info.posScale,
                       pResMdl->info.posScale,
                       pResMdl->info.posScale);
    }

    if (sendMat)
    {
        NNS_G3D_ASSERT(matID < pResMdl->info.numMat);
        if (matID < pResMdl->info.numMat)
        {
            //
            // when sending materials:
            // -It is impossible to play the material animation. (If the material data stored in the model data is rewritten, the same result can be produced.)
            //   
            // -Global material information that is stored in the NNS_G3dGlb structure is not reflected.
            //   The material data stored in the model data is reflected.
            //
            const NNSG3dResMatData* mat;
            u32 cmd[7];
            
            {
                // Gets the pointer to NNSG3dResMatData.
                const NNSG3dResMat* p = NNS_G3dGetMat(pResMdl);
                NNS_G3D_NULL_ASSERT(p);
                mat = NNS_G3dGetMatDataByIdx(p, matID);
                NNS_G3D_NULL_ASSERT(mat);
            }

            // Returns without rendering if transparent.
            if (!(mat->polyAttr & REG_G3_POLYGON_ATTR_ALPHA_MASK))
                return;

            // G3_MaterialColorDiffAmb(mat->diffAmb);
            // G3_MaterialColorSpecEmi(mat->specEmi);
            // G3_PolygonAttr(...);
            cmd[0] = GX_PACK_OP(G3OP_DIF_AMB, G3OP_SPE_EMI, G3OP_POLYGON_ATTR, G3OP_NOP);
            cmd[1] = mat->diffAmb;
            cmd[2] = mat->specEmi;
            cmd[3] = mat->polyAttr;

            if (mat->flag & NNS_G3D_MATFLAG_WIREFRAME)
            {
                // For wire frame display, alpha to send to the geometry engine is 0.
                // Set the alpha bit to 0.
                cmd[3] &= ~REG_G3_POLYGON_ATTR_ALPHA_MASK;
            }

            // G3_TexImageParam(...);
            // G3_TexPlttBase(...);
            cmd[4] = GX_PACK_OP(G3OP_TEXIMAGE_PARAM, G3OP_TEXPLTT_BASE, G3OP_NOP, G3OP_NOP);
            cmd[5] = mat->texImageParam;
            cmd[6] = mat->texPlttBase;

            // send together to the FIFO
            NNS_G3dGeBufferData_N(&cmd[0], 7);

            if (mat->flag & NNS_G3D_MATFLAG_TEXMTX_USE)
            {
                // set the texture matrix
                NNSG3dSendTexSRT func;
                NNSG3dMatAnmResult dummy;

                // Depending on the value of mat->flag, there is SRT data at the back of NNSG3dResMatData.
                const u8* p = (const u8*)mat + sizeof(NNSG3dResMatData);
                
                dummy.flag = NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SET;
                dummy.origWidth = mat->origWidth;
                dummy.origHeight = mat->origHeight;
                dummy.magW = mat->magW;
                dummy.magH = mat->magH;

                //
                // Below, get the latter part of the NNSG3dResMatData data while looking at the flag value.
                //

                // set Scale of the texture matrix
                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_SCALEONE))
                {
                    const fx32* p_fx32 = (const fx32*)p;

                    dummy.scaleS = *(p_fx32 + 0);
                    dummy.scaleT = *(p_fx32 + 1);
                    p += 2 * sizeof(fx32);
                }
                else
                {
                    dummy.flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SCALEONE;
                }

                // set Rotation of the texture matrix
                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_ROTZERO))
                {
                    const fx16* p_fx16 = (const fx16*)p;

                    dummy.sinR = *(p_fx16 + 0);
                    dummy.cosR = *(p_fx16 + 1);
                    p += 2 * sizeof(fx16);
                }
                else
                {
                    dummy.flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_ROTZERO;
                }

                // set Translation of the texture matrix
                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_TRANSZERO))
                {
                    const fx32* p_fx32 = (const fx32*)p;

                    dummy.transS = *(p_fx32 + 0);
                    dummy.transT = *(p_fx32 + 1);
                }
                else
                {
                    dummy.flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_TRANSZERO;
                }

                func = NNS_G3dSendTexSRT_FuncArray[pResMdl->info.texMtxMode];
                NNS_G3D_NULL_ASSERT(func);
                if (func)
                {
                    // set the texture matrix to the geometry engine
                    // configure each of the CG tools used to create the model
                    (*func)(&dummy);
                }
            }
        }
    }

    NNS_G3D_ASSERT(shpID < pResMdl->info.numShp);
    if (shpID < pResMdl->info.numShp)
    {
        // send Shp
        const NNSG3dResShp* p;
        const NNSG3dResShpData* shp;
        
        p = NNS_G3dGetShp(pResMdl);
        NNS_G3D_NULL_ASSERT(p);
        shp = NNS_G3dGetShpDataByIdx(p, shpID);
        NNS_G3D_NULL_ASSERT(shp);

        NNS_G3dGeSendDL(NNS_G3dGetShpDLPtr(shp), NNS_G3dGetShpDLSize(shp));
    }

    if (pResMdl->info.invPosScale != FX32_ONE)
    {
        // restore to original if the scale correction is applied by pos_scale
        NNS_G3dGeScale(pResMdl->info.invPosScale,
                       pResMdl->info.invPosScale,
                       pResMdl->info.invPosScale);
    }
}








