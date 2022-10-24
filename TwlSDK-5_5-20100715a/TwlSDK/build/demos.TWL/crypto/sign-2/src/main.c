/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     main.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  This demo confirms that the CRYPTO library's RSA electronic signature functions operate properly.
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>
#include <string.h>
#include "DEMO.h"

//#define PRINT_ALLOCATION
//#define PRINT_TIME

extern const unsigned char rsa1024_pub[], rsa1024_sec[];
extern const unsigned long rsa1024_pub_len, rsa1024_sec_len;

static void InitializeAllocateSystem(void);
static void VBlankIntr(void);
static void DisplayInit(void);
static void FillScreen(u16 col);
static BOOL RsaTest(void);
static BOOL sign_verify( char *teststr, void *pubkey, void *seckey, const unsigned long seckey_len );
static u32 GetStringLength(char* str);
static void PrintBinary(u8* ptr, int len);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

#ifdef PRINT_ALLOCATION
static u32 total_allocated_size = 0;
#endif

static const unsigned char rsa512_pub[]={
    0x30, 0x5C, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05,
    0x00, 0x03, 0x4B, 0x00, 0x30, 0x48, 0x02, 0x41, 0x00, 0xAA, 0x3D, 0x39, 0x23, 0x81, 0x1E, 0x04,
    0xDB, 0x25, 0x91, 0xE4, 0x15, 0xA5, 0x3A, 0x9A, 0xCF, 0x64, 0x7F, 0x2F, 0xD6, 0x03, 0xB1, 0x6C,
    0xA4, 0x1E, 0x6A, 0x4D, 0x7C, 0x9A, 0x02, 0x7D, 0x9A, 0x8E, 0xF9, 0x8F, 0x61, 0x9C, 0x0E, 0x42,
    0x49, 0x58, 0xBB, 0x96, 0xB1, 0x1F, 0x7C, 0xCA, 0x7B, 0x2A, 0x75, 0x3A, 0x6C, 0x82, 0x4F, 0xF9,
    0xF5, 0xB2, 0x11, 0xB7, 0xC4, 0xCC, 0xF9, 0x6D, 0xE7, 0x02, 0x03, 0x01, 0x00, 0x01
};
static const int rsa512_pub_len = 0x5E;

