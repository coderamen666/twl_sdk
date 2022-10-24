/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     import.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <twl/el/common/import.h>

const char* ELi_LastFunctionName = NULL;

void ELi_UnresolvedFunc(void)
{
    const char* const message = "An unresolved function is called!";
#ifdef SDK_DEBUG
    OS_TPanic("%s: %s()", message, ELi_LastFunctionName ? ELi_LastFunctionName : "unknown" );
#else
    OS_TPanic("%s", message );
#endif
}

void EL_LoadImportTable(ELDlld dlld, ELImportEntry entries[], u32 count)
{
    void* fp;
    u32 i;
    for ( i = 0; i < count; ++i )
    {
        fp = EL_GetGlobalAdr(dlld, entries[i].fn);
        if (fp)
        {
            entries[i].fp = fp;
        }
        else
        {
          OS_TWarning("EL_GetGlobalAdr failed: %s", entries[i].fn);
        }
    }
}

void EL_UnloadImportTable(ELImportEntry entries[], u32 count)
{
    u32 i;
    for ( i = 0; i < count; ++i )
    {
        entries[i].fp = ELi_UnresolvedFunc;
    }
}
