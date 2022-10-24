/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_wram.c

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

#include <nitro/mi/wram.h>
#include <nitro/misc.h>


//    assert definition
#define MIi_WRAM_ASSERT( x ) \
    SDK_ASSERT( (x) == MI_WRAM_ARM9_ALL || \
                (x) == MI_WRAM_1616_1   || \
                (x) == MI_WRAM_1616_2   || \
                (x) == MI_WRAM_ARM7_ALL )


#ifdef SDK_ARM9
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
#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/code32.h>
#endif
void MI_SetWramBank(MIWram cnt)
{
    MIi_WRAM_ASSERT(cnt);
    reg_GX_VRAMCNT_WRAM = (u8)cnt;     // safe byte access
}

#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/codereset.h>
#endif

#endif // SDK_ARM9


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
MIWram MI_GetWramBank(void)
{
// (register names are different between ARM9 and ARM7)
#ifdef SDK_ARM9
    return (MIWram)(reg_GX_VRAMCNT_WRAM & MI_WRAM_ARM9_ALL);    // safe byte access
#else
    return (MIWram)(reg_GX_WVRAMSTAT & MI_WRAM_ARM9_ALL);
#endif
}
