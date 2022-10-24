/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - file_io - nitro
  File:     main.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1316 $
 *---------------------------------------------------------------------------*/

// ============================================================================
//  Explanation of the demo:
//      This uses the MCS library to read, write, and list files on a PC.
//      
//      The specific actions are explained below:
//
//        1. Reads the build\demos\mcs\file_io\nitro\data\test.txt file from the TWL-System root directory, reverses its contents, and then writes it out as a separate file: build\demos\mcs\file_io\nitro\data\test_wr.txt.
//             
//           
//             
//           
//        2. Searches for files with a .txt extension in the build\demos\mcs\file_io\nitro\data directory, which is directly under the TWL-System root directory.
//             
//           
//
// ============================================================================

#include <nitro.h>
#include <extras.h>

#include <nnsys/fnd.h>
#include <nnsys/mcs.h>
#include <nnsys/misc.h>

// The directory for this project
#define PROJECT_DIR "%TWLSYSTEM_ROOT%\\build\\demos\\mcs\\file_io\\nitro"

// The structure for managing the file data
typedef struct FileData FileData;
struct FileData
{
    char*   buf;
    u32     size;
};

static NNSFndHeapHandle gExpHeap;

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  The V-Blank interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);

    NNS_McsVBlankInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         CartIntrFunc

  Description:  Cartridge interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
