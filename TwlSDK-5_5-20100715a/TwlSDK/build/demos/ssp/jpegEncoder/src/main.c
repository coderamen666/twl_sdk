/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - demos - jpegEncoder
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-27#$
  $Rev: 10467 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

// The output buffer and the JPEG file that is actually created have the same size
#define OUTPUT_DATA_SIZE 307200

static u8 encodeOutputBuffer[OUTPUT_DATA_SIZE] ATTRIBUTE_ALIGN(4);
static u32 encodeOutputSize;

#include <twl/ssp/ARM9/jpegenc.h>
#include <twl/ssp/ARM9/exifenc.h>


static u8* decodeOutputBuffer;

#include <twl/ssp/ARM9/jpegdec.h>
#include <twl/ssp/ARM9/exifdec.h>

#include "DEMO.h"

static const char* sDebugMessageFunc[4] = 
{
    "SSP_StartJpegEncoder",
    "SSP_StartJpegEncoderWithEncodeData",
    "SSP_StartJpegEncoderLite",
    "SSP_StartJpegEncoderFast"
};

/*---------------------------------------------------------------------------*
  Name:         MyAlloc

  Description:  OS_Alloc wrapper function.
 *---------------------------------------------------------------------------*/
static void* MyAlloc(u32 size)
{
    void* ptr;
    
    ptr = OS_Alloc(size);
    
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         MyFree

  Description:  OS_Free wrapper function.
 *---------------------------------------------------------------------------*/
static void MyFree(void* ptr)
{
    OS_Free(ptr);
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


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlankIntr

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain / NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int retryCount=0;

    /* OS initialization */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();

    RTC_Init();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);

    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    // SD initialization
    FS_Init( FS_DMA_NOT_USE );

    InitializeAllocateSystem();

    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("    SSP JpegEncoder Test\n");
    OS_TPrintf("===================================\n");
    
    do
    {
        {
            u8 *data;
            u32 length;
            FSFile file[1];
            
            u8 dateTime[20];
            
            OSTick jpegEncode_begin,jpegEncode_end;
            
            OS_TPrintf("Start Encode ...\n");
            
            jpegEncode_begin = OS_GetTick();

            {
                OSTick core_begin, core_end;
                u16* yuvBuf = NULL;
                u8* tmpBuf;
                
                static char userData[32] ATTRIBUTE_ALIGN(32);
                
                FS_InitFile(file);
                if( !FS_OpenFileEx(file, "rom:/rgb555.dat", FS_FILEMODE_R) )
                {
                    OS_Panic("failed FS_OpenFileEx");
                }
                length = FS_GetFileLength(file);
                data = MyAlloc(length);
                if(data == 0)
                {
                    OS_Panic("Cannot allocate memory ... data!");
                }
                if( FS_ReadFile(file, data, (s32)length) == -1)
                {
                    OS_Panic("failed FS_ReadFile");
                }
                (void)FS_CloseFile(file);
                
                {
                    const u32 option = SSP_JPEG_THUMBNAIL | SSP_JPEG_RGB555;
                    
                    MI_CpuFill8(userData, 0x10, 32);
                    MI_CpuCopy8("USER_DATA", userData, 9);
                    userData[9] = '\0';
                    
                    SSP_SetJpegEncoderMakerNoteEx(SSP_MAKERNOTE_USER, (u8*)userData, 32);
                    
                    if(retryCount < 2)
                    {
                        tmpBuf = (u8*)MyAlloc(SSP_GetJpegEncoderBufferSize(256,192,SSP_JPEG_OUTPUT_YUV422,option));
                        OS_TPrintf("SSP_GetJpegEncoderBufferSize()=%d\n", SSP_GetJpegEncoderBufferSize(256,192,SSP_JPEG_OUTPUT_YUV422,option));
                    }
                    else if(retryCount == 2)
                    {
                        tmpBuf = (u8*)MyAlloc(SSP_GetJpegEncoderLiteBufferSize(option));
                        OS_TPrintf("SSP_GetJpegEncoderLiteBufferSize()=%d\n", SSP_GetJpegEncoderLiteBufferSize(option));
                    }
                    else
                    {
                        tmpBuf = (u8*)MyAlloc(SSP_GetJpegEncoderFastBufferSize(option));
                        OS_TPrintf("SSP_GetJpegEncoderFastBufferSize()=%d\n", SSP_GetJpegEncoderFastBufferSize(option));
                    }
                    
                    if ( !SSP_SetJpegEncoderDateTimeNow() )
                    {
                        MI_CpuFill8(dateTime, 0x00, 20);
                        SSP_SetJpegEncoderDateTime(dateTime);
                    }
                    
                    core_begin = OS_GetTick();
                    
                    switch (retryCount)
                    {
                    case 0:
                        encodeOutputSize = SSP_StartJpegEncoder(data, encodeOutputBuffer, OUTPUT_DATA_SIZE, tmpBuf, 256, 192, 90, SSP_JPEG_OUTPUT_YUV422, option);
                        MyFree(data);
                        break;
                    case 1:
                        if(SSP_ConvertJpegEncodeData(data, tmpBuf, 256, 192, SSP_JPEG_OUTPUT_YUV422, option) == FALSE)
                            OS_Panic("failed SSP_ConvertRGB555");                
                        MyFree(data);
                        encodeOutputSize = SSP_StartJpegEncoderWithEncodeData(encodeOutputBuffer, OUTPUT_DATA_SIZE, tmpBuf, 256, 192, 90, SSP_JPEG_OUTPUT_YUV422, option);
                        break;
                    case 2:
                        encodeOutputSize = SSP_StartJpegEncoderLite(data, encodeOutputBuffer, OUTPUT_DATA_SIZE, tmpBuf, 256, 192, 90, SSP_JPEG_OUTPUT_YUV422, option);
                        MyFree(data);
                        break;
                    case 3:
                        encodeOutputSize = SSP_StartJpegEncoderFast(data, encodeOutputBuffer, OUTPUT_DATA_SIZE, tmpBuf, 256, 192, 90, SSP_JPEG_OUTPUT_YUV422, option);
                        MyFree(data);
                        break;
                    }
                    
                    core_end = OS_GetTick();
                    
                    OS_TPrintf("[TIMER] Core %s time[usec] = %d\n", sDebugMessageFunc[retryCount], OS_TicksToMicroSeconds(core_end - core_begin));

                    MyFree( tmpBuf );
                }
            }

            jpegEncode_end = OS_GetTick();
            
            OS_TPrintf("[TIMER] %s time[usec] = %d\n", sDebugMessageFunc[retryCount], OS_TicksToMicroSeconds(jpegEncode_end - jpegEncode_begin));
            OS_TPrintf("encodeOutputSize=%d\n", encodeOutputSize);
        }
        
        {
            s16 decode_file_width, decode_file_height;
            
            OSTick jpegDecode_begin,jpegDecode_end;
            BOOL result;
            
            OS_TPrintf("Start Decode ... encodeOutputSize = %d\n", encodeOutputSize);
            
            jpegDecode_begin = OS_GetTick();

            decode_file_width = 256;
            decode_file_height = 192;
            decodeOutputBuffer = MyAlloc(decode_file_width * decode_file_height * sizeof(u16));
            
            result = SSP_StartJpegDecoder(encodeOutputBuffer, encodeOutputSize, decodeOutputBuffer, &decode_file_width, &decode_file_height, SSP_JPEG_RGB555);
            
            jpegDecode_end = OS_GetTick();
            
            if(result == 0)
                OS_Panic("Failed Cpu Decode\n");
            
            OS_TPrintf("[TIMER] MAIN SSP_StartJpegDecoder time[usec] = %d\n", OS_TicksToMicroSeconds(jpegDecode_end - jpegDecode_begin));
            OS_TPrintf("[DEBUG] MAIN decode_file_width = %d : decode_file_height = %d\n", decode_file_width, decode_file_height);
            
            {
                int i,j;
                float i_step,j_step;
                int i_count,j_count;
                s16 t_width = decode_file_width;
                
                j_step = (float)decode_file_width / 256;
                i_step = (float)decode_file_height / 192;
                
                i_count = j_count = 0;
                for(i = 0; i < 192; i++)
                {
                    for(j = 0; j < 256; j++)
                    {
                        *((u16*)((int)G2_GetBG3ScrPtr() + i*256*2 + j*2)) = *((u16*)((u32)decodeOutputBuffer + (u32)(i_count * decode_file_width*2) + (u32)(j_count*2)));
                        j_count = (int)((double)j*j_step);
                    }
                    i_count=(int)((double)i*i_step);
                }
            }
            
            // Try to display the date and time
            {
                u8 dateTime[20];
                
                (void)SSP_GetJpegDecoderDateTime(dateTime);
                OS_TPrintf("DateTimeOriginal = %s\n", dateTime);
            }
            {
                char software[30];
                u32 initialCode;
                
                (void)SSP_GetJpegDecoderSoftware(software);
                // The Exif software tag contains a little-endian game code
                MI_CpuCopy8(software, &initialCode, 4);
                OS_TPrintf("InitialCode = %c%c%c%c\n", initialCode>>24, initialCode>>16, initialCode>>8, initialCode);
            }
            // Get the MakerNote address and size
            {
                u8 *makerNotePhotoAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_PHOTO);
                u16 makerNotePhotoSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_PHOTO);
                u8 *makerNoteUserAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_USER);
                u16 makerNoteUserSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_USER);
                
                OS_TPrintf("makerNotePhotoAddr = 0x%X\n", makerNotePhotoAddr);
                OS_TPrintf("makerNotePhotoSize = 0x%X\n", makerNotePhotoSize);
                OS_TPrintf("MakerNoteUserAddr = 0x%X\n", makerNoteUserAddr);
                OS_TPrintf("MakerNoteUserSize = 0x%X\n", makerNoteUserSize);
            }
            MyFree(decodeOutputBuffer);
        }

        if(retryCount < 3)
        {
            OS_TPrintf("Press A button\n");
            while(1)
            {
                DEMOReadKey();
                if(DEMO_IS_PRESS(PAD_BUTTON_A))
                {
                    retryCount++;
                    break;
                }
                OS_WaitVBlankIntr();
            }
        }
        else
            retryCount++;
    }while(retryCount < 4);
    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("    End\n");
    OS_TPrintf("===================================\n");
    OS_Terminate();
}
