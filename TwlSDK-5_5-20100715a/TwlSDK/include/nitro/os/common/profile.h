/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     profile.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//================================================================
#ifndef NITRO_OS_PROFILE_H_
#define NITRO_OS_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

//---- if available CallTrace
#ifdef SDK_PROFILE_CALLTRACE
#  define OS_PROFILE_AVAILABLE
#  define OS_PROFILE_CALL_TRACE
#endif

//---- if available FunctionCost
#ifdef SDK_PROFILE_FUNCTIONCOST
#  define OS_PROFILE_AVAILABLE
#  define OS_PROFILE_FUNCTION_COST
#endif

//================================================================
#ifdef OS_PROFILE_AVAILABLE

//---- check OS_PROFILE_XXX define
#if defined( OS_PROFILE_CALL_TRACE ) && defined( OS_PROFILE_FUNCTION_COST )
*** ERROR:cannot specify both options of OS_PROFILE_CALL_TRACE and OS_PROFILE_FUNCTION_COST at a same
    time.
#endif
//---- if FINALROM, never be available CallTrace and FunctionCost.
#ifdef SDK_FINALROM
#  define OS_NO_CALLTRACE
#  define OS_NO_FUNCTIONCOST
#endif // ifdef SDK_FINALROM
#endif // ifdef OS_PROFILE_AVAILABLE
//---- prototype
void    __PROFILE_ENTRY(void);
void    __PROFILE_EXIT(void);


//----------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_PROFILE_H_ */
#endif
