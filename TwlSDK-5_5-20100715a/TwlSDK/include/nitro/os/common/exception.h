/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     exception.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_EXCEPTION_H_
#define NITRO_OS_EXCEPTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

//---- context for exception display
typedef struct
{
    OSContext   context;
    u32         cp15;
    u32         spsr;
	u32			exinfo;
} OSExcpContext;

//--------------------------------------------------------------------------------
//---- Exception Handler
typedef void (*OSExceptionHandler) (u32, void *);


//--------------------------------------------------------------------------------
/******************************** Exception Vector ********************************/
/*---------------------------------------------------------------------------*
  Name:         OS_SetExceptionVectorUpper

  Description:  set exception vector to hi-address

  Arguments:    none

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetExceptionVectorUpper(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetExceptionVectorLower

  Description:  set exception vector to low-address

  Arguments:    none

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetExceptionVectorLower(void);




/******************************** Exception Handler ********************************/
/*---------------------------------------------------------------------------*
  Name:         OS_InitException

  Description:  initialize exception system of sdk os.
                should be called once at first.

  Arguments:    none

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitException(void);



/*---------------------------------------------------------------------------*
  Name:         OS_SetUserExceptionHandler

  Description:  set user exception handler

  Arguments:    handler     user routine when exception occurred
                arg         argument in call user handler

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetUserExceptionHandler(OSExceptionHandler handler, void *arg);

/*---------------------------------------------------------------------------*
  Name:         OS_EnableUserExceptionHandlerOnDebugger

  Description:  enable user exception handler even if running on the debugger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_EnableUserExceptionHandlerOnDebugger(void);



#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_EXCEPTION_H_ */
#endif
