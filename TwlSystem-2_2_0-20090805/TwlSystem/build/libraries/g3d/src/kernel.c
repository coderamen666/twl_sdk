/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     kernel.c

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

#include <nnsys/g3d/kernel.h>
#include <nnsys/g3d/anm.h>
#include <nnsys/g3d/binres/res_struct.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>


/*---------------------------------------------------------------------------*
    NNS_G3dAnmObjCalcSizeRequired

    Returns the memory size needed for the NNSG3dAnmObj corresponding to the animation and model resources.
    
 *---------------------------------------------------------------------------*/
u32
NNS_G3dAnmObjCalcSizeRequired(const void* pAnm,
                              const NNSG3dResMdl* pMdl)
{
    const NNSG3dResAnmHeader* hdr;
    NNS_G3D_NULL_ASSERT(pAnm);
    NNS_G3D_NULL_ASSERT(pMdl);
    if (!pAnm || !pMdl)
        return 0;

    hdr = (const NNSG3dResAnmHeader*)pAnm;
    switch(hdr->category0)
    {
    case 'M': // material animation
        return NNS_G3D_ANMOBJ_SIZE_MATANM(pMdl);
        break;
    case 'J': // character (joint) animation
    case 'V': // visibility animation
        return NNS_G3D_ANMOBJ_SIZE_JNTANM(pMdl);
        break;
    default:
        NNS_G3D_ASSERT(0);
        return 0;
        break;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dInitAnmObj

    initialize pAnmObj for pResAnm and pResMdl

    NOTICE:
    pAnmObj must have a minimum size of NNS_G3dAnmObjCalcSizeRequired(pResAnm, pResMdl).
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dAnmObjInit(NNSG3dAnmObj* pAnmObj,
                  void* pResAnm,
                  const NNSG3dResMdl* pResMdl,
                  const NNSG3dResTex* pResTex)
{
    // When the programmer is going to define an animation or when there are no resources, simply create a fake header and use that instead.
    // 
    const NNSG3dResAnmHeader* hdr;
    u32 i;
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pResAnm);
    NNS_G3D_NULL_ASSERT(pResMdl);
    // resTex can be NULL, except for texture pattern animation.
    NNS_G3D_ASSERT(NNS_G3dAnmFmtNum <= NNS_G3D_ANMFMT_MAX);

    pAnmObj->frame = 0;
    pAnmObj->resAnm = (void*)pResAnm;

    // Animation function settings are needed.
    // This is determined based on category1.
    pAnmObj->next = NULL;
    pAnmObj->priority = 127;
    pAnmObj->ratio = FX32_ONE;
    pAnmObj->resTex = pResTex;
    pAnmObj->numMapData = 0; // for supporting the animation disable macro (NNS_G3D_NS***_DISABLE)
    pAnmObj->funcAnm = NULL; // for supporting the animation disable macro (NNS_G3D_NS***_DISABLE)

    hdr = (const NNSG3dResAnmHeader*)pResAnm;

    for (i = 0; i < NNS_G3dAnmFmtNum; ++i)
    {
        if (NNS_G3dAnmObjInitFuncArray[i].category0 == hdr->category0 &&
            NNS_G3dAnmObjInitFuncArray[i].category1 == hdr->category1)
        {
            if (NNS_G3dAnmObjInitFuncArray[i].func)
            {
                (*NNS_G3dAnmObjInitFuncArray[i].func)(pAnmObj, pResAnm, pResMdl);
            }
            break;
        }
    }
    NNS_G3D_ASSERT(i != NNS_G3dAnmFmtNum);
}


/*---------------------------------------------------------------------------
    NNS_G3dAnmObjEnableID

    Attempts to play the animation corresponding to the JntID or the MatID.
    (default)
 *---------------------------------------------------------------------------*/
void
NNS_G3dAnmObjEnableID(NNSG3dAnmObj* pAnmObj, int id)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);

    if (id >= 0 && id < pAnmObj->numMapData &&
        (pAnmObj->mapData[id] & NNS_G3D_ANMOBJ_MAPDATA_EXIST))
        pAnmObj->mapData[id] &= ~NNS_G3D_ANMOBJ_MAPDATA_DISABLED;
}


/*---------------------------------------------------------------------------
    NNS_G3dAnmObjEnableID

    Tries not to play the animation corresponding to the JntID or the MatID.
 *---------------------------------------------------------------------------*/
void
NNS_G3dAnmObjDisableID(NNSG3dAnmObj* pAnmObj, int id)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);

    if (id >= 0 && id < pAnmObj->numMapData &&
        (pAnmObj->mapData[id] & NNS_G3D_ANMOBJ_MAPDATA_EXIST))
        pAnmObj->mapData[id] |= NNS_G3D_ANMOBJ_MAPDATA_DISABLED;
}



