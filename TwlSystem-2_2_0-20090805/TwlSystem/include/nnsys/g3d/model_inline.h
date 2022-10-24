/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     model_inline.h

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

#ifndef NNSG3D_MODEL_INLINE_
#define NNSG3D_MODEL_INLINE_

//
// DO NOT INCLUDE THIS FILE DIRECTLY
//

#ifdef __cplusplus
extern "C" {
#endif

void NNSi_G3dModifyMatFlag(NNSG3dResMdl* pMdl, BOOL isOn, NNSG3dMatFlag flag);
void NNSi_G3dModifyPolygonAttrMask(NNSG3dResMdl* pMdl, BOOL isOn, u32 mask);

//
// To Use Global data
//

/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbDiff

    Uses the diffuse set with the NNS_G3dGlbMaterialColorDiffAmb function as the diffuse for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbDiff(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          FALSE,
                          NNS_G3D_MATFLAG_DIFFUSE);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbAmb

    Uses the ambient set with the NNS_G3dGlbMaterialColorDiffAmb function as the ambient for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbAmb(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          FALSE,
                          NNS_G3D_MATFLAG_AMBIENT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbSpec

    Uses the specular set with the NNS_G3dGlbMaterialColorSpecEmi function as the specular for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbSpec(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          FALSE,
                          NNS_G3D_MATFLAG_SPECULAR);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbEmi

    Uses the emission set with the NNS_G3dGlbMaterialColorSpecEmi function as the emission for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbEmi(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          FALSE,
                          NNS_G3D_MATFLAG_EMISSION);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbLightEnableFlag

    Uses the light enable flag set with the NNS_G3dGlbPolygonAttr function as the light enable flag for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbLightEnableFlag(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_LE_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbPolygonMode

    Uses the polygon mode set with the NNS_G3dGlbPolygonAttr function as the polygon mode for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbPolygonMode(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_PM_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbCullMode

    Uses the cull mode set with the NNS_G3dGlbPolygonAttr function as the cull mode for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbCullMode(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_BK_MASK |
                                  REG_G3_POLYGON_ATTR_FR_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbPolygonID

    Uses the polygon ID set with the NNS_G3dGlbPolygonAttr function as the polygon ID for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbPolygonID(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_ID_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbAlpha

    Uses the alpha value set with the NNS_G3dGlbPolygonAttr function as the alpha value for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbAlpha(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_ALPHA_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbFogEnableFlag

    Uses the fog enable flag set with the NNS_G3dGlbPolygonAttr function as the fog enable flag for materials within the model.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbFogEnableFlag(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_FE_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbDepthTestCond

    Uses the depth test condition set with the NNS_G3dGlbPolygonAttr function as the depth test condition for materials within the model.
    
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbDepthTestCond(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_DT_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlb1Dot

    Uses the 1-dot polygon display specification set with the NNS_G3dGlbPolygonAttr function as the 1-dot polygon display specification for materials within the model.
    
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlb1Dot(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_D1_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbFarClip

    Uses the far plane intersection polygon display specification set with the NNS_G3dGlbPolygonAttr function as the far plane intersection polygon display specification for materials within the model.
    
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbFarClip(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_FC_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseGlbXLDepthUpdate

    Uses the translucent polygon depth value update enable flag set with the NNS_G3dGlbPolygonAttr function as the translucent polygon depth value update enable flag for materials within the model.
    
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseGlbXLDepthUpdate(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  FALSE,
                                  REG_G3_POLYGON_ATTR_XL_MASK);
}


//
// To Use Model's Data
//

/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlDiff

    This function uses the diffuse values for the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlDiff(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          TRUE,
                          NNS_G3D_MATFLAG_DIFFUSE);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlAmb

    This function uses the ambient values for the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlAmb(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          TRUE,
                          NNS_G3D_MATFLAG_AMBIENT);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlSpec

    This function uses the specular values for the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlSpec(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          TRUE,
                          NNS_G3D_MATFLAG_SPECULAR);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlEmi

    This function uses the emission values for the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlEmi(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyMatFlag(pMdl,
                          TRUE,
                          NNS_G3D_MATFLAG_EMISSION);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlLightEnableFlag

    This function uses the light enable flags of the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlLightEnableFlag(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_LE_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlPolygonMode

    This function uses the polygon modes of the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlPolygonMode(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_PM_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlCullMode

    This function uses the cull modes of the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlCullMode(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_BK_MASK |
                                  REG_G3_POLYGON_ATTR_FR_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlPolygonID

    This function uses the polygon IDs of the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlPolygonID(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_ID_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlAlpha

    This function uses the alpha values of the individual materials that are configured inside the model resource.
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlAlpha(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_ALPHA_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlFogEnableFlag

    This function uses the fog enable flags of the individual materials that are configured inside the model resource.
    
    (This is the default configuration of the .nsbmd file)
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlFogEnableFlag(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_FE_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlDepthTestCond

    This function uses the depth test conditions for the individual materials that are configured inside the model resource.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlDepthTestCond(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_DT_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdl1Dot

    This function uses the 1-dot polygon display specifications for the individual materials  that are configured inside the model resource.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdl1Dot(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_D1_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlFarClip

    This function uses the far plane intersecting polygon display specifications for the individual materials that are configured inside the model resource.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlFarClip(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_FC_MASK);
}


/*---------------------------------------------------------------------------*
    NNS_G3dMdlUseMdlXLDepthUpdate

    Use the update enable flag when updating the depth value of translucent polygons set within the model resource.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dMdlUseMdlXLDepthUpdate(NNSG3dResMdl* pMdl)
{
    NNSi_G3dModifyPolygonAttrMask(pMdl,
                                  TRUE,
                                  REG_G3_POLYGON_ATTR_XL_MASK);
}





#ifdef __cplusplus
}
#endif

#endif
