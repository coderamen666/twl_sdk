/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     main.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11030 $
  $Author: tominaga_masafumi $
*---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"

/*
 * Sample application that implements clone booting.
 *
 * Because the MB library samples use multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
 * 
 * The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/ROM directory provide the same functionality as that found in a retail unit that is operating as a multiboot child. Load these binaries on some other units in the same manner as loading a normal sample program, and run them together with this sample.
 * 
 * 
 * 
 * 
 *
 */

/*
 * This demo uses the WH library for wireless communications, but does not perform wireless shutdown processing; this lack is particularly noticeable on the child side.
 * 
 * For details on WH library wireless shutdown processing, see the comments at the top of the WH library source code or the wm/dataShare-Model demo.
 * 
 * 
 */

/******************************************************************************/



//============================================================================
//   Function definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  Main routine

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    /*
     * This sample uses the multiboot-Model sample demo unchanged and simply divides processing by determining whether this is a DS Download Play child program.
     * 
     * 
     *
     * The following are primary differences between the parent and child environments when clone booting.
     *  1. A child does not have card access
     *  2. You must include 8 KB or less of code specific to the parent
     *  3. Parents and children use different wireless communications procedures
     * 
     *
     * If you heed these points and handle them with wrapper processes that match your application design, you can create an efficient program that keeps most content common between parents and children, supports both single-player and versus play, and conserves CARD-ROM memory usage.
     * Conversely, if there are absolutely no commonalities between the parent and children in DS Download Play, it will probably not be possible to achieve the aforementioned benefits.
     * 
     * 
     * 
     */
    if (!MB_IsMultiBootChild())
    {
        ParentMain();
    }
    else
    {
        ChildMain();
    }

    /* Control should not arrive here */
}
