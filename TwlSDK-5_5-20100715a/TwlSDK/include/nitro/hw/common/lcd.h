/*---------------------------------------------------------------------------*
  Project:  TwlSDK - HW - include
  File:     lcd.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_HW_COMMON_LCD_H_
#define NITRO_HW_COMMON_LCD_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
// Number of horizontal dots
#define HW_LCD_WIDTH                256 // The number of dots of the display screen width
#define HW_LCD_HBLANK               99 // The number of dots in the H-blank interval
#define HW_LCD_COLUMNS              ( HW_LCD_WIDTH + HW_LCD_HBLANK )

// The number of vertical lines
#define HW_LCD_HEIGHT               192 // The number of lines in the display screen height
#define HW_LCD_VBLANK               71 // The number of lines in the V-blank interval
#define HW_LCD_LINES                ( HW_LCD_HEIGHT + HW_LCD_VBLANK )

// LCD clock
#define HW_LCD_CLOCK                33513982    // 33.513982 MHz
#define HW_LCD_CLOCK_PER_DOT        6  // 6 frequency divisions for LCD clock

// H cycle ( 63.5556us )
#define HW_LCD_H_CYCLE_NS           ((u32)( 1000000000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_H_CYCLE_US           ((u32)(    1000000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_H_CYCLE_MS           ((u32)(       1000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK )

// V cycle ( 16.7151ms )
#define HW_LCD_V_CYCLE_NS           ((u32)( 1000000000ULL * HW_LCD_LINES * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_V_CYCLE_US           ((u32)(    1000000ULL * HW_LCD_LINES * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_V_CYCLE_MS           ((u32)(       1000ULL * HW_LCD_LINES * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))

// The time required to scan n lines
#define HW_LCD_LINES_CYCLE_NS(n)    ((u32)( 1000000000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_LINES_CYCLE_US(n)    ((u32)(    1000000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))
#define HW_LCD_LINES_CYCLE_MS(n)    ((u32)(       1000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK ))


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_HW_COMMON_LCD_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
