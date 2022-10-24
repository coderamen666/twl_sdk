/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wmPadRead-child
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11030 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

//
// This is the child program distributed by the mpdldemo/mpdlntr2rvl demo included with RevoEX, a development kit for the Wii's wireless features.
// 
// It communicates with the Wii and sends button input.
//

/*
 * This demo uses the WH library for wireless communications, but does not perform wireless shutdown processing.
 * 
 * For details on WH library wireless shutdown processing, see the comments at the top of the WH library source code or the wm/dataShare-Model demo.
 * 
 * 
 */

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wm.h>
#include "data.h"
#include "wh.h"

#include "tpdata.h"

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
#define PICTURE_FRAME_PER_GAME_FRAME    1

/* The GGID used in this demo */
#define MY_GGID SDK_MAKEGGID_SYSTEM(0x20)

#define DMA_NO  3

#define SAMPLING_BUFFER_SIZE ( 1024 * 1024 ) // 1M

#define FFT_NSHIFT      9
#define FFT_N           (1 << FFT_NSHIFT)

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

static WMBssDesc mbParentBssDesc ATTRIBUTE_ALIGN(32);
static BOOL isMultiBooted;

static GXOamAttr oamBak[128];

typedef struct MyPadData {
    u16 keyData;
    u8 touchPanel_x;
    u8 touchPanel_y;
    u8 mic;
    u8 touch;
    u8 padding[2];
} MyPadData;

// Send/receive buffer for display
static MyPadData gRecvData[1 + WH_CHILD_MAX]; // Touch information for all players, delivered by the parent
static BOOL gRecvFlag[1 + WH_CHILD_MAX];

u16     keyData;

static GXOamAttr gOam[128];

TPData  raw_point;
TPData  disp_point;
TPCalibrateParam calibrate;

static BOOL input_mic;
static MICAutoParam gMicAutoParam;
static u8 *gMicData;


// FFT buffers
static fx16 sinTable[FFT_N - FFT_N / 4];
#ifdef USE_FFTREAL
static fx16 sinTable2[(FFT_N - FFT_N / 4) / 2];
static fx32 data[FFT_N];
#else
static fx32 data[FFT_N * 2];
#endif
static s32 power[FFT_N/2+1];
static s32 smoothedPower[FFT_N/2+1];

/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void);
static void SetPoint16x16(int objNo, u16 pos_x, u16 pos_y, int charNo, int paletteNo);
static void Initialize(void);
static BOOL DoConnect(void);
static void DoPadSharing(void);
static BOOL CheckMicData(void *address);
static void ObjSet(int objNo, int x, int y, int charNo, int paletteNo);
static void VBlankIntr(void);


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain()
#else
void NitroMain()
#endif
{
    Initialize();

    // Initialize shared data
    MI_CpuClear8( gRecvData, sizeof(gRecvData) );

    // The local host checks to see if it is a child started from multiboot.
    isMultiBooted = MB_IsMultiBootChild();

    if (isMultiBooted)
    {
        //--------------------------------------------------------------
        // If it was started from multi-boot, it will be reset once, and communication will be disconnected.
        // The child maintains the BssDesc of the parent that booted it. Use this information to reconnect to the parent.
        // 
        // There is no particular problem with extracting the MAC address from BssDesc, then specifying that MAC address and scanning for and connecting to that parent. However, to connect to the parent directly using the stored BssDesc, it is necessary to set the parent's and child's communications size and transfer mode to match ahead of time.
        // 
        // 
        // 
        //--------------------------------------------------------------

        /* 
         * Gets parent data for reconnecting to the parent.                   
         * The WMBssDesc used for the connection must be 32-byte aligned.
         * When reconnecting without using the parent's MAC address to rescan, make the KS/CS flags and the maximum send size match in advance for the parent and the child.
         * 
         * All of these values may be 0 if you rescan before connecting.
         */
        MB_ReadMultiBootParentBssDesc(&mbParentBssDesc,
                                      WH_PARENT_MAX_SIZE, // Maximum parent send size
                                      WH_CHILD_MAX_SIZE,  // Maximum child send size
                                      0,   // Reserved region (always 0)
                                      0);  // Reserved region (always 0)
    }

    while (TRUE)
    {
        int connectedFlag = FALSE;

        // Initialize wireless
        if (!WH_Initialize())
        {
            OS_Panic("WH_Initialize failed.");
        }

        connectedFlag = DoConnect();
        // At this point the transition to the child state is complete

        if (connectedFlag)
        {
            DoPadSharing();
            // Control does not return here until exiting from communication mode
        }

        WH_Finalize();
    }
}

