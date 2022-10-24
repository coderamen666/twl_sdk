/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - explorer
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-04 #$
  $Rev: 9513 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif


#include "DEMO.h"


//================================================================================


static const GXRgb  icon_file[] =
{
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,
    0xFFFF,0x8000,0x8000,0x8000,0x8000,0x8000,0xFFFF,0x0000,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,
    0xFFFF,0x8000,0x8000,0x8000,0x8000,0x8000,0xFFFF,0x0000,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,
    0xFFFF,0x8000,0x8000,0x8000,0x8000,0x8000,0xFFFF,0x0000,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,0x0000,
};
static const GXRgb  icon_directory[] =
{
    0x8210,0x8210,0x8210,0x8210,0x8210,0x0000,0x0000,0x0000,
    0x8210,0x8210,0x8210,0x8210,0x8210,0x0000,0x0000,0x0000,
    0x8210,0x8210,0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,
    0x8210,0x8210,0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,
    0x8210,0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,0x0000,
    0x8210,0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,0x0000,
    0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,0x0000,0x0000,
    0x83FF,0x83FF,0x83FF,0x83FF,0x83FF,0x0000,0x0000,0x0000,
};

enum { entry_max = 20 };
typedef struct ExplorerContext
{
    char                    current[FS_ENTRY_LONGNAME_MAX];
    int                     entry_count;
    FSDirectoryEntryInfo    entry[entry_max];
}
ExplorerContext;

static ExplorerContext explorer[1];


