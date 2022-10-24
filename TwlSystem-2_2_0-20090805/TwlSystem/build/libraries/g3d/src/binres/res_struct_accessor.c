/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - binres
  File:     res_struct_accessor.c

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


////////////////////////////////////////////////////////////////////////////////
//
// Accessors for dictionaries
//
//

/*---------------------------------------------------------------------------*
    NNS_G3dGetResDataByName

    Gets a pointer to the entry corresponding to the resource name 'name' in the dictionary 'dict'.
    Returns NULL if 'name' is not in 'dict'.
 *---------------------------------------------------------------------------*/
void*
NNS_G3dGetResDataByName(const NNSG3dResDict* dict, const NNSG3dResName* name)
{
    NNS_G3D_NULL_ASSERT(dict);
    // NULL is returned when name is NULL.

    if (!name)
        return NULL;

    if (dict->numEntry < 16)
    {
        u32 idx;
        const NNSG3dResName* n;
        u32 v0 = name->val[0];
        u32 v1 = name->val[1];
        u32 v2 = name->val[2];
        u32 v3 = name->val[3];        
        
        for (idx = 0; idx < dict->numEntry; ++idx)
        {
            n = NNS_G3dGetResNameByIdx(dict, idx);
            if (n->val[0] == v0 &&
                n->val[1] == v1 &&
                n->val[2] == v2 &&
                n->val[3] == v3)
            {
                return NNS_G3dGetResDataByIdx(dict, idx);
            }
        }
    }
    else
    {
        const NNSG3dResName* n;
        const NNSG3dResDictTreeNode* treeBase;
        const NNSG3dResDictTreeNode *p, *x;

        treeBase = &dict->node[0];
        p = treeBase;

        if (p->idxLeft != 0)
        {
            x = treeBase + p->idxLeft;
            while(p->refBit > x->refBit)
            {
                p = x;

                x = treeBase + *(&x->idxLeft + 
                                ((name->val[x->refBit >> 5] >> (x->refBit & 31)) & 1));
            }

            n = NNS_G3dGetResNameByIdx(dict, x->idxEntry);

            if (n->val[0] == name->val[0] &&
                n->val[1] == name->val[1] &&
                n->val[2] == name->val[2] &&
                n->val[3] == name->val[3])
            {
                return NNS_G3dGetResDataByIdx(dict, x->idxEntry);
            }
        }
    }
    return NULL;
}


/*---------------------------------------------------------------------------*
    NNS_G3dGetResDictIdxByName

    Returns a negative value when the index name corresponding to the resource name is not found within dict, the dictionary.
    
 *---------------------------------------------------------------------------*/
int
NNS_G3dGetResDictIdxByName(const NNSG3dResDict* dict, const NNSG3dResName* name)
{
    NNS_G3D_NULL_ASSERT(dict);
    // Returns -1 when name is NULL.

    if (!name)
        return -1;

    if (dict->numEntry < 16)
    {
        u32 idx;
        const NNSG3dResName* n;
        u32 v0 = name->val[0];
        u32 v1 = name->val[1];
        u32 v2 = name->val[2];
        u32 v3 = name->val[3];
        
        for (idx = 0; idx < dict->numEntry; ++idx)
        {
            n = NNS_G3dGetResNameByIdx(dict, idx);
            if (n->val[0] == v0 &&
                n->val[1] == v1 &&
                n->val[2] == v2 &&
                n->val[3] == v3)
            {
                return (int)idx;
            }
        }
    }
    else
    {
        const NNSG3dResName* n;
        const NNSG3dResDictTreeNode* treeBase;
        const NNSG3dResDictTreeNode *p, *x;

        treeBase = &dict->node[0];

        p = treeBase;
        
        if (p->idxLeft != 0)
        {
            x = treeBase + p->idxLeft;
            while(p->refBit > x->refBit)
            {
                p = x;

                x = treeBase + *(&x->idxLeft + 
                                ((name->val[x->refBit >> 5] >> (x->refBit & 31)) & 1));
            }

            n = NNS_G3dGetResNameByIdx(dict, x->idxEntry);

            if (n->val[0] == name->val[0] &&
                n->val[1] == name->val[1] &&
                n->val[2] == name->val[2] &&
                n->val[3] == name->val[3])
            {
                return x->idxEntry;
            }
        }
    }
    return -1;
}


////////////////////////////////////////////////////////////////////////////////
//
// Accessor for FileHeader
//
//

/*---------------------------------------------------------------------------*
    NNS_G3dGetDataBlockHeaderByIdx

    Receives the pointer to the file header and returns pointer to the idx-numbered block header.
    
 *---------------------------------------------------------------------------*/
NNSG3dResDataBlockHeader*
NNS_G3dGetDataBlockHeaderByIdx(const NNSG3dResFileHeader* header, u32 idx)
{
    u32* blks;
    NNS_G3D_NULL_ASSERT(header);
    // NULL is returned when idx is out of range.

    if (idx < header->dataBlocks)
    {
        blks = (u32*)((u8*)header + header->headerSize);
        return (NNSG3dResDataBlockHeader*)((u8*)header + blks[idx]);
    }
    else
    {
        return NULL;
    }
}


NNSG3dResMdlSet*
NNS_G3dGetMdlSet(const NNSG3dResFileHeader* header)
{
    u32* blks;
    NNS_G3D_NULL_ASSERT(header);
    NNS_G3D_ASSERT(header->sigVal == NNS_G3D_SIGNATURE_NSBMD);
    NNS_G3D_ASSERTMSG(header->version == (2 | (0 << 8)), ".nsbmd version is different. convert again.");

    blks = (u32*)((u8*)header + header->headerSize);
    return (NNSG3dResMdlSet*)((u8*)header + blks[0]);
}


NNSG3dResTex*
NNS_G3dGetTex(const NNSG3dResFileHeader* header)
{
    u32* blks;
    NNS_G3D_NULL_ASSERT(header);
    NNS_G3D_ASSERT(header->sigVal == NNS_G3D_SIGNATURE_NSBMD ||
                   header->sigVal == NNS_G3D_SIGNATURE_NSBTX);

    blks = (u32*)((u8*)header + header->headerSize);

    if (header->dataBlocks == 1)
    {
        if (header->sigVal == NNS_G3D_SIGNATURE_NSBTX)
        {
            return (NNSG3dResTex*)((u8*)header + blks[0]);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return (NNSG3dResTex*)((u8*)header + blks[1]);
    }
}
