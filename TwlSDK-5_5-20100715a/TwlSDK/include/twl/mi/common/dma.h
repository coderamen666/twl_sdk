/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     dma.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-16#$
  $Rev: 8977 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MI_DMA_H_
#define TWL_MI_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/misc.h>
#include <twl/types.h>
#include <twl/memorymap.h>

#include <nitro/mi/dma.h>

//================================================================================
//                    new-DMA control definition
//================================================================================
//---- maximum new-DMA channel No.
#define MI_NDMA_MAX_NUM            3

//---------------- global control register
//---- arbitrament type
#define MI_NDMA_ARBITRAMENT_FIX    (0UL << REG_MI_NDMAGCNT_ARBITER_SHIFT)
#define MI_NDMA_ARBITRAMENT_ROUND  (1UL << REG_MI_NDMAGCNT_ARBITER_SHIFT)

//---- DSP-DMA & CPU Cycle select
#define MI_NDMA_RCYCLE_MASK        (REG_MI_NDMAGCNT_CPUCYCLE_MASK)
#define MI_NDMA_RCYCLE_0           ( 0UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_1           ( 1UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_2           ( 2UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_4           ( 3UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_8           ( 4UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_16          ( 5UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_32          ( 6UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_64          ( 7UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_128         ( 8UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_256         ( 9UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_512         (10UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_1024        (11UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_2048        (12UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_4096        (13UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_8192        (14UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#define MI_NDMA_RCYCLE_16384       (15UL << REG_MI_NDMAGCNT_CPUCYCLE_SHIFT)
#ifdef SDK_ARM9
#define MI_NDMA_RCYCLE_DEFAULT     MI_NDMA_RCYCLE_32
#else // SDK_ARM7
#define MI_NDMA_RCYCLE_DEFAULT     MI_NDMA_RCYCLE_16
#endif // SDK_ARM7

//---------------- control register
//---- enable
#define MI_NDMA_ENABLE_MASK        (1UL << REG_MI_NDMA0CNT_E_SHIFT)      // DMA enable mask
#define MI_NDMA_ENABLE             (1UL << REG_MI_NDMA0CNT_E_SHIFT)      // DMA enable
#define MI_NDMA_DISABLE            (0UL << REG_MI_NDMA0CNT_E_SHIFT)      // DMA disable

//---- interrupt enable
#define MI_NDMA_IF_ENABLE          (1UL << REG_MI_NDMA0CNT_I_SHIFT)      // interrupt enable
#define MI_NDMA_IF_DISABLE         (0UL << REG_MI_NDMA0CNT_I_SHIFT)      // interrupt disable

//---- continuous mode
#define MI_NDMA_CONTINUOUS_ON      (1UL << REG_MI_NDMA0CNT_CM_SHIFT)     // continuous mode on
#define MI_NDMA_CONTINUOUS_OFF     (0UL << REG_MI_NDMA0CNT_CM_SHIFT)     // continuous mode off

//---- immidiate mode
#define MI_NDMA_IMM_MODE_ON        (1UL << REG_MI_NDMA0CNT_IM_SHIFT)     // immidiate mode on
#define MI_NDMA_IMM_MODE_OFF       (0UL << REG_MI_NDMA0CNT_IM_SHIFT)     // immidiate mode on

