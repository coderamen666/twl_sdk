/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ccm
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include "DEMO.h"
#include <twl/aes.h>


/*---------------------------------------------------------------------------*
    Internal variables
 *---------------------------------------------------------------------------*/

typedef struct DEMOSizeData
{
    u32             aSize;
    u32             pSize;
    AESMacLength    macLength;
}
DEMOSizeData;


// Message queue used for waiting for the end of AES processing
#define DEMO_MESSAGE_Q_SIZE     1
static OSMessageQueue   sMsgQ;
static OSMessage        sMsgQBuffer[DEMO_MESSAGE_Q_SIZE];

// Sample data to encrypt
static const char DEMO_TEXT[] =
    "These coded instructions, statements, and computer programs contain "
    "proprietary information of Nintendo of America Inc. and/or Nintendo "
    "Company Ltd., and are protected by Federal copyright law.  They may "
    "not be disclosed to third parties or copied or duplicated in any form, "
    "in whole or in part, without the prior written consent of Nintendo.";

// Appropriate key to use for encryption
static const AESKey DEMO_KEY =
    { 0xb3,0x2f,0x3a,0x91,0xe6,0x98,0xc2,0x76,
      0x70,0x6d,0xfd,0x71,0xbc,0xdd,0xb3,0x93, };



/*---------------------------------------------------------------------------*
    Internal function declarations
 *---------------------------------------------------------------------------*/

static void         DEMO_InitInteruptSystem(void);
static void         DEMO_InitAllocSystem(void);
static void         DEMO_AESCallback(AESResult result, void* arg);
static AESResult    DEMO_WaitAes(OSMessageQueue* pQ);
static void         DEMO_PrintBytes(const void* pvoid, u32 size);



/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  The main body of the sample.

  Arguments:    None.

  Returns:      Whether sample processing succeeded.
 *---------------------------------------------------------------------------*/
static BOOL SampleMain(void)
{
    const AESMacLength macLength    = AES_MAC_LENGTH_16;
    BOOL bSuccess = FALSE;

    AESResult aesResult;
    AESNonce nonce;
    void* pPlainText;
    void* pEncrypted;
    void* pDecrypted;
    void* pPlainTextA;
    void* pPlainTextP;
    void* pEncryptedA;
    void* pEncryptedC;
    void* pEncryptedM;
    u32 srcASize;
    u32 srcPSize;
    u32 macSize;
    DEMOSizeData* pSizeData;

    // Initialize the message queue for waiting on AES processing
    OS_InitMessageQueue(&sMsgQ, sMsgQBuffer, DEMO_MESSAGE_Q_SIZE);

    // Size calculation
    // srcASize must be a multiple of AES_BLOCK_SIZE.
    srcASize    = MATH_ROUNDUP(sizeof(DEMOSizeData), AES_BLOCK_SIZE);
    srcPSize    = (u32)STD_GetStringLength(DEMO_TEXT) + 1;
    macSize     = AES_GetMacLengthValue(macLength);

    // Memory allocation and pointer calculation
    // During both encryption and decryption, Pdata and Adata must be stored in contiguous buffers.
    // 
    pPlainText  = OS_Alloc(srcASize + srcPSize);
    pEncrypted  = OS_Alloc(srcASize + srcPSize + macSize);
    pDecrypted  = OS_Alloc(srcPSize);

    if( ! SDK_IS_VALID_POINTER(pPlainText)
     || ! SDK_IS_VALID_POINTER(pEncrypted)
     || ! SDK_IS_VALID_POINTER(pDecrypted) )
    {
        OS_TPrintf("fail to OS_Alloc\n");
        return FALSE;
    }

    pPlainTextA = pPlainText;
    pPlainTextP = (u8*)pPlainTextA + srcASize;
    pEncryptedA = pEncrypted;
    pEncryptedC = (u8*)pEncryptedA + srcASize;
    pEncryptedM = (u8*)pEncryptedC + srcPSize;

    // In this sample, each region's size is stored as Adata
    pSizeData = (DEMOSizeData*)pPlainTextA;
    pSizeData->aSize        = srcASize;
    pSizeData->pSize        = srcPSize;
    pSizeData->macLength    = macLength;
    MI_CpuClear8(pSizeData + 1, srcASize - sizeof(DEMOSizeData));

    // Pdata is copied to the buffer following after Adata
    MI_CpuCopy8(DEMO_TEXT, (u8*)pPlainText + srcASize, srcPSize);

    // The Adata used during encryption is copied for use during decryption.
    // The only output of encryption is Cdata, which is Pdata after it is encrypted.
    // Because a separate Adata is needed to confirm validity during decryption, it is copied.
    MI_CpuCopy8(pPlainText, pEncrypted, srcASize);


    // The first AES function to call must be AES_Init
    AES_Init();

    // The AES_Rand function is used to generate the nonce.
    // The nonce can be any number, but you must avoid ever reusing values if at all possible.
    // 
    aesResult = AES_Rand(&nonce, sizeof(nonce));
    if( aesResult != AES_RESULT_SUCCESS )
    {
        OS_TPrintf("AES_Rand failed(%d)\n", aesResult);
        goto error_exit;
    }

    // Configure the encryption key
    aesResult = AES_SetKey(&DEMO_KEY);
    if( aesResult != AES_RESULT_SUCCESS )
    {
        OS_TPrintf("AES_SetKey failed(%d)\n", aesResult);
        goto error_exit;
    }


    // Display parameters
    OS_TPrintf("---- parameter -----------\n");
    OS_TPrintf("-- key\n");
    DEMO_PrintBytes(&DEMO_KEY, sizeof(DEMO_KEY));
    OS_TPrintf("-- nonce\n");
    DEMO_PrintBytes(&nonce, sizeof(nonce));


    // Encryption
    {
        // Performs encryption in AES CCM Mode
        aesResult = AES_CcmEncryptAndSign(
                        &nonce,             // Nonce
                        pPlainText,         // Buffer where Adata and Pdata are placed, one after the next.
                        srcASize,           // Adata's size
                        srcPSize,           // Pdata's size
                        macLength,          // MAC size
                        pEncryptedC,        // Buffer that stores encrypted data
                        DEMO_AESCallback,   // Callback called at completion
                        &sMsgQ );           // Callback parameters
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmEncryptAndSign failed(%d)\n", aesResult);
            goto error_exit;
        }

        // Wait for encryption to end
        aesResult = DEMO_WaitAes(&sMsgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmEncryptAndSign failed(%d)\n", aesResult);
            goto error_exit;
        }


        // Display results
        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- plain text (ascii)\n");
        OS_PutString(pPlainTextP);   // Use OS_PutString because OS_*Printf has a limit of 256 characters
        OS_TPrintf("\n");

        OS_TPrintf("-- plain text (hex)\n");
        DEMO_PrintBytes(pPlainText, srcASize + srcPSize);

        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcASize + srcPSize + macSize);
    }

    // Decryption
    {
        // Performs decryption in AES CCM Mode
        aesResult = AES_CcmDecryptAndVerify(
                        &nonce,             // Nonce (must be the same as that used during encryption)
                        pEncrypted,         // Buffer where Adata and Cdata are placed, one after the next.
                        srcASize,           // Adata's size
                        srcPSize,           // Cdata's size
                        macLength,          // MAC size
                        pDecrypted,         // Buffer that stores decrypted data
                        DEMO_AESCallback,   // Callback called at completion
                        &sMsgQ );           // Callback parameters
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmDecryptAndVerify failed(%d)\n", aesResult);
            goto error_exit;
        }

        // Wait for decryption to end
        aesResult = DEMO_WaitAes(&sMsgQ);

        // If the encrypted data has been falsified, the aesResult obtained here is AES_RESULT_VERIFICATION_FAILED
        // 
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmDecryptAndVerify failed(%d)\n", aesResult);
            goto error_exit;
        }


        // Display results
        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcASize + srcPSize + macSize);

        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, srcPSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        OS_PutString(pDecrypted);   // Use OS_PutString because OS_*Printf has a limit of 256 characters
        OS_TPrintf("\n");
    }

    // Confirm that data that was encrypted, then decrypted, matches original
    if( MI_CpuComp8(pDecrypted, pPlainTextP, srcPSize) == 0 )
    {
        OS_TPrintf("** pDecrypted == pPlainTextP\n");
        bSuccess = TRUE;
    }
    else
    {
        OS_TPrintf("** pDecrypted != pPlainTextP\n");
    }