static const unsigned char rsa512_sec[]={
    0x30, 0x82, 0x01, 0x3A, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00, 0xAA, 0x3D, 0x39, 0x23, 0x81, 0x1E,
    0x04, 0xDB, 0x25, 0x91, 0xE4, 0x15, 0xA5, 0x3A, 0x9A, 0xCF, 0x64, 0x7F, 0x2F, 0xD6, 0x03, 0xB1,
    0x6C, 0xA4, 0x1E, 0x6A, 0x4D, 0x7C, 0x9A, 0x02, 0x7D, 0x9A, 0x8E, 0xF9, 0x8F, 0x61, 0x9C, 0x0E,
    0x42, 0x49, 0x58, 0xBB, 0x96, 0xB1, 0x1F, 0x7C, 0xCA, 0x7B, 0x2A, 0x75, 0x3A, 0x6C, 0x82, 0x4F,
    0xF9, 0xF5, 0xB2, 0x11, 0xB7, 0xC4, 0xCC, 0xF9, 0x6D, 0xE7, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
    0x40, 0x75, 0x8E, 0xA1, 0x8E, 0x6A, 0xF3, 0x17, 0xD7, 0x5D, 0x32, 0x49, 0x96, 0xC8, 0xEA, 0x97,
    0x4C, 0xEF, 0xD1, 0x04, 0x6F, 0x7D, 0x4D, 0x60, 0x00, 0x55, 0x20, 0x83, 0x31, 0xB0, 0x08, 0x58,
    0x42, 0xEA, 0x75, 0x28, 0xB7, 0xB7, 0x44, 0xA7, 0x03, 0xF8, 0xEC, 0x80, 0x67, 0xD2, 0xF5, 0x44,
    0x14, 0x77, 0x7D, 0x3F, 0xE7, 0x12, 0x2C, 0xAB, 0xE8, 0xBA, 0x52, 0x0E, 0xD5, 0x1F, 0xE5, 0xC1,
    0x81, 0x02, 0x21, 0x00, 0xD1, 0xE4, 0x7B, 0xDE, 0x44, 0xE2, 0x4E, 0x03, 0xE5, 0x74, 0xCD, 0x89,
    0xFD, 0x15, 0x8A, 0xD8, 0xAE, 0xF4, 0x67, 0x87, 0xCF, 0x65, 0x52, 0x3C, 0x23, 0x13, 0x2B, 0xE9,
    0x97, 0x33, 0x26, 0x69, 0x02, 0x21, 0x00, 0xCF, 0xA2, 0xCA, 0xF1, 0x9F, 0xB0, 0x49, 0x1E, 0x4E,
    0x64, 0x5F, 0xC7, 0x21, 0xE0, 0x71, 0x7E, 0x93, 0x7A, 0x5B, 0x20, 0xB1, 0x89, 0xAF, 0xF9, 0xC8,
    0x96, 0x23, 0xBB, 0x72, 0xFC, 0x87, 0xCF, 0x02, 0x21, 0x00, 0xA8, 0xF7, 0x3C, 0x58, 0x44, 0x2F,
    0xC2, 0x0A, 0x14, 0xEF, 0xA0, 0x7F, 0x13, 0x04, 0x02, 0x90, 0x48, 0xD7, 0x6B, 0x78, 0xC3, 0x16,
    0x97, 0xCA, 0xDD, 0x99, 0x93, 0x62, 0x2A, 0x5B, 0xFC, 0xF1, 0x02, 0x20, 0x11, 0x44, 0x4E, 0x70,
    0x2D, 0x81, 0x71, 0x73, 0x2D, 0xBD, 0xB7, 0x21, 0x4E, 0x35, 0xE5, 0xFA, 0x4A, 0xB5, 0x60, 0x22,
    0xA5, 0xE0, 0xF7, 0x5B, 0x64, 0x4C, 0xE8, 0x07, 0xCC, 0x96, 0x27, 0x8D, 0x02, 0x20, 0x42, 0xEA,
    0x98, 0x73, 0x96, 0x3E, 0xAC, 0x03, 0x3B, 0x19, 0xD0, 0xB7, 0x29, 0x11, 0x94, 0x25, 0x0E, 0x98,
    0xC4, 0x7C, 0xA5, 0x95, 0x64, 0x02, 0x34, 0xB8, 0x4B, 0x66, 0x1E, 0x7B, 0x99, 0xB4
};
static const int rsa512_sec_len = 0x13E;


/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MyAlloc

  Description:  OS_Alloc wrapper function.
 *---------------------------------------------------------------------------*/
static void* MyAlloc(u32 size)
{
    void* ptr;
#ifdef PRINT_ALLOCATION
    ptr = OS_Alloc(size+4);
    *(u32*)ptr = size;
    (u8*)ptr += 4;
    total_allocated_size += size;
    OS_TPrintf("Allocate %d bytes from %p: total %d bytes\n", size, ptr, total_allocated_size);
#else
    ptr = OS_Alloc(size);
#endif
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         MyFree

  Description:  OS_Free wrapper function.
 *---------------------------------------------------------------------------*/
static void MyFree(void* ptr)
{
#ifdef PRINT_ALLOCATION
    u32 size;
    (u8*)ptr -= 4;
    size = *(u32*)ptr;
    total_allocated_size -= size;
    OS_TPrintf("Free %d bytes from %p: total %d bytes\n", size, ptr, total_allocated_size);
#endif
    OS_Free(ptr);
}

/*---------------------------------------------------------------------------*
  Name:         MyRealloc

  Description:  The realloc function.
 *---------------------------------------------------------------------------*/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

static void* MyRealloc(void* ptr, u32 new_size, u32 old_size)
{
    void *buf;
#ifdef PRINT_ALLOCATION
    u32 size;

	buf = OS_Alloc(new_size+4);
    *(u32*)buf = new_size;
    (u8*)buf += 4;
    total_allocated_size += new_size;
    OS_TPrintf("Reallocate %d bytes from %p: total %d bytes\n", new_size, ptr, total_allocated_size);

    (u8*)ptr -= 4;
    size = *(u32*)ptr;
    total_allocated_size -= size;
    OS_TPrintf("Free(for realloc) %d bytes from %p: total %d bytes\n", size, ptr, total_allocated_size);
	SDK_ASSERT(size == old_size);
    OS_Free(ptr);
#else
    if ((buf = OS_Alloc(new_size)) == NULL)
    {
        return NULL;
    }

    MI_CpuCopy8(ptr, buf, MIN(new_size, old_size));
    OS_Free(ptr);
#endif

    return buf;
}

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
    InitializeAllocateSystem();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    DisplayInit();

    if (RsaTest())
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


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A, B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJ visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();              // Waiting for the end of the V-Blank interrupt
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
  Name:         RsaTest

  Description:  Test routine for the electronic signature functions.

  Arguments:    None.

  Returns:      TRUE if test succeeds.
 *---------------------------------------------------------------------------*/
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }

