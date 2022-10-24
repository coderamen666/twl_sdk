/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - gfd - VramManager
  File:     gfd_VramMan.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFD_VRAMMAN_H_
#define NNS_GFD_VRAMMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

//
// Switch to enable the debug output.
// Debug output is enabled when defining.
//
//#define NNS_GFD_DEBUG


#ifdef SDK_DEBUG

    #define NNS_GFD_PRINTF( ... )   OS_Printf( __VA_ARGS__ )
    #define NNS_GFD_WARNING( ... )  OS_Warning( __VA_ARGS__ )

#else // SDK_DEBUG

    #define NNS_GFD_PRINTF( ... )   ((void) 0)
    #define NNS_GFD_WARNING( ... )  ((void) 0)

#endif // SDK_DEBUG
//------------------------------------------------------------------------------
#ifdef NNS_GFD_DEBUG
    #define NNS_GFD_DBGMSG( ... )   OS_Printf( __VA_ARGS__ )
#else // NNS_GFD_DEBUG
    #define NNS_GFD_DBGMSG( ... )   ((void) 0)
#endif // NNS_GFD_DEBUG


//
//	Constant that indicates the direction of memory allocation from VRAM
// ----------------------------------------------------------------------------
enum
{
	NNS_GFD_ALLOC_FROM_HIGH		= 0,		// Allocate memory from the top of VRAM
	NNS_GFD_ALLOC_FROM_LOW		= 1			// Allocate memory from the bottom of VRAM
};


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_GFD_VRAMMAN_H_