void Initialize(void)
{
    //================ Initialization
    //---- OS Initialization
    OS_Init();

    //---- TP Initialization
    TP_Init();

    // Get CalibrateParameter from FlashMemory
    if (!TP_GetUserInfo(&calibrate))
    {
        OS_Panic("FATAL ERROR: can't read UserOwnerInfo\n");
    }
    else
    {
        OS_Printf("Get Calibration Parameter from NVRAM\n");
    }

    TP_SetCalibrateParam(&calibrate);

    //---- GX Initialization
    GX_Init();

    //================ Initial settings
    //---- All Power ON
    GX_SetPower(GX_POWER_ALL);

    //----  Enable V-Blank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- V-Blank occurrence settings
    (void)GX_VBlankIntr(TRUE);

    //---- Clear VRAM
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- OAM and palette clear
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- Set bank A for OBJ
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);

    GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);

    //---- Set to graphics display mode
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    //---- Set only OBJ display ON
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);
    
    //---- Used with 32-KB OBJ in 2D mapping mode
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    //---- Data load
    GX_LoadOBJ( sampleCharData, 0, sizeof(sampleCharData) );
    GX_LoadOBJPltt( samplePlttData, 0, sizeof(samplePlttData) );

    GXS_SetGraphicsMode(GX_BGMODE_0);    // BGMODE 0

    GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJ visible
    GXS_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    /* Load character bitmap data */
    GXS_LoadOBJ( sampleCharData, 0, sizeof(sampleCharData) );
    GXS_LoadOBJPltt( samplePlttData, 0, sizeof(samplePlttData) );

    //---- Move hidden OBJ off screen
    MI_DmaFill32(DMA_NO, oamBak, 0xc0, sizeof(oamBak));

    //================ Miscellaneous initialization

    /////////////////////
    // Memory allocation
    InitializeAllocateSystem();

    /////////////////////
    // Microphone-related initialization

    //---- MIC Initialization
    MIC_Init();

    //---- PMIC Initialization
    PM_Init();
    // AMP on
    (void)PM_SetAmp(PM_AMP_ON);
    // Adjust AMP gain
    (void)PM_SetAmpGain(PM_AMPGAIN_80);

    // Auto sampling settings
    // Because the memory allocated with OS_Alloc is 32-byte aligned, other memory is not destroyed even if the cache is manipulated
    // 
    gMicData = (u8 *)OS_Alloc(SAMPLING_BUFFER_SIZE);
    // Perform 12-bit sampling to increase the accuracy of Fourier transforms
    gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
    gMicAutoParam.buffer = (void *)gMicData;
    gMicAutoParam.size = SAMPLING_BUFFER_SIZE;
    gMicAutoParam.loop_enable = TRUE;
    gMicAutoParam.full_callback = NULL;
#ifdef SDK_TWL
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8180;
    (void)MIC_StartLimitedSampling(&gMicAutoParam);
#else   // ifdef SDK_TWL
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
    (void)MIC_StartAutoSampling(&gMicAutoParam);
#endif  // ifdef SDK_TWL else

    /////////////////////
    // FFT-related

    MATH_MakeFFTSinTable(sinTable, FFT_NSHIFT);
#ifdef USE_FFTREAL
    MATH_MakeFFTSinTable(sinTable2, FFT_NSHIFT - 1);