#define CheckAndPrintErr( result, str ) \
    { if(OS_IsRunOnTwl() == TRUE){      \
          if(result < 0) { OS_TPrintf( "Error: %d (%s)\n", result, str ); return FALSE;}  \
      }else{                            \
          if(result >= 0){ OS_TPrintf( "Error: %d (%s)\n", result, str ); return FALSE;}  \
      }  \
    }

static BOOL RsaTest(void)
{
    BOOL flag = TRUE;

    // Perform electronic signature operation test
    {
        char teststr[] = "Copyright 2003-2005 Nintendo.  All rights reserved.";
		BOOL ret;

        CRYPTO_SetMemAllocator(MyAlloc, MyFree, MyRealloc);

        /* RSA sign and verify */
		ret = sign_verify(teststr, (void *)rsa1024_pub, (void *)rsa1024_sec, rsa1024_sec_len);
		PrintResultEq(ret, TRUE, flag);	OS_TPrintf("RSA sign test\n");
    }


    return flag;
}

static BOOL sign_verify( char *teststr, void *pubkey,
								void *seckey, const unsigned long seckey_len )
{
	char                         signstr[144];
	unsigned char                *mod_ptr;
    s32                          result;
	s32                          len;
    CRYPTORSASignContext         context;
    CRYPTORSASignInitParam       signinitparam;
    CRYPTORSASignParam           signparam;
#ifdef PRINT_TIME
    OSTick                       begin, end, term;
#endif

#ifdef PRINT_TIME
	begin = OS_GetTick();
#endif

    /* RSA sign */
    signinitparam.key     = (void*)seckey;
    signinitparam.key_len = seckey_len;
    result = CRYPTO_RSA_SignInit(&context, &signinitparam);
    CheckAndPrintErr(result, "CRYPTO_RSA_SignInit");

    signparam.in      = teststr;
    signparam.in_len  = strlen(teststr);
    signparam.out     = signstr;
    signparam.out_size = sizeof(signstr);
    len = CRYPTO_RSA_Sign(&context, &signparam);
    CheckAndPrintErr(len, "CRYPTO_RSA_Sign");

    result = CRYPTO_RSA_SignTerminate(&context);
    CheckAndPrintErr(result, "CRYPTO_RSA_SignTerminate");

#ifdef PRINT_TIME
    end = OS_GetTick();
	term = OS_TicksToMicroSeconds(end - begin);
    OS_TPrintf("   RSA sign: %d.%03d ms - %d tick\n", (u32)(term/1000), (u32)(term%1000), term);

	begin = OS_GetTick();
#endif

	if(OS_IsRunOnTwl() == FALSE){
		return TRUE;
	}

    /* SIGN verify */
	if((mod_ptr = CRYPTO_SIGN_GetModulus(pubkey)) == NULL)
	{
		OS_TPrintf( "Error: pubkey modulus find error . (CRYPTO_SIGN_GetModulus)\n" );
		return FALSE;
	}
	if(CRYPTO_VerifySignature(teststr, (int)strlen(teststr), signstr, mod_ptr) == FALSE)
	{
		OS_TPrintf( "Error: SIGN verify error . (CRYPTO_VerifySignature)\n" );
		return FALSE;
	}

#ifdef PRINT_TIME
    end = OS_GetTick();
	term = OS_TicksToMicroSeconds(end - begin);
    OS_TPrintf("   SIGN verify: %d.%03d ms - %d tick\n", (u32)(term/1000), (u32)(term%1000), term);
#endif

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

  Description:  binary string output

  Arguments:    ptr: Memory location to output
                len: Length to output

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBinary(u8* ptr, int len)
{
#ifndef SDK_FINALROM
    int i;
    for (i = 0; i < len; i++)
    {
        if (i != 0)
        {
            OS_PutString(":");
        }
        OS_TPrintf("%02x", ptr[i]);
    }
#else
#pragma unused(ptr,len)
#endif
    return;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
