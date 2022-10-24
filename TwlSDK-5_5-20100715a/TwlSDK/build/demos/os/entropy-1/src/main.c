/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - entropy-1
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include "DEMO.h"
#include "wmscan.h"


/*---------------------------------------------------------------------------*/
/* Variables */

static u8 randomSeed[20];
static u8 micData[1024] ATTRIBUTE_ALIGN(32);
static MICAutoParam micAutoParam;
static volatile BOOL bufferFullFlag;
static u8 wmBuffer[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* Functions */

static void    ChurnRandomSeed(void);
static u32     GetRandomNumber(void);
static void    MicFullCallback(MICResult result, void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();

    // Increase the randomness of the data returned by OS_GetLowEntropyData function by starting Tick
    OS_InitTick();
    OS_InitAlarm();

    // Touch panel initialization
    {
        TPCalibrateParam calibrate;
        TP_Init();
        // Get CalibrateParameter from FlashMemory
        if (!TP_GetUserInfo(&calibrate))
        {
            OS_TPanic("FATAL ERROR: can't read UserOwnerInfo\n");
        }
        TP_SetCalibrateParam(&calibrate);
    }

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    // Initialize mic and start auto-sampling
    {
        MICResult result = MIC_RESULT_ILLEGAL_STATUS;
        MIC_Init();
        (void)PM_SetAmp(PM_AMP_ON);

        micAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
        micAutoParam.buffer = (void *)micData;
        micAutoParam.size = sizeof(micData);
        micAutoParam.rate = MIC_SAMPLING_RATE_8K;
        micAutoParam.loop_enable = TRUE;
        micAutoParam.full_callback = MicFullCallback;

        bufferFullFlag = FALSE;
        if (!OS_IsRunOnTwl())
        {
            result = MIC_StartAutoSampling(&micAutoParam);
        }
#ifdef SDK_TWL
        else
        {
            SNDEXFrequency  freq;
            SNDEX_Init();
            if (SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            {
                OS_TWarning("failed to check I2C-master frequency.\n");
                result = MIC_RESULT_ILLEGAL_STATUS;
            }
            else
            {
                micAutoParam.rate = (freq == SNDEX_FREQUENCY_32730) ? MIC_SAMPLING_RATE_8180 : MIC_SAMPLING_RATE_11900;
                result = MIC_StartLimitedSampling(&micAutoParam);
            }
        }
#endif // SDK_TWL
        if (result != MIC_RESULT_SUCCESS)
        {
            bufferFullFlag = TRUE;
            OS_TPanic("MIC_StartAutoSampling failed. result = %d\n", result);
        }
    }

    // Initialize wireless communication.
    // Because the OS_GetLowEntropyData function also returns data based on the reception strength history
    // this demo starts wireless communication and searches for a parent.
    if (!WS_Initialize(wmBuffer, 3))
    {
        OS_TPanic("WS_Initialize failed.\n");
    }
    WS_TurnOnPictoCatch();
    {
        const u8 mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

        // Wait for initialization to complete
        while (WS_GetSystemState() != WS_SYSSTATE_IDLE)
        {
        }
        // Start search for parent
        if (!WS_StartScan(NULL, mac, TRUE))
        {
            OS_TPanic("WS_StartScan failed.\n");
        }
    }

    // Initialize random number seed using mic sampling result
    while (bufferFullFlag == FALSE)
    {
        // Wait until mic sampling goes through the buffer
        ;
    }
    MATH_CalcSHA1(randomSeed, micData, sizeof(micData));

    // Use the OS_GetLowEntropyData function to randomize the random number seed
    ChurnRandomSeed();

    // Main loop
    while (1)
    {
        int     i;
        int     j;
        int     y;
        TPData  raw_point;
        u32     data[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];

        DEMO_DrawFlip();
        // Wait for V-Blank interrupt
        OS_WaitVBlankIntr();

        (void)TP_RequestRawSampling(&raw_point);

        // Display result of OS_GetLowEntropyData on screen every 2ms
        y = 0;
        for (j = 0; j < 6; j++)
        {
            if (j != 0)
            {
                OS_Sleep(2);
            }
            OS_GetLowEntropyData(data);
            for (i = 0; i < OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32); i++)
            {
                char    tmp[32 + 1];
                int     x;
                for (x = 0; x < 32; ++x)
                {
                    tmp[x] = (char)('0' + ((data[i] >> (31 - x)) & 1));
                }
                tmp[x] = '\0';
                DEMOPutString(0, y, "%s", tmp);
                y += 1;
            }
        }

        if ((OS_GetVBlankCount() % 16) == 0)
        {
            // Use the OS_GetLowEntropyData function every 16 frames to randomize the random number seed
            // 
            ChurnRandomSeed();
        }

        // Display the random number
        OS_TPrintf("%08x\n", GetRandomNumber());
    }
}

//--------------------------------------------------------------------------------
// Example of changing the random number seed based on the OS_GetLowEntropyData function
//
void ChurnRandomSeed(void)
{
    u32     data[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];
    MATHSHA1Context context;

    OS_GetLowEntropyData(data);
    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, randomSeed, sizeof(randomSeed));
    MATH_SHA1Update(&context, data, sizeof(data));
    MATH_SHA1GetHash(&context, randomSeed);
}

//--------------------------------------------------------------------------------
// Example of getting random number from random number seed (Not thread-safe)
//
u32 GetRandomNumber(void)
{
    static u32 count = 0;
    u32     randomData[20 / sizeof(u32)];
    MATHSHA1Context context;

    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, randomSeed, sizeof(randomSeed));
    MATH_SHA1Update(&context, &count, sizeof(count));
    MATH_SHA1GetHash(&context, randomData);
    count++;

    // Although you can use the entire randomData as a random number,
    // you can also remove any part of randomData and use it as a random number.
    return randomData[0];
}

//--------------------------------------------------------------------------------
//    Auto-sampling buffer callback
//
void MicFullCallback(MICResult result, void *arg)
{
#pragma unused(result)
#pragma unused(arg)
    bufferFullFlag = TRUE;
}

/*====== End of main.c ======*/