#endif

    //Start display
    OS_WaitVBlankIntr();
    GX_DispOn();

    GXS_DispOn();
}

BOOL DoConnect(void)
{
    s32 retry = 0;
    enum
    {
        MAX_RETRY = 5
    };

    ObjSet(0, 100, 80, 'C', 4);
    ObjSet(1, 110, 80, 'O', 4);
    ObjSet(2, 120, 80, 'N', 4);
    ObjSet(3, 130, 80, 'N', 4);
    ObjSet(4, 140, 80, 'E', 4);
    ObjSet(5, 150, 80, 'C', 4);
    ObjSet(6, 160, 80, 'T', 4);
    MI_CpuClear8( (GXOamAttr *)&oamBak[7], sizeof(GXOamAttr) * 12);

    //---- Wait for V-Blank interrupt completion
    OS_WaitVBlankIntr();

    // If this is multiboot, use the GGID declared by the parent device.
    //   Normally, this should match MY_GGID.
    WH_SetGgid(isMultiBooted ? mbParentBssDesc.gameInfo.ggid : MY_GGID);
    retry = 0;

    while (TRUE)
    {
        // Distributes processes based on communication status
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_CONNECT_FAIL:
            {
                // Because the internal WM state is invalid if the WM_StartConnect function fails, WM_Reset must be called once to reset the state to IDLE
                // 
                WH_Reset();
            }
            break;
        case WH_SYSSTATE_IDLE:
            {
                if (retry < MAX_RETRY)
                {
                    // Scan for a parent device and connect.
                    //   When the local device was multibooted, use the parent device BSSID it remembers and connect only to the parent device that was the multiboot source.
                    //   
                    (void)WH_ChildConnectAuto(WH_CONNECTMODE_DS_CHILD, isMultiBooted ? mbParentBssDesc.bssid : NULL, 0);
                    retry++;
                    break;
                }
                // If connection to parent is not possible after MAX_RETRY, display ERROR
            }
        case WH_SYSSTATE_ERROR:
            return FALSE;

        case WH_SYSSTATE_DATASHARING:
            return TRUE;

        case WH_SYSSTATE_BUSY:
        case WH_SYSSTATE_SCANNING:
        case WH_SYSSTATE_CONNECTED:
        default:
            break;
        }

        OS_WaitVBlankIntr();
    }

    // Can't reach here
    return FALSE;
}

