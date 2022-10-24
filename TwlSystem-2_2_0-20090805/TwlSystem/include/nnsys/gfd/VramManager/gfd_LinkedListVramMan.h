/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - gfd - VramManager
  File:     gfd_LinkedListVramMan.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFD_LINKEDLISTVRAMMAN_H_ 
#define NNS_GFD_LINKEDLISTVRAMMAN_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>



//------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Debug output function for the linked list manager
typedef void (*NNSGfdLnkDumpCallBack)( 
    u32                             addr, 
    u32                             szByte, 
    void*                           pUserData );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_GFD_LINKEDLISTVRAMMAN_H_ 