/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjInit

    Initializes NNSG3dRenderObj. pResMdl may be specified as NULL.
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjInit(NNSG3dRenderObj* pRenderObj,
                     NNSG3dResMdl* pResMdl)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    // pResMdl can be NULL

    MI_CpuClear32(pRenderObj, sizeof(NNSG3dRenderObj));

    // Sets the default handler for blending.
    pRenderObj->funcBlendMat = NNS_G3dFuncBlendMatDefault;
    pRenderObj->funcBlendJnt = NNS_G3dFuncBlendJntDefault;
    pRenderObj->funcBlendVis = NNS_G3dFuncBlendVisDefault;

    pRenderObj->resMdl = pResMdl;
}


/*---------------------------------------------------------------------------*
    addLink_

    Corresponds to when item is already present in the list, but an incorrect order may result when the list has not been sorted.
    
 *---------------------------------------------------------------------------*/
static void
addLink_(NNSG3dAnmObj** l, NNSG3dAnmObj* item)
{
#ifdef NITRO_DEBUG
    // Error if "item" is already in "l."
    {
        NNSG3dAnmObj* p = *l;
        while(p)
        {
            NNS_G3D_ASSERT(p != item);
            p = p->next;
        }
    }
#endif
    if (!(*l))
    {
        // Becomes first element if no list exists.
        *l = item;
    }
    else if (!((*l)->next))
    {
        // If the list has 1 element:
        if ((*l)->priority > item->priority)
        {
            // connect list to tail of "item"
            NNSG3dAnmObj* p = item;
            while(p->next)
            {
                p = p->next;
            }
            p->next = (*l);
            *l = item;
        }
        else
        {
            // connect to end of existing list (though just 1 element)
            (*l)->next = item;
        }
    }
    else
    {
        NNSG3dAnmObj* p = *l;
        NNSG3dAnmObj* x = (*l)->next;

        while(x)
        {
            if (x->priority >= item->priority)
            {
                // insert into existing list
                NNSG3dAnmObj* pp = item;
                while(pp->next)
                {
                    pp = pp->next;
                }
                p->next = item;
                pp->next = x;
                return;
            }
            p = x;
            x = x->next;
        }
        // connect to the end of existing list
        p->next = item;
    }
}


/*---------------------------------------------------------------------------*
    updateHintVec_

    Bit corresponding to resource ID registered in pAnmObj is set to ON.
 *---------------------------------------------------------------------------*/
static void
updateHintVec_(u32* pVec, const NNSG3dAnmObj* pAnmObj)
{
    const NNSG3dAnmObj* p = pAnmObj;
    while(p)
    {
        int i;
        for (i = 0; i < p->numMapData; ++i)
        {
            if (p->mapData[i] & NNS_G3D_ANMOBJ_MAPDATA_EXIST)
            {
                pVec[i >> 5] |= 1 << (i & 31);
            }
        }
        p = p->next;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjAddAnm

    Adds pAnmObj to pRenderObj.
    Adds pAnmObj to the appropriate list after viewing its header information.
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjAddAnmObj(NNSG3dRenderObj* pRenderObj,
                          NNSG3dAnmObj* pAnmObj)
{
    const NNSG3dResAnmHeader* hdr;
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pAnmObj->resAnm);

    if (pAnmObj && pAnmObj->resAnm)
    {
        hdr = (const NNSG3dResAnmHeader*)pAnmObj->resAnm;

        switch(hdr->category0)
        {
        case 'M': // material animation
            updateHintVec_(&pRenderObj->hintMatAnmExist[0], pAnmObj);
            addLink_(&pRenderObj->anmMat, pAnmObj);
            break;
        case 'J': // character (joint) animation
            updateHintVec_(&pRenderObj->hintJntAnmExist[0], pAnmObj);
            addLink_(&pRenderObj->anmJnt, pAnmObj);
            break;
        case 'V': // visibility animation
            updateHintVec_(&pRenderObj->hintVisAnmExist[0], pAnmObj);
            addLink_(&pRenderObj->anmVis, pAnmObj);
            break;
        default:
            NNS_G3D_ASSERT(0);
            break;
        }
    }
}


/*---------------------------------------------------------------------------*
    removeLink_

    Removes "item" from the list.
    Returns TRUE when item is in the list, FALSE when it is not.
 *---------------------------------------------------------------------------*/
static BOOL
removeLink_(NNSG3dAnmObj** l, NNSG3dAnmObj* item)
{
    NNSG3dAnmObj* p;
    NNSG3dAnmObj* pp;

    if (!*l)
    {
        // if not even one AnmObj exists:
        return FALSE;
    }
    
    if (*l == item)
    {
        // if first AnmObj has been removed:
        *l = (*l)->next;
        item->next = NULL;
        return TRUE;
    }

    // search list
    p = (*l)->next;
    pp = (*l);
    while(p)
    {
        if (p == item)
        {
            pp->next = p->next;
            p->next = NULL;

            return TRUE;
        }
        pp = p;
        p = p->next;
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjRemoveAnmObj

    Removes pAnmObj from pRenderObj.
    Hint-use bit vector is not changed.
    An update occurs when NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE is checked within the NNS_G3dDraw function.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjRemoveAnmObj(NNSG3dRenderObj* pRenderObj,
                             NNSG3dAnmObj* pAnmObj)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(pAnmObj);

    if (removeLink_(&pRenderObj->anmMat, pAnmObj) ||
        removeLink_(&pRenderObj->anmJnt, pAnmObj) ||
        removeLink_(&pRenderObj->anmVis, pAnmObj))
    {
        NNS_G3dRenderObjSetFlag(pRenderObj, NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE);
        return;
    }
    
    // pAnmObj was not connected to pRenderObj
    NNS_G3D_WARNING(0, "An AnmObj was not removed in NNS_G3dRenderObjRemoveAnmObj");
}


/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjSetCallBack

    Registers the callback process to be done while rendering.
    When the cmd command is run, the callback can be called with any of the three types of timing specified.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjSetCallBack(NNSG3dRenderObj* pRenderObj,
                            NNSG3dSbcCallBackFunc func,
                            u8*,          // NOTICE: eliminated the address specification of callbacks
                            u8 cmd,
                            NNSG3dSbcCallBackTiming timing)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);

    pRenderObj->cbFunc = func;
    pRenderObj->cbCmd = cmd;
    pRenderObj->cbTiming = (u8)timing;
}



