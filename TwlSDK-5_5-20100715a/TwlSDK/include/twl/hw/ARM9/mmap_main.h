/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl - HW - ARM9
  File:     mmap_main.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_HW_ARM9_MMAP_MAIN_H_
#define TWL_HW_ARM9_MMAP_MAIN_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    MEMORY MAP of MAIN MEMORY
 *---------------------------------------------------------------------------*/
/*
        Retail NITRO         Retail TWL           Development TWL
  32M -+---------------+----+---------------+----+---------------+- 0x0e000000
       |               |    |               |    | System        |
       |               |    |               |    | Reserved      |
       |               |    |               |    |---------------+- 0x0dfc0000
       |               |    |               |    | Debugger Work |
       |               |    | No Device     |    |---------------+- 0x0df00000
       |               |    | ( N/A )       |    | Main          |
       |               |    |               |    | Processor     |
       |               |    |               |    | Extra         |
       |               |    |               |    | Reserved      |
  16M -+ Indefinite    +----+---------------+----+---------------+- 0x0d000000
       |               |    | Wrap around   |    | Wrap around   |
       |               |    | 0x02000000 -  |    | 0x02000000 -  |
       |               |    |  0x02ffffff   |    |  0x02ffffff   |
       |               |    | ( N/A )       |    | ( N/A )       |
    0 -+---------------+----+---------------+----+---------------+- 0x0c000000


  16M -+---------------+----+---------------+----+---------------+- 0x03000000
       | System Shared |    | System Shared |    | System Shared |
       +---------------+----+---------------+----+---------------+- 0x02fff680
       | User Shared   |    | User Shared   |    | User Shared   |
       +---------------+----+---------------+----+---------------+- 0x02fff000
       |               |    | System Shared |    | System Shared |
       | Sub Processor |    +---------------+----+---------------+- 0x02ffc000
       | Reserved      |    | Sub Processor |    | Sub Processor |
       |               |    | Reserved      |    | Reserved      |
       +------------+  |    +------------+  |    +------------+  +- 0x02fe4000
       |Default DTCM|  |    |Default DTCM|  |    |Default DTCM|  |
       +-+----------+--+----+------------+  +    +------------+  +- 0x02fe0000
       | |Main         |    |               |    |               |
       |W|Processor    |    +---------------+----+---------------+- 0x02f80000
       |R|Reserved     |    |               |    |               |
  12M -+A+-------------+----+               +----+               +- 0x02c00000
       |P|System Shared|    |               |    |               |
       | +-------------+    | Main          |    | Main          |
       |A|User Shared  |    | Processor     |    | Processor     |
       |R+-------------+    | Reserved      |    | Reserved      |
       |O|Sub Processor|    |               |    |               |
       |U|Reserved     |    |               |    |               |
       |N+-------------+    |  for          |    |  for          |
       |D|Main         |    |               |    |               |
       | |Processor    |    | TWL Private   |    | TWL Private   |
       |/|Reserved     |    | Static Data   |    | Static Data   |
   8M -+ +-------------+----+               +----+               +- 0x02800000
       |N|System Shared|    |  or           |    |  or           |
       |O+-------------+    |               |    |               |
       |T|User Shared  |    | Arena         |    | Arena         |
       | +-------------+    |               |    |               |
       |A|Sub Processor|    |               |    |               |
       |V|Reserved     |    |               |    |               |
       |A+-------------+    |               |    |               |
       |I|Main         |    |               |    |               |
       |L|Processor    |    |               |    |               |
       |A|Reserved     |    |               |    |               |
   4M -+B+-------------+----+               +----+               +- 0x02400000
       |L|System Shared|    |               |    |               |
       |E+-------------+    |               |    |               |
       | |User Shared  |    |               |    |               |
       | +-------------+    |               |    |               |
       | |Sub Processor|    |               |    |               |
       | |Reserved     |    |               |    |               |
       +-+-------------+----+ - - - - - - - +----+ - - - - - - - +- 0x023e0000
       |               |    |               |    |               |
       | Main          |    | Main          |    | Main          |
       | Processor     |    | Processor     |    | Processor     |
       | Reserved      |    | Reserved      |    | Reserved      |
       |               |    |               |    |               |
    0 -+------------+--+----+------------+--+----+------------+--+- 0x02000000
       |Default ITCM|       |Default ITCM|       |Default ITCM|
       +------------+       +------------+       +------------+   - 0x01ff8000
*/

