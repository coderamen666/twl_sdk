/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     common.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#if !defined(NITRO_MB_DEMO_MULTIBOOT_DATA_H_)
#define NITRO_MB_DEMO_MULTIBOOT_DATA_H_


#include <nitro.h>
#include <nitro/mb.h>


/*
 * Common definitions for the entire multiboot demo.
 */


/******************************************************************************/
/* Constants */

/*
 * Game group ID
 *
 * You can now set a GGID for individual files with the MBGameRegistry structure, so you may not need to specify values with the MB_Init function in the future.
 * 
 * 
 */
#define SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))
#define MY_GGID         SDK_MAKEGGID_SYSTEM(0x20)

/*
 * Channel for the parent device to distribute
 *
 * The multiboot child cycles through all possible channels, so use one of the values permitted by the WM library (currently 1, 7, and 13).
 * 
 *
 * Make this variable to avoid communication congestion in user applications.
 * Applications will decide when to change channels, but it is possible that the user will be given the opportunity to press a Restart button, for example, to deal with poor responsiveness.
 * 
 * 
 */
#define PARENT_CHANNEL  13

/* The maximum number of child device lists to display */
#define DISP_LIST_NUM   (15)

/* DMA number to allocate to the MB library */
#define MB_DMA_NO       2

/* The total number of games that this demo registers */
#define GAME_PROG_MAX   2

/* The multiboot game program information that this demo provides */
extern const MBGameRegistry mbGameList[GAME_PROG_MAX];

/* The game sequence state value of this demo */
enum
{
    STATE_NONE,
    STATE_MENU,
    STATE_MB_PARENTINIT,
    STATE_MB_PARENT
};


/******************************************************************************/
/* Variables */

/* The work region to be allocated to the MB library */
extern u32 cwork[MB_SYSTEM_BUF_SIZE / sizeof(u32)];

/* The game sequence state of this demo */
extern u8 prog_state;


/******************************************************************************/
/* Functions */

/* Key trigger detection */
u16     ReadKeySetTrigger(u16 keyset);

/* Rotate the val value the offset amount in within the min - max range */
BOOL    RotateU8(u8 *val, u8 min, u8 max, s8 offset);

/* Parent device initialization */
void    ParentInit(void);

/* Parent device main process in each single frame */
void    ParentMain(void);

/* Initialize the file buffer pointer */
void    InitFileBuffer(void);

/* Set parent device MP send size */
void    SetParentSendSize(u16 p_size);

/* Get parent device MP send size */
u16     GetParentSendSize(void);

/* Set maximum number of child device connections */
void    SetMaxEntry(u8 num);

/* Get the maximum number of child device connections */
u8      GetMaxEntry(void);

/* Configure parent device auto operations */
void    SetAutoOperation(u8 fAccept, u8 fSend, u8 fBoot);

#endif /* !defined(NITRO_MB_DEMO_MULTIBOOT_DATA_H_) */
