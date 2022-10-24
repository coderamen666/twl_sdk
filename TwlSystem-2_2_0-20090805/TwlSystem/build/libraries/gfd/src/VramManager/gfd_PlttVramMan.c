/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd - src - VramManager
  File:     gfd_PlttVramMan.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/gfd/VramManager/gfd_VramMan.h>
#include <nnsys/gfd/VramManager/gfd_PlttVramMan_Types.h>

static NNSGfdPlttKey	AllocPlttVram_( u32 szByte, BOOL b4Pltt, u32 bAllocFromLo );
static int				FreePlttVram_( NNSGfdPlttKey key );


NNSGfdFuncAllocPlttVram NNS_GfdDefaultFuncAllocPlttVram = AllocPlttVram_;
NNSGfdFuncFreePlttVram  NNS_GfdDefaultFuncFreePlttVram  = FreePlttVram_;

/*---------------------------------------------------------------------------*
  Name:         AllocPlttVram_

  Description:  Memory allocation dummy function.
  				This function is called when a default memory allocation function is not registered.
                 
  Arguments:   szByte:  Size
               bPltt4           :  4 color texture?
               bAllocFromHead   :  Allocate from the top of the region (lower position)?
                            
  Returns:     texture palette key
 *---------------------------------------------------------------------------*/
static NNSGfdPlttKey
AllocPlttVram_( u32 /* szByte */, BOOL /* b4Pltt */, u32 /* bAllocFromLo */ )
{
    NNS_GFD_WARNING("no default AllocPlttVram function.");

    // Error: Returns a PlttKey that expresses an error.
    return NNS_GFD_ALLOC_ERROR_PLTTKEY;
}

/*---------------------------------------------------------------------------*
  Name:         FreePlttTexVram_

  Description:  Memory deallocation dummy function.
  				This function is called when a default memory deallocation function is not registered.
                 
  Arguments:   plttKey          :   texture palette key
                            
  Returns:     success or failure value (if successful, 0)
 *---------------------------------------------------------------------------*/
static int
FreePlttVram_( NNSGfdPlttKey /* plttKey */ )
{
    NNS_GFD_WARNING("no default FreePlttVram function.");

    return -1;
}