/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjResetCallBack

    Deletes the callback process done while rendering.
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjResetCallBack(NNSG3dRenderObj* pRenderObj)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);

    pRenderObj->cbFunc = NULL;
    pRenderObj->cbCmd = 0;
    pRenderObj->cbTiming = 0;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjSetInitFunc

    Sets the callback to be called immediately before rendering.
 *---------------------------------------------------------------------------*/
void
NNS_G3dRenderObjSetInitFunc(NNSG3dRenderObj* pRenderObj,
                            NNSG3dSbcCallBackFunc func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    pRenderObj->cbInitFunc = func;
}


////////////////////////////////////////////////////////////////////////////////
//
// model code <-> texture code
//
//

//
// Texture / 4x4Texture / Pltt lifecycle
//
// 1 get size with GetRequiredSize
// 2 get texKey or plttKey (VRAM Manager code is not called)
// 3 set the key with SetTexKey or SetPlttKey
// 4 load texture/palette data into VRAM
// 5 bind texture/palette data to model resource you want to render
// 6 render using the model resource
//(7 unbind if model resource will use different texture set)
// 8 release if it will no longer be used from any model (VRAM Manager code is not called)
//
// no special order for 4, 5
//

/*---------------------------------------------------------------------------*
    NNS_G3dTexGetRequiredSize

    Returns the size of the texture (other than 4x4 compressed texture) held by the Texture block.
 *---------------------------------------------------------------------------*/