#define HW_MAIN_MEM_MAIN_SIZE       0x003e0000      // 3.875 MB
#define HW_MAIN_MEM_SUB_SIZE        0x0001f000      // 124 KB
#define HW_MAIN_MEM_SHARED_SIZE     0x00001000      // 4 KB

#define HW_MAIN_MEM_MAIN            (HW_MAIN_MEM)
#define HW_MAIN_MEM_MAIN_END        (HW_MAIN_MEM_MAIN + HW_MAIN_MEM_MAIN_SIZE)
#define HW_MAIN_MEM_SUB             (HW_MAIN_MEM_SUB_END - HW_MAIN_MEM_SUB_SIZE)
#define HW_MAIN_MEM_SUB_END         (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE)

#define HW_MAIN_MEM_SHARED          (HW_MAIN_MEM_SHARED_END - HW_MAIN_MEM_SHARED_SIZE)
#define HW_MAIN_MEM_SHARED_END      (HW_TWL_MAIN_MEM_END)

#define HW_MAIN_MEM_IM_SHARED_SIZE  (HW_MAIN_MEM_SHARED_SIZE)
#define HW_MAIN_MEM_IM_SHARED       (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE)
#define HW_MAIN_MEM_IM_SHARED_END   (HW_MAIN_MEM + 0x00800000)

#define HW_TWL_MAIN_MEM_MAIN_SIZE   0x00f80000      // 15.5 MB
#define HW_TWL_MAIN_MEM_SUB_SIZE    0x0007c000      // 496 KB
#define HW_TWL_MAIN_MEM_SHARED_SIZE 0x00004000      // 16 KB

#define HW_TWL_MAIN_MEM_MAIN        (HW_TWL_MAIN_MEM)
#define HW_TWL_MAIN_MEM_MAIN_END    (HW_TWL_MAIN_MEM_MAIN + HW_TWL_MAIN_MEM_MAIN_SIZE)
#define HW_TWL_MAIN_MEM_SUB         (HW_TWL_MAIN_MEM_SUB_END - HW_TWL_MAIN_MEM_SUB_SIZE)
#define HW_TWL_MAIN_MEM_SUB_END     (HW_TWL_MAIN_MEM_SHARED)
#define HW_TWL_MAIN_MEM_SHARED      (HW_TWL_MAIN_MEM_SHARED_END - HW_TWL_MAIN_MEM_SHARED_SIZE)
#define HW_TWL_MAIN_MEM_SHARED_END  (HW_TWL_MAIN_MEM_END)


/*----------------------------------------------------------------------*
    memory area for parameter buffer
 *----------------------------------------------------------------------*/
#define HW_MAIN_MEM_PARAMETER_BUF		0x02000000
#define HW_MAIN_MEM_PARAMETER_BUF_SIZE	0x4000
#define HW_MAIN_MEM_PARAMETER_BUF_END	(HW_MAIN_MEM_PARAMETER_BUF + HW_MAIN_MEM_PARAMETER_BUF_SIZE )

/*----------------------------------------------------------------------*
    memory area debugger uses
 *----------------------------------------------------------------------*/
//---- debugger for NITRO
#define HW_MAIN_MEM_DEBUGGER_OFFSET 0x700000
#define HW_MAIN_MEM_DEBUGGER_SIZE   0x080000
#define HW_MAIN_MEM_DEBUGGER        (HW_MAIN_MEM + HW_MAIN_MEM_DEBUGGER_OFFSET)
#define HW_MAIN_MEM_DEBUGGER_END    (HW_MAIN_MEM_DEBUGGER + HW_MAIN_MEM_DEBUGGER_SIZE)

//---- debugger for TWL
#define HW_TWL_MAIN_MEM_DEBUGGER_OFFSET (HW_TWL_MAIN_MEM_DEBUGGER - HW_TWL_MAIN_MEM)
#define HW_TWL_MAIN_MEM_DEBUGGER_SIZE   0x0c0000
#define HW_TWL_MAIN_MEM_DEBUGGER        0x0df00000
#define HW_TWL_MAIN_MEM_DEBUGGER_END    (HW_TWL_MAIN_MEM_DEBUGGER + HW_TWL_MAIN_MEM_DEBUGGER_SIZE)


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_HW_ARM9_MMAP_MAIN_H_ */
