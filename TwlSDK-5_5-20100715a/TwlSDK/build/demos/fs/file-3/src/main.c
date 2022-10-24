/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - demos - file-3
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-02 #$
  $Rev: 8827 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include <nitro.h>


// Read one line from a file
static char* GetLine(char *buf, int len, FSFile *file)
{
    char   *retval = NULL;
    int     ret = FS_ReadFile(file, buf, len - 1);
    if (ret >= 0)
    {
        int     i;
        for (i = 0; i < ret; ++i)
        {
            if (buf[i] == '\n')
            {
                ++i;
                break;
            }
        }
        buf[i] = '\0';
        (void)FS_SeekFile(file, i - ret, FS_SEEK_CUR);
        retval = buf;
    }
    return retval;
}


void NitroMain(void)
{
    // Initialize the OS and memory allocator
    OS_Init();
    {
        OSHeapHandle hh;
        void   *tmp;
        tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
        OS_SetArenaLo(OS_ARENA_MAIN, tmp);
        hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
        if (hh < 0)
        {
            OS_TPanic("ARM9: Fail to create heap...\n");
        }
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
    }
    (void)OS_EnableIrq();
    // Initialize the FS
    FS_Init(3);
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

    OS_TPrintf("\n"
              "++++++++++++++++++++++++++++++++++++++++\n"
              "test 1 : create temporary file from memory... \n\n");
    {
        FSFile  file;
        BOOL    ret;
        char    buf[256];

        static char map_mem[] =
            "here is memory buffer read as file.\n"
            "***********************************\n" "***********************************\n";

        /* Open memory file */
        FS_InitFile(&file);
        ret = FS_CreateFileFromMemory(&file, map_mem, sizeof(map_mem));
        SDK_ASSERT(ret);
        OS_TPrintf("{\n");
        while (GetLine(buf, sizeof(buf), &file) && (*buf != '\0'))
        {
            OS_TPrintf("    %s", buf);
        }
        OS_TPrintf("\n}\n");

        /* Change content with write-access */
        (void)FS_SeekFileToBegin(&file);
        while (FS_WriteFile(&file, "#", 1) > 0)
        {
        }
        (void)FS_SeekFileToBegin(&file);
        OS_TPrintf("{\n");
        while (GetLine(buf, sizeof(buf), &file) && (*buf != '\0'))
        {
            OS_TPrintf("    %s", buf);
        }
        OS_TPrintf("\n}\n");

        ret = FS_CloseFile(&file);
        SDK_ASSERT(ret);
    }

    OS_TPrintf("\n"
              "++++++++++++++++++++++++++++++++++++++++\n"
              "test 2 : create temporary file from ROM... \n\n");
    {
        FSFile  file, src;
        BOOL    ret;
        char    buf[256];


        FS_InitFile(&src);
        ret = FS_OpenFileEx(&src, "rom:/main.c", FS_FILEMODE_R);
        SDK_ASSERT(ret);

        FS_InitFile(&file);
        ret = FS_CreateFileFromRom(&file, FS_GetFileImageTop(&src), FS_GetFileLength(&src));
        SDK_ASSERT(ret);

        ret = FS_CloseFile(&src);
        SDK_ASSERT(ret);

        OS_TPrintf("{\n");
        while (GetLine(buf, sizeof(buf), &file) && (*buf != '\0'))
            OS_TPrintf("    %s", buf);
        OS_TPrintf("\n}\n");

        ret = FS_CloseFile(&file);
        SDK_ASSERT(ret);

    }

    OS_TPrintf("\n" "++++++++++++++++++++++++++++++++++++++++\n" "end\n\n");
    OS_Terminate();
}
