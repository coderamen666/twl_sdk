/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - demos
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Demo to confirm operation of digest functions in Math library.
  Runs high-speed comparisons.
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

static void VBlankIntr(void);
static void DisplayInit(void);
static void CompareDigestSpeed(void);
static u16 CalcCRC16(void *datap, u32 size);
static u32 CalcCRC32(void *datap, u32 size);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    // Various types of initialization
    OS_Init();
    OS_InitTick();

    DisplayInit();

    CompareDigestSpeed();

    // Main loop
    while (TRUE)
    {
        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics Initialization

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void DisplayInit(void)
{

    GX_Init();
    FX_Init();

    GX_DispOff();
    GXS_DispOff();

    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
    (void)OS_EnableIrq();


    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A,B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJs visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         CompareDigestSpeed

  Description:  Test routine for digest function

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#define DATA_SIZE (1024*1024)
static void CompareDigestSpeed(void)
{
    static u8 data[DATA_SIZE] ATTRIBUTE_ALIGN(32);
    static MATHCRC8Table table_crc8;
    static MATHCRC16Table table_crc16, table_crc16_ccitt;
    static MATHCRC32Table table_crc32, table_crc32_posix;

    int     i;
    MATHRandContext32 rand;
    OSTick  start, end;
    u8      digest[20];

    OS_TPrintf("Initialize Array...\n");
    // Initializes data
    MATH_InitRand32(&rand, 0);
    for (i = 0; i < DATA_SIZE; i++)
    {
        data[i] = (u8)MATH_Rand32(&rand, 0x100);
    }

    OS_TPrintf("Initialize CRC Tables...\n");
    // Initializes table
    MATH_CRC8InitTable(&table_crc8);
    MATH_CRC16InitTable(&table_crc16);
    MATH_CRC16CCITTInitTable(&table_crc16_ccitt);
    MATH_CRC32InitTable(&table_crc32);
    MATH_CRC32POSIXInitTable(&table_crc32_posix);

    OS_TPrintf("Start Comparing...\n");

    OS_TPrintf("Checksum8:\t");
    start = OS_GetTick();
    (void)MATH_CalcChecksum8(data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("Checksum16:\t");
    start = OS_GetTick();
    (void)MATH_CalcChecksum16(data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-8:\t\t");
    start = OS_GetTick();
    (void)MATH_CalcCRC8(&table_crc8, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-16:\t\t");
    start = OS_GetTick();
    (void)MATH_CalcCRC16(&table_crc16, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-16/CCITT:\t");
    start = OS_GetTick();
    (void)MATH_CalcCRC16CCITT(&table_crc16_ccitt, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-32:\t\t");
    start = OS_GetTick();
    (void)MATH_CalcCRC32(&table_crc32, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-32/POSIX:\t");
    start = OS_GetTick();
    (void)MATH_CalcCRC32POSIX(&table_crc32_posix, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("MD5:\t\t");
    start = OS_GetTick();
    MATH_CalcMD5(digest, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("SHA-1:\t\t");
    start = OS_GetTick();
    MATH_CalcSHA1(digest, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("SHA256:\t\t");
    start = OS_GetTick();
    MATH_CalcSHA256(digest, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("HMAC-MD5:\t");
    start = OS_GetTick();
    MATH_CalcHMACMD5(digest, data, sizeof(data), data, 32);
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("HMAC-SHA-1:\t");
    start = OS_GetTick();
    MATH_CalcHMACSHA1(digest, data, sizeof(data), data, 32);
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("HMAC-SHA-256:\t");
    start = OS_GetTick();
    MATH_CalcHMACSHA256(digest, data, sizeof(data), data, 32);
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-16(SVC):\t");
    start = OS_GetTick();
    (void)SVC_GetCRC16(0, data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

#ifdef SDK_TWL
    if ( OS_IsRunOnTwl() )
    {
        OS_TPrintf("SHA-1(SVC):\t");
        start = OS_GetTick();
        SVC_CalcSHA1(digest, data, sizeof(data));
        end = OS_GetTick();
        OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

        OS_TPrintf("HMAC-SHA-1(SVC):");
        start = OS_GetTick();
        SVC_CalcHMACSHA1(digest, data, sizeof(data), data, 32);
        end = OS_GetTick();
        OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));
    }
#endif

    // Reference mounting using a small table
    OS_TPrintf("CRC-16(4bit):\t");
    start = OS_GetTick();
    (void)CalcCRC16(data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("CRC-32(4bit):\t");
    start = OS_GetTick();
    (void)CalcCRC32(data, sizeof(data));
    end = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("Done.\n");

    return;
}

// Reference mounting
/*---------------------------------------------------------------------------*
  Name:         CalcCRC16

  Description:  Calculates CRC-16 using 4-bit units.
                Although the table size is small (32 bytes), this is 1.5 times slower than the MATH_CalcCRC16 function.
                

  Arguments:    datap - Pointer to data
                size - Size of calculated data

  Returns:      u16   - Result of CRC calculation
 *---------------------------------------------------------------------------*/
static u16 CalcCRC16(void *datap, u32 size)
{
    // Table for CRC-16 calculation (for calculating in 4-bit units)
    // It is the same as the table calculated by the MATH_CRC16InitTable function with the following values removed: 0, 0x10, 0x20, and so on up to 0xf0
    // 
    // If the crc16_table is set by the u32 array, speed increases even more
    static const u16 crc16_table[16] = {
        0x0000, 0xCC01, 0xD801, 0x1400,
        0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401,
        0x5000, 0x9C01, 0x8801, 0x4400,
    };

    u32     r = 0;                     // Initializes CRC
    u8     *p = (u8 *)datap;

    while (size--)
    {
        u32     data = *p;
        p++;

        // Calculates CRC in 4-bit units
        r = (r >> 4) ^ crc16_table[(r ^ data) & 0xf];
        data >>= 4;

        r = (r >> 4) ^ crc16_table[(r ^ data) & 0xf];
    }

    return (u16)r;
}

/*---------------------------------------------------------------------------*
  Name:         CalcCRC32

  Description:  Calculates CRC-32 using 4-bit units.
                Although the table size is small (64 bytes), this is 1.5 times slower than the MATH_CalcCRC32 function.
                

  Arguments:    datap - Pointer to data
                size - Size of calculated data

  Returns:      u32   - CRC calculation results
 *---------------------------------------------------------------------------*/
static u32 CalcCRC32(void *datap, u32 size)
{
    // Table for CRC-32 calculation (for calculating in 4-bit units)
    // It the same as the table calculated by the MATH_CRC32InitTable function with the following values removed: 0, 0x10, 0x20, and so on up to 0xf0
    // 
    static const u32 crc32_table[16] = {
        0x00000000U, 0x1DB71064U, 0x3B6E20C8U, 0x26D930ACU,
        0x76DC4190U, 0x6B6B51F4U, 0x4DB26158U, 0x5005713CU,
        0xEDB88320U, 0xF00F9344U, 0xD6D6A3E8U, 0xCB61B38CU,
        0x9B64C2B0U, 0x86D3D2D4U, 0xA00AE278U, 0xBDBDF21CU,
    };

    u32     r = 0xffffffffU;           // Initializes CRC
    u8     *p = (u8 *)datap;

    while (size--)
    {
        u32     data = *p;
        p++;

        // Calculates CRC in 4-bit units
        r = (r >> 4) ^ crc32_table[(r ^ data) & 0xf];
        data >>= 4;

        r = (r >> 4) ^ crc32_table[(r ^ data) & 0xf];
    }

    return ~r;                         // Output is inverted
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