error_exit:
    OS_Free(pDecrypted);
    OS_Free(pEncrypted);
    OS_Free(pPlainText);

    return bSuccess;
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  The main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    BOOL bSampleSucceeded = FALSE;

    // Initialization
    OS_Init();
    DEMO_InitInteruptSystem();
    DEMO_InitAllocSystem();
    OS_TPrintf("*** start aes demo\n");

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();
    // Run demo
    bSampleSucceeded = SampleMain();

    // Display demo run results
    OS_TPrintf("*** End of demo\n");
    OS_TPrintf("demo %s\n", (bSampleSucceeded ? "succeeded": "failed"));
    GX_Init();
    *(GXRgb*)HW_BG_PLTT = (GXRgb)(bSampleSucceeded ? GX_RGB(0, 31, 0): GX_RGB(31, 0, 0));
    GX_DispOn();
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         DEMO_AESCallback

  Description:  This is the callback called at AES processing completion.
                It is of type AESCallback.

  Arguments:    result: AES processing result.
                arg:    Last argument passed in AES_Ctr

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_AESCallback(AESResult result, void* arg)
{
    OSMessageQueue* pQ = (OSMessageQueue*)arg;
    (void)OS_SendMessage(pQ, (OSMessage)result, OS_MESSAGE_BLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_WaitAes

  Description:  Returns the result of awaiting AES processing completion.
                DEMO_AESCallback must have been specified in AES_Ctr.

  Arguments:    pQ: Message queue passed as the last AES_Ctr argument

  Returns:      AES processing result.
 *---------------------------------------------------------------------------*/
static AESResult DEMO_WaitAes(OSMessageQueue* pQ)
{
    OSMessage msg;
    (void)OS_ReceiveMessage(pQ, &msg, OS_MESSAGE_BLOCK);
    return (AESResult)msg;
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_PrintBytes

  Description:  Outputs the specified binary array in hexadecimal to debug output.

  Arguments:    pvoid:  Pointer to the target binary array.
                size:   Number of bytes in the target binary array.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_PrintBytes(const void* pvoid, u32 size)
{
    const u8* p = (const u8*)pvoid;
    u32 i;

    for( i = 0; i < size; ++i )
    {
        OS_TPrintf("%02X ", p[i]);
        if( i % 16 == 15 )
        {
            OS_TPrintf("\n");
        }
    }

    if( i % 16 != 0 )
    {
        OS_TPrintf("\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitInteruptSystem

  Description:  Initializes interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_InitInteruptSystem(void)
{
    // Enable master interrupt flag
    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitAllocSystem

  Description:  Creates a heap and makes OS_Alloc usable.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_InitAllocSystem(void)
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // Initialize the main arena's allocation system
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // Create a heap in the main arena
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}
