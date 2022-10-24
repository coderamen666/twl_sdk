/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

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
  Demo of attack-proof encryption using the CRYPTO library RC4 algorithm

  This uses a PRNG to get a random initial vector (IV), different every time, and safely encrypt with the RC4 algorithm.
  
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>

#include "prng.h"
#include "rc4enc.h"

#define TEST_BUFFER_SIZE 256

static void InitializeAllocateSystem(void);
static void VBlankIntr(void);
static void DisplayInit(void);
static void PRNGInit(void);
static void FillScreen(u16 col);
static BOOL RC4EncText(void);
static BOOL CompareBinary(void* ptr1, void* ptr2, u32 n);
static u32 GetStringLength(char* str);
static void PrintBinary(u8* ptr, u32 len);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

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
    InitializeAllocateSystem();

    DisplayInit();

    PRNGInit();

    if (RC4EncText())
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
    // The internal state of the PRNG is churned by the current system status.
    // It is periodically churned to improve security because initialization with PRNGInit() only is insufficient.
    // 
    // Be sure to call this function periodically as calling it just once does not provide sufficient churning.
    ChurnRandomPool();

    // Sets the IRQ check flag
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
    void *tempLo;
    OSHeapHandle hh;

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


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A,B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJs visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();              // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}

/*---------------------------------------------------------------------------*
  Name:         PRNGInit

  Description:  Initializes the pseudorandom number generator.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PRNGInit(void)
{
    int i;
    for ( i = 0; i < 10; i++ )
    {
        ChurnRandomPool();
        OS_WaitVBlankIntr();
    }
    // You must continue to call ChurnRandomPool() periodically because calling it several times will not collect enough entropy.
    // 

    // If greater security is required, you could call the AddRandomSeed function with several kilobytes of microphone input data as an argument to initialize the PRNG.
    // 
    // 
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
  Name:         RC4EncText

  Description:  RC4 algorithm test routine.

  Arguments:    None.

  Returns:      TRUE if test succeeds.
 *---------------------------------------------------------------------------*/
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }
#define PrintResultBinaryEq( a, b, l, f ) \
    { OS_TPrintf( (CompareBinary((a), (b), (l))) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && (CompareBinary((a), (b), (l))); }

static BOOL RC4EncText(void)
{
    int i;
    int testcase = 1;
    BOOL flag = TRUE;

    // Encode and decode using RC4Enc and check that the results match
    {
        char* d[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Copyright 2006 Nintendo.  All rights reserved.",
        };
        char* key[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67",
            "Nintendo DS.",
        };

        for (i = 0; i < sizeof(d) / sizeof(char*); i++)
        {
            RC4EncoderContext enc_context;
            RC4DecoderContext dec_context;
            u8 result[TEST_BUFFER_SIZE];
            u8 result2[TEST_BUFFER_SIZE];
            u32 len;
            u32 encoded_len;
            u32 decoded_len;

            InitRC4Encoder(&enc_context, key[i]);
            len = GetStringLength(d[i]);
            encoded_len = EncodeRC4(&enc_context, d[i], len, result, sizeof(result));

            PrintResultEq(encoded_len, len + RC4ENC_ADDITIONAL_SIZE, flag);
            OS_TPrintf("RC4Enc: Test Case %d: encryption phase: length check\n", testcase);

            PrintBinary(result, encoded_len);
            OS_TPrintf("\n");

            InitRC4Decoder(&dec_context, key[i]);
            decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, sizeof(result2));

            PrintResultEq(decoded_len, len, flag);
            OS_TPrintf("RC4Enc: Test Case %d: decryption phase: length check\n", testcase);

            PrintResultBinaryEq(d[i], result2, len, flag);
            OS_TPrintf("RC4Enc: Test Case %d: decryption phase: binary comparing\n", testcase);

            testcase++;
        }
    }

    {
        char* d = "abcdefghijklmnopqrstuvwxyz";
        char* key = "123456789012";
        u8 result[TEST_BUFFER_SIZE];
        u8 result2[TEST_BUFFER_SIZE];
        u8 result_backup[TEST_BUFFER_SIZE];
        u32 len;
        u32 encoded_len;
        u32 decoded_len;
        RC4EncoderContext enc_context;
        RC4DecoderContext dec_context;

        // Check the buffer size used during encoding

        InitRC4Encoder(&enc_context, key);
        len = GetStringLength(d);

        encoded_len = EncodeRC4(&enc_context, d, len, result, 0);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, 1);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, len + RC4ENC_ADDITIONAL_SIZE - 1);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, len + RC4ENC_ADDITIONAL_SIZE);
        PrintResultEq(encoded_len, len + RC4ENC_ADDITIONAL_SIZE, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        // Check the buffer size used during decoding

        InitRC4Decoder(&dec_context, key);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, 0);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, 1);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len - 1);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, len, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        // If part of the following encoded data is changed, check whether an error occurs when decoding
        // 

        // Create a backup of encoded data
        MI_CpuCopy8(result, result_backup, encoded_len);

        result[0] ^= 1;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // Restore from backup

        result[3] ^= 0xff;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // Restore from backup

        result[4] ^= 1;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // Restore from backup

        result[4+len] ^= 0x55;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // Restore from backup

        result[encoded_len-1] ^= 0x80;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);
    }





    return flag;
}


/*---------------------------------------------------------------------------*
  Name:         CompareBinary

  Description:  Compares memory contents.

  Arguments:    ptr1, ptr2: Memory locations to compare
                n: Length to compar

  Returns:      If they match, TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CompareBinary(void* ptr1, void* ptr2, u32 n)
{
    u32 i;
    u8* p1 = (u8*)ptr1;
    u8* p2 = (u8*)ptr2;

    for (i = 0; i < n; i++)
    {
        if (*(p1++) != *(p2++))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GetStringLength

  Description:  Gets the length of a string.

  Arguments:    str: Pointer to a string

  Returns:      Length of string
 *---------------------------------------------------------------------------*/
static u32 GetStringLength(char* str)
{
    u32 i;
    for (i = 0; ; i++)
    {
        if (*(str++) == '\0')
        {
            return i;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintBinary

  Description:  Binary array output.

  Arguments:    ptr: Memory location to be output
                len: Output length

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBinary(u8* ptr, u32 len)
{
#ifndef SDK_FINALROM
    u32 i;
    for (i = 0; i < len; i++)
    {
        if (i != 0)
        {
            OS_PutString(", ");
        }
        OS_TPrintf("0x%02x", ptr[i]);
    }
#else
#pragma unused(ptr,len)
#endif
    return;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
