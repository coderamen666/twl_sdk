/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - demos
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  MATH library: Demo to confirm operation of fast Fourier transform
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/fx/fx_trig.h>


static void VBlankIntr(void);
static void DisplayInit(void);
static void FillScreen(u16 col);
static BOOL FFTTest(void);
static void PrintFX32(fx32 f);
static void PrintRealArray(fx32 *data);
static void PrintHalfComplexArray(fx32 *data);
static void PrintComplexArray(fx32 *data);
static BOOL PrintError(fx32 *orig, fx32 *data);

static void InitBitRevTable(void);
static void FFT(fx32 *data, int nShift);
static void IFFT(fx32 *data, int nShift);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/
#define FFT_NSHIFT         10
#define FFT_N              (1 << FFT_NSHIFT)
#define FFT_VALUE_MAX      ((1 << (31 - FFT_NSHIFT))-1)
#define FFT_VALUE_MIN      (-(1 << (31 - FFT_NSHIFT)))
#define FFT_VALUE_RANGE    (1U << (32 - FFT_NSHIFT))

#define N_RANDOM_INPUT     16
#define ERROR_THRETHOLD    (MATH_MAX(((double)FFT_VALUE_RANGE)/(1 << 23), (double)(1 << (FFT_NSHIFT-12))))

static fx32 gFFTCos[FFT_N], gFFTSin[FFT_N];
static int gBitRevTable[FFT_N];

/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Various types of initialization
    OS_Init();
    OS_InitTick();

    DisplayInit();

    if (FFTTest())
    {
        // Success
        OS_TPrintf("------ Test Succeeded ------\n");
        FillScreen(GX_RGB(0, 31, 0));
    }
    else
    {
        // Failed
        OS_TPrintf("****** Test Failed ******\n");
        FillScreen(GX_RGB(31, 0, 0));
    }

    // Main loop
    while (TRUE)
    {
        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics initialization.

  Arguments:    None.

  Returns:      None.
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


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A, B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJ visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping object

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         FillScreen

  Description:  Fills the screen.

  Arguments:    col: FillColor

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FillScreen(u16 col)
{
    MI_CpuFill16((void *)HW_LCDC_VRAM_C, col, 256 * 192 * 2);
}

/*---------------------------------------------------------------------------*
  Name:         FFTTest

  Description:  Test routine for fast Fourier transform.

  Arguments:    None.

  Returns:      TRUE if test succeeds.
 *---------------------------------------------------------------------------*/
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }

static BOOL FFTTest(void)
{
    static fx32 data[FFT_N * 2];
    static fx32 orig[FFT_N * 2];
    static fx16 sinTable[FFT_N - FFT_N / 4];
    static fx16 sinTable2[(FFT_N - FFT_N / 4) / 2];

    BOOL    flag = TRUE;
    int     i;
    OSTick  start, end;

    MATH_MakeFFTSinTable(sinTable, FFT_NSHIFT);
    MATH_MakeFFTSinTable(sinTable2, FFT_NSHIFT - 1);

    OS_TPrintf("N = %d\n", FFT_N);
    OS_TPrintf("\nMATH_FFT: Sin\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] = FX_SinIdx((65536 / FFT_N) * i);
            orig[i * 2 + 1] = data[i * 2 + 1] = 0;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Cos\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] = FX_CosIdx((65536 / FFT_N) * i);
            orig[i * 2 + 1] = data[i * 2 + 1] = 0;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Cos + Sin\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] =
                FX_CosIdx((65536 / FFT_N) * i) + FX_SinIdx((65536 / FFT_N) * i);
            orig[i * 2 + 1] = data[i * 2 + 1] = 0;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Highest Freqency (Real)\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] = (i & 1) ? FFT_VALUE_MIN : FFT_VALUE_MAX;
            orig[i * 2 + 1] = data[i * 2 + 1] = 0;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Highest Freqency (Complex)\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] = (i & 1) ? FFT_VALUE_MIN : FFT_VALUE_MAX;
            orig[i * 2 + 1] = data[i * 2 + 1] = (i & 1) ? FFT_VALUE_MIN : FFT_VALUE_MAX;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Constant\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i * 2] = data[i * 2] = FFT_VALUE_MAX;
            orig[i * 2 + 1] = data[i * 2 + 1] = FFT_VALUE_MAX;
        }
        start = OS_GetTick();
        MATH_FFT(data, FFT_NSHIFT, sinTable);
        end = OS_GetTick();
