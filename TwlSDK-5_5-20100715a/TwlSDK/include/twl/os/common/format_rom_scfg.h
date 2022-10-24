/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     format_rom_scfg.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-09#$
  $Rev: 11047 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#ifndef	FORMAT_ROM_SCFG_H_
#define	FORMAT_ROM_SCFG_H_

//---- for ARM7-SCFG-EXT register
#define ROM_SCFG_EXT_OFFSET             0x1B8
#define ROM_SCFG_EXT_SIZE               4

#define ROM_SCFG_EXT_CFG_MASK           0x80000000
#define ROM_SCFG_EXT_SD1_MASK           0x00040000
#define ROM_SCFG_EXT_DSEL_MASK          0x00000400
#define ROM_SCFG_EXT_SND_MASK           0x00000004
#define ROM_SCFG_EXT_SDMA_MASK          0x00000002
#define ROM_SCFG_EXT_DMA_MASK           0x00000001

#define ROM_SCFG_EXT_SPECMASK   \
        (ROM_SCFG_EXT_CFG_MASK | \
		 ROM_SCFG_EXT_SD1_MASK | ROM_SCFG_EXT_DSEL_MASK | \
         ROM_SCFG_EXT_SND_MASK | ROM_SCFG_EXT_SDMA_MASK | ROM_SCFG_EXT_DMA_MASK )

#define ROM_SCFG_EXT_SPEC_DEFAULT       ( ROM_SCFG_EXT_DSEL_MASK | ROM_SCFG_EXT_SND_MASK | ROM_SCFG_EXT_SDMA_MASK )


//---- for ARM7-SCFG-CLK register
#define ROM_SCFG_CLK_SD1HCLK_MASK       0x0001

#define ROM_SCFG_CLK_SPECMASK           (ROM_SCFG_CLK_SD1HCLK_MASK)

#define ROM_SCFG_CLK_SPEC_DEFAULT       ( 0 )


#endif //#define	FORMAT_ROM_SCFG_H_
