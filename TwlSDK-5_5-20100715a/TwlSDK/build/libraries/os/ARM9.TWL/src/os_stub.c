/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NA - libraries
  File:     na_archive.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/na/ARM9/sub_banner.h>
#include <twl/na/ARM9/shared_font.h>

#include <twl/ltdmain_begin.h>

SDK_WEAK_SYMBOL BOOL    NAi_InitSharedFont(void)
{
    return FALSE;
}
SDK_WEAK_SYMBOL int     NAi_GetSharedFontTableSize(void)
{
    return -1;
}
SDK_WEAK_SYMBOL BOOL    NAi_LoadSharedFontTable(void* pBuffer)
{
#pragma unused(pBuffer)
    return FALSE;
}
SDK_WEAK_SYMBOL int     NAi_GetSharedFontSize(NASharedFontIndex index)
{
#pragma unused(index)
    return -1;
}
SDK_WEAK_SYMBOL BOOL    NAi_LoadSharedFont(NASharedFontIndex index, void* pBuffer)
{
#pragma unused(index)
#pragma unused(pBuffer)
    return FALSE;
}

SDK_WEAK_SYMBOL BOOL    NAi_SaveSubBanner( const NASubBanner *banner )
{
#pragma unused(banner)
    return FALSE;
}
SDK_WEAK_SYMBOL BOOL    NAi_DeleteSubBanner( NASubBanner *workBuffer )
{
#pragma unused(workBuffer)
    return FALSE;
}

#include <twl/ltdmain_end.h>



/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