//        PrintComplexArray(data);
        MATH_IFFT(data, FFT_NSHIFT, sinTable);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFT: Random Input\n");
    {
        u32     seed;
        for (seed = 0; seed < N_RANDOM_INPUT; seed++)
        {
            MATHRandContext32 rand;

            MATH_InitRand32(&rand, seed);
            for (i = 0; i < FFT_N; i++)
            {
                orig[i * 2] = data[i * 2] =
                    (fx32)(MATH_Rand32(&rand, FFT_VALUE_RANGE) - (FFT_VALUE_RANGE / 2));
                orig[i * 2 + 1] = data[i * 2 + 1] =
                    (fx32)(MATH_Rand32(&rand, FFT_VALUE_RANGE) - (FFT_VALUE_RANGE / 2));
            }
            start = OS_GetTick();
            MATH_FFT(data, FFT_NSHIFT, sinTable);
            end = OS_GetTick();
//            PrintComplexArray(data);
            MATH_IFFT(data, FFT_NSHIFT, sinTable);
            OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
            flag = PrintError(orig, data) && flag;
        }
    }

    OS_TPrintf("\nMATH_FFTReal: Sin\n");
    {
        for (i = 0; i < FFT_N; i++)
        {
            orig[i] = data[i] = FX_SinIdx((65536 / FFT_N) * i);
        }
        for (; i < FFT_N * 2; i++)
        {
            orig[i] = data[i] = 0;
        }
        start = OS_GetTick();
        MATH_FFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
        end = OS_GetTick();
//        PrintHalfComplexArray(data);
        MATH_IFFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
//        PrintRealArray(data);
        OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
        flag = PrintError(orig, data) && flag;
    }

    OS_TPrintf("\nMATH_FFTReal: Random Input\n");
    {
        u32     seed;
        for (seed = 0; seed < N_RANDOM_INPUT; seed++)
        {
            MATHRandContext32 rand;

            MATH_InitRand32(&rand, seed);
            for (i = 0; i < FFT_N; i++)
            {
                orig[i] = data[i] =
                    (fx32)(MATH_Rand32(&rand, FFT_VALUE_RANGE) - (FFT_VALUE_RANGE / 2));
            }
            for (; i < FFT_N * 2; i++)
            {
                orig[i] = data[i] = 0;
            }
            start = OS_GetTick();
            MATH_FFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
            end = OS_GetTick();
//            PrintComplexArray(data);
            MATH_IFFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
            OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));
            flag = PrintError(orig, data) && flag;
        }
    }

/*
FFT and IFFT below are reference implementations to compare with MATH_FFT and MATH_IFFT.
MATH_FFT is an optimized implementation, so a reference algorithm has been implemented to compare behavior.
The margin of error may differ between the two calculation results.
*/
#if 0
    OS_TPrintf("\nLocal FFT Function\n");
    InitBitRevTable();
    {
        for (i = 0; i < FFT_N; i++)
        {
            data[i * 2] = FX_SinIdx((65536 / FFT_N) * i);
            data[i * 2 + 1] = 0;
        }
        FFT(data, FFT_NSHIFT);
//        PrintComplexArray(data);
    }

    {
        u32     seed;
        for (seed = 0; seed < N_RANDOM_INPUT; seed++)
        {
            MATHRandContext32 rand;

            MATH_InitRand32(&rand, seed);
            for (i = 0; i < FFT_N; i++)
            {
                orig[i * 2] = data[i * 2] =
                    (fx32)(MATH_Rand32(&rand, FFT_VALUE_RANGE) - (FFT_VALUE_RANGE / 2));
                orig[i * 2 + 1] = data[i * 2 + 1] =
                    (fx32)(MATH_Rand32(&rand, FFT_VALUE_RANGE) - (FFT_VALUE_RANGE / 2));
            }
            FFT(data, FFT_NSHIFT);
//            PrintComplexArray(data);
            IFFT(data, FFT_NSHIFT);
            flag = PrintError(orig, data) && flag;
        }
    }

