/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_BmpBg_MainRam
  File:     image.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "image.h"

/* Image on MRAM */
const unsigned short IMAGE_MRAM256x192[4][256 * 192] = {
    {
#include "picture_256_192_a.autogen.c"
     },
    {
#include "picture_256_192_b.autogen.c"
     },
    {
#include "picture_256_192_c.autogen.c"
     },
    {
#include "picture_256_192_d.autogen.c"
     }
};
