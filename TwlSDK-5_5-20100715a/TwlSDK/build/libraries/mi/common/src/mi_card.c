#if 0
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_card.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/


#include <nitro/os.h>
#include <nitro/mi/card.h>


/********************************************************************/
/* Variables */

// Variables for card exclusive processing
typedef struct
{
    BOOL    is_init;                   // Initialized
    u16     lock_id;                   //Lock ID
    u16     padding;
    MIDmaCallback dma_callback;
}
MIi_CardParam;

static MIi_CardParam mii_card_param;


/********************************************************************/
/* Functions */

// Initialize card access via MI
static void MIi_InitCard(void)
{
    MIi_CardParam *const p = &mii_card_param;
    OSIntrMode bak_psr = OS_DisableInterrupts();

    if (!p->is_init)
    {
        s32     lock_id = OS_GetLockID();

#ifndef	SDK_FINALROM
        if (lock_id < 0)
        {
            OS_TPanic("Invalid lock ID.");
        }
#endif
        p->is_init = TRUE;
        p->lock_id = (u16)lock_id;
    }
    (void)OS_RestoreInterrupts(bak_psr);
}

// Allocate device using exclusive control of processor
void MIi_LockCard(void)
{
    // Initialize here if necessary, if that is convenient
    MIi_InitCard();
    CARD_LockRom(mii_card_param.lock_id);
}

// Release the device using exclusive control of processor
void MIi_UnlockCard(void)
{
    SDK_ASSERT(mii_card_param.is_init);
    CARD_UnlockRom(mii_card_param.lock_id);
}

// Unlock the MI itself before user callback
static void MIi_OnAsyncEnd(void *arg)
{
    MIi_UnlockCard();
    {
        MIDmaCallback func = mii_card_param.dma_callback;
        mii_card_param.dma_callback = NULL;
        if (func)
            (*func) (arg);
    }
}

// Load the card (asynchronous)
void MIi_ReadCardAsync(u32 dmaNo, const void *src, void *dst, u32 size,
                       MIDmaCallback callback, void *arg)
{
    MIi_LockCard();
    mii_card_param.dma_callback = callback;
    (void)CARD_ReadRomAsync(dmaNo, src, dst, size, MIi_OnAsyncEnd, arg);
}

#endif //#if 0
