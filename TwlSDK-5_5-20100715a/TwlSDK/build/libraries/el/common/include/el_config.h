/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader Configuration
  File:     el_config.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-05#$
  $Rev: 9518 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#ifndef __ELF_LOADER_CONFIG_H__
#define __ELF_LOADER_CONFIG_H__

/***********************************************************************
 Print debugging statements: on or off
***********************************************************************/
#define DEBUG_PRINT_ON     (0)

/***********************************************************************
 Specify the target ARM architecture (ARM7TDMI is V4)
***********************************************************************/
#ifdef SDK_ARM7
  #define TARGET_ARM_V4      (1)
#else
  #define TARGET_ARM_V4      (0)
#endif

#define TARGET_ARM_V5      (TARGET_ARM_V4 ^ 1)



#if( DEBUG_PRINT_ON == 1)
    #define PRINTDEBUG          OS_TPrintf
#else
    #define PRINTDEBUG( ...)    ((void)0)
#endif

#define OSAPI_CPUFILL8         MI_CpuFill8
#define OSAPI_CPUCOPY8         MI_CpuCopy8


/***********************************************************************
 Debug notification-related (select istdbglibpriv.h definitions)
***********************************************************************/
#ifdef SDK_ARM7
#define ISTDRELOCATIONPROC_AUTO ISTDOVERLAYPROC_ARM7
#else
#define ISTDRELOCATIONPROC_AUTO ISTDOVERLAYPROC_ARM9
#endif

#if (TARGET_ARM_V5 == 1)
#define ISTDVENEERTYPE_AUTO     ISTDVENEERTYPE_ARM
#else
#define ISTDVENEERTYPE_AUTO     ISTDVENEERTYPE_ARMV4T
#endif


/***********************************************************************
 TWL OS 
***********************************************************************/
//#ifndef SDK_TWL
#if 0 // Do not use OS_Alloc unchanged
    #define OSAPI_MALLOC           OS_Alloc
    #define OSAPI_FREE             OS_Free
    #define OSAPI_STRLEN           STD_GetStringLength
    #define OSAPI_STRNCMP          STD_CompareNString
    #define OSAPI_STRCMP           STD_CompareString
#else
//#include <string.h>
    #define OSAPI_MALLOC           i_elAlloc
    #define OSAPI_FREE             i_elFree
    #define OSAPI_STRLEN           STD_GetStringLength
    #define OSAPI_STRNCMP          STD_CompareNString
    #define OSAPI_STRCMP           STD_CompareString
//    #define OSAPI_STRLEN           strlen
//    #define OSAPI_STRNCMP          strncmp
//    #define OSAPI_STRCMP           strcmp
#endif

#define OSAPI_FLUSHCACHEALL    IC_InvalidateAll(), DC_FlushAll
#define OSAPI_WAITCACHEBUF     DC_WaitWriteBufferEmpty


#endif /*__ELF_LOADER_CONFIG_H__*/