void DoPadSharing(void)
{
    int myAid;
    myAid = WM_GetAID();

    //================ Main loop
    while (TRUE)
    {
        // Draw Marker by calibrated point
        while (TP_RequestRawSampling(&raw_point) != 0)
        {
        };
        TP_GetCalibratedPoint(&disp_point, &raw_point);

        if (disp_point.touch)
        {
            SetPoint16x16(0, disp_point.x, disp_point.y, myAid + '0', 1);

            switch (disp_point.validity)
            {
            case TP_VALIDITY_VALID:
                OS_TPrintf("( %d, %d ) -> ( %d, %d )\n", raw_point.x, raw_point.y, disp_point.x,
                           disp_point.y);
                break;
            case TP_VALIDITY_INVALID_X:
                OS_TPrintf("( *%d, %d ) -> ( *%d, %d )\n", raw_point.x, raw_point.y, disp_point.x,
                           disp_point.y);
                break;
            case TP_VALIDITY_INVALID_Y:
                OS_TPrintf("( %d, *%d ) -> ( %d, *%d )\n", raw_point.x, raw_point.y, disp_point.x,
                           disp_point.y);
                break;
            case TP_VALIDITY_INVALID_XY:
                OS_TPrintf("( *%d, *%d ) -> ( *%d, *%d )\n", raw_point.x, raw_point.y, disp_point.x,
                           disp_point.y);
                break;
            }
        }
        //---- Wait for V-Blank interrupt completion
        OS_WaitVBlankIntr();

        //---- Load pad data
        keyData = PAD_Read();

        {
            static MyPadData sendData ATTRIBUTE_ALIGN(32);
            int i;

            // Prepare data to send
            MI_CpuClear8(&sendData, sizeof(sendData));
            sendData.keyData = (u16)(keyData | (PAD_DetectFold() ? PAD_DETECT_FOLD_MASK : 0));
            sendData.touch        = (u8)disp_point.touch;
            sendData.touchPanel_x = (u8)disp_point.x;
            sendData.touchPanel_y = (u8)disp_point.y;
            sendData.mic = (u8)input_mic;

            if ( WH_StepDS((void*)&sendData) )
            {
                for (i = 0; i < (1 + WH_CHILD_MAX); i++)
                {
                    u8* data;

                    data = (u8 *)WH_GetSharedDataAdr((u16)i);

                    if (data != NULL)
                    {
                        gRecvFlag[i] = TRUE;
                        MI_CpuCopy8(data, &gRecvData[i], sizeof(gRecvData[0]));
                    }
                    else
                    {
                        gRecvFlag[i] = FALSE;
                    }
                }
            }
        }

        //---- Display pad information as OBJ
        ObjSet(0, 200, 90, 'A', (keyData & PAD_BUTTON_A) ? 1 : 2);
        ObjSet(1, 180, 95, 'B', (keyData & PAD_BUTTON_B) ? 1 : 2);

        ObjSet(2, 60, 50, 'L', (keyData & PAD_BUTTON_L) ? 1 : 2);
        ObjSet(3, 180, 50, 'R', (keyData & PAD_BUTTON_R) ? 1 : 2);

        ObjSet(4, 60, 80, 'U', (keyData & PAD_KEY_UP) ? 1 : 2);
        ObjSet(5, 60, 100, 'D', (keyData & PAD_KEY_DOWN) ? 1 : 2);
        ObjSet(6, 50, 90, 'L', (keyData & PAD_KEY_LEFT) ? 1 : 2);
        ObjSet(7, 70, 90, 'R', (keyData & PAD_KEY_RIGHT) ? 1 : 2);

        ObjSet(8, 130, 95, 'S', (keyData & PAD_BUTTON_START) ? 1 : 2);
        ObjSet(9, 110, 95, 'S', (keyData & PAD_BUTTON_SELECT) ? 1 : 2);

        ObjSet(10, 200, 75, 'X', (keyData & PAD_BUTTON_X) ? 1 : 2);
        ObjSet(11, 180, 80, 'Y', (keyData & PAD_BUTTON_Y) ? 1 : 2);

        //---- Display Folding Detect Status with OBJ
        ObjSet(12, 120, 30, 'F', (PAD_DetectFold())? 1 : 2);

        ObjSet(13, 100, 5, 'A', 4);
        ObjSet(14, 110, 5, 'I', 4);
        ObjSet(15, 120, 5, 'D', 4);
        ObjSet(16, 130, 5, (myAid / 10) ? (myAid / 10) + '0' : ' ', 4);
        ObjSet(17, 140, 5, (myAid % 10) + '0', 4);

        // Microphone input check
        input_mic = CheckMicData(MIC_GetLastSamplingAddress());
        ObjSet(18, 120, 120, 'M', input_mic ? 1 : 2);

        if( WH_GetSystemState() != WH_SYSSTATE_DATASHARING )
        {
            break;
        }

        // Render all players' touch information from the received information
        {
            int i;

            for(i = 0; i < WH_CHILD_MAX + 1; i++)
            {
                if( i != myAid && gRecvData[i].touch )
                {
                    SetPoint16x16(i+1, gRecvData[i].touchPanel_x, gRecvData[i].touchPanel_y, '0'+i, 2);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
//  Set OBJ
//
void ObjSet(int objNo, int x, int y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}


//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    //---- OAM updating
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(DMA_NO, oamBak, (void *)HW_OAM, sizeof(oamBak));

    /* Flush cache of OAM buffers to main memory */
    DC_FlushRange(gOam, sizeof(gOam));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadOAM(gOam, 0, sizeof(gOam));
    MI_DmaFill32(3, gOam, 192, sizeof(gOam));       // Clear OAM buffer

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // Based on the premise that OS_Init has been already called
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         SetPoint16x16

  Description:  Displays a 16x16 OBJ at indicated point.

  Arguments:    x: Position X
                y: Position Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetPoint16x16(int objNo, u16 pos_x, u16 pos_y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&gOam[objNo],
                  pos_x - 8,
                  pos_y - 8,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CheckMicData

  Arguments:    address: Mic sampling data

  Returns:      Whether mic input exists.
 *---------------------------------------------------------------------------*/
static BOOL CheckMicData(void *address)
{
    s32     i;

    u16    *p;

    // If sampling has never been performed, do nothing and stop.
    // (Because it would delete memory cache(s) unrelated to the microphone)
    if ((address < gMicData) || (address >= (gMicData + SAMPLING_BUFFER_SIZE)))
    {
        return FALSE;
    }

    // Apply a FFT for the FFT_N most recent sampling values
    // With 12-bit sampling, each value is two bytes
    p = (u16 *)((u32)address - (FFT_N-1)*2);
    if ((u32)p < (u32)gMicData)
    {
        p = (u16 *)((u32)p + SAMPLING_BUFFER_SIZE);
    }
    DC_InvalidateRange((void *)((u32)p & 0xffffffe0), 32);
    for (i = 0; i < FFT_N; i++)
    {
#ifdef USE_FFTREAL
        data[i] = ((*p) << (FX32_SHIFT - (16 - 12)));
#else
        data[i * 2] = ((*p) << (FX32_SHIFT - (16 - 12)));
        data[i * 2 + 1] = 0; // Substitute 0 in the imaginary part
#endif
        p++;
        if ((u32)p >= (u32)(gMicData + SAMPLING_BUFFER_SIZE))
        {
            p = (u16 *)((u32)p - SAMPLING_BUFFER_SIZE);
        }
        if (((u32)p % 32) == 0)
        {
            DC_InvalidateRange(p, 32);
        }
    }

#ifdef USE_FFTREAL
    MATH_FFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
#else
    MATH_FFT(data, FFT_NSHIFT, sinTable);
#endif

    // Do not calculate above FFT_N/2, because only conjugated, inverted values of FFT_N/2 and below will be yielded
    for (i = 0; i <= FFT_N/2; i++)
    {
        fx32 real;
        fx32 imm;

#ifdef USE_FFTREAL
        if (0 < i  && i < FFT_N/2)
        {
            real = data[i * 2];
            imm  = data[i * 2 + 1];
        }
        else
        {
            if (i == 0)
            {
                real = data[0];
            }
            else // i == FFT_N/2
            {
                real = data[1];
            }
            imm  = 0;
        }
#else
        real = data[i * 2];
        imm  = data[i * 2 + 1];
#endif

        // Calculate the power of each frequency
        // If only the relative value size is necessary, omitting the sqrt can result in a speed increase 
        power[i] = FX_Whole(FX_Sqrt(FX_MUL(real, real) + FX_MUL(imm, imm)));

        // Record the accumulated time values for the power of each frequency
        // Perform damping a little at a time
        smoothedPower[i] += power[i] - (smoothedPower[i]/8);
    }

    // Microphone input check
    {
        s32 totalPower = 0;

#define FILTER_LOW 12    // ((8000 Hz/2)/(FFT_N/2)) * 12 = 187.5 Hz
#define FILTER_HIGH 64   // ((8000 Hz/2)/(FFT_N/2)) * 64 = 1000.5 Hz
        for (i = FILTER_LOW; i < FILTER_HIGH; i++)
        {
            totalPower += smoothedPower[i];
        }
        //OS_TPrintf("totalPower = %d, ave = %d\n", totalPower, totalPower/(FILTER_HIGH-FILTER_LOW));
        // Check whether the input value is above a fixed amount
        if (totalPower > (FILTER_HIGH-FILTER_LOW)*64)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*====== End of main.c ======*/