/*---------------------------------------------------------------------------*
  Name:         CommandLS

  Description:  Gets the current directory list.

  Arguments:    context: ExplorerContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CommandLS(ExplorerContext *context)
{
    FSFile  file[1];
    FS_InitFile(file);
    context->entry_count = 0;
    if (*context->current == '\0')
    {
        (void)FS_OpenTopLevelDirectory(file);
    }
    else
    {
        char    path[FS_ENTRY_LONGNAME_MAX];
        (void)STD_TSPrintf(path, "%s/*", context->current);
        (void)FS_OpenDirectory(file, path, FS_FILEMODE_R);
    }
    if (FS_IsDir(file))
    {
        while ((context->entry_count < entry_max) &&
               FS_ReadDirectory(file, &context->entry[context->entry_count]))
        {
            ++context->entry_count;
        }
        (void)FS_CloseDirectory(file);
    }
    if (context->entry_count == 0)
    {
        FSDirectoryEntryInfo   *p = &context->entry[context->entry_count];
        MI_CpuClear8(p, sizeof(*p));
        (void)STD_TSPrintf(p->longname, "..");
        p->longname_length = (u32)STD_GetStringLength(p->longname);
        p->attributes = FS_ATTRIBUTE_IS_DIRECTORY;
        p->id = FS_INVALID_DIRECTORY_ID;
        ++context->entry_count;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CommandCD

  Description:  Moves the current directory.

  Arguments:    context: ExplorerContext structure
                path: Relative path from the moved location

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CommandCD(ExplorerContext *context, const char *path)
{
    char   *cur = context->current;
    int     i = STD_GetStringLength(cur);
    if (STD_CompareString(path, ".") == 0)
    {
    }
    else if(STD_CompareString(path, "..") == 0)
    {
        while ((i > 0) && (--i, (cur[i] != '/') && (cur[i] != '\\')))
        {
        }
        cur[i] = '\0';
    }
    else if(*path)
    {
        (void)STD_TSNPrintf(&cur[i], sizeof(context->current) - i, "%s%s",
                            (cur[0] != '\0') ? "/" : "", path);
    }
    CommandLS(context);
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    {
        BOOL    update = TRUE;
        int     cursor = 0;

        CommandCD(explorer, "");
        for (;;)
        {
            DEMOReadKey();
            // Move the cursor with the +Control Pad
            if (DEMO_IS_TRIG(PAD_KEY_DOWN))
            {
                if (++cursor >= explorer->entry_count)
                {
                    cursor -= explorer->entry_count;
                }
                update = TRUE;
            }
            if (DEMO_IS_TRIG(PAD_KEY_UP))
            {
                if (--cursor < 0)
                {
                    cursor += explorer->entry_count;
                }
                update = TRUE;
            }
            // Move the directory or load a file with the A Button
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                if (cursor < explorer->entry_count)
                {
                    if ((explorer->entry[cursor].attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0)
                    {
                        CommandCD(explorer, explorer->entry[cursor].longname);
                        cursor = 0;
                        update = TRUE;
                    }
                    else
                    {
                        char    path[FS_ENTRY_LONGNAME_MAX];
                        FSFile  file[1];
                        FS_InitFile(file);
                        (void)STD_TSPrintf(path, "%s/%s", explorer->current, explorer->entry[cursor].longname);
                        if (!FS_OpenFileEx(file, path, FS_FILEMODE_R))
                        {
                            // Check the error if the file failed to open
                            FSResult    result = FS_GetResultCode(file);
                            OS_TPrintf("%s\nOpenFile() failed (err=%d)\n", path, result);
                            // The following errors may be caused by various states
                            if (result == FS_RESULT_NO_ENTRY)
                            {
                                OS_TPrintf("cannot find specified file.\n");
                            }
                            else if (result == FS_RESULT_PERMISSION_DENIED)
                            {
                                OS_TPrintf("permission denied.%s\n",
                                           (explorer->entry[cursor].filesize > 0x7FFFFFFFUL) ?
                                           "\n  (file exceeds by 2G-1[BYTE])" : "");
                            }
                            else if ((result == FS_RESULT_MEDIA_NOTHING) ||
                                     (result == FS_RESULT_MEDIA_UNKNOWN))
                            {
                                OS_TPrintf("no valid media inserted now.\n");
                            }
                            else if (result == FS_RESULT_NO_MORE_RESOURCE)
                            {
                                OS_TPrintf("no more resource to open new file-handle.\n");
                            }
                            // The following errors are fatal to the system or file system
                            else if (result == FS_RESULT_BAD_FORMAT)
                            {
                                OS_TPrintf("filesystem has gotten corrupted!\n");
                            }
                            else if (result == FS_RESULT_MEDIA_FATAL)
                            {
                                OS_TPrintf("device I/O fatal error!\n");
                            }
                            // The rest are program errors caused by incorrect use of the API
                            else
                            {
                                OS_TPrintf("program logic error!\n");
                            }
                        }
                        else
                        {
                            // Treat the opened file as text and output it to the console
                            OS_TPrintf("\n---\n");
                            for (;;)
                            {
                                static char tmpbuf[1024 + 1];
                                int     n = FS_ReadFile(file, tmpbuf, sizeof(tmpbuf) - 1);
                                if (n <= 0)
                                {
                                    if (n < 0)
                                    {
                                        OS_TPrintf("\n---FS error(%d)---\n", FS_GetResultCode(file));
                                    }
                                    break;
                                }
                                else
                                {
                                    tmpbuf[n] = '\0';
                                    OS_TPrintf("%s", tmpbuf);
                                }
                            }
                            OS_TPrintf("\n---\n");
                            (void)FS_CloseFile(file);
                        }
                    }
                }
            }
            // Move to a higher-level directory with the B Button
            if (DEMO_IS_TRIG(PAD_BUTTON_B))
            {
                CommandCD(explorer, "..");
                cursor = 0;
                update = TRUE;
            }

            // Update rendering contents if the directory is moved
            if (update)
            {
                int     ox = 10;
                int     oy = 30;
                int     wx = 240;
                int     wy = 15 * explorer->entry_count + 10;
                int     i;
                DEMOFillRect(0, 0, 256, 192, DEMO_RGB_CLEAR);
                DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
                DEMODrawText(ox, 10, "%s", explorer->current);
                DEMODrawFrame(ox, oy, wx, wy, GX_RGBA( 0, 31, 0, 1));
                for (i = 0; i < explorer->entry_count; ++i)
                {
                    int     y = oy + 5 + 15 * i;
                    const FSDirectoryEntryInfo *p = &explorer->entry[i];
                    BOOL    focus = (i == cursor);
                    BOOL    is_dir = ((p->attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0);
                    const int   omitmax = 14;
                    DEMOSetBitmapTextColor((GXRgb)(focus ? GX_RGBA(31, 31, 0, 1) : GX_RGBA(31, 31, 31, 1)));
                    DEMODrawText(ox + 20, y, "%s", focus ? ">" : "");
                    DEMOBlitRect(ox + 30, y, 8, 8, is_dir ? icon_directory : icon_file, 8);
                    DEMODrawText(ox + 45, y, "%.*s%s", (p->longname_length <= omitmax) ? omitmax : (omitmax - 3),
                                 p->longname, (p->longname_length <= omitmax) ? "" : "...");
                    if (!is_dir)
                    {
                        DEMODrawText(ox + 160, y, "%8dB", p->filesize);
                    }
                }
            }

            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}


/*====== End of main.c ======*/
