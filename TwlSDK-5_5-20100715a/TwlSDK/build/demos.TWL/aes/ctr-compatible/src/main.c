/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ctr-compatible
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
    Internal variable declarations
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Internal function declarations
 *---------------------------------------------------------------------------*/

static void         DEMO_InitInteruptSystem(void);
static void         DEMO_InitAllocSystem(void);
static void         DEMO_InitFileSystem(void);
static void         DEMO_AESCallback(AESResult result, void* arg);
static AESResult    DEMO_WaitAes(OSMessageQueue* pQ);
static void         DEMO_PrintText(const void* pvoid, u32 size);
static void         DEMO_PrintBytes(const void* pvoid, u32 size);
static void*        DEMO_LoadFile(u32* pSize, const char* path);

static AESResult    DEMO_CtrCompatible(
                        const AESKey*   pKey,
                        AESCounter*     pCounter,
                        void*           src,
                        u32             srcSize,
                        void*           dst );



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
    BOOL bSuccess = FALSE;
    AESResult aesResult;

    // The first AES function called must be AES_Init.
    AES_Init();

    // CTR Mode Encryption
    {
        void* pPlaintext;
        void* pEncrypted;
        void* pKey;
        void* pCounter;
        u32 srcSize;

        // Load the data needed for encryption.
        // In this sample, the initial counter value is loaded from a file so data can be compared with data encrypted on a PC, but ordinarily you must not use a fixed value for the initial counter value.
        // 
        // 
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pCounter    = DEMO_LoadFile(NULL, "counter.bin");
        pPlaintext  = DEMO_LoadFile(&srcSize, "sample16.txt");

        // Allocate the buffer to store encrypted data.
        // Data size doesn't change with encryption.
        pEncrypted = OS_Alloc(srcSize);

        if( ! SDK_IS_VALID_POINTER(pEncrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // Display parameters
        OS_TPrintf("== CTR encrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- counter initial value\n");
        DEMO_PrintBytes(pCounter, sizeof(AESCounter));

        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- plaintext (ascii)\n");
        DEMO_PrintText(pPlaintext, srcSize);
        OS_TPrintf("-- plaintext (hex)\n");
        DEMO_PrintBytes(pPlaintext, srcSize);


        // Perform encryption that is compatible with the general-purpose AES libraries
        aesResult = DEMO_CtrCompatible(
                        pKey,               // Key
                        pCounter,           // Initial counter value
                        pPlaintext,         // Data to be encrypted
                        srcSize,            // Size of the data to encrypt
                        pEncrypted );       // Buffer that stores encrypted data
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CtrCompatible failed(%d)\n", aesResult);
            OS_Free(pEncrypted);
            OS_Free(pPlaintext);
            OS_Free(pCounter);
            OS_Free(pKey);
            return FALSE;
        }


        // Display results
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);

        // Confirm that the data matches data encrypted on the PC
        {
            void* pEncryptedOnPC;
            u32 size;

            pEncryptedOnPC = DEMO_LoadFile(&size, "encrypted.bin");

            if( (srcSize == size) && MI_CpuComp8(pEncrypted, pEncryptedOnPC, size) == 0 )
            {
                OS_TPrintf("** pEncrypted == pEncryptedOnPC\n");
            }
            else
            {
                OS_TPrintf("** pEncrypted != pEncryptedOnPC\n");
            }

            OS_Free(pEncryptedOnPC);
        }

        OS_Free(pEncrypted);
        OS_Free(pPlaintext);
        OS_Free(pCounter);
        OS_Free(pKey);
    }

    // CTR Mode Decryption
    {
        void* pEncrypted;
        void* pDecrypted;
        void* pKey;
        void* pCounter;
        u32 srcSize;

        // Load data encrypted on a PC.
        // In the sample, both the key and the ciphertext are in ROM, but they should normally not both be obtainable via the same method.
        // 
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pCounter    = DEMO_LoadFile(NULL, "counter.bin");
        pEncrypted  = DEMO_LoadFile(&srcSize, "encrypted.bin");

        // Allocate the buffer to store decrypted data
        // Data size doesn't change with decryption.
        pDecrypted = OS_Alloc(srcSize);

        if( ! SDK_IS_VALID_POINTER(pDecrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // Display parameters
        OS_TPrintf("== CTR decrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- counter initial value\n");
        DEMO_PrintBytes(pCounter, sizeof(AESCounter));

        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);


        // Performs decryption that is compatible with general-purpose AES libraries
        aesResult = DEMO_CtrCompatible(
                        pKey,               // Key (must be the same as that used during encryption)
                        pCounter,           // Initial counter value (must be the same as that used during encryption)
                        pEncrypted,         // Data to be decrypted
                        srcSize,            // Size of the data to decrypt
                        pDecrypted );       // Buffer that stores decrypted data
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CtrCompatible failed(%d)\n", aesResult);
            OS_Free(pDecrypted);
            OS_Free(pEncrypted);
            OS_Free(pCounter);
            OS_Free(pKey);
            return FALSE;
        }


        // Display results
        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, srcSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        DEMO_PrintText(pDecrypted, srcSize);
        OS_TPrintf("\n");

        // Confirm that it matches the original
        {
            void* pOriginal;
            u32 size;

            pOriginal = DEMO_LoadFile(&size, "sample16.txt");

            if( (srcSize == size) && MI_CpuComp8(pDecrypted, pOriginal, size) == 0 )
            {
                OS_TPrintf("** pDecrypted == pOriginal\n");
                bSuccess = TRUE;
            }
            else
            {
                OS_TPrintf("** pDecrypted != pOriginal\n");
            }

            OS_Free(pOriginal);
        }


        OS_Free(pDecrypted);
        OS_Free(pEncrypted);
        OS_Free(pCounter);
        OS_Free(pKey);
    }

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
    DEMO_InitFileSystem();
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
  Name:         DEMO_CtrCompatible

  Description:  There is no compatibility for output results between those of the TWL-SDK AES library and those of general AES libraries.
                This function uses appropriate processing before and after AES processes to perform encryption/decryption compatible with general AES libraries.
                However, there is a limitation compared to AES_Ctr: srcSize must be a multiple of AES_BLOCK_SIZE (=16).
                
                
                
                

  Arguments:    pKey:       Key to use
                pCounter:   Initial counter value to use
                src:        Pointer to either the plaintext or the ciphertext
                            The pointer must be 4-byte aligned.
                srcSize:    Size of the plaintext or ciphertext.
                            Must be a multiple of 16.
                dst:        Pointer to the buffer where the plaintext or ciphertext is written out.
                            
                            The pointer must be 4-byte aligned.

  Returns:      Returns the processing result.
 *---------------------------------------------------------------------------*/