#endif


    return flag;
}

static void PrintRealArray(fx32 *data)
{
    u32     i;
    for (i = 0; i < FFT_N; i++)
    {
        OS_TPrintf("%3x: ", i);
        PrintFX32(data[i]);
        OS_TPrintf("\n");
    }
}

static void PrintHalfComplexArray(fx32 *data)
{
    u32     i;

	i = 0;
    OS_TPrintf("%3x: ", i);
	PrintFX32(data[0]);
    OS_TPrintf("\n");
    for (i = 1; i < FFT_N / 2; i++)
    {
        OS_TPrintf("%3x: ", i);
        PrintFX32(data[i * 2]);
        OS_TPrintf(", ");
        PrintFX32(data[i * 2 + 1]);
        OS_TPrintf("\n");
    }
    OS_TPrintf("%3x: ", i);
	PrintFX32(data[1]);
    OS_TPrintf("\n");
}

static void PrintComplexArray(fx32 *data)
{
    u32     i;
    for (i = 0; i < FFT_N; i++)
    {
        OS_TPrintf("%3x: ", i);
        PrintFX32(data[i * 2]);
        OS_TPrintf(", ");
        PrintFX32(data[i * 2 + 1]);
        OS_TPrintf("\n");
    }
}

static void PrintFX32(fx32 f)
{
#pragma unused(f)                      // Required for FINALROM build
    OS_Printf("%f", FX_FX32_TO_F32(f));
#if 0
    if (f >= 0)
    {
        OS_TPrintf(" %6d.%03d", f >> FX32_SHIFT, (f & FX32_DEC_MASK) * 1000 / 4096);
    }
    else
    {
        OS_TPrintf("-%6d.%03d", (-f) >> FX32_SHIFT, ((-f) & FX32_DEC_MASK) * 1000 / 4096);
    }
#endif
}

static BOOL PrintError(fx32 *orig, fx32 *data)
{
    u32     i;
    fx32    max_error;
    double  sum_sqd, e;

    max_error = 0;
    sum_sqd = 0;
    for (i = 0; i < FFT_N * 2; i++)
    {
        fx32    d = MATH_ABS(data[i] - orig[i]);
        double  dd = FX_FX32_TO_F32(d);
        double  od = FX_FX32_TO_F32(orig[i]);

        if (d > max_error)
        {
            max_error = d;
        }

        sum_sqd += dd * dd;
    }
    sum_sqd /= FFT_N * 2;
    e = FX_FX32_TO_F32(max_error);
    OS_Printf("Max Error: %f, Dist.^2: %.4g\n", e, sum_sqd);

    return (e <= ERROR_THRETHOLD);
}

/*
FFT and IFFT below are reference implementations to compare with MATH_FFT and MATH_IFFT.
MATH_FFT is an optimized implementation, so a reference algorithm has been implemented to compare behavior.
The margin of error may differ between the two calculation results.
*/
#if 0
static void InitBitRevTable(void)
{
    int     i, j, k;

    i = j = 0;
    for (;;)
    {
        gBitRevTable[i] = j;
        if (++i >= FFT_N)
            break;
        k = FFT_N / 2;
        while (k <= j)
        {
            j -= k;
            k /= 2;
        }
        j += k;
    }
}

