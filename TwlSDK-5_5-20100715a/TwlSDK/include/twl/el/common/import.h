/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     import.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
 
#ifndef TWL_COMMON_IMPORT_H_
#define TWL_COMMON_IMPORT_H_

#include <twl/el.h>

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------
  This header file includes definitions used by the import library.
  
  In general, developers do not need to refer to this header file.
  
 -----------------------------------------------------*/

typedef struct
{
    void*       fp;
    const char* fn;
} ELImportEntry;


#ifdef SDK_DEBUG
extern const char* ELi_LastFunctionName;

SDK_INLINE void ELi_SetLastFunctionName(const char* fn)
{
    ELi_LastFunctionName = fn;
}
#else
#define ELi_SetLastFunctionName(fn)   (void)(fn);
#endif

void ELi_UnresolvedFunc(void);
void EL_LoadImportTable(ELDlld dlld, ELImportEntry entries[], u32 count);
void EL_UnloadImportTable(ELImportEntry entries[], u32 count);

#ifdef __cplusplus
}    /* extern "C" */
#endif

#endif    /*TWL_COMMON_IMPORT_H_*/