static AESResult DEMO_CtrCompatible(
    const AESKey*       pKey,
    AESCounter*         pCounter,
    void*               src,
    u32                 srcSize,
    void*               dst )
{
    AESKey          key;

    // Switch the byte order of all input
    {
        AES_ReverseBytes(pKey, &key, sizeof(key));
        AES_ReverseBytes(pCounter, pCounter, sizeof(*pCounter));
        AES_SwapEndianEach128(src, src, srcSize);
    }

    // AES processing
    {
        OSMessageQueue  msgQ;
        OSMessage       msgQBuffer[1];
        AESResult       aesResult;

        OS_InitMessageQueue(&msgQ, msgQBuffer, sizeof(msgQBuffer)/sizeof(*msgQBuffer));

        aesResult = AES_SetKey(&key);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }

        aesResult = AES_Ctr( pCounter,
                             src,
                             srcSize,
                             dst,
                             DEMO_AESCallback,
                             &msgQ );
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }

        aesResult = DEMO_WaitAes(&msgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }
    }

    // Switch the output byte order, too
    {
        AES_SwapEndianEach128(dst, dst, srcSize);
    }

    return AES_RESULT_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         DEMO_LoadFile

  Description:  Allocates memory and loads a file.

  Arguments:    pSize:  Pointer to the buffer that stores the size of the file that was loaded.
                        Specify NULL when size is unnecessary.
                path:   Path of the file to load

  Returns:      Returns a pointer to the buffer where the file content is stored.
                This buffer must be deallocated with the OS_Free function when it is no longer needed.
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
static void* DEMO_LoadFile(u32* pSize, const char* path)
{
    BOOL bSuccess;
    FSFile f;
    u32 fileSize;
    s32 readSize;
    void* pBuffer;

    FS_InitFile(&f);

    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);
    if( ! bSuccess )
    {
        OS_Warning("fail to FS_OpenFileEx(%s)\n", path);
        return NULL;
    }

    fileSize = FS_GetFileLength(&f);
    pBuffer = OS_Alloc(fileSize + 1);
    if( pBuffer == NULL )
    {
        (void)FS_CloseFile(&f);
        OS_Warning("fail to OS_Alloc(%d)\n", fileSize + 1);
        return NULL;
    }

    readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
    if( readSize != fileSize )
    {
        OS_Free(pBuffer);
        (void)FS_CloseFile(&f);
        OS_Warning("fail to FS_ReadFile(%d)\n", fileSize + 1);
        return NULL;
    }

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );

    ((char*)pBuffer)[fileSize] = '\0';

    if( pSize != NULL )
    {
        *pSize = fileSize;
    }

    return pBuffer;
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

  Description:  Waits for AES processing completion and returns results.
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
  Name:         DEMO_PrintText

  Description:  Prints a string of specified length to debug output.

  Arguments:    pvoid:  Target string
                size:   Target string byte count

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_PrintText(const void* pvoid, u32 size)
{
    static const u32 TMP_BUFFER_LEN = 128 - 1;
    char tmp_buffer[TMP_BUFFER_LEN + 1];
    const u8* p = (const u8*)pvoid;

    while( size >= TMP_BUFFER_LEN )
    {
        MI_CpuCopy8(p, tmp_buffer, TMP_BUFFER_LEN);
        tmp_buffer[TMP_BUFFER_LEN] = '\0';
        OS_PutString(tmp_buffer);

        size -= TMP_BUFFER_LEN;
        p    += TMP_BUFFER_LEN;
    }

    if( size > 0 )
    {
        MI_CpuCopy8(p, tmp_buffer, size);
        tmp_buffer[size] = '\0';
        OS_PutString(tmp_buffer);
    }
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

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitFileSystem

  Description:  Initializes the file system and makes the ROM accessible.
                The DEMO_InitInteruptSystem and DEMO_InitAllocSystem functions must have been called before calling this function.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_InitFileSystem(void)
{
    void* p_table;
    u32 need_size;

    // Enables interrupts for the communications FIFO with the ARM7
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );

    // File table cache
    need_size = FS_GetTableSize();
    p_table = OS_Alloc(need_size);
    SDK_POINTER_ASSERT(p_table);
    (void)FS_LoadTable(p_table, need_size);
}
