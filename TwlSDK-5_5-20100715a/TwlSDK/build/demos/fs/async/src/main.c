/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - demos - async
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


// Behavioral test of asynchronous FS operations

enum
{
    data_00,
    data_ff,
    data_inc,
    data_dec,
    datamode_max
};

#define	TASK_THREAD_MAX	4
#define	TASK_REPEAT_TIMES	1

typedef struct
{
    OSThread thread;
    u8      stack[4096] ATTRIBUTE_ALIGN(32);
    u8      buf[CARD_ROM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);
    int     mode;
    u32     priority;
    u8      padding[24];
}
TaskThread;


static TaskThread task_thread[TASK_THREAD_MAX] ATTRIBUTE_ALIGN(32);
static u32 total_read_size = 0;

// A task that opens a file and continues to read
static void AsyncAccessTask(void *p)
{
    TaskThread *const pt = (TaskThread *) p;
    int     i;
    int     repeat;
    int     count;
    int     total_size;
    BOOL    ret;
    FSFile  file;
    count = 0;
    repeat = 0;
    total_size = 0;
    for (;;)
    {
        switch (pt->mode)
        {
        case data_00:
            ret = FS_OpenFileEx(&file, "rom:/00.bin", FS_FILEMODE_R);
            break;
        case data_ff:
            ret = FS_OpenFileEx(&file, "rom:/ff.bin", FS_FILEMODE_R);
            break;
        case data_inc:
            ret = FS_OpenFileEx(&file, "rom:/inc.bin", FS_FILEMODE_R);
            break;
        case data_dec:
            ret = FS_OpenFileEx(&file, "rom:/dec.bin", FS_FILEMODE_R);
            break;
        default:
            ret = FALSE;
            break;
        }
        SDK_ASSERT(ret);
        for (;;)
        {
            int     n = FS_ReadFile(&file, pt->buf, sizeof(pt->buf));
            SDK_ASSERT(n >= 0);
            if (n == 0)
                break;
            switch (pt->mode)
            {
            case data_00:
                for (i = 0; i < n; ++i)
                {
                    SDK_ASSERT(pt->buf[i] == 0x00);
                }
                break;
            case data_ff:
                for (i = 0; i < n; ++i)
                {
                    SDK_ASSERT(pt->buf[i] == 0xFF);
                }
                break;
            case data_inc:
                for (i = 0; i < n; ++i)
                {
                    SDK_ASSERT(pt->buf[i] == (u8)i);
                }
                break;
            case data_dec:
                for (i = 0; i < n; ++i)
                {
                    SDK_ASSERT(pt->buf[i] == (u8)~i);
                }
                break;
            }
            total_size += n;
        }
        ret = FS_CloseFile(&file);
        SDK_ASSERT(ret);
        ++count;
        if (++repeat >= TASK_REPEAT_TIMES)
        {
            u32     current_total;
            OSIntrMode bak_psr = OS_DisableInterrupts();
            total_read_size += total_size;
            current_total = total_read_size;
            total_size = 0;
            OS_TPrintf("thread[%d](priority=%2d) : %8d times done. (mode=%d) ... total %8d byte\n",
                       pt - task_thread, pt->priority, count, pt->mode, current_total);
            (void)OS_RestoreInterrupts(bak_psr);
            repeat = 0;
            if (++pt->mode >= datamode_max)
            {
                pt->mode = data_00;
            }
        }
    }
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
               "test : create %d threads to check asynchronous access ... \n\n", TASK_THREAD_MAX);
    {
        int     i;
        // Create multiple threads that continue to read a file
        for (i = 0; i < TASK_THREAD_MAX; ++i)
        {
            TaskThread * pt = &task_thread[i];
            pt->mode = data_00 + i;
            pt->priority = (u32)(OS_THREAD_LAUNCHER_PRIORITY - (pt - task_thread));
            OS_CreateThread(&pt->thread,
                            AsyncAccessTask, pt,
                            pt->stack + sizeof(pt->stack), sizeof(pt->stack), pt->priority);
            OS_WakeupThreadDirect(&pt->thread);
        }
    }

    // As for the rest, a log will continue to be output for each thread
    for (;;)
    {
        OS_Halt();
    }

}