//---- dma timing
#define MI_NDMA_TIMING_MASK        (REG_MI_NDMA0CNT_MODE_MASK)
#define MI_NDMA_TIMING_SHIFT       (REG_MI_NDMA0CNT_MODE_SHIFT)
#define MI_NDMA_TIMING_TIMER0      ( 0UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by timer0
#define MI_NDMA_TIMING_TIMER1      ( 1UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by timer1
#define MI_NDMA_TIMING_TIMER2      ( 2UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by timer2
#define MI_NDMA_TIMING_TIMER3      ( 3UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by timer3
#define MI_NDMA_TIMING_CARD        ( 4UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by card A
#define MI_NDMA_TIMING_CARD_A      MI_NDMA_TIMING_CARD
#define MI_NDMA_TIMING_V_BLANK     ( 6UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by VBlank
#ifdef SDK_ARM9
#define MI_NDMA_TIMING_H_BLANK     ( 7UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by HBlank
#define MI_NDMA_TIMING_DISP        ( 8UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by display sync
#define MI_NDMA_TIMING_DISP_MMEM   ( 9UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by main memory display sync
#define MI_NDMA_TIMING_GXFIFO      (10UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by geometry FIFO
#define MI_NDMA_TIMING_CAMERA      (11UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by camera
#else // SDK_ARM7
#define MI_NDMA_TIMING_WIRELESS    ( 7UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by 
#define MI_NDMA_TIMING_SD_1        ( 8UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by SD I/F 1
#define MI_NDMA_TIMING_SD_2        ( 9UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by SD I/F 2
#define MI_NDMA_TIMING_AES_IN      (10UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by AES in
#define MI_NDMA_TIMING_AES_OUT     (11UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by AES out
#define MI_NDMA_TIMING_MIC         (11UL << REG_MI_NDMA0CNT_MODE_SHIFT)  // start by mic
#endif // SDK_ARM7
#define MIi_NDMA_TIMING_IMMIDIATE  0xffffffff                            // for internal use

//---- DMA block word count
#define MI_NDMA_BWORD_MASK         (REG_MI_NDMA0CNT_WORDCNT_MASK)
#define MI_NDMA_BWORD_1            ( 0UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //     1 word
#define MI_NDMA_BWORD_2            ( 1UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //     2 words
#define MI_NDMA_BWORD_4            ( 2UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //     4 words
#define MI_NDMA_BWORD_8            ( 3UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //     8 words
#define MI_NDMA_BWORD_16           ( 4UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //    16 words
#define MI_NDMA_BWORD_32           ( 5UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //    32 words
#define MI_NDMA_BWORD_64           ( 6UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //    64 words
#define MI_NDMA_BWORD_128          ( 7UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //   128 words
#define MI_NDMA_BWORD_256          ( 8UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //   256 words
#define MI_NDMA_BWORD_512          ( 9UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //   512 words
#define MI_NDMA_BWORD_1024         (10UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //  1024 words
#define MI_NDMA_BWORD_2048         (11UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //  2048 words
#define MI_NDMA_BWORD_4096         (12UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //  4096 words
#define MI_NDMA_BWORD_8192         (13UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  //  8192 words
#define MI_NDMA_BWORD_16384        (14UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  // 16384 words
#define MI_NDMA_BWORD_32768        (15UL << REG_MI_NDMA0CNT_WORDCNT_SHIFT)  // 32768 words

//---- src address reload enable
#define MI_NDMA_SRC_RELOAD_ENABLE  (1UL << REG_MI_NDMA0CNT_SRL_SHIFT)       // reload after transferring specified words
#define MI_NDMA_SRC_RELOAD_DISABLE (0UL << REG_MI_NDMA0CNT_SRL_SHIFT)       // not reload

//---- direction of src address
#define MI_NDMA_SRC_INC            (0UL << REG_MI_NDMA0CNT_SAR_SHIFT)       // increment
#define MI_NDMA_SRC_DEC            (1UL << REG_MI_NDMA0CNT_SAR_SHIFT)       // decrement
#define MI_NDMA_SRC_FIX            (2UL << REG_MI_NDMA0CNT_SAR_SHIFT)       // fix
#define MI_NDMA_SRC_FILLDATA       (3UL << REG_MI_NDMA0CNT_SAR_SHIFT)       // no address (= use fill data)

//---- src address reload enable
#define MI_NDMA_DEST_RELOAD_ENABLE  (1UL << REG_MI_NDMA0CNT_DRL_SHIFT)      // reload after transferring specified words
#define MI_NDMA_DEST_RELOAD_DISABLE (0UL << REG_MI_NDMA0CNT_DRL_SHIFT)      // not reload

// direction of dest address
#define MI_NDMA_DEST_INC           (0UL << REG_MI_NDMA0CNT_DAR_SHIFT)       // increment
#define MI_NDMA_DEST_DEC           (1UL << REG_MI_NDMA0CNT_DAR_SHIFT)       // decrement
#define MI_NDMA_DEST_FIX           (2UL << REG_MI_NDMA0CNT_DAR_SHIFT)       // fix

