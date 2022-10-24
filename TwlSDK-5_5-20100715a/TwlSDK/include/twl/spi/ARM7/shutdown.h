/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM - include
  File:     shutdown.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-14#$
  $Rev: 7332 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#ifndef TWL_PM_ARM7_SHUTDOWN_H_
#define TWL_PM_ARM7_SHUTDOWN_H_

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
//             SHUTDOWN
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_FlipHeartBeat

  Description:  flip debugger heart beat for avoid power off in resetHW
                (for DEBUGGER)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_FlipHeartBeat(void);


/*---------------------------------------------------------------------------*
  Name:         PM_SetMcuForTerminate

  Description:  set mcu interrupts for terminate

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_SetMcuForTerminate(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_PM_ARM7_SHUTDOWN_H_ */
#endif
