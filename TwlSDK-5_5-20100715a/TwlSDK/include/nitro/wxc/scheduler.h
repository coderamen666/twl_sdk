/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     scheduler.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_SCHEDULER_H_
#define	NITRO_WXC_SCHEDULER_H_


#include <nitro.h>


/*****************************************************************************/
/* Constants */

/* Parent/child switch pattern */
#define WXC_SCHEDULER_PATTERN_MAX   4
#define WXC_SCHEDULER_SEQ_MAX       4


/*****************************************************************************/
/* Declaration */

/* Structure managing switching between parent and child mode */
typedef struct WXCScheduler
{
    /* Current switch sequence */
    int     seq;
    int     pattern;
    int     start;
    BOOL    child_mode;
    /* Switch table (TRUE is parent, FALSE is child) */
    BOOL    table[WXC_SCHEDULER_PATTERN_MAX][WXC_SCHEDULER_SEQ_MAX];
}
WXCScheduler;


/*****************************************************************************/
/* Functions */

#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*
  Name:         WXCi_InitScheduler

  Description:  Initializes the mode-switch scheduler.

  Arguments:    p: WXCScheduler structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_InitScheduler(WXCScheduler * p);

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetChildMode

  Description:  Sets scheduler to operate with child device fixed.

  Arguments:    p: WXCScheduler structure
                enable: If it can only be run on the child side, TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_SetChildMode(WXCScheduler * p, BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         WXCi_UpdateScheduler

  Description:  Updates mode switch scheduler.

  Arguments:    p: WXCScheduler structure

  Returns:      Return TRUE if currently in parent mode. Return FALSE if currently in child mode.
 *---------------------------------------------------------------------------*/
BOOL    WXCi_UpdateScheduler(WXCScheduler * p);


#ifdef __cplusplus
}
#endif


#endif /* NITRO_WXC_SCHEDULER_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
