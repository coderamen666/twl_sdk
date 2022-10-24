/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd - src - VramManager
  File:     gfd_TexVramMan.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/gfd/VramManager/gfd_VramMan.h>
#include <nnsys/gfd/VramManager/gfd_TexVramMan_Types.h>

static NNSGfdTexKey	AllocTexVram_( u32 szByte, BOOL is4x4comp, u32 opt );
static int			FreeTexVram_( NNSGfdTexKey memKey );


NNSGfdFuncAllocTexVram  NNS_GfdDefaultFuncAllocTexVram = AllocTexVram_;
NNSGfdFuncFreeTexVram   NNS_GfdDefaultFuncFreeTexVram  = FreeTexVram_;

/*---------------------------------------------------------------------------*
  Name:         AllocTexVram_

  Description:  Memory allocation dummy function.
  				This function is called when a default memory allocation function is not registered.
                 
  Arguments:   szByte:  Size
               is4x4comp:  4x4 compressed texture?
               opt:  implementation-dependent parameter
                            
  Returns:     texture key
 *---------------------------------------------------------------------------*/
static NNSGfdTexKey
AllocTexVram_( u32 /* szByte */, BOOL /* is4x4comp */, u32 /* opt */ )
{
    NNS_GFD_WARNING("no default AllocTexVram function.");

    // Error: Return a TexKey that expresses an error.
    return NNS_GFD_ALLOC_ERROR_TEXKEY;
}

/*---------------------------------------------------------------------------*
  Name:         FreeFrmTexVram_

  Description:  Memory deallocation dummy function.
  				This function is called when a default memory deallocation function is not registered.
                 
  Arguments:   memKey          :   texture key
                            
  Returns:     success or failure value (if successful, 0)
 *---------------------------------------------------------------------------*/
static int
FreeTexVram_( NNSGfdTexKey /* memKey */ )
{
    NNS_GFD_WARNING("no default FreeTexVram function.");

    return -1;
}
