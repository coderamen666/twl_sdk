/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     wram.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_WRAM_H_
#define NITRO_MI_WRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

//----------------------------------------------------------------
//    enum definition
//
#ifdef SDK_ARM9
typedef enum
{
    MI_WRAM_ARM9_ALL = 0 << REG_GX_VRAMCNT_WRAM_BANK_SHIFT,
    MI_WRAM_1616_1 = 1 << REG_GX_VRAMCNT_WRAM_BANK_SHIFT,
    MI_WRAM_1616_2 = 2 << REG_GX_VRAMCNT_WRAM_BANK_SHIFT,
    MI_WRAM_ARM7_ALL = 3 << REG_GX_VRAMCNT_WRAM_BANK_SHIFT
}
MIWram;
#else  // SDK_ARM9
typedef enum
{
    MI_WRAM_ARM9_ALL = (REG_MI_WVRAMSTAT_WRAM_1_MASK | REG_MI_WVRAMSTAT_WRAM_0_MASK),
    MI_WRAM_1616_1 = (REG_MI_WVRAMSTAT_WRAM_0_MASK),
    MI_WRAM_1616_2 = (REG_MI_WVRAMSTAT_WRAM_1_MASK),
    MI_WRAM_ARM7_ALL = 0
}
MIWram;
#endif // SDK_ARM9


/*---------------------------------------------------------------------------*
  Name:         MI_SetWramBank

  Description:  set common-Wram bank status

  Arguments:    cnt      status of common-Wram.
                   MI_WRAM_ARM9_ALL :  allocate all blocks for ARM9
                   MI_WRAM_1616_1   :  allocate block1 for ARM9, block for ARM7
                   MI_WRAM_1616_2   :  allocate block0 for ARM9, block for ARM7
                   MI_WRAM_ARM7_ALL :  allocate all blocks for ARM7

  Returns:      None.

      *Notice: only ARM9 can set common-Wram status.
               ARM7 can read only.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void    MI_SetWramBank(MIWram cnt);
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramBank

  Description:  get common-Wram bank status

  Arguments:    None.

  Returns:      status of common-Wram.
                   MI_WRAM_ARM9_ALL :  allocate all blocks for ARM9
                   MI_WRAM_1616_1   :  allocate block1 for ARM9, block for ARM7
                   MI_WRAM_1616_2   :  allocate block0 for ARM9, block for ARM7
                   MI_WRAM_ARM7_ALL :  allocate all blocks for ARM7
 *---------------------------------------------------------------------------*/
MIWram  MI_GetWramBank(void);



#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_WRAM_H_ */
#endif
