/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     model.c

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

#include <nnsys/g3d/model.h>


/*---------------------------------------------------------------------------*
    NNSi_G3dModifyMatFlag

    Turns the flag specified by 'flag' ON or OFF for all of the materials in the model.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dModifyMatFlag(NNSG3dResMdl* pMdl, BOOL isOn, NNSG3dMatFlag flag)
{
    u32 numMat;
    u32 i;
    NNSG3dResMat* mat;
    NNS_G3D_NULL_ASSERT(pMdl);

    numMat = pMdl->info.numMat;
    mat = NNS_G3dGetMat(pMdl);

    for (i = 0; i < numMat; ++i)
    {
        NNSG3dResMatData* data = NNS_G3dGetMatDataByIdx(mat, i);
        NNS_G3D_NULL_ASSERT(data);

        if (isOn)
        {
            data->flag |= flag;
        }
        else
        {
            data->flag &= ~flag;
        }
    }
}


/*---------------------------------------------------------------------------*
    NNSi_G3dModifyPolygonAttrMask

    Turns the mask ON or OFF in the PolygonAttr of all materials in the model.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dModifyPolygonAttrMask(NNSG3dResMdl* pMdl, BOOL isOn, u32 mask)
{
    u32 numMat;
    u32 i;
    NNSG3dResMat* mat;
    NNS_G3D_NULL_ASSERT(pMdl);

    numMat = pMdl->info.numMat;
    mat = NNS_G3dGetMat(pMdl);

    for (i = 0; i < numMat; ++i)
    {
        NNSG3dResMatData* data = NNS_G3dGetMatDataByIdx(mat, i);
        NNS_G3D_NULL_ASSERT(data);

        if (isOn)
        {
            data->polyAttrMask |= mask;
        }
        else
        {
            data->polyAttrMask &= ~mask;
        }
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlDiff

    Configures the diffuse of the matID number's material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlDiff(NNSG3dResMdl* pMdl, u32 matID, GXRgb col)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GXRGB_ASSERT(col);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlDiff is available only to standard(default) materials");

    data->diffAmb = (data->diffAmb & ~(REG_G3_DIF_AMB_DIFFUSE_RED_MASK | REG_G3_DIF_AMB_DIFFUSE_GREEN_MASK | REG_G3_DIF_AMB_DIFFUSE_BLUE_MASK)) |
                    (col << REG_G3_DIF_AMB_DIFFUSE_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlAmb

    Configures the ambient of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlAmb(NNSG3dResMdl* pMdl, u32 matID, GXRgb col)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GXRGB_ASSERT(col);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlAmb is available only to standard(default) materials");

    data->diffAmb = (data->diffAmb & ~(REG_G3_DIF_AMB_AMBIENT_RED_MASK | REG_G3_DIF_AMB_AMBIENT_GREEN_MASK | REG_G3_DIF_AMB_AMBIENT_BLUE_MASK)) |
                    (col << REG_G3_DIF_AMB_AMBIENT_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlSpec

    Configures the specular of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlSpec(NNSG3dResMdl* pMdl, u32 matID, GXRgb col)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GXRGB_ASSERT(col);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlSpec is available only to standard(default) materials");

    data->specEmi = (data->specEmi & ~(REG_G3_SPE_EMI_SPECULAR_RED_MASK | REG_G3_SPE_EMI_SPECULAR_GREEN_MASK | REG_G3_SPE_EMI_SPECULAR_BLUE_MASK)) |
                    (col << REG_G3_SPE_EMI_SPECULAR_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlEmi

    Configures the emission of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlEmi(NNSG3dResMdl* pMdl, u32 matID, GXRgb col)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GXRGB_ASSERT(col);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlEmi is available only to standard(default) materials");

    data->specEmi = (data->specEmi & ~(REG_G3_SPE_EMI_EMISSION_RED_MASK | REG_G3_SPE_EMI_EMISSION_GREEN_MASK | REG_G3_SPE_EMI_EMISSION_BLUE_MASK)) |
                    (col << REG_G3_SPE_EMI_EMISSION_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlLightEnableFlag

    Configures the light enable flag of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlLightEnableFlag(NNSG3dResMdl* pMdl, u32 matID, int light)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GX_LIGHTMASK_ASSERT(light);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlLightEnableFlag is available only to standard(default) materials");

    data->polyAttr = (data->polyAttr & ~REG_G3_POLYGON_ATTR_LE_MASK) |
                     (light << REG_G3_POLYGON_ATTR_LE_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlPolygonMode

    Configures the polygon mode of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlPolygonMode(NNSG3dResMdl* pMdl, u32 matID, GXPolygonMode polyMode)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GX_POLYGONMODE_ASSERT(polyMode);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlPolygonMode is available only to standard(default) materials");

    data->polyAttr = (data->polyAttr & ~REG_G3_POLYGON_ATTR_PM_MASK) |
                     (polyMode << REG_G3_POLYGON_ATTR_PM_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlCullMode

    Configures the cull mode of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlCullMode(NNSG3dResMdl* pMdl, u32 matID, GXCull cullMode)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GX_CULL_ASSERT(cullMode);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlCullMode is available only to standard(default) materials");

    data->polyAttr = (data->polyAttr & ~(REG_G3_POLYGON_ATTR_BK_MASK | REG_G3_POLYGON_ATTR_FR_MASK)) |
                     (cullMode << REG_G3_POLYGON_ATTR_BK_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlPolygonID

    Configures the polygon ID of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlPolygonID(NNSG3dResMdl* pMdl, u32 matID, int polygonID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GX_POLYGON_ATTR_POLYGONID_ASSERT(polygonID);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlPolygonID is available only to standard(default) materials");

    data->polyAttr = (data->polyAttr & ~REG_G3_POLYGON_ATTR_ID_MASK) |
                     (polygonID << REG_G3_POLYGON_ATTR_ID_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlAlpha

    Configures the alpha of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlAlpha(NNSG3dResMdl* pMdl, u32 matID, int alpha)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);
    GX_POLYGON_ATTR_ALPHA_ASSERT(alpha);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlAlpha is available only to standard(default) materials");

    data->polyAttr = (data->polyAttr & ~REG_G3_POLYGON_ATTR_ALPHA_MASK) |
                     (alpha << REG_G3_POLYGON_ATTR_ALPHA_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetFogEnableFlag

    Configures the fog enable flag of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlFogEnableFlag(NNSG3dResMdl* pMdl, u32 matID, BOOL flag)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlFogEnableFlag is available only to standard(default) materials");

    if (flag)
        data->polyAttr |= REG_G3_POLYGON_ATTR_FE_MASK;
    else
        data->polyAttr &= ~REG_G3_POLYGON_ATTR_FE_MASK;
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetDepthTestCond

    Configures the matID number's depth test conditions of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlDepthTestCond(NNSG3dResMdl* pMdl, u32 matID, BOOL flag)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlDepthTestCond is available only to standard(default) materials");

    if (flag)
        data->polyAttr |= REG_G3_POLYGON_ATTR_DT_MASK;
    else
        data->polyAttr &= ~REG_G3_POLYGON_ATTR_DT_MASK;
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdl1Dot

    Configures the matID number's 1-dot polygon display designation of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdl1Dot(NNSG3dResMdl* pMdl, u32 matID, BOOL flag)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdl1Dot is available only to standard(default) materials");

    if (flag)
        data->polyAttr |= REG_G3_POLYGON_ATTR_D1_MASK;
    else
        data->polyAttr &= ~REG_G3_POLYGON_ATTR_D1_MASK;
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlFarClip

    Configures the matID number's FAR plane intersection polygon display designation of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlFarClip(NNSG3dResMdl* pMdl, u32 matID, BOOL flag)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlFarClip is available only to standard(default) materials");

    if (flag)
        data->polyAttr |= REG_G3_POLYGON_ATTR_FC_MASK;
    else
        data->polyAttr &= ~REG_G3_POLYGON_ATTR_FC_MASK;
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlXLDepthUpdate

    Configures the depth value update enable flag of the matID number's translucent polygon of the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlXLDepthUpdate(NNSG3dResMdl* pMdl, u32 matID, BOOL flag)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);
    NNS_G3D_ASSERTMSG(data->itemTag == NNS_G3D_ITEMTAG_MAT_STANDARD,
                      "NNS_G3dMdlSetMdlXLDepthUpdate is available only to standard(default) materials");

    if (flag)
        data->polyAttr |= REG_G3_POLYGON_ATTR_XL_MASK;
    else
        data->polyAttr &= ~REG_G3_POLYGON_ATTR_XL_MASK;
}

/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlDiff

    Gets the diffuse of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXRgb
NNS_G3dMdlGetMdlDiff(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXRgb)((data->diffAmb & (REG_G3_DIF_AMB_DIFFUSE_RED_MASK | REG_G3_DIF_AMB_DIFFUSE_GREEN_MASK | REG_G3_DIF_AMB_DIFFUSE_BLUE_MASK))
                >> REG_G3_DIF_AMB_DIFFUSE_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlAmb

    Gets the ambient of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXRgb
NNS_G3dMdlGetMdlAmb(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXRgb)((data->diffAmb & (REG_G3_DIF_AMB_AMBIENT_RED_MASK | REG_G3_DIF_AMB_AMBIENT_GREEN_MASK | REG_G3_DIF_AMB_AMBIENT_BLUE_MASK))
                >> REG_G3_DIF_AMB_AMBIENT_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlSpec

    Gets the specular of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXRgb
NNS_G3dMdlGetMdlSpec(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXRgb)((data->specEmi & (REG_G3_SPE_EMI_SPECULAR_RED_MASK | REG_G3_SPE_EMI_SPECULAR_GREEN_MASK | REG_G3_SPE_EMI_SPECULAR_BLUE_MASK))
                >> REG_G3_SPE_EMI_SPECULAR_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlEmi

    Gets the emission of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXRgb
NNS_G3dMdlGetMdlEmi(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXRgb)((data->specEmi & (REG_G3_SPE_EMI_EMISSION_RED_MASK | REG_G3_SPE_EMI_EMISSION_GREEN_MASK | REG_G3_SPE_EMI_EMISSION_BLUE_MASK))
                >> REG_G3_SPE_EMI_EMISSION_RED_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlLightEnableFlag

    Gets the light enable flag of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
int
NNS_G3dMdlGetMdlLightEnableFlag(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_LE_MASK)
                    >> REG_G3_POLYGON_ATTR_LE_SHIFT );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlPolygonMode

    Gets the polygon mode of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXPolygonMode
NNS_G3dMdlGetMdlPolygonMode(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXPolygonMode)((data->polyAttr & REG_G3_POLYGON_ATTR_PM_MASK)
                                >> REG_G3_POLYGON_ATTR_PM_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlCullMode

    Gets the cull mode of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
GXCull
NNS_G3dMdlGetMdlCullMode(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (GXCull)((data->polyAttr & (REG_G3_POLYGON_ATTR_BK_MASK | REG_G3_POLYGON_ATTR_FR_MASK))
                                >> REG_G3_POLYGON_ATTR_BK_SHIFT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlPolygonID

    Gets the polygon ID of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
int
NNS_G3dMdlGetMdlPolygonID(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_ID_MASK)
                                >> REG_G3_POLYGON_ATTR_ID_SHIFT );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlAlpha

    Gets the alpha of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
int
NNS_G3dMdlGetMdlAlpha(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_ALPHA_MASK)
                                >> REG_G3_POLYGON_ATTR_ALPHA_SHIFT );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlFogEnableFlag

    Gets the fog enable flag of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dMdlGetMdlFogEnableFlag(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_FE_MASK) );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlDepthTestCond

    Gets the depth test conditions of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dMdlGetMdlDepthTestCond(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_DT_MASK) );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdl1Dot

    Gets the 1-dot polygon display designation of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dMdlGetMdl1Dot(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_D1_MASK) );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlFarClip

    Gets the FAR plane intersection polygon display designation of the matID number's material of the model resource.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dMdlGetMdlFarClip(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_FC_MASK) );
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlGetMdlXLDepthUpdate

    Gets the update enable flag used for updating the depth value of translucent polygons in the material specified by matID in the model resource.
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dMdlGetMdlXLDepthUpdate(const NNSG3dResMdl* pMdl, u32 matID)
{
    NNSG3dResMatData* data;
    NNS_G3D_NULL_ASSERT(pMdl);

    data = NNS_G3dGetMatDataByIdx(NNS_G3dGetMat(pMdl), matID);
    NNS_G3D_NULL_ASSERT(data);

    return (int)( (data->polyAttr & REG_G3_POLYGON_ATTR_XL_MASK) );
}

/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlDiffAll

    Configures all diffuse settings of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlDiffAll(NNSG3dResMdl* pMdl, GXRgb col)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlDiff(pMdl, matID, col);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlAmbAll

    Configures all ambient settings of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlAmbAll(NNSG3dResMdl* pMdl, GXRgb col)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlAmb(pMdl, matID, col);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlSpecAll

    Configures all specular settings of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlSpecAll(NNSG3dResMdl* pMdl, GXRgb col)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlSpec(pMdl, matID, col);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlEmiAll

    Configures all emission settings of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlEmiAll(NNSG3dResMdl* pMdl, GXRgb col)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlEmi(pMdl, matID, col);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlLightEnableFlagAll

    Configures all light enable flags of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlLightEnableFlagAll(NNSG3dResMdl* pMdl, int light)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlLightEnableFlag(pMdl, matID, light);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlPolygonModeAll

    Configures all polygon modes of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlPolygonModeAll(NNSG3dResMdl* pMdl, GXPolygonMode polyMode)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlPolygonMode(pMdl, matID, polyMode);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlCullModeAll

    Configures all cull modes of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlCullModeAll(NNSG3dResMdl* pMdl, GXCull cullMode)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlCullMode(pMdl, matID, cullMode);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlPolygonIDAll

    Configures all polygon IDs of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlPolygonIDAll(NNSG3dResMdl* pMdl, int polygonID)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlPolygonID(pMdl, matID, polygonID);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlAlphaAll

    Configures all alphas of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlAlphaAll(NNSG3dResMdl* pMdl, int alpha)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlAlpha(pMdl, matID, alpha);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlFogEnableFlagAll

    Configures all fog enable flags of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlFogEnableFlagAll(NNSG3dResMdl* pMdl, BOOL flag)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlFogEnableFlag(pMdl, matID, flag);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlDepthTestCondAll

    Configures all depth test conditions of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlDepthTestCondAll(NNSG3dResMdl* pMdl, BOOL flag)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlDepthTestCond(pMdl, matID, flag);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdl1DotAll

    Configures all 1-dot polygon display designations of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdl1DotAll(NNSG3dResMdl* pMdl, BOOL flag)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdl1Dot(pMdl, matID, flag);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetFarClipAll

    Configures all FAR plane intersection polygon display designations of the material inside the model resource.
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlFarClipAll(NNSG3dResMdl* pMdl, BOOL flag)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlFarClip(pMdl, matID, flag);
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlSetMdlXLDepthUpdateAll

    Sets all update enable flags used to update the depth values of translucent polygons in materials in the model resource.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dMdlSetMdlXLDepthUpdateAll(NNSG3dResMdl* pMdl, BOOL flag)
{
    u32 matID;
    for (matID = 0; matID < pMdl->info.numMat; ++matID)
    {
        NNS_G3dMdlSetMdlXLDepthUpdate(pMdl, matID, flag);
    }
}