//----------------  block transfer interval setting
#define MI_NDMA_INTERVAL_PS_1      (0UL << REG_MI_NDMA0BCNT_PS_SHIFT)       // system clock      (33.514MHz)
#define MI_NDMA_INTERVAL_PS_4      (1UL << REG_MI_NDMA0BCNT_PS_SHIFT)       // system clock x  4
#define MI_NDMA_INTERVAL_PS_16     (2UL << REG_MI_NDMA0BCNT_PS_SHIFT)       // system clock x 16
#define MI_NDMA_INTERVAL_PS_64     (3UL << REG_MI_NDMA0BCNT_PS_SHIFT)       // system clock x 64


//================================================================
// for convinience to access NDMA register
//================================================================
#define MI_NDMA_NUM_WOFFSET(ndmaNo)     (ndmaNo * 7)
#define MI_NDMA_REG_SAD_WOFFSET         0
#define MI_NDMA_REG_DAD_WOFFSET     	1
#define MI_NDMA_REG_TCNT_WOFFSET  		2
#define MI_NDMA_REG_WCNT_WOFFSET  		3
#define MI_NDMA_REG_BCNT_WOFFSET  		4
#define MI_NDMA_REG_FDATA_WOFFSET 		5
#define MI_NDMA_REG_CNT_WOFFSET 		6

#define MI_NDMA_REGADDR(ndmaNo, reg)    (((vu32*)REG_NDMA0SAD_ADDR) + MI_NDMA_NUM_WOFFSET(ndmaNo) + reg )
#define MI_NDMA_REG(ndmaNo, reg)        ( *( MI_NDMA_REGADDR(ndmaNo, reg) ) )

//---------------- for internal parameter
#define MI_NDMA_NO_INTERVAL        		0
#define MI_NDMA_AT_A_TIME				0xffffffff


//---- NDMA config
typedef struct _MINDmaConfig
{
	u32 intervalTimer;
	u32 prescaler;
	u32 blockWord;
	u32 wordCount;
}
MINDmaConfig;

//---- NDMA callback
typedef void (*MINDmaCallback) (void *);

//---- NDMA device
typedef u32 MINDmaDevice;


//================================================================================
//        initialize NDMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_InitNDma

  Description:  initialize NDMA.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_InitNDma( void );

//================================================================
//         do Fill, Copy, Send, Recv, Pipe by using NDMA
//
//  (1) MI_NDmaXXX                         imm
//  (2) MI_NDmaXXXAsync                    async 
//  (3) MI_NDmaXXXEx            useConfig  imm
//  (4) MI_NDmaXXXExAsync       useConfig, async,
//  (5) MI_NDmaXXX_Dev                     imm    drivenByDevice
//  (6) MI_NDmaXXXAsync_Dev                async, drivenByDevice
//  (7) MI_NDmaXXXEx_Dev        useConfig, imm    drivenByDevice
//  (8) MI_NDmaXXXExAsync_Dev   useConfig, async, drivenByDevice
//
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_NDmaFill

  Description:  fill memory with specified data.

  Arguments:    dmaNo : NDMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  Name:         MI_NDmaCopy

  Description:  copy memory with NDMA.

  Arguments:    dmaNo : NDMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  Name:         MI_NDmaSend

  Description:  send data to fixed address.

  Arguments:    dmaNo : NDMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  Name:         MI_NDmaRecv

  Description:  receive data from fixed address.

  Arguments:    dmaNo : NDMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  Name:         MI_NDmaPipe

  Description:  pipe data from fixed address to fixed address.

  Arguments:    dmaNo : NDMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  Name:         MI_NDmaClear

  Description:  fill memory with 0
                (define of MI_NDmaFill)

  Arguments:    dmaNo : NDMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 (Common)
  Arguments:    callback : callback which will be called at NDMA finish
                arg      : argument of callback
				config   : pointer to NDMA config struct
				dev      : device which causes NDMA    (MI_NDMA_TIMING_xxxx)

 *---------------------------------------------------------------------------*/
//   (1) 
void MI_NDmaFill			(u32 ndmaNo,
							 void *dest, u32 data, u32 size );
void MI_NDmaFill_SetUp		(u32 ndmaNo,
							 void *dest, u32 data, u32 size );