CartIntrFunc(void)
{
    OS_SetIrqCheckFlag(OS_IE_CARTRIDGE);

    NNS_McsCartridgeInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         InitInterrupt

  Description:  Performs the initialization related to interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitInterrupt()
{
    (void)OS_DisableIrq();

    // Enable V-Blank interrupts and set so NNS_McsVBlankInterrupt() is called from inside the V-Blank interrupt
    // 
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // Enable cartridge interrupts and configure NNS_McsCartridgeInterrupt() to be called from within a cartridge interrupt
    // 
    OS_SetIrqFunction(OS_IE_CARTRIDGE, CartIntrFunc);
    (void)OS_EnableIrqMask(OS_IE_CARTRIDGE);

    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         SampleFileRead

  Description:  Reads the contents of the file.

  Arguments:    pFileData:  Pointer to the structure where the file information is stored.

  Returns:      TRUE if the read succeeds. FALSE if it fails.
 *---------------------------------------------------------------------------*/
static BOOL
SampleFileRead(FileData* pFileData)
{
    void* pDataBuf = 0;
    BOOL bSuccess = FALSE;
    NNSMcsFile info;
    u32 errCode;
    u32 readSize;

    NNS_McsPrintf("read test.txt\n");

    // Open the file
    errCode = NNS_McsOpenFile(
        &info,
        PROJECT_DIR "\\data\\test.txt",
            NNS_MCS_FILEIO_FLAG_READ
          | NNS_MCS_FILEIO_FLAG_INCENVVAR); // Expand the PC's environment variables string
    if (errCode)
    {
        NNS_McsPrintf("error : NNS_McsOpenFile() test.txt .\n");
        return FALSE;
    }

    // Allocate a buffer for loading the entire file.
    // Entire file regarded as string, so secure an extra 1 byte for the termination string.
    pDataBuf = NNS_FndAllocFromExpHeap(gExpHeap, NNS_McsGetFileSize(&info) + 1);
    NNS_NULL_ASSERT(pDataBuf);

    // Load file
    errCode = NNS_McsReadFile(&info, pDataBuf, NNS_McsGetFileSize(&info), &readSize);
    if (0 == errCode)
    {
        pFileData->buf = pDataBuf;
        pFileData->size = readSize;
        pFileData->buf[readSize] = '\0';
        bSuccess = TRUE;
    }
    else
    {
        // Release memory because loading failed
        NNS_FndFreeToExpHeap(gExpHeap, pDataBuf);
    }

    // Closes the file
    (void)NNS_McsCloseFile(&info);

    return bSuccess;
}

/*---------------------------------------------------------------------------*
  Name:         SampleFileWrite

  Description:  Writes to a file.

  Arguments:    pFileData:  Pointer to the structure where the file information is stored.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleFileWrite(FileData* pFileData)
{
    NNSMcsFile info;
    u32 errCode;

    NNS_McsPrintf("write test_wr.txt\n");

    // Open the file
    errCode = NNS_McsOpenFile(
        &info,
        PROJECT_DIR "\\data\\test_wr.txt",
            NNS_MCS_FILEIO_FLAG_WRITE       // Write mode
          | NNS_MCS_FILEIO_FLAG_FORCE       // Overwrite if file exists
          | NNS_MCS_FILEIO_FLAG_INCENVVAR); // Expand the PC's environment variables string
    if (errCode)
    {
        NNS_McsPrintf("error : NNS_McsOpenFile() test_wr.txt .\n");
        return;
    }

    // Write to the file
    errCode = NNS_McsWriteFile(&info, pFileData->buf, pFileData->size);
    if (errCode)
    {
        NNS_McsPrintf("error : NNS_McsWriteFile().\n");
    }

    // Closes the file
    (void)NNS_McsCloseFile(&info);
}

/*---------------------------------------------------------------------------*
  Name:         SampleFileIO

  Description:  Reads data from a file, reverses it, and writes it out to a separate file.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleFileIO()
{
    BOOL bSuccess = FALSE;
    FileData fileData;

    // Read the file
    bSuccess = SampleFileRead(&fileData);

    if (bSuccess)
    {
        // Invert the content
        (void)strrev(fileData.buf);

        // Write to a file
        SampleFileWrite(&fileData);
        NNS_FndFreeToExpHeap(gExpHeap, fileData.buf);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SampleFind

  Description:  Lists files that have a .txt extension.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleFind()
{
    NNSMcsFile info;
    NNSMcsFileFindData findData;
    u32 errCode;

    // Search for files with the same pattern
    errCode = NNS_McsFindFirstFile(
        &info,
        &findData,
        PROJECT_DIR "\\data\\*.txt");

    // When no equivalent file can be found:
    if (errCode == NNS_MCS_FILEIO_ERROR_NOMOREFILES)
    {
        NNS_McsPrintf("not match *.txt .\n");
        return;
    }

    // Other errors:
    if (errCode != 0)
    {
        NNS_McsPrintf("error : NNS_McsFindFirstFile()\n");
        return;
    }

    // Search for the next file with the same pattern.
    // Loop until the NNS_McsFindNextFile function returns a non-zero value.
    // NNS_MCS_FILEIO_ERROR_NOMOREFILES is returned when there are no more matching files.
    // 
    do
    {
        NNS_McsPrintf("find filename %s %dbytes.\n", findData.name, findData.size);
        errCode = NNS_McsFindNextFile(&info, &findData);
    }while (0 == errCode);

    if (errCode != NNS_MCS_FILEIO_ERROR_NOMOREFILES)
    {
        NNS_McsPrintf("error : NNS_McsFindNextFile()\n");
    }

    (void)NNS_McsCloseFind(&info);
}

void
NitroMain(void)
{
    u8* mcsWorkMem;
    NNSMcsDeviceCaps deviceCaps;
    u8* mcsFileIOWorkMem;
    void* printBuffer;

    OS_Init();

    InitInterrupt();                // Interrupt-related initialization

    /*
        Allocate all of the main arena to the expanded heap
    */
    {
        void *const arenaLo = OS_GetArenaLo(OS_ARENA_MAIN);

        OS_SetArenaLo(OS_ARENA_MAIN, OS_GetArenaHi(OS_ARENA_MAIN));

        gExpHeap = NNS_FndCreateExpHeap(arenaLo, (u32)OS_GetArenaHi(OS_ARENA_MAIN) - (u32)arenaLo);
    }

    // Initialize MCS
    mcsWorkMem = NNS_FndAllocFromExpHeapEx(gExpHeap, NNS_MCS_WORKMEM_SIZE, 4); // allocate the MCS work memory
    NNS_McsInit(mcsWorkMem);

    if (NNS_McsGetMaxCaps() > 0)
    {
        // Open the device
        if (NNS_McsOpen(&deviceCaps))
        {
            // Wait for the MCS server to connect because it may be disconnected during communications with a DS system using IS-NITRO-UIC
            // 
            // 
            if (! NNS_McsIsServerConnect())
            {
                OS_Printf("Waiting server connect.\n");
                do
                {
                    SVC_WaitVBlankIntr();
                }while (! NNS_McsIsServerConnect());
            }

            // Initialize MCS string output
            printBuffer = NNS_FndAllocFromExpHeap(gExpHeap, 1024);        // Allocate buffer for printing
            NNS_McsInitPrint(printBuffer, NNS_FndGetSizeForMBlockExpHeap(printBuffer));

            // Initialize the file input/output features
            mcsFileIOWorkMem = NNS_FndAllocFromExpHeapEx(gExpHeap, NNS_MCS_FILEIO_WORKMEM_SIZE, 4);     // Allocate the MCS File I/O work memory
            NNS_McsInitFileIO(mcsFileIOWorkMem);

            SampleFileIO();
            SampleFind();

            NNS_McsFinalizeFileIO();
            NNS_FndFreeToExpHeap(gExpHeap, mcsFileIOWorkMem);

            NNS_McsFinalizePrint();
            NNS_FndFreeToExpHeap(gExpHeap, printBuffer);

            // Close the device
            (void)NNS_McsClose();
        }
        else
        {
            OS_Printf("device open fail.\n");
        }
    }
    else
    {
        OS_Printf("device not found.\n");
    }

    NNS_McsFinalize();
    NNS_FndFreeToExpHeap(gExpHeap, mcsWorkMem);

    while (TRUE) {}
}