u32
NNS_G3dTexGetRequiredSize(const NNSG3dResTex* pTex)
{
    NNS_G3D_NULL_ASSERT(pTex);

    if (pTex)
    {
        return (u32)(pTex->texInfo.sizeTex << 3);
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dTex4x4GetRequiredSize

    Returns the size of the 4x4 compressed texture held by the Texture block.
 *---------------------------------------------------------------------------*/
u32
NNS_G3dTex4x4GetRequiredSize(const NNSG3dResTex* pTex)
{
    NNS_G3D_NULL_ASSERT(pTex);

    if (pTex)
    {
        return (u32)(pTex->tex4x4Info.sizeTex << 3);
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dTexSetTexKey

    Assigns texture key to the texture inside the Texture block.
 *---------------------------------------------------------------------------*/
void
NNS_G3dTexSetTexKey(NNSG3dResTex* pTex,
                    NNSG3dTexKey texKey,
                    NNSG3dTexKey tex4x4Key)
{
    if (texKey > 0)
    {
        NNS_G3D_NULL_ASSERT(pTex);
        NNS_G3D_ASSERTMSG(!(pTex->texInfo.flag & NNS_G3D_RESTEX_LOADED),
                        "NNS_G3dTexSetTexKey: Tex already loaded.");
        NNS_G3D_ASSERTMSG((pTex->texInfo.sizeTex << 3) <= NNS_GfdGetTexKeySize(texKey),
                        "NNS_G3dTexSetTexKey: texKey size not enough, "
                        "0x%05x bytes required.", pTex->texInfo.sizeTex << 3);
        pTex->texInfo.vramKey = texKey;
    }

    if (tex4x4Key > 0)
    {
        NNS_G3D_ASSERTMSG(!(pTex->tex4x4Info.flag & NNS_G3D_RESTEX4x4_LOADED),
                        "NNS_G3dTexSetTexKey(4x4): Tex already loaded.");
        NNS_G3D_ASSERTMSG(tex4x4Key & 0x80000000,
                        "NNS_G3dTexSetTexKey(4x4): texKey is not for 4x4comp textures");
        NNS_G3D_ASSERTMSG((pTex->tex4x4Info.sizeTex << 3) <= NNS_GfdGetTexKeySize(tex4x4Key),
                        "NNS_G3dTexSetTexKey(4x4): texKey size not enough, "
                        "0x%05x bytes required.", pTex->tex4x4Info.sizeTex << 3);
        pTex->tex4x4Info.vramKey = tex4x4Key;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dTexLoad

    Loads the texture inside pTex to the location indicated by the texture key.
    When exec_begin_end is specified as TRUE, VRAM bank switching occurs internally.
    When FALSE, the user has to perform VRAM bank switching both before and after.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dTexLoad(NNSG3dResTex* pTex, BOOL exec_begin_end)
{
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_WARNING(!(pTex->texInfo.flag & NNS_G3D_RESTEX_LOADED),
                    "NNS_G3dTexLoad: texture already loaded");
    NNS_G3D_WARNING(!(pTex->tex4x4Info.flag & NNS_G3D_RESTEX4x4_LOADED),
                    "NNS_G3dTex4x4Load: texture already loaded");

    if (exec_begin_end)
    {
        // Switch the VRAM bank and bring it into the main memory address space.
        GX_BeginLoadTex();
    }

    {
        //
        // Load a normal texture
        //

        u32 sz;
        const void* pData;
        u32 from;

        sz = (u32)pTex->texInfo.sizeTex << 3;
        if (sz > 0) // if texture data does not exist, sz == 0
        {
            NNS_G3D_ASSERT(pTex->texInfo.vramKey != 0);
            NNS_G3D_ASSERTMSG(sz <= NNS_GfdGetTexKeySize(pTex->texInfo.vramKey),
                              "NNS_G3dTexLoad: texKey size not enough, "
                              "0x%05x bytes required.", sz);
            
            pData = NNS_G3dGetTexData(pTex);
            from = NNS_GfdGetTexKeyAddr(pTex->texInfo.vramKey);

            GX_LoadTex(pData, from, sz);
    
            pTex->texInfo.flag |= NNS_G3D_RESTEX_LOADED;
        }
    }

    {
        //
        // Load a 4x4 compressed texture.
        //

        u32 sz;
        const void* pData;
        const void* pDataPlttIdx;
        u32 from;

        sz = (u32)pTex->tex4x4Info.sizeTex << 3;

        if (sz > 0) // if texture data does not exist, sz == 0
        {
            NNS_G3D_ASSERT(pTex->tex4x4Info.sizeTex != 0);
            NNS_G3D_ASSERTMSG(NNS_GfdGetTexKey4x4Flag(pTex->tex4x4Info.vramKey),
                              "NNS_G3dTexLoad(4x4): texKey is not for 4x4comp textures");
            NNS_G3D_ASSERTMSG(sz <= NNS_GfdGetTexKeySize(pTex->tex4x4Info.vramKey),
                              "NNS_G3dTexLoad(4x4): texKey size not enough, "
                              "0x%05x bytes required.", sz);

            pData = NNS_G3dGetTex4x4Data(pTex);
            pDataPlttIdx = NNS_G3dGetTex4x4PlttIdxData(pTex);
            from = NNS_GfdGetTexKeyAddr(pTex->tex4x4Info.vramKey);

            GX_LoadTex(pData, from, sz);
            GX_LoadTex(pDataPlttIdx, GX_COMP4x4_PLTT_IDX(from), sz >> 1);
    
            pTex->tex4x4Info.flag |= NNS_G3D_RESTEX4x4_LOADED;
        }
    }

    if (exec_begin_end)
    {
        // restore items whose bank was switched
        GX_EndLoadTex();
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dTexReleaseTexKey

    Releases the texture-key assignment inside the Texture block.
    The key is passed as the return value to the user.
    The VRAM's texture region itself does not get released.
    The user must release this using the key.
 *---------------------------------------------------------------------------*/
void
NNS_G3dTexReleaseTexKey(NNSG3dResTex* pTex,
                        NNSG3dTexKey* texKey,
                        NNSG3dTexKey* tex4x4Key)
{
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_NULL_ASSERT(texKey);
    NNS_G3D_NULL_ASSERT(tex4x4Key);

    if (texKey)
    {
        pTex->texInfo.flag &= ~NNS_G3D_RESTEX_LOADED;
        *texKey = pTex->texInfo.vramKey;
        pTex->texInfo.vramKey = 0;
    }

    if (tex4x4Key)
    {
        pTex->tex4x4Info.flag &= ~NNS_G3D_RESTEX4x4_LOADED;
        *tex4x4Key = pTex->tex4x4Info.vramKey;
        pTex->tex4x4Info.vramKey = 0;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dPlttGetRequiredSize

    Returns the size of the palette held by the Texture block.
 *---------------------------------------------------------------------------*/
u32
NNS_G3dPlttGetRequiredSize(const NNSG3dResTex* pTex)
{
    NNS_G3D_NULL_ASSERT(pTex);

    if (pTex)
    {
        return (u32)(pTex->plttInfo.sizePltt << 3);
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dPlttSetPlttKey

    Assigns palette key to palette inside the Texture block.
 *---------------------------------------------------------------------------*/
void
NNS_G3dPlttSetPlttKey(NNSG3dResTex* pTex, NNSG3dPlttKey plttKey)
{
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_ASSERTMSG(!(pTex->plttInfo.flag & NNS_G3D_RESPLTT_LOADED),
                      "NNS_G3dPlttSetPlttKey: Palette already loaded.");
    NNS_G3D_ASSERTMSG(!(pTex->plttInfo.sizePltt > (plttKey >> 16)),
                      "NNS_G3dPlttSetPlttKey: plttKey size not enough, "
                      "0x%05x bytes required.", pTex->plttInfo.sizePltt << 3);

    pTex->plttInfo.vramKey = plttKey;
}


/*---------------------------------------------------------------------------*
    NNS_G3dPlttLoad

    Loads the palette inside pTex to the location indicated by the palette key.
    When exec_begin_end is specified as TRUE, VRAM bank switching occurs internally.
    When FALSE, the user has to perform VRAM bank switching both before and after.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dPlttLoad(NNSG3dResTex* pTex, BOOL exec_begin_end)
{
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_WARNING(!(pTex->plttInfo.flag & NNS_G3D_RESPLTT_LOADED),
                    "NNS_G3dPlttLoad: texture already loaded");

    if (exec_begin_end)
    {
        // Switch the VRAM bank and bring it into the main memory address space.
        GX_BeginLoadTexPltt();
    }

    {
        u32 sz;
        const void* pData;
        u32 from;

        sz = (u32)pTex->plttInfo.sizePltt << 3;
        pData = NNS_G3dGetPlttData(pTex);
        from = NNS_GfdGetTexKeyAddr(pTex->plttInfo.vramKey);

        GX_LoadTexPltt(pData, from, sz);
    
        pTex->plttInfo.flag |= NNS_G3D_RESPLTT_LOADED;
    }

    if (exec_begin_end)
    {
        // restore items whose bank was switched
        GX_EndLoadTexPltt();
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dPlttReleasePlttKey

    Releases the palette-key assignment inside the Texture block.
    The key is passed as the return value to the user.
    The VRAM's texture region itself does not get released.
    The user must release this using the key.
 *---------------------------------------------------------------------------*/
NNSG3dPlttKey
NNS_G3dPlttReleasePlttKey(NNSG3dResTex* pTex)
{
    u32 rval;
    NNS_G3D_NULL_ASSERT(pTex);

    pTex->plttInfo.flag &= ~NNS_G3D_RESPLTT_LOADED;
    
    rval = pTex->plttInfo.vramKey;
    pTex->plttInfo.vramKey = 0;
    return rval;
}


static void
bindMdlTex_Internal_(NNSG3dResMat* pMat,
                     NNSG3dResDictTexToMatIdxData* pBindData,
                     const NNSG3dResTex* pTex,
                     const NNSG3dResDictTexData* pTexData)
{
    u8* base = (u8*)pMat + pBindData->offset;
    u32 vramOffset;
    u32 j;

    NNS_G3D_ASSERTMSG((pTex->texInfo.vramKey != 0 || pTex->texInfo.sizeTex == 0),
                      "No texture key assigned");
    NNS_G3D_ASSERTMSG((pTex->tex4x4Info.vramKey != 0 || pTex->tex4x4Info.sizeTex == 0),
                      "No texture(4x4) key assigned");

    if ((pTexData->texImageParam & REG_G3_TEXIMAGE_PARAM_TEXFMT_MASK) !=
                    (GX_TEXFMT_COMP4x4 << REG_G3_TEXIMAGE_PARAM_TEXFMT_SHIFT))
    {
        // some texture other than 4x4
        vramOffset = NNS_GfdGetTexKeyAddr(pTex->texInfo.vramKey)
                            >> NNS_GFD_TEXKEY_ADDR_SHIFT;
    }
    else
    {
        // a 4x4 texture
        vramOffset = NNS_GfdGetTexKeyAddr(pTex->tex4x4Info.vramKey)
                            >> NNS_GFD_TEXKEY_ADDR_SHIFT;
    }

    for (j = 0; j < pBindData->numIdx; ++j)
    {
        s32 w, h;

        // Perform setup of texture information in each matData.
        NNSG3dResMatData* matData = NNS_G3dGetMatDataByIdx(pMat, *(base + j));

        matData->texImageParam |= (pTexData->texImageParam + vramOffset);
        w = (s32)(((pTexData->extraParam) & NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_MASK) >> NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_SHIFT);
        h = (s32)(((pTexData->extraParam) & NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_MASK) >> NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_SHIFT);

        matData->magW = (w != matData->origWidth) ?
                        FX_Div(w << FX32_SHIFT, matData->origWidth << FX32_SHIFT) :
                        FX32_ONE;
        matData->magH = (h != matData->origHeight) ?
                        FX_Div(h << FX32_SHIFT, matData->origHeight << FX32_SHIFT) :
                        FX32_ONE;
    }

    // Set state to bind state.
    pBindData->flag |= 1;
}

static void
releaseMdlTex_Internal_(NNSG3dResMat* pMat,
                        NNSG3dResDictTexToMatIdxData* pData)
{
    // if state is bound state, release
    u8* base = (u8*)pMat + pData->offset;
    u32 j;

    for (j = 0; j < pData->numIdx; ++j)
    {
        // Perform setup of texture information in each matData.
        NNSG3dResMatData* matData = NNS_G3dGetMatDataByIdx(pMat, *(base + j));

        // leave flip & repeat & texgen and reset them
        matData->texImageParam &= REG_G3_TEXIMAGE_PARAM_TGEN_MASK |
                                  REG_G3_TEXIMAGE_PARAM_FT_MASK | REG_G3_TEXIMAGE_PARAM_FS_MASK |
                                  REG_G3_TEXIMAGE_PARAM_RT_MASK | REG_G3_TEXIMAGE_PARAM_RS_MASK;
        matData->magH = matData->magW = FX32_ONE;
    }

    // set to an unbound state
    pData->flag &= ~1;
}


/*---------------------------------------------------------------------------*
    NNS_G3dBindMdlTex

    Within a model, binds a material's texture entry associated with a texture name to texture data having the same texture name within the texture block.
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dBindMdlTex(NNSG3dResMdl* pMdl, const NNSG3dResTex* pTex)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictTex;
    u32 i;
    BOOL result = TRUE;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);

    mat     = NNS_G3dGetMat(pMdl);
    dictTex = (NNSG3dResDict*)((u8*)mat + mat->ofsDictTexToMatList);
    
    // Run a loop for both the model resource's texture name and its entry within the material index string dictionary.
    // 
    for (i = 0; i < dictTex->numEntry; ++i)
    {
        const NNSG3dResName* name = NNS_G3dGetResNameByIdx(dictTex, i);

        // search inside Texture block for entries with same name
        const NNSG3dResDictTexData* texData = NNS_G3dGetTexDataByName(pTex, name);

        if (texData)
        {
            // If there, get the bind data relating to the i-numbered texture.
            NNSG3dResDictTexToMatIdxData* data =
                (NNSG3dResDictTexToMatIdxData*) NNS_G3dGetResDataByIdx(dictTex, i);
        
            if (!(data->flag & 1))
            {
                // bind if not in the bound state
                bindMdlTex_Internal_(mat, data, pTex, texData);
            }
        }
        else
        {
            result = FALSE;
        }
    }
    return result;
}


/*---------------------------------------------------------------------------*
    NNS_G3dBindMdlTexEx

    Within a model, binds a material's texture entry associated with a pResName texture name to a texture having the same pResName name within the texture block.
    
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dBindMdlTexEx(NNSG3dResMdl* pMdl,
                    const NNSG3dResTex* pTex,
                    const NNSG3dResName* pResName)
{
    NNSG3dResMat* mat;
    NNSG3dResDict* dictTex;
    const NNSG3dResDictTexData* texData;
    NNSG3dResDictTexToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_NULL_ASSERT(pResName);

    mat     = NNS_G3dGetMat(pMdl);
    dictTex = (NNSG3dResDict*)((u8*)mat + mat->ofsDictTexToMatList);

    // search the dictionary inside the Texture block
    texData = NNS_G3dGetTexDataByName(pTex, pResName);

    if (texData)
    {
        // search inside the model's texture -> material index list dictionary
        data = (NNSG3dResDictTexToMatIdxData*)
                    NNS_G3dGetResDataByName(dictTex, pResName);

        if (data && !(data->flag & 1))
        {
            // 
            // bind when there is data corresponding to pResName for both the model and the texture and when not in the bound state
            bindMdlTex_Internal_(mat, data, pTex, texData);
            return TRUE;
        }
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dForceBindMdlTex

    Within a model, forcibly binds the texture entry for a material stored in the texToMatListIdx numbered entry in the texture name to material list dictionary and the texIdx numbered texture within the texture block.
    
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dForceBindMdlTex(NNSG3dResMdl* pMdl,
                       const NNSG3dResTex* pTex,
                       u32 texToMatListIdx,
                       u32 texIdx)
{
    NNSG3dResMat* mat;
    NNSG3dResDict* dictTex;
    const NNSG3dResDictTexData* texData;
    NNSG3dResDictTexToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);

    mat     = NNS_G3dGetMat(pMdl);
    dictTex = (NNSG3dResDict*)((u8*)mat + mat->ofsDictTexToMatList);

    // uses index lookup to get the texture data in the texture block
    texData = NNS_G3dGetTexDataByIdx(pTex, texIdx);

    // search inside the model's texture -> material index list dictionary
    data = (NNSG3dResDictTexToMatIdxData*)
                NNS_G3dGetResDataByIdx(dictTex, texToMatListIdx);

    if (data)
    {
        // Forcibly binds the textures if the texture having the name pResName exists for the material.
        bindMdlTex_Internal_(mat, data, pTex, texData);
        return TRUE;
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dReleaseMdlTex

    Within a model, releases the association for the material's texture entry bound to the texture.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dReleaseMdlTex(NNSG3dResMdl* pMdl)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictTex;
    u32 i;

    NNS_G3D_NULL_ASSERT(pMdl);

    mat      = NNS_G3dGetMat(pMdl);
    dictTex  = (NNSG3dResDict*)((u8*)mat + mat->ofsDictTexToMatList);
    
    // Run a loop for both the model resource's texture name and its entry within the material index string dictionary.
    // 
    for (i = 0; i < dictTex->numEntry; ++i)
    {
        NNSG3dResDictTexToMatIdxData* data =
            (NNSG3dResDictTexToMatIdxData*) NNS_G3dGetResDataByIdx(dictTex, i);

        if (data->flag & 1)
        {
            // release if in the bound state
            releaseMdlTex_Internal_(mat, data);
        }
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dReleaseMdlTexEx

    Within a model, releases the association for the material's texture entry bound to the texture named with the pResName.
    
 *---------------------------------------------------------------------------*/
BOOL NNS_G3dReleaseMdlTexEx(NNSG3dResMdl* pMdl, const NNSG3dResName* pResName)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictTex;
    NNSG3dResDictTexToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pResName);

    mat      = NNS_G3dGetMat(pMdl);
    dictTex  = (NNSG3dResDict*)((u8*)mat + mat->ofsDictTexToMatList);
    data     = (NNSG3dResDictTexToMatIdxData*)NNS_G3dGetResDataByName(dictTex, pResName);

    if (data && (data->flag & 1))
    {
        // release if in the bound state
        releaseMdlTex_Internal_(mat, data);
        return TRUE;
    }
    return FALSE;
}


static void
bindMdlPltt_Internal_(NNSG3dResMat* pMat,
                      NNSG3dResDictPlttToMatIdxData* pBindData,
                      const NNSG3dResTex* pTex,
                      const NNSG3dResDictPlttData* pPlttData)
{
    // bind if not in the bound state
    u8* base = (u8*)pMat + pBindData->offset;
    u16 plttBase = pPlttData->offset;
    u16 vramOffset = (u16)(NNS_GfdGetTexKeyAddr(pTex->plttInfo.vramKey) >> NNS_GFD_TEXKEY_ADDR_SHIFT);
    u32 j;

    NNS_G3D_ASSERTMSG((pTex->plttInfo.vramKey != 0 || pTex->plttInfo.sizePltt == 0),
                       "No palette key assigned");
    
    // if 4 colors, this bit is set
    if (!(pPlttData->flag & 1))
    {
        // four-bit shift if other than 4colors
        // if 4 colors, shift is 3-bit shift, so it's left as is
        plttBase >>= 1;
        vramOffset >>= 1;
    }

    for (j = 0; j < pBindData->numIdx; ++j)
    {
        // Perform setup of palette information for each matData.
        NNSG3dResMatData* matData = NNS_G3dGetMatDataByIdx(pMat, *(base + j));
        matData->texPlttBase = (u16)(plttBase + vramOffset);
    }

    // set to bind state
    pBindData->flag |= 1;
}


/*---------------------------------------------------------------------------*
    NNS_G3dBindMdlPltt

    Within a model, binds a material's palette entry associated with a palette name to palette data having the same palette name within the texture block.
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dBindMdlPltt(NNSG3dResMdl* pMdl, const NNSG3dResTex* pTex)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictPltt;
    u32 i;
    BOOL result = TRUE;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);

    mat      = NNS_G3dGetMat(pMdl);
    dictPltt = (NNSG3dResDict*)((u8*)mat + mat->ofsDictPlttToMatList);

    for (i = 0; i < dictPltt->numEntry; ++i)
    {
        // Get the palette name from the palette name inside the model -> material index array dictionary
        const NNSG3dResName* name = NNS_G3dGetResNameByIdx(dictPltt, i);
        
        // Get the data field corresponding to the palette name from the Texture block.
        const NNSG3dResDictPlttData* plttData = NNS_G3dGetPlttDataByName(pTex, name);

        if (plttData)
        {
            // palette name exists in Texture block
            NNSG3dResDictPlttToMatIdxData* data;
            data = (NNSG3dResDictPlttToMatIdxData*) NNS_G3dGetResDataByIdx(dictPltt, i);

            if (!(data->flag & 1))
            {
                bindMdlPltt_Internal_(mat, data, pTex, plttData);
            }
        }
        else
        {
            result = FALSE;
        }
    }
    return result;
}


/*---------------------------------------------------------------------------*
    NNS_G3dBindMdlPlttEx

    Within a model, binds a material's palette entry associated with a pResName palette name to a palette having the same pResName name within the texture block.
    
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dBindMdlPlttEx(NNSG3dResMdl* pMdl,
                     const NNSG3dResTex* pTex,
                     const NNSG3dResName* pResName)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictPltt;
    const NNSG3dResDictPlttData* plttData;
    NNSG3dResDictPlttToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);
    NNS_G3D_NULL_ASSERT(pResName);

    mat      = NNS_G3dGetMat(pMdl);
    dictPltt = (NNSG3dResDict*)((u8*)mat + mat->ofsDictPlttToMatList);

    // Get the data field corresponding to the palette name from the Texture block.
    plttData = NNS_G3dGetPlttDataByName(pTex, pResName);

    if (plttData)
    {
        // palette name exists in Texture block
        data = (NNSG3dResDictPlttToMatIdxData*)
                    NNS_G3dGetResDataByName(dictPltt, pResName);
        
        if (data && !(data->flag & 1))
        {
            bindMdlPltt_Internal_(mat, data, pTex, plttData);
            return TRUE;
        }
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dForceBindMdlPltt

    Within a model, forcibly binds the palette entry for a material stored in the plttToMatListIdx numbered entry in the palette name to material list dictionary and the plttIdx numbered palette within the texture block.
    
    
 *---------------------------------------------------------------------------*/
BOOL NNS_G3dForceBindMdlPltt(NNSG3dResMdl* pMdl,
                             const NNSG3dResTex* pTex,
                             u32 plttToMatListIdx,
                             u32 plttIdx)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictPltt;
    const NNSG3dResDictPlttData* plttData;
    NNSG3dResDictPlttToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pTex);

    mat      = NNS_G3dGetMat(pMdl);
    dictPltt = (NNSG3dResDict*)((u8*)mat + mat->ofsDictPlttToMatList);
    plttData = NNS_G3dGetPlttDataByIdx(pTex, plttIdx);

    data = (NNSG3dResDictPlttToMatIdxData*)
                NNS_G3dGetResDataByIdx(dictPltt, plttToMatListIdx);
        
    if (data && !(data->flag & 1))
    {
        bindMdlPltt_Internal_(mat, data, pTex, plttData);
        return TRUE;
    }
    return FALSE;
}



/*---------------------------------------------------------------------------*
    NNS_G3dReleaseMdlPltt

    Within a model, releases the association for the material's palette entry bound to the palette.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dReleaseMdlPltt(NNSG3dResMdl* pMdl)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictPltt;
    u32 i;

    NNS_G3D_NULL_ASSERT(pMdl);

    mat      = NNS_G3dGetMat(pMdl);
    dictPltt = (NNSG3dResDict*)((u8*)mat + mat->ofsDictPlttToMatList);
    for (i = 0; i < dictPltt->numEntry; ++i)
    {
        NNSG3dResDictPlttToMatIdxData* data =
            (NNSG3dResDictPlttToMatIdxData*) NNS_G3dGetResDataByIdx(dictPltt, i);
         
        if (data->flag & 1)
        {
            // if state is bound state, release
#if 0
            u32 j;
            // do not execute, since OK with just flag operations
            u8* base = (u8*)mat + data->offset;

            for (j = 0; j < data->numIdx; ++j)
            {
                // reset each matData
                NNSG3dResMatData* matData = NNS_G3dGetMatDataByIdx(mat, *(base + j));
                matData->texPlttBase = 0;
            }
#endif
            // unbind
            data->flag &= ~1;
        }
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dReleaseMdlPlttEx

    Within a model, releases the association for the material's palette entry bound to the palette named with the pResName.
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dReleaseMdlPlttEx(NNSG3dResMdl* pMdl, const NNSG3dResName* pResName)
{
    NNSG3dResMat*  mat;
    NNSG3dResDict* dictPltt;
    NNSG3dResDictPlttToMatIdxData* data;

    NNS_G3D_NULL_ASSERT(pMdl);
    NNS_G3D_NULL_ASSERT(pResName);

    mat      = NNS_G3dGetMat(pMdl);
    dictPltt = (NNSG3dResDict*)((u8*)mat + mat->ofsDictPlttToMatList);
    data = (NNSG3dResDictPlttToMatIdxData*)
                NNS_G3dGetResDataByName(dictPltt, pResName);

    if (data && (data->flag & 1))
    {
        // if state is bound state, release
        data->flag &= ~1;
        return TRUE;
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dBindMdlSet

    Associates each model with texture/palette.
    Texture must have TexKey, PlttKey set.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dBindMdlSet(NNSG3dResMdlSet* pMdlSet, const NNSG3dResTex* pTex)
{
    u32 i;
    BOOL result = TRUE;

    NNS_G3D_NULL_ASSERT(pMdlSet);
    NNS_G3D_NULL_ASSERT(pTex);

    for (i = 0; i < pMdlSet->dict.numEntry; ++i)
    {
        NNSG3dResMdl* mdl = NNS_G3dGetMdlByIdx(pMdlSet, i);
        NNS_G3D_NULL_ASSERT(mdl);

        result &= NNS_G3dBindMdlTex(mdl, pTex);
        result &= NNS_G3dBindMdlPltt(mdl, pTex);
    }
    return result;
}


/*---------------------------------------------------------------------------*
    NNS_G3dReleaseMdlSet

    Releases each model from the texture/palette association.
 *---------------------------------------------------------------------------*/
void
NNS_G3dReleaseMdlSet(NNSG3dResMdlSet* pMdlSet)
{
    u32 i;
    NNS_G3D_NULL_ASSERT(pMdlSet);

    for (i = 0; i < pMdlSet->dict.numEntry; ++i)
    {
        NNSG3dResMdl* mdl = NNS_G3dGetMdlByIdx(pMdlSet, i);
        NNS_G3D_NULL_ASSERT(mdl);

        NNS_G3dReleaseMdlTex(mdl);
        NNS_G3dReleaseMdlPltt(mdl);
    }
}
