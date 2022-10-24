/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     common.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
*---------------------------------------------------------------------------*/
#ifndef MB_DEMO_COMMON_H_
#define MB_DEMO_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

//============================================================================
//  Function Declarations
//============================================================================


/*
 * This function is called NitroMain() on the multiboot-Model parent.
 * It is called in this sample when MB_IsMultiBootChild() == FALSE.
 */
void    ParentMain(void);

/*
 * This function is called NitroMain() on the multiboot-Model child.
 * It is called in this sample when MB_IsMultiBootChild() == TRUE.
 */
void    ChildMain(void);

/*
 * This function is located in the .parent section (an area reserved for the parent device).
 * It simply calls the ParentMain function.
 */
void    ParentIdentifier(void);

/* Everything else is identical to the multiboot-model. */

void    CommonInit();
void    ReadKey(void);
u16     GetPressKey(void);
u16     GetTrigKey(void);
void    InitAllocateSystem(void);

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_PRESS

  Description:  Key determination

  Arguments:    Key flag to determine

  Returns:      TRUE if specified key is held down
                Otherwise, FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_PRESS(u16 flag)
{
    return (GetPressKey() & flag) == flag;
}

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_TRIGGER

  Description:  Determines key trigger

  Arguments:    Key flag to determine

  Returns:      If specified key trigger is enabled, TRUE
                Otherwise, FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_TRIGGER(u16 flag)
{
    return (GetTrigKey() & flag) == flag;
}


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_COMMON_H_
