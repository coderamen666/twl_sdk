/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ccm-compatible
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
    Declare internal variables
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

#define DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE    0xFEFF
#define DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE    AES_ADATA_SIZE_MAX  // Originally and normally 0xFFFFFFFF
#define DEMO_CCM_COMPATIBLE_2_HEADER_SIZE       2
#define DEMO_CCM_COMPATIBLE_6_HEADER_SIZE       6
#define DEMO_CCM_COMPATIBLE_6_MAGIC_NUMBER      0xFFFE

static u32  DEMO_CalcAdataHeaderSize(u32 adataSize);
static void DEMO_MakeAdataHeader(void* pv, u32 adataSize);
static AESResult    DEMO_CcmEncryptCompatible(
                        const AESKey*   pKey,
                        const AESNonce* pNonce,
                        void*           pWorkBuffer,
                        const void*     pAdata,
                        u32             adataSize,
                        const void*     pPdata,
                        u32             pdataSize,
                        AESMacLength    macLength,
                        void*           dst );
static AESResult    DEMO_CcmDecryptCompatible(
                        const AESKey*   pKey,
                        const AESNonce* pNonce,
                        void*           pWorkBuffer,
                        const void*     pAdata,
                        u32             adataSize,
                        const void*     pCdata,
                        u32             cdataSize,
                        AESMacLength    macLength,
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

    // The first AES function to call must be AES_Init
    AES_Init();

    // CCM mode encryption
    {
        const AESMacLength macLength = AES_MAC_LENGTH_16;

        const u32 macSize = AES_GetMacLengthValue(macLength);
        void* pAdata;
        void* pPdata;
        void* pEncrypted;
        void* pKey;
        void* pNonce;
        u32 adataSize;
        u32 pdataSize;

        // Load the data needed for encryption.
        // In this sample, the nonce is loaded from a file so data can be compared with data encrypted on a PC, but ordinarily you must not use a fixed value for the nonce.
        // 
        // 
        pKey    = DEMO_LoadFile(NULL, "key.bin");
        pNonce  = DEMO_LoadFile(NULL, "nonce.bin");
        pAdata  = DEMO_LoadFile(&adataSize, "sample_adata.txt");
        pPdata  = DEMO_LoadFile(&pdataSize, "sample_pdata.txt");

        if( (pdataSize % AES_BLOCK_SIZE) != 0 )
        {
            OS_TPrintf("pdataSize(=%d) must be multiple of 16.\n", pdataSize);
            return FALSE;
        }


        // Allocate the buffer to store encrypted data.
        // Only the MAC portion requires a large buffer.
        pEncrypted = OS_Alloc(pdataSize + macSize);

        if( ! SDK_IS_VALID_POINTER(pEncrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // Display parameters
        OS_TPrintf("== CCM encrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- nonce\n");
        DEMO_PrintBytes(pNonce, sizeof(AESNonce));

        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- Adata (ascii)\n");
        DEMO_PrintText(pAdata, adataSize);
        OS_TPrintf("-- Pdata (ascii)\n");
        DEMO_PrintText(pPdata, pdataSize);
        OS_TPrintf("\n");
        OS_TPrintf("-- Adata (hex)\n");
        DEMO_PrintBytes(pAdata, adataSize);
        OS_TPrintf("-- Pdata (hex)\n");
        DEMO_PrintBytes(pPdata, pdataSize);


        // Perform encryption that is compatible with the general-purpose AES libraries
        if( adataSize <= DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE )
        {
            const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
            const u32 workSize   = headerSize + adataSize + pdataSize;
            void* pWorkBuffer    = OS_Alloc(workSize);

            aesResult = DEMO_CcmEncryptCompatible(
                            pKey,               // Key
                            pNonce,             // Nonce
                            pWorkBuffer,        // Working buffer
                            pAdata,             // Pointer to buffer where Adata is stored
                            adataSize,          // Adata's size
                            pPdata,             // Pointer to buffer where Pdata is stored
                            pdataSize,          // Pdata's size
                            macLength,          // MAC size
                            pEncrypted );       // Buffer that stores encrypted data

            OS_Free(pWorkBuffer);
        }
        else
        {
            OS_TPrintf("Too huge Adata size(=%d)\n", adataSize);
            OS_Free(pEncrypted);
            OS_Free(pPdata);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CcmEncryptCompatible failed(%d)\n", aesResult);
            OS_Free(pEncrypted);
            OS_Free(pPdata);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }


        // Display results
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, pdataSize);

        OS_TPrintf("-- mac (hex)\n");
        DEMO_PrintBytes((u8*)pEncrypted + pdataSize, macSize);

        // Confirm that the data matches data encrypted on the PC
        {
            void* pEncryptedOnPC;
            u32 size;

            pEncryptedOnPC = DEMO_LoadFile(&size, "encrypted.bin");

            if( (pdataSize + macSize == size) && MI_CpuComp8(pEncrypted, pEncryptedOnPC, size) == 0 )
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
        OS_Free(pPdata);
        OS_Free(pAdata);
        OS_Free(pNonce);
        OS_Free(pKey);
    }

    // CCM Mode Decryption
    {
        const AESMacLength macLength = AES_MAC_LENGTH_16;

        const u32 macSize = AES_GetMacLengthValue(macLength);
        void* pAdata;
        void* pEncrypted;
        void* pDecrypted;
        void* pKey;
        void* pNonce;
        u32 adataSize;
        u32 cdataSize;
        u32 srcFileSize;

        // Load data encrypted on a PC.
        // In the sample, both the key and the ciphertext are in ROM, but they should normally not both be obtainable via the same method.
        // 
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pNonce      = DEMO_LoadFile(NULL, "nonce.bin");
        pAdata      = DEMO_LoadFile(&adataSize, "sample_adata.txt");
        pEncrypted  = DEMO_LoadFile(&srcFileSize, "encrypted.bin");
        cdataSize = srcFileSize - macSize;

        // Allocate the buffer to store decrypted data
        pDecrypted = OS_Alloc(cdataSize);

        if( ! SDK_IS_VALID_POINTER(pDecrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }

        // Display parameters
        OS_TPrintf("== CCM decrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- nonce\n");
        DEMO_PrintBytes(pNonce, sizeof(AESNonce));

        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- Adata (ascii)\n");
        DEMO_PrintText(pAdata, adataSize);
        OS_TPrintf("\n");
        OS_TPrintf("-- Adata (hex)\n");
        DEMO_PrintBytes(pAdata, adataSize);
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, cdataSize + macSize);

        // Performs decryption that is compatible with general-purpose AES libraries
        if( adataSize <= DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE )
        {
            const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
            const u32 workSize   = headerSize + adataSize + cdataSize + macSize;
            void* pWorkBuffer    = OS_Alloc(workSize);

            aesResult = DEMO_CcmDecryptCompatible(
                            pKey,               // Key
                            pNonce,             // Nonce
                            pWorkBuffer,        // Working buffer
                            pAdata,             // Pointer to buffer where Adata is stored
                            adataSize,          // Adata's size
                            pEncrypted,         // Pointer to buffer where ciphertext and MAC are stored
                            cdataSize,          // Ciphertext size
                            macLength,          // MAC size
                            pDecrypted );       // Buffer that stores decrypted data

            OS_Free(pWorkBuffer);
        }
        else
        {
            OS_TPrintf("Too huge Adata size(=%d)\n", adataSize);
            OS_Free(pEncrypted);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }
        // If the encrypted data has been falsified, the aesResult obtained here is AES_RESULT_VERIFICATION_FAILED
        // 
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CcmDecryptCompatible failed(%d)\n", aesResult);
            OS_Free(pDecrypted);
            OS_Free(pEncrypted);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }


        // Display results
        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, cdataSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        DEMO_PrintText(pDecrypted, cdataSize);
        OS_TPrintf("\n");

        // Confirm that it matches the original
        {
            void* pOriginal;
            u32 size;

            pOriginal = DEMO_LoadFile(&size, "sample_pdata.txt");

            if( (cdataSize == size) && MI_CpuComp8(pDecrypted, pOriginal, size) == 0 )
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
        OS_Free(pAdata);
        OS_Free(pNonce);
        OS_Free(pKey);
    }

    return bSuccess;
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main function.

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
  Name:         DEMO_CalcAdataHeaderSize

  Description:  There is no compatibility for output results between those of the TWL-SDK AES library and those of general AES libraries.
                As one example of incompatibility, sometimes the size of the Adata immediately following the CCM header is NOT automatically added.
                This function calculates the size of the region needed to store the Adata size.
                
                
                
                

  Arguments:    adataSize:      Adata's size

  Returns:      Returns the region size needed to store a size equivalent to the Adata size.
 *---------------------------------------------------------------------------*/
static u32 DEMO_CalcAdataHeaderSize(u32 adataSize)
{
    return (u32)( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE)
                    ? DEMO_CCM_COMPATIBLE_2_HEADER_SIZE:
                      DEMO_CCM_COMPATIBLE_6_HEADER_SIZE );
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_MakeAdataHeader

  Description:  There is no compatibility for output results between those of the TWL-SDK AES library and those of general AES libraries.
                As one example of incompatibility, sometimes the size of the Adata immediately following the CCM header is NOT automatically added.
                This function writes out the Adata size in the prescribed format to a buffer.
                
                
                
                

  Arguments:    pv:             Pointer to the buffer where the Adata size is written
                adataSize:      Adata's size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DEMO_MakeAdataHeader(void* pv, u32 adataSize)
{
    u8* p = (u8*)pv;

    if( adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE )
    {
        MI_StoreBE16(p, (u16)adataSize);
    }
    else
    {
        MI_StoreBE16(p + 0, DEMO_CCM_COMPATIBLE_6_MAGIC_NUMBER);
        MI_StoreBE32(p + 2, adataSize);
    }
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_CcmEncryptCompatible

  Description:  There is no compatibility for output results between those of the TWL-SDK AES library and those of general AES libraries.
                This function uses appropriate processing before and after AES processes to perform encryption compatible with general AES libraries.
                However, there is a limitation compared to AES_CcmEncryptAndSign: pdataSize must be a multiple of AES_BLOCK_SIZE (=16).
                Also, the limit on adataSize is not a multiple of 16. In some cases it will be a multiple of 16 minus 2 or a multiple of 16 minus 4.
                
                
                
                

  Arguments:    pKey:           Key to use
                pNonce:         Nonce to use
                pWorkBuffer:    Passes a pointer to the working buffer used by the function internally.
                                
                                A size equivalent to [adataSize + pdataSize + DEMO_CalcAdataHeaderSize(adataSize)] is needed.
                                         
                                The pointer must be 4-byte aligned.
                                
                pAdata: Pointer to Adata
                adataSize:      Adata size.
                                Must be a multiple of 16 minus DEMO_CalcAdataHeaderSize(adataSize).
                                
                pPdata:         Pointer to Pdata
                pdataSize:      Pdata size.
                                Must be a multiple of 16.
                macLength:      MAC size
                dst:            Pointer to the buffer where the ciphertext is written out.
                                A size equivalent to [pdataSize + DEMO_CalcAdataHeaderSize(adataSize)] is needed.
                                The pointer must be 4-byte aligned.
                                

  Returns:      Returns the processing result.
 *---------------------------------------------------------------------------*/
AESResult DEMO_CcmEncryptCompatible(
    const AESKey*   pKey,
    const AESNonce* pNonce,
    void*           pWorkBuffer,
    const void*     pAdata,
    u32             adataSize,
    const void*     pPdata,
    u32             pdataSize,
    AESMacLength    macLength,
    void*           dst )
{
    const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
    AESKey    key;
    AESNonce  nonce;

    SDK_POINTER_ASSERT(pKey);
    SDK_POINTER_ASSERT(pNonce);
    SDK_POINTER_ASSERT(pWorkBuffer);
    SDK_POINTER_ASSERT(pAdata);
    SDK_POINTER_ASSERT(pPdata);
    SDK_POINTER_ASSERT(dst);
    SDK_MAX_ASSERT( adataSize, AES_ADATA_SIZE_MAX );
    SDK_MAX_ASSERT( pdataSize, AES_PDATA_SIZE_MAX );
    SDK_ASSERT( (adataSize >  DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 14) );
    SDK_ASSERT( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 10) );
    SDK_ASSERT( (pdataSize % 16) ==  0 );

    // Create the input data
    {
        const u32 offsetHeader = 0;
        const u32 offsetAdata  = offsetHeader + headerSize;
        const u32 offsetPdata  = offsetAdata  + adataSize;

        u8* pWork = (u8*)pWorkBuffer;
        DEMO_MakeAdataHeader(pWork + offsetHeader, adataSize);  // Adata size
        MI_CpuCopy(pAdata, pWork + offsetAdata, adataSize);     // Adata
        MI_CpuCopy(pPdata, pWork + offsetPdata, pdataSize);     // Pdata
    }

    // Switch the byte order of all input
    {
        const u32 workSize = headerSize + adataSize + pdataSize;

        AES_ReverseBytes(pKey, &key, sizeof(key));                  // Key
        AES_ReverseBytes(pNonce, &nonce, sizeof(nonce));            // Nonce
        AES_SwapEndianEach128(pWorkBuffer, pWorkBuffer, workSize);  // Adata size, Adata, Pdata
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

        aesResult = AES_CcmEncryptAndSign(
                        &nonce,
                        pWorkBuffer,
                        adataSize + headerSize,
                        pdataSize,
                        macLength,
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
        void* mac = (u8*)dst + pdataSize;
        u32 macSize = AES_GetMacLengthValue(macLength);

        AES_SwapEndianEach128(dst, dst, pdataSize);     // Cdata
        AES_ReverseBytes(mac, mac, macSize);            // MAC
    }

    return AES_RESULT_SUCCESS;
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_CcmDecryptCompatible

  Description:  There is no compatibility for output results between those of the TWL-SDK AES library and those of general AES libraries.
                This function uses appropriate processing before and after AES processes to perform decryption compatible with general AES libraries.
                However, there is a limitation compared to AES_CcmDecryptAndVerify: cdataSize must be a multiple of AES_BLOCK_SIZE (=16).
                Also, the limit on adataSize is not a multiple of 16. In some cases it will be a multiple of 16 minus 2 or a multiple of 16 minus 4.
                
                
                
                

  Arguments:    pKey:           Key to use
                pNonce:         Nonce to use
                pWorkBuffer:    Passes a pointer to the working buffer used by the function internally.
                                
                                A size equivalent to [adataSize + pdataSize + DEMO_CalcAdataHeaderSize(adataSize) + AES_GetMacLengthValue(macLength)] is needed.
                                         
                                The pointer must be 4-byte aligned.         
                                
                                
                pAdata: Pointer to Adata
                adataSize:      Adata size.
                                Must be a multiple of 16 minus DEMO_CalcAdataHeaderSize(adataSize).
                                
                pPdata:         Pointer to buffer where ciphertext and MAC are stored
                pdataSize:      Size of the ciphertext.
                                Must be a multiple of 16.
                macLength:      MAC size
                dst:            Pointer to the buffer where the ciphertext is written out.
                                The pointer must be 4-byte aligned.

  Returns:      Returns the processing result.
 *---------------------------------------------------------------------------*/
AESResult DEMO_CcmDecryptCompatible(
    const AESKey*   pKey,
    const AESNonce* pNonce,
    void*           pWorkBuffer,
    const void*     pAdata,
    u32             adataSize,
    const void*     pCdata,
    u32             cdataSize,
    AESMacLength    macLength,
    void*           dst )
{
    const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
    const u32 macSize    = AES_GetMacLengthValue(macLength);
    AESKey    key;
    AESNonce  nonce;

    SDK_POINTER_ASSERT(pKey);
    SDK_POINTER_ASSERT(pNonce);
    SDK_POINTER_ASSERT(pWorkBuffer);
    SDK_POINTER_ASSERT(pAdata);
    SDK_POINTER_ASSERT(pCdata);
    SDK_POINTER_ASSERT(dst);
    SDK_MAX_ASSERT( adataSize, AES_ADATA_SIZE_MAX );
    SDK_MAX_ASSERT( cdataSize, AES_PDATA_SIZE_MAX );
    SDK_ASSERT( (adataSize >  DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 14) );
    SDK_ASSERT( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 10) );
    SDK_ASSERT( (cdataSize % 16) ==  0 );

    // Create the input data
    {
        const u32 offsetHeader = 0;
        const u32 offsetAdata  = offsetHeader + headerSize;
        const u32 offsetCdata  = offsetAdata  + adataSize;

        u8* pWork = (u8*)pWorkBuffer;
        DEMO_MakeAdataHeader(pWork + offsetHeader, adataSize);          // Adata size
        MI_CpuCopy(pAdata, pWork + offsetAdata, adataSize);             // Adata
        MI_CpuCopy(pCdata, pWork + offsetCdata, cdataSize + macSize);   // Cdata, MAC
    }

    // Switch the byte order of all input
    {
        const u32 workSize = headerSize + adataSize + cdataSize + macSize;

        AES_ReverseBytes(pKey, &key, sizeof(key));                  // Key
        AES_ReverseBytes(pNonce, &nonce, sizeof(nonce));            // Nonce
        AES_SwapEndianEach128(pWorkBuffer, pWorkBuffer, workSize);  // Adata size, Adata, Cdata, MAC
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

        aesResult = AES_CcmDecryptAndVerify(
                        &nonce,
                        pWorkBuffer,
                        adataSize + headerSize,
                        cdataSize,
                        macLength,
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
        AES_SwapEndianEach128(dst, dst, cdataSize);     // Pdata
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

  Arguments:    pvoid:  Pointer to the target binary array
                size:   Number of bytes in the target binary array

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
