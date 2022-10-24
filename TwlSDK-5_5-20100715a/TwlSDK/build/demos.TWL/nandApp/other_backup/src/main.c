/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - backup
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/na.h>
#include <DEMO.h>
static void PrintBootType();
static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

static char* LoadFile(const char* path);
static BOOL SaveFile(const char* path, void* pData, u32 size);

static void PrintDirectory(const char* path);
static void CreateTree(const char* arc_name);
static void DeleteData(char *path);
static BOOL WriteData(const char *path, void* pData, u32 size);
static void ReadData(const char* arc_name);

static char* GetTestData(char *out, u32 size);
static void DrawString(const char *fmt, ...);
static void PrintTree(const char* path, u32 space);
static void MountOtherSaveData(const char *code, BOOL flag);

static const u32 BUF_SIZE = 256;

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    RTC_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
    DrawString("*** start nandApp demo\n");

    DrawString("Build:%s %s\n", __DATE__, __TIME__);
    PrintBootType();
    DrawString("\n");
    DrawString("A - Delete, Create, Read\n");
    DrawString("B - Print Directory Tree\n");

    // It is possible for NAND applications to access the save data of other NAND applications. 
    // 
    //  
    // The NA_LoadOtherTitleArchive function can be used to mount the save data of another NAND application and access it as archives named "otherPub:" and "otherPrv:".
    // 
    // Only one region can be mounted at a time, however.
    //
    // To mount other regions, you must call the NA_UnloadOtherTitleArchive function to unmount the current save data.
    // 
    // 
    // This program accesses the backup region of the 'backup' demo. It provides examples of the following operations: 
	//
    // 1. Loading files
    // 2. Deleting backup data
    // 3. Creating directory trees and files
    //  
    
    // "dataPub" is the archive name of the backup data region copied to the SD card.
    //  
    // This is used as general data.

    // "dataPrv" is the archive name of the backup data region that is not copied to the SD card.
    //  
    // This is used to save data you do not want copied.
    {
        u32 mode = 0;
        const char code[5] = "4BNA";
        DrawString("Target App Code = %s\n", code);
        // To make the hardware reset valid, do not terminate
        for (;;)
        {
            // Update the frame
            DEMOReadKey();
            if(DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                DrawString("");
                switch(mode)
                {
                    case 0: // Execute loading of files
                        DrawString("Read\n");

                        MountOtherSaveData(code, TRUE);
                        ReadData("otherPub:");

                        MountOtherSaveData(code, FALSE);
                        ReadData("otherPrv:");
                        break;
                    case 1: // Execute delete
                        DrawString("Delete\n");

                        MountOtherSaveData(code, TRUE);
                        DeleteData("otherPub:");

                        MountOtherSaveData(code, FALSE);
                        DeleteData("otherPrv:");
                        break;
                    case 2: // Execute creation of directories and files
                        DrawString("Create\n");

                        MountOtherSaveData(code, TRUE);
                        CreateTree("otherPub:");

                       MountOtherSaveData(code, FALSE);
                        CreateTree("otherPrv:");
                        break;
                }
                mode = (mode + 1) % 3;
            }else if(DEMO_IS_TRIG(PAD_BUTTON_B))
            {
                // Display the directory tree
                DrawString("");

                DrawString("Tree\notherPub:\n");

                MountOtherSaveData(code, TRUE);
                PrintTree("otherPub:", 1);

                DrawString("\notherPrv:\n");

                MountOtherSaveData(code, FALSE);
                PrintTree("otherPrv:", 1);
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  Internally allocates memory and loads a file.

  Arguments:    path:   Path of the file to load

  Returns:      If the file exists, returns a pointer to the internally allocated buffer into which the file content was loaded.
                
                This pointer must be deallocated with the FS_Free function.
 *---------------------------------------------------------------------------*/
static char* LoadFile(const char* path)
{
    FSFile f;
    BOOL bSuccess;
    char* pBuffer;
    u32 fileSize;
    s32 readSize;

    FS_InitFile(&f);

    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);
    if( ! bSuccess )
    {
        return NULL;
    }

    fileSize = FS_GetFileLength(&f);
    pBuffer = (char*)OS_Alloc(fileSize + 1);
    SDK_POINTER_ASSERT(pBuffer);

    readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
    SDK_ASSERT( readSize == fileSize );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );

    pBuffer[fileSize] = '\0';
    return pBuffer;
}

/*---------------------------------------------------------------------------*
  Name:         SaveFile

  Description:  Creates a file and writes data to it.
                Associated directories are not created.

  Arguments:    path:   Path of the file to create
                pData:  Data to be written
                size:   Total size of data to be written

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
static BOOL SaveFile(const char* path, void* pData, u32 size)
{
    FSFile f;
    BOOL bSuccess;
    FSResult fsResult;
    s32 writtenSize;

    FS_InitFile(&f);

    (void)FS_CreateFile(path, (FS_PERMIT_R|FS_PERMIT_W));
    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_W);
    if (bSuccess == FALSE)
    {
        FSResult res = FS_GetArchiveResultCode(path);
        DrawString("Failed create file:%d\n", res);
        return FALSE;
    }
    SDK_ASSERT( bSuccess );

    fsResult = FS_SetFileLength(&f, 0);
    SDK_ASSERT( fsResult == FS_RESULT_SUCCESS );

    writtenSize = FS_WriteFile(&f, pData, (s32)size);
    SDK_ASSERT( writtenSize == size );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );
    return TRUE;
}




/*---------------------------------------------------------------------------*
  Name:         CreateTree

  Description:  Creates a directory tree.
                
  Arguments:    arc_name: Target archive

  Returns:      None. 
 *---------------------------------------------------------------------------*/
static void CreateTree(const char* arc_name){
    char *dir_path[] = {
        "/",
        "/testDir/", 
        "/testDir2/test/", 
    };
    char *filename = "test";
    
    u32 PATH_COUNT = 3;
    u32 FILE_COUNT = 2;
    char buf[BUF_SIZE];
    BOOL bSuccess;

    
    DrawString("Create:%s\n", arc_name);
    // Generate directory
    {
        u32 i = 0, j = 0;
        for(i = 1; i < PATH_COUNT; ++i)
        {
            (void)STD_TSNPrintf(buf, BUF_SIZE, "%s%s", arc_name, dir_path[i]);

            bSuccess = FS_CreateDirectoryAuto(buf, FS_PERMIT_W | FS_PERMIT_R);
            SDK_ASSERT(bSuccess);
            DrawString("  %s\n", buf);
        }
    }

    // Generate file
    {
        u32 i = 0, j = 0;
        for(i = 0; i < PATH_COUNT; ++i)
        {
            for(j = 0; j < FILE_COUNT; ++j)
            {
                char data[BUF_SIZE];
                (void)STD_TSNPrintf(buf, BUF_SIZE, "%s%s%s%d", arc_name, dir_path[i], filename, j);
                (void)SaveFile(buf, GetTestData(data, BUF_SIZE), BUF_SIZE);
                DrawString("  %s\n", buf);
            }
        }
    }
    DrawString("\n");    
}


/*---------------------------------------------------------------------------*
  Name:         DeleteData

  Description:  Deletes all files and directories within a directory.
                
  Arguments:    path: Target directory path

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DeleteData(char *path)
{
    FSFile fp;
    FSDirectoryEntryInfo entry;
    char buf[BUF_SIZE];
    BOOL bSuccess;
    BOOL bDeleted = FALSE;
    DrawString("Delete:%s\n", path);
    FS_InitFile(&fp);
    bSuccess = FS_OpenDirectory(&fp, path, FS_PERMIT_W | FS_PERMIT_R);
    if(!bSuccess)
    {
        DrawString("Failed Open Directory\n");
        return;
    }

    while(FS_ReadDirectory(&fp, &entry))
    {
        if(!STD_StrCmp(".", entry.longname) || !STD_StrCmp("..", entry.longname))
            continue;

        (void)STD_TSNPrintf(buf, BUF_SIZE, "%s/%s", path, entry.longname);
        if(entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
        {
            // For directories:
            bSuccess = FS_DeleteDirectoryAuto(buf);
        }
        else
        {
            // For files:
            bSuccess = FS_DeleteFile(buf);
        }
        if(!bSuccess)
        {
            DrawString("Failed Delete %s\n", buf);
            continue;
        }
        else
        {
            DrawString("  %s\n", buf);
            bDeleted = TRUE;
        }
    }
    bSuccess = FS_CloseDirectory(&fp);
    SDK_ASSERT(bSuccess);
    
    if(!bDeleted)
    {
        DrawString("No File\n");
    }
    
    DrawString("\n");
}

/*---------------------------------------------------------------------------*
  Name:         GetTestData

  Description:  Creates data to write to test files.
                
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static char* GetTestData(char* out, u32 size)
{
    RTCResult rtcResult;
    RTCDate date;
    RTCTime time;

    // Generate content to write to a file
    rtcResult = RTC_GetDateTime(&date, &time);
    SDK_ASSERT( rtcResult == RTC_RESULT_SUCCESS );

    (void)STD_TSNPrintf(out, size,
        "Hello. %04d/%02d/%02d %02d:%02d:%02d\n",
        date.year + 2000,
        date.month,
        date.day,
        time.hour,
        time.minute,
        time.second );
    
    return out;
}

/*---------------------------------------------------------------------------*
  Name:         ReadData

  Description:  Traverses within a directory and displays the content of the first found file.
                
  Arguments:    path: Target directory path

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadData(const char* arc_name)
{
    FSFile fp;
    FSDirectoryEntryInfo entry;
    
    char buf[BUF_SIZE];
    BOOL bSuccess;
    BOOL bRead = FALSE;
    
    DrawString("Read:%s\n", arc_name);
    // Open directory
    FS_InitFile(&fp);
    bSuccess = FS_OpenDirectory(&fp, arc_name, FS_PERMIT_W | FS_PERMIT_R);
    if(!bSuccess)
    {
        DrawString("Failed Open Directory");
        return;
    }    

    // Work within the directory and display content of first found file
    while(FS_ReadDirectory(&fp, &entry))
    {
        if(!STD_StrCmp(".", entry.longname) || !STD_StrCmp("..", entry.longname))
            continue;

        (void)STD_TSNPrintf(buf, BUF_SIZE, "%s/%s", arc_name, entry.longname);
        if(!(entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY))
        {
            // For files:
            char *data = NULL;
            data = LoadFile(buf);
            SDK_POINTER_ASSERT(buf);
            
            DrawString("%s\n%s\n", buf, data);
            OS_Free(data);
            bRead = TRUE;
            break;
        }
    }
    bSuccess = FS_CloseDirectory(&fp);
    SDK_ASSERT(bSuccess);
    if(!bRead)
    {
        DrawString("No File\n");
    }
    DrawString("\n");
}

/*---------------------------------------------------------------------------*
  Name:         DrawString

  Description:  Renders using DEMODrawString.
                
  Arguments:    fmt: Based on DEMODrawString.
                     When fmt is " ", however, this function returns the cursor position to [0,0].
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawString(const char* fmt, ...)
{
    static s32 x = 0, y = 0;
    char dst[256];
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);

    if(fmt[0] == '\0')
    {
        x = y = 0;
        DEMOFillRect(0, 0, 256, 192, GX_RGBA(0, 0, 0, 1));
        return;
    }    
    DEMODrawText(x, y, dst);
    {
        s32 i, max = STD_StrLen(dst) - 1;
        u32 cr = 0;
        for(i = max; i >= 0; --i)
        {
            if(dst[i] == '\n')
            {
                x = (cr == 0) ? (max - i) * 8 : x;
                cr++;
            }
        }
        y += cr * 8;
    }
}


/*---------------------------------------------------------------------------*
  Name:         PrintTree

  Description:  Displays a directory tree.
                
  Arguments:    path: Root path
                space: For recursive calls. Normally 0 is specified. 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintTree(const char* path, u32 space)
{
    FSFile f;
    FSDirectoryEntryInfo entry;
    BOOL bSuccess;
    char buf[BUF_SIZE];

    FS_InitFile(&f);
    bSuccess = FS_OpenDirectory(&f, path, FS_PERMIT_R);
    if(!bSuccess)
        return;
    
    while( FS_ReadDirectory(&f, &entry) )
    {
        MI_CpuFill8(buf, ' ', space);
        buf[space] = '\0';
        // Skip entries indicative of self or parent.
        if( (STD_StrCmp(entry.longname, ".")  == 0)
         || (STD_StrCmp(entry.longname, "..") == 0) )
        {
             continue;
        }
        if( (entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0 )
        {
            // Directory
            (void)STD_StrCat(buf, entry.longname);
            DrawString("%s\n", buf);
            
            // Combine paths and recursively call.
            (void)STD_StrCpy(buf, path);
            (void)STD_StrCat(buf, "/");
            (void)STD_StrCat(buf, entry.longname);
            PrintTree(buf, space + 1);
        }
        else
        {
            // Files
            (void)STD_StrCat(buf, entry.longname);
            DrawString("%s\n", buf);
        }
    }
    bSuccess = FS_CloseDirectory(&f);
    SDK_ASSERT( bSuccess );
}

/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  Prints the BootType.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   DrawString("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  DrawString("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  Configures display settings for console screen output.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // Initialize screen display
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  Initializes interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // Enable master interrupt flag
    (void)OS_EnableIrq();

    // Allow IRQ interrupts
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  Creates a heap and makes OS_Alloc usable.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
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
  Name:         InitFileSystem

  Description:  Initializes the file system.
                The InitInteruptSystem function must have been called before this function is.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );
}

/*---------------------------------------------------------------------------*
  Name:         MountOtherSaveData

  Description:  Mounts the save data of another title. 
  
  Arguments:    code:   Target application's game code
                flag:   Mounts the Public save data when TRUE, mounts the Private save data when FALSE
                        
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MountOtherSaveData(const char* code, BOOL flag)
{
    FSResult result;
    result = NA_UnloadOtherTitleArchive();
    SDK_ASSERT( result == FS_RESULT_SUCCESS || result == FS_RESULT_ALREADY_DONE );

    result = NA_LoadOtherTitleArchive(code, 
        ((flag) ? NA_TITLE_ARCHIVE_DATAPUB : NA_TITLE_ARCHIVE_DATAPRV));
    if(result != FS_RESULT_SUCCESS)
    {
        DrawString("Failed Mount %s\n", code);
    }
}