void MI_NDmaCopy			(u32 ndmaNo,
							 const void *src, void *dest, u32 size );
void MI_NDmaCopy_SetUp		(u32 ndmaNo,
							 const void *src, void *dest, u32 size );
void MI_NDmaSend			(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size );
void MI_NDmaSend_SetUp		(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size );
void MI_NDmaRecv			(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size );
void MI_NDmaRecv_SetUp		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size );
void MI_NDmaPipe			(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size );
void MI_NDmaPipe_SetUp		(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size );
#define MI_NDmaClear(ndmaNo, dest, size) \
		MI_NDmaFill((ndmaNo), (dest), 0, (size))
#define MI_NDmaClear_SetUp(ndmaNo, dest, size) \
		MI_NDmaFill_SetUp((ndmaNo), (dest), 0, (size))

//   (2) 
void MI_NDmaFillAsync		(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void *arg );
void MI_NDmaFillAsync_SetUp	(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void *arg );
void MI_NDmaCopyAsync		(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaCopyAsync_SetUp	(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaSendAsync		(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaSendAsync_SetUp	(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaRecvAsync		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaRecvAsync_SetUp	(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaPipeAsync		(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
void MI_NDmaPipeAsync_SetUp	(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg );
#define MI_NDmaClearAsync(ndmaNo, dest, size, callback, arg) \
		MI_NDmaFillAsync((ndmaNo), (dest), 0, (size), (callback), (arg))
#define MI_NDmaClearAsync_SetUp(ndmaNo, dest, size, callback, arg) \
		MI_NDmaFillAsync_SetUp((ndmaNo), (dest), 0, (size), (callback), (arg))

//   (3) 
void MI_NDmaFillEx			(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 const MINDmaConfig *config);
void MI_NDmaFillEx_SetUp	(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 const MINDmaConfig *config);
void MI_NDmaCopyEx			(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaCopyEx_SetUp	(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaSendEx			(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaSendEx_SetUp	(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaRecvEx			(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaRecvEx_SetUp	(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaPipeEx			(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config );
void MI_NDmaPipeEx_SetUp	(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config );
#define MI_NDmaClearEx(ndmaNo, dest, size, config) \
		MI_NDmaFillEx((ndmaNo), (dest), 0, (size), (config))
#define MI_NDmaClearEx_SetUp(ndmaNo, dest, size, config) \
		MI_NDmaFillEx_SetUp((ndmaNo), (dest), 0, (size), (config))

//   (4) 
void MI_NDmaFillExAsync		(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void *arg, const MINDmaConfig *config);
void MI_NDmaFillExAsync_SetUp(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void *arg, const MINDmaConfig *config);
void MI_NDmaCopyExAsync		(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config );
void MI_NDmaCopyExAsync_SetUp(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config );
void MI_NDmaSendExAsync		(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
void MI_NDmaSendExAsync_SetUp(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
void MI_NDmaRecvExAsync		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
void MI_NDmaRecvExAsync_SetUp(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
void MI_NDmaPipeExAsync		(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
void MI_NDmaPipeExAsync_SetUp(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config);
#define MI_NDmaClearExAsync(ndmaNo, dest, size, callback, arg, config) \
		MI_NDmaFillExAsync((ndmaNo), (dest), 0, (size), (callback), (arg), (config))
#define MI_NDmaClearExAsync_SetUp(ndmaNo, dest, size, callback, arg, config) \
		MI_NDmaFillExAsync_SetUp((ndmaNo), (dest), 0, (size), (callback), (arg), (config))

//   (5) 
void MI_NDmaFill_Dev		(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaDevice dev );
void MI_NDmaFill_Dev_SetUp	(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaDevice dev );
void MI_NDmaCopy_Dev		(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaCopy_Dev_SetUp	(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaSend_Dev		(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaSend_Dev_SetUp	(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaRecv_Dev		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaRecv_Dev_SetUp		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaPipe_Dev		(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaDevice dev );
void MI_NDmaPipe_Dev_SetUp	(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaDevice dev );
#define MI_NDmaClear_Dev(ndmaNo, dest, size, dev) \
		MI_NDmaFill_Dev((ndmaNo), (dest), 0, (size), (dev))
#define MI_NDmaClear_Dev_SetUp(ndmaNo, dest, size, dev) \
		MI_NDmaFill_Dev_SetUp((ndmaNo), (dest), 0, (size), (dev))

//   (6) 
void MI_NDmaFillAsync_Dev	(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev);
void MI_NDmaFillAsync_Dev_SetUp(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev);
void MI_NDmaCopyAsync_Dev	(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaCopyAsync_Dev_SetUp(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaSendAsync_Dev	(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaSendAsync_Dev_SetUp(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaRecvAsync_Dev	(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaRecvAsync_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaPipeAsync_Dev	(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
void MI_NDmaPipeAsync_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, MINDmaDevice dev );
#define MI_NDmaClearAsync_Dev(ndmaNo, dest, size, callback, arg, dev) \
		MI_NDmaFillAsync_Dev((ndmaNo), (dest), 0, (size), (callback), (arg), (dev))
#define MI_NDmaClearAsync_Dev_SetUp(ndmaNo, dest, size, callback, arg, dev) \
		MI_NDmaFillAsync_Dev_SetUp((ndmaNo), (dest), 0, (size), (callback), (arg), (dev))

//   (7) 
void MI_NDmaFillEx_Dev		(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev);
void MI_NDmaFillEx_Dev_SetUp(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev);
void MI_NDmaCopyEx_Dev		(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaCopyEx_Dev_SetUp(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaSendEx_Dev		(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaSendEx_Dev_SetUp(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaRecvEx_Dev		(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaRecvEx_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaPipeEx_Dev		(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaPipeEx_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 const MINDmaConfig *config, MINDmaDevice dev );
#define MI_NDmaClearEx_Dev(ndmaNo, dest, size, config, dev) \
		MI_NDmaFillEx_Dev((ndmaNo), (dest), 0, (size), (config), (dev))
#define MI_NDmaClearEx_Dev_SetUp(ndmaNo, dest, size, config, dev) \
		MI_NDmaFillEx_Dev_SetUp((ndmaNo), (dest), 0, (size), (config), (dev))

//   (8) 
void MI_NDmaFillExAsync_Dev	(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev);
void MI_NDmaFillExAsync_Dev_SetUp(u32 ndmaNo,
							 void *dest, u32 data, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev);
void MI_NDmaCopyExAsync_Dev	(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaCopyExAsync_Dev_SetUp(u32 ndmaNo,
							 const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaSendExAsync_Dev	(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaSendExAsync_Dev_SetUp(u32 ndmaNo,
							 const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaRecvExAsync_Dev	(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaRecvExAsync_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaPipeExAsync_Dev	(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
void MI_NDmaPipeExAsync_Dev_SetUp(u32 ndmaNo,
							 volatile const void *src, volatile void *dest, u32 size,
							 MINDmaCallback callback, void* arg, const MINDmaConfig *config, MINDmaDevice dev );
#define MI_NDmaClearExAsync_Dev(ndmaNo, dest, size, callback, arg, config, dev)	\
		MI_NDmaFillExAsync_Dev((ndmaNo), (dest), 0, (size), (callback), (arg), (config), (dev))
#define MI_NDmaClearExAsync_Dev_SetUp(ndmaNo, dest, size, callback, arg, config, dev)	\
		MI_NDmaFillExAsync_Dev_SetUp((ndmaNo), (dest), 0, (size), (callback), (arg), (config), (dev))


//================================================================================
//        DMA WAIT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_IsNDmaBusy

  Description:  check whether DMA is busy or not

  Arguments:    ndmaNo : DMA channel No.

  Returns:      TRUE if DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL MI_IsNDmaBusy(u32 ndmaNo);

/*---------------------------------------------------------------------------*
  Name:         MI_WaitNDma

  Description:  wait while DMA is busy

  Arguments:    ndmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_WaitNDma(u32 ndmaNo);

/*---------------------------------------------------------------------------*
  Name:         MI_StopNDma

  Description:  stop DMA

  Arguments:    ndmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_StopNDma(u32 ndmaNo);


/*---------------------------------------------------------------------------*
  Name:         MI_StopAllNDma

  Description:  stop all DMA

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_StopAllNDma(void);

//================================================================================
//        restart NDMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_NDmaRestart

  Description:  restart DMA
                Just set the enable bit.

  Arguments:    ndmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_NDmaRestart(u32 ndmaNo);

//================================================================================
//        NDMA configure
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_SetNDmaArbitrament

  Description:  set arbittament mode and cycle for DSP and CPU access to AHB

  Arguments:    mode  : Arbittament mode

                       MI_NDMA_ARBITRAMENT_FIX       fix
                       MI_NDMA_ARBITRAMENT_ROUND     round robin

                cycle : Cycles for DSP and CPU access to AHB
                        This parameter is available only when mode is round robin.

                       MI_NDMA_RCYCLE_n
                            (n = 0, 1, 2, 4, 8, ..., 8192, 16384)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SetNDmaArbitrament( u32 mode, u32 cycle );

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaArbitramentMode

  Description:  get NDMA arbitrament mode setting

  Arguments:    None

  Returns:      value which is set

                       MI_NDMA_ARBITRAMENT_FIX       fix
                       MI_NDMA_ARBITRAMENT_ROUND     round robin

 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaArbitramentMode(void);

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaArbitramentRoundRobinCycle

  Description:  get cycle setting for DSP and CPU access to AHB

  Arguments:    None

  Returns:      value which is set

                       MI_NDMA_RCYCLE_n (n = 0, 1, 2, 4, 8, ..., 16384)

 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaArbitramentRoundRobinCycle(void);

/*---------------------------------------------------------------------------*
  Name:         MI_SetNDmaInterval

  Description:  set interval time and prescaler

  Arguments:    ndmaNo       : NDMA number. (0-3)
                intervalTime : interval time. (0-0xffff)
                prescaler    : prescaler.  (MI_NDMA_INTERVAL_PS_n (n=1,4,16,64))

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SetNDmaInterval( u32 ndmaNo, u32 intervalTime, u32 prescaler );

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaIntervalTimer

  Description:  get interval time which is set

  Arguments:    ndmaNo       : NDMA number. (0-3)

  Returns:      interval time (0-0xffff)
 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaIntervalTimer( u32 ndmaNo );

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaIntervalPrescaler

  Description:  get prescaler setting which is set

  Arguments:    ndmaNo       : NDMA number. (0-3)

  Returns:      prescaler
                       MI_NDMA_INTERVAL_PS_n (n=1,4,16,64)
 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaIntervalPrescaler( u32 ndmaNo );

/*---------------------------------------------------------------------------*
  Name:         MI_SetNDmaBlockWord

  Description:  set block word which is transferred by DMA at a time

  Arguments:    ndmaNo       : NDMA number. (0-3)
                word         ; block word

                                 MI_NDMA_WORD_n (n=1,2,4,8,...,32768)

  Returns:      None

 *---------------------------------------------------------------------------*/
void MI_SetNDmaBlockWord( u32 ndmaNo, u32 word );

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaBlockWord

  Description:  get block word which is transferrd by DMA at a time

  Arguments:    ndmaNo

  Returns:      value which is set

                       MI_NDMA_RCYCLE_n
                            (n = 0, 1, 2, 4, 8, ..., 8192, 16384)

 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaBlockWord( u32 ndmaNo );

/*---------------------------------------------------------------------------*
  Name:         MI_SetNDmaWordCount

  Description:  set word count for each DMA request to start

  Arguments:    ndmaNo       : NDMA number. (0-3)
                wordCount    : word count for each DMA request to start

                                 0 - 0xffff
                                 (0 means 0x100000000)

  Returns:      None

 *---------------------------------------------------------------------------*/
void MI_SetNDmaWordCount( u32 ndmaNo, u32 wordCount );

/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaWordCount

  Description:  get word count for each DMA request to start

  Arguments:    ndmaNo

  Returns:      value which is set

                                 0 - 0xffff
                                 (0 means 0x100000000)

 *---------------------------------------------------------------------------*/
u32 MI_GetNDmaWordCount( u32 ndmaNo );

/*---------------------------------------------------------------------------*
  Name:         MI_InitNDmaConfig

  Description:  initialize NDMA config.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_InitNDmaConfig( void );


/*---------------------------------------------------------------------------*
  Name:         MI_GetNDmaConfig

  Description:  copy NDMA config data to local variable

  Arguments:    ndmaNo : NDMA channel No.
                config : pointer to NDMA config struct

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_GetNDmaConfig( u32 ndmaNo, MINDmaConfig *config );

/*---------------------------------------------------------------------------*
  Name:         MI_SetNDmaConfig

  Description:  copy NDMA config data from local variable

  Arguments:    ndmaNo : NDMA channel No.
                config : pointer to NDMA config struct

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SetNDmaConfig( u32 ndmaNo, const MINDmaConfig *config );


//================================================================================
//        Timer DMA
//================================================================================
void    MI_TimerNDmaCopy(u32 ndmaNo, u32 timerNo, const void *src, void *dest, u32 size);

//================================================================================
//        HBlank DMA
//================================================================================
#ifdef SDK_ARM9
void    MI_HBlankNDmaCopy(u32 ndmaNo, const void *src, void *dest, u32 size);
void    MI_HBlankNDmaCopyIf(u32 ndmaNo, const void *src, void *dest, u32 size);
#endif

//================================================================================
//        VBlank DMA
//================================================================================
void    MI_VBlankNDmaCopy(u32 dmaNo, const void *src, void *dest, u32 size);
void    MI_VBlankNDmaCopyAsync(u32 dmaNo, const void *src, void *dest, u32 size,
							  MINDmaCallback callback, void *arg);
//================================================================================
//        CARD DMA
//================================================================================
void    MI_Card_NDmaCopy(u32 dmaNo, const void *src, void *dest, u32 size);
#define MI_Card_A_NDmaCopy MI_Card_NDmaCopy  

//================================================================================
//        main memory display DMA
//================================================================================
//void    MI_DispMemDmaCopy(u32 dmaNo, const void *src);

//================================================================================
//        geometry FIFO DMA
//================================================================================
#ifdef SDK_ARM9
void    MI_SendNDmaGXCommand(u32 dmaNo, const void *src, u32 commandLength);
void    MI_SendNDmaGXCommandAsync(u32 dmaNo, const void *src, u32 commandLength, MINDmaCallback callback, void *arg);
void    MI_SendNDmaGXCommandFast(u32 ndmaNo, const void *src, u32 commandLength);
void    MI_SendNDmaGXCommandAsyncFast(u32 ndmaNo, const void *src, u32 commandLength, MINDmaCallback callback, void *arg);
#endif

//================================================================================
//        Camera DMA
//================================================================================
#ifdef SDK_ARM9
void    MI_Camera_NDmaRecv(u32 ndmaNo, void *dest, u32 unit, u32 size, BOOL contSw );
void    MI_Camera_NDmaRecvEx(u32 ndmaNo, void *dest, u32 size, BOOL contSw, const MINDmaConfig *config );
void    MI_Camera_NDmaRecvAsync(u32 ndmaNo, void *dest, u32 unit, u32 size, BOOL contSw, MINDmaCallback callback, void* arg );
void    MI_Camera_NDmaRecvAsyncEx(u32 ndmaNo, void *dest, u32 size, BOOL contSw, MINDmaCallback callback, void* arg, const MINDmaConfig *config );
#endif

//================================================================================
//        AES DMA
//================================================================================
#ifndef SDK_ARM9
void    MIi_Aes_NDmaSend(u32 ndmaNo, const void *src, u32 size, MINDmaCallback callback, void* arg, const MINDmaConfig* pConfig);
void    MIi_Aes_NDmaRecv(u32 ndmaNo,       void *dst, u32 size, MINDmaCallback callback, void* arg, const MINDmaConfig* pConfig);
#endif

//================================================================================
//		 setting each register directly (internal)
//================================================================================
void MIi_SetNDmaSrc( u32 ndmaNo, void *src );
void MIi_SetNDmaDest( u32 ndmaNo, void *dest );
void MIi_SetNDmaTotalWordCount( u32 ndmaNo, u32 size );
void MIi_SetNDmaWordCount( u32 ndmaNo, u32 size );
void MIi_SetNDmaInterval( u32 ndmaNo, u32 intervalTimer, u32 prescaler );

//--------------------------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_MI_DMA_H_ */
#endif