static void FFT(fx32 *data, int nShift)
{
    int     i, j, k, ik, h, d, k2;
    fx32    t, s, c, dx, dy;
    u32     n = 1U << nShift;
    OSTick  start, end;

    SDK_ASSERT(n == FFT_N);

    for (i = 0; i < FFT_N; i++)
    {
        gFFTCos[i] = data[i * 2];
        gFFTSin[i] = data[i * 2 + 1];
    }

    start = OS_GetTick();
    for (i = 0; i < FFT_N; i++)
    {                                  /* Bit inversion */
        j = gBitRevTable[i];
        if (i < j)
        {
            t = gFFTCos[i];
            gFFTCos[i] = gFFTCos[j];
            gFFTCos[j] = t;
            t = gFFTSin[i];
            gFFTSin[i] = gFFTSin[j];
            gFFTSin[j] = t;
        }
    }
    for (k = 1; k < FFT_N; k = k2)
    {                                  /* Conversion */
        h = 0;
        k2 = k + k;
        d = FFT_N / k2;
        for (j = 0; j < k; j++)
        {
            c = FX_CosIdx(h * (65536 / FFT_N));
            s = FX_SinIdx(h * (65536 / FFT_N));
            for (i = j; i < FFT_N; i += k2)
            {
                ik = i + k;
                dx = FX_Mul(s, gFFTSin[ik]) + FX_Mul(c, gFFTCos[ik]);
                dy = FX_Mul(c, gFFTSin[ik]) - FX_Mul(s, gFFTCos[ik]);
                gFFTCos[ik] = gFFTCos[i] - dx;
                gFFTCos[i] += dx;
                gFFTSin[ik] = gFFTSin[i] - dy;
                gFFTSin[i] += dy;
            }
            h += d;
        }
    }
    end = OS_GetTick();
    OS_Printf("%lld us, ", OS_TicksToMicroSeconds(end - start));

    for (i = 0; i < FFT_N; i++)
    {
        data[i * 2] = gFFTCos[i] >> nShift;
        data[i * 2 + 1] = gFFTSin[i] >> nShift;
    }
}

static void IFFT(fx32 *data, int nShift)
{
    int     i, j, k, ik, h, d, k2;
    fx32    t, s, c, dx, dy;
    u32     n = 1U << nShift;

    SDK_ASSERT(n == FFT_N);

    for (i = 0; i < FFT_N; i++)
    {
        gFFTCos[i] = data[i * 2];
        gFFTSin[i] = data[i * 2 + 1];
    }

    for (i = 0; i < FFT_N; i++)
    {                                  /* Bit inversion */
        j = gBitRevTable[i];
        if (i < j)
        {
            t = gFFTCos[i];
            gFFTCos[i] = gFFTCos[j];
            gFFTCos[j] = t;
            t = gFFTSin[i];
            gFFTSin[i] = gFFTSin[j];
            gFFTSin[j] = t;
        }
    }
    for (k = 1; k < FFT_N; k = k2)
    {                                  /* Conversion */
        h = 0;
        k2 = k + k;
        d = FFT_N / k2;
        for (j = 0; j < k; j++)
        {
            c = FX_CosIdx(h * (65536 / FFT_N));
            s = FX_SinIdx(h * (65536 / FFT_N));
            for (i = j; i < FFT_N; i += k2)
            {
                ik = i + k;
                dx = -FX_Mul(s, gFFTSin[ik]) + FX_Mul(c, gFFTCos[ik]);
                dy = FX_Mul(c, gFFTSin[ik]) + FX_Mul(s, gFFTCos[ik]);
                gFFTCos[ik] = gFFTCos[i] - dx;
                gFFTCos[i] += dx;
                gFFTSin[ik] = gFFTSin[i] - dy;
                gFFTSin[i] += dy;
            }
            h += d;
        }
    }

    for (i = 0; i < FFT_N; i++)
    {
        data[i * 2] = gFFTCos[i];
        data[i * 2 + 1] = gFFTSin[i];
    }
}
#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
