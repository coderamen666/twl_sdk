/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - mcs
  File:     print.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1316 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_MCS_PRINT_H_
#define NNS_MCS_PRINT_H_

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================
    Function Prototypes
   ======================================================================== */

#if defined(NNS_FINALROM)

    #define         NNS_McsInitPrint(pBuffer, size)         ((void)( (void)(pBuffer), (size) ))

    #define         NNS_McsFinalizePrint()                  ((void)0)

    #define         NNS_McsPutString(string)                ((void)0)

    #define         NNS_McsPrintf(...)                      ((void)0)

/* #if defined(NNS_FINALROM) */
#else

    void            NNS_McsInitPrint(
                        void*   pBuffer,
                        u32     size);

    void            NNS_McsFinalizePrint();

    BOOL            NNS_McsPutString(const char* string);

    BOOL            NNS_McsPrintf(
                        const char* format,
                        ...);

/* #if defined(NNS_FINALROM) */
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_MCS_PRINT_H_ */
#endif

