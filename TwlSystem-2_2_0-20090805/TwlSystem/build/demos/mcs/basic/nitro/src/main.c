/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - nitro
  File:     main.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1316 $
 *---------------------------------------------------------------------------*/

// ============================================================================
//  Explanation of the demo:
//      The MCS library is used to read and write data with the PC.
//      First, a picture is displayed to the BG screen.
//      When the PC program basic.exe is run, the picture moves horizontally.
//      The next time basic.exe is run, the picture moves diagonally.
//
//      Pressing the A Button quits the program.
//
// ============================================================================

#include "sdk_init.h"
#include "nns_util.h"

#include <nnsys/mcs.h>
#include <nnsys/misc.h>

// The DMA number specified for the FS_Init function
#define DEFAULT_DMA_NUMBER      MI_DMA_MAX_NUM

// The channel values used to distinguish from Windows applications
static const u16 MCS_CHANNEL0 = 0;
static const u16 MCS_CHANNEL1 = 1;

// The structure for positional information
typedef struct Vec2 Vec2;
struct Vec2
{
    s32     x;
    s32     y;
};

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
VBlankIntr()
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);

    NNS_McsVBlankInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         CartIntrFunc

  Description:  Game Pak interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
CartIntrFunc()
{
    OS_SetIrqCheckFlag(OS_IE_CARTRIDGE);

    NNS_McsCartridgeInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         InitInterrupt

  Description:  Performs the initialization related to interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitInterrupt()
{
    (void)OS_DisableIrq();

    // Enable V-Blank interrupts and set so NNS_McsVBlankInterrupt() is called from inside the V-Blank interrupt
    // 
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)GX_VBlankIntr(TRUE);

	// Enable Game Pak interrupts and configure NNS_McsCartridgeInterrupt() to be called from within a Game Pak interrupt
    // 
    OS_SetIrqFunction(OS_IE_CARTRIDGE, CartIntrFunc);
	(void)OS_EnableIrqMask(OS_IE_CARTRIDGE);

    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         DataRecvCallback

  Description:  The callback function that gets called when data is received from the PC.

                Do not receive data inside the callback function to register.
                Also, do not use an interrupt wait loop because interrupts may be prohibited.
                

  Arguments:    recv: Pointer to the buffer where some or all of the received data is stored
                            
                recvSize: Size of the data stored in the buffer indicated by recv
                            
                userData: Value specified the userData argument from the NNS_McsRegisterRecvCallback function
                            
                offset: 0 when all of the received data has been stored in the buffer indicated by recv.
                            When only a portion of the received data has been stored, the offset position, where 0 is the standard for when all data is received.
                            
                            
                totalSize: Total size of the received data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
DataRecvCallback(
    const void* pRecv,
    u32         recvSize,
    u32         userData,
    u32         offset,
    u32         /* totalSize */
)
{
    Vec2 *const pPos = (Vec2*)userData;

	// Check the receive buffer
	if (pRecv != NULL && recvSize == sizeof(pPos->y) && offset == 0)
	{
        // Update the y position with the received value
        const s32* pRecvPosY = pRecv;
        pPos->y = *pRecvPosY;
    }
}

/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  A sample program that reads BG data from an archive and displays it on the screen.
                

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
MainLoop(Vec2* pPos)
{
    while (TRUE)
    {
        u32 nLength;
        s32 posX;

        NNS_McsPollingIdle();

        // Get the size of data that can be received
		nLength = NNS_McsGetStreamReadableSize(MCS_CHANNEL0);
        if (nLength > 0 && sizeof(posX) == nLength)
        {
            u32 readSize;

            // Read the data
            if ( NNS_McsReadStream(MCS_CHANNEL0, &posX, sizeof(posX), &readSize)
              && readSize == sizeof(posX)
            )
            {
                pPos->x = posX;
            }
        }

        // If outside a certain range, convey to PC as inverse of incremental value
        if (pPos->x < 0 || 64 < pPos->x)
        {
            s32 writeData = pPos->x < 0 ? 1: -1;
            // Write the data
            if (! NNS_McsWriteStream(MCS_CHANNEL0, &writeData, sizeof(writeData)))
            {
                break;
            }
        }

        // If outside a certain range, convey to PC as inverse of incremental value
        if (pPos->y < 0 || 64 < pPos->y)
        {
            s32 writeData = pPos->y < 0 ? 1: -1;
            // Write the data
            if (! NNS_McsWriteStream(MCS_CHANNEL1, &writeData, sizeof(writeData)))
            {
                break;
            }
        }

        SVC_WaitVBlankIntr();

        G2_SetBG0Offset(pPos->x, pPos->y);

        ReadGamePad();
        if (IS_TRIGGER(PAD_BUTTON_A))
        {
            break;
        }
    }
}

/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  A sample program that reads BG data from an archive and displays it on the screen.
                

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    u8* mcsWorkMem;
    NNSMcsDeviceCaps deviceCaps;
    NNSMcsRecvCBInfo recvCBInfo;
    Vec2* pPos;

    InitSystem();

    InitInterrupt();                // Interrupt-related initialization

    FS_Init(DEFAULT_DMA_NUMBER);    // Initialize the file system
    
    InitMemory();                   // Creation of heap
    
    InitVRAM();

    InitDisplay();
    LoadPicture();

    pPos = NNS_FndAllocFromExpHeapEx(gAppHeap, sizeof(Vec2), 4);  // Allocate memory for data

    // Initialize data
    pPos->x = 0;
    pPos->y = 0;

    G2_SetBG0Offset(pPos->x, pPos->y);

    // Initialize MCS
    mcsWorkMem = NNS_FndAllocFromExpHeapEx(gAppHeap, NNS_MCS_WORKMEM_SIZE, 4); // Allocate the MCS work memory
    NNS_McsInit(mcsWorkMem);

    // Open the device
    if (NNS_McsGetMaxCaps() > 0 && NNS_McsOpen(&deviceCaps))
    {
        void* printBuffer = NNS_FndAllocFromExpHeap(gAppHeap, 1024);        // Allocate buffer for printing
        void* recvBuf = NNS_FndAllocFromExpHeapEx(gAppHeap, 1024, 4);       // Allocate buffer for receiving data

        NNS_NULL_ASSERT(printBuffer);
        NNS_NULL_ASSERT(recvBuf);
        NNS_NULL_ASSERT(pPos);

        // Output with OS_Printf
        OS_Printf("device open\n");

        // For TWL: At present, the MCS server delays the transition to connection status for programs running on the TWL.
        //        
        //        Therefore, wait until connected status is reached.
        if (! NNS_McsIsServerConnect())
        {
            OS_Printf("Waiting server connect.\n");
            do
            {
                SVC_WaitVBlankIntr();
            }while (! NNS_McsIsServerConnect());
        }

        // Initialize MCS string output
        NNS_McsInitPrint(printBuffer, NNS_FndGetSizeForMBlockExpHeap(printBuffer));

        // Output with NNS_McsPrintf
        // If the MCS server is connected at this time, display to the console.
        (void)NNS_McsPrintf("device ID %08X\n", deviceCaps.deviceID);

        // Register buffer for reading of data
        NNS_McsRegisterStreamRecvBuffer(MCS_CHANNEL0, recvBuf, NNS_FndGetSizeForMBlockExpHeap(recvBuf));

        // Register the receive callback function
        NNS_McsRegisterRecvCallback(&recvCBInfo, MCS_CHANNEL1, DataRecvCallback, (u32)pPos);

        MainLoop(pPos);

        OS_Printf("device close\n");

        NNS_McsUnregisterRecvResource(MCS_CHANNEL1);
        NNS_McsUnregisterRecvResource(MCS_CHANNEL0);
        NNS_McsFinalizePrint();

        NNS_FndFreeToExpHeap(gAppHeap, recvBuf);        // Free the receive buffer
        NNS_FndFreeToExpHeap(gAppHeap, printBuffer);    // Free the buffer used for printing

        // Close the device
        (void)NNS_McsClose();

        NNS_McsFinalize();                              // MCS end processing
        NNS_FndFreeToExpHeap(gAppHeap, mcsWorkMem);     // Free the MCS work memory
    }
    else
    {
        OS_Printf("device open fail.\n");
        MainLoop(pPos);
    }

    NNS_FndFreeToExpHeap(gAppHeap, pPos);           // Free the memory used for data

    while (TRUE) {}
}

