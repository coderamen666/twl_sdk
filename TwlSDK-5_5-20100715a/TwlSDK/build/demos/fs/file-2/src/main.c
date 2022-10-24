/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - demos - file-2
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

// List the directories in a ROM archive
static void DumpRomDirectorySub(int tab, FSDirEntry *pe)
{
    FSFile  d;
    FS_InitFile(&d);
    OS_TPrintf("%*s%s/\n", tab, "", pe->name);
    if (FS_SeekDir(&d, &pe->dir_id))
    {
        tab += 4;
        while (FS_ReadDir(&d, pe))
        {
            if (pe->is_directory)
            {
                DumpRomDirectorySub(tab, pe);
            }
            else
            {
                OS_Printf("%*s%s\n", tab, "", pe->name);
            }
        }
    }
}
static void DumpRomDir(const char *path)
{
    FSDirEntry  entry;
    FSFile      dir;
    FS_InitFile(&dir);
    (void)FS_ChangeDir(path);
    (void)FS_FindDir(&dir, "");
    entry.name[0] = '\0';
    (void)FS_TellDir(&dir, &entry.dir_id);
    DumpRomDirectorySub(0, &entry);
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
              "test 1 : open specified files ... \n\n");
    {
        static const char *(paths[]) =
        {
        "\\hello.txt",
                "test1.txt", "/./../.\\.\\tesT2.tXT",};

        int     i;
        for (i = 0; i < sizeof(paths) / sizeof(*paths); ++i)
        {
            FSFile  file;
            const char *path = paths[i];
            BOOL    open_is_ok;
            FS_InitFile(&file);
            open_is_ok = FS_OpenFileEx(&file, path, FS_FILEMODE_R);
            OS_TPrintf("FS_OpenFileEx(\"%s\", FS_FILEMODE_R) ... %s!\n", path, open_is_ok ? "OK" : "ERROR");
            if (open_is_ok)
            {
                char    buf[256];
                OS_TPrintf("{\n");
                while (GetLine(buf, sizeof(buf), &file) && (*buf != '\0'))
                {
                    OS_TPrintf("    %s", buf);
                }
                OS_TPrintf("\n}\n");
            }
        }

    }

    OS_TPrintf("\n"
               "++++++++++++++++++++++++++++++++++++++++\n" "test 2 : query directories ... \n\n");

    DumpRomDir("rom:/");


    OS_TPrintf("\n"
               "++++++++++++++++++++++++++++++++++++++++\n"
               "test 3 : FS_TellDir() and FS_SeekDir() ... \n\n");
    {
        FSFile  d;
        FSDirEntry e;
        FSDirPos pos_list[10];
        int     file_count = 0;

        BOOL    ret;

        FS_InitFile(&d);
        ret = FS_FindDir(&d, "rom:/");
        SDK_ASSERT(ret);

        for (;;)
        {
            FSDirPos pos;
            (void)FS_TellDir(&d, &pos);
            if (!FS_ReadDir(&d, &e))
            {
                break;
            }
            if (!e.is_directory)
            {
                if (file_count < sizeof(pos_list) / sizeof(*pos_list))
                {
                    pos_list[file_count] = pos;
                    OS_TPrintf("TellDir[%d] : %s\n", file_count, e.name);
                    ++file_count;
                }
            }
        }
        ret = FS_RewindDir(&d);
        SDK_ASSERT(ret);
        ret = FS_ReadDir(&d, &e);
        SDK_ASSERT(ret);
        OS_TPrintf("RewindDir : %s\n", e.name);

        while (--file_count >= 0)
        {
            ret = FS_SeekDir(&d, &pos_list[file_count]);
            SDK_ASSERT(ret);
            ret = FS_ReadDir(&d, &e);
            SDK_ASSERT(ret);
            OS_TPrintf("SeekDir[%d] : %s\n", file_count, e.name);
        }

    }

    OS_TPrintf("\n" "++++++++++++++++++++++++++++++++++++++++\n" "end\n\n");
    OS_Terminate();
}
