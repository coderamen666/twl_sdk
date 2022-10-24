/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - demos - arc-2
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>


struct FixedDirectoryList;

// File image structure for a custom archive
typedef struct FixedFile
{
    char    buffer[256];
}
FixedFile;

// Directory structure for a custom archive
typedef struct FixedDirectory
{
    const char *name;
    union
    {
        void   *common_ptr;
        struct FixedDirectoryList *dir;
        FixedFile *file;
    }
    obj;
}
FixedDirectory;

// Directory list structure for a custom archive (equivalent to FNT)
typedef struct FixedDirectoryList
{
    int     parent;
    FixedDirectory *own;
}
FixedDirectoryList;


// Define the directory structure of a custom archive
extern FixedFile file_list[];
extern FixedDirectoryList dir_list[];
extern const int file_max;
extern const int dir_max;

static FixedFile file_list[] = {
    {
     "hello, world!\n" "fixed file of root.\n"},
    {
     "fixed file 1.\n"},
    {
     "fixed file 2.\n"},
    {
     "fixed file 3.\n"},
};
static FixedDirectory sub_dir1[] = {
    {"file1.txt", file_list + 1,},
    {"file2.txt", file_list + 2,},
    {"file3.txt", file_list + 3,},
    {NULL,},
};
static FixedDirectory sub_dir2[] = {
    {"test1.txt", file_list + 1,},
    {"test2.txt", file_list + 2,},
    {"test3.txt", file_list + 3,},
    {NULL,},
};
static FixedDirectory root_dir[] = {
    {"root.txt", file_list + 0,},
    {"sub1", dir_list + 1,},
    {"sub2", dir_list + 2,},
    {NULL,},
};
FixedDirectoryList dir_list[] = {
    {-1, root_dir,},
    {0, sub_dir1,},
    {0, sub_dir2,},
};
const int dir_max = sizeof(dir_list) / sizeof(*dir_list);
const int file_max = sizeof(file_list) / sizeof(*file_list);


// Access callback from the FS library to an archive.
// This simply reads from and writes to memory.
static FSResult CustomRom_ReadCallback(FSArchive *arc, void *dst, u32 pos, u32 size)
{
    (void)arc;
    MI_CpuCopy8((const void *)pos, dst, size);
    return FS_RESULT_SUCCESS;
}
static FSResult CustomRom_WriteCallback(FSArchive *arc, const void *src, u32 pos, u32 size)
{
    (void)arc;
    MI_CpuCopy8(src, (void *)pos, size);
    return FS_RESULT_SUCCESS;
}

// User procedure for a custom archive
static FSResult CustomRom_ArchiveProc(FSFile *file, FSCommandType cmd)
{
    FSArchive *const p_rom = FS_GetAttachedArchive(file);

    switch (cmd)
    {

        // Customize only low-level commands according to FS library specifications

    case FS_COMMAND_SEEKDIR:
        {
            const FSDirPos *const arg = &file->arg.seekdir.pos;
            const int dir_id = arg->own_id;
            file->prop.dir.pos = *arg;
            // Seek to the start of the directory when index and pos are both 0
            if ((arg->index == 0) && (arg->pos == 0))
            {
                file->prop.dir.pos.index = 0; /* Not used */
                file->prop.dir.pos.pos = (u32)dir_list[dir_id].own;
            }
            // At the root directory, parent-ID shows the total number of directories
            file->prop.dir.parent = (u16)((dir_id != 0) ? dir_list[dir_id].parent : dir_max);
            return FS_RESULT_SUCCESS;
        }

    case FS_COMMAND_READDIR:
        {
            FSDirEntry *const p_entry = file->arg.readdir.p_entry;
            const FixedDirectory *const cur = (const FixedDirectory *)file->prop.dir.pos.pos;
            // FS_RESULT_FAILURE is returned at the end of the directory
            if (!cur->name)
            {
                return FS_RESULT_FAILURE;
            }
            p_entry->name_len = (u32)STD_GetStringLength(cur->name);
            // The entry name is also returned if skip_string has not been specified
            if (!file->arg.readdir.skip_string)
            {
                MI_CpuCopy8(cur->name, p_entry->name, p_entry->name_len + 1);
            }
            // If the entry indicates a directory, information will be returned as an FSDirPos value
            if ((cur->obj.dir >= dir_list) && (cur->obj.dir < dir_list + dir_max))
            {
                p_entry->is_directory = 1;
                p_entry->dir_id.arc = file->arc;
                p_entry->dir_id.own_id = (u16)(cur->obj.dir - dir_list);
                p_entry->dir_id.index = 0;
                p_entry->dir_id.pos = 0;
            }
            // If the entry indicates a file, information will be returned as an FSFileID value
            else
            {
                p_entry->is_directory = 0;
                p_entry->file_id.arc = file->arc;
                p_entry->file_id.file_id = (u32)(cur->obj.file - file_list);
            }
            // Advance the position by 1 if the entry could be read normally
            file->prop.dir.pos.pos = (u32)(cur + 1);
            return FS_RESULT_SUCCESS;
        }

    case FS_COMMAND_OPENFILEFAST:
        {
            const int id = (int)file->arg.openfilefast.id.file_id;
            // FS_RESULT_FAILURE is returned if an invalid file ID has been specified
            if ((id < 0) || (id >= file_max))
            {
                return FS_RESULT_FAILURE;
            }
            else
            {
                // Specify the appropriate top and bottom if the file is a linear image
                const char *text = file_list[id].buffer;
                file->arg.openfiledirect.top = (u32)text;
                file->arg.openfiledirect.bottom = (u32)(text + STD_GetStringLength(text));
                file->arg.openfiledirect.index = (u32)id;
            }
            // Continue to FS_COMMAND_OPENFILEDIRECT processing
        }

    case FS_COMMAND_OPENFILEDIRECT:
        {
            // Configure the parameters as specified by the caller
            file->prop.file.top = file->arg.openfiledirect.top;
            file->prop.file.pos = file->arg.openfiledirect.top;
            file->prop.file.bottom = file->arg.openfiledirect.bottom;
            file->prop.file.own_id = file->arg.openfiledirect.index;
            return FS_RESULT_SUCCESS;
        }

        // If FS_RESULT_PROC_UNKNOWN is returned to a high-level command and default behavior is used, the low-level commands customized here will be called as necessary
        // 

    default:
    case FS_COMMAND_FINDPATH:
    case FS_COMMAND_GETPATH:
    case FS_COMMAND_CLOSEFILE:
    case FS_COMMAND_READFILE:
    case FS_COMMAND_WRITEFILE:
    case FS_COMMAND_ACTIVATE:
    case FS_COMMAND_IDLE:
        return FS_RESULT_PROC_UNKNOWN;

    }

}

// Initialize a custom archive
static void CreateCustomArchive(FSArchive *arc, const char *name)
{
    FS_InitArchive(arc);
    if (!FS_RegisterArchiveName(arc, name, (u32)STD_GetStringLength(name)))
    {
        OS_TPanic("error! FS_RegisterArchiveName(%s) failed.\n", name);
    }
    FS_SetArchiveProc(arc, CustomRom_ArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
    // All commands that require FAT and FNT by default are customized, so you do not need to specify anything in particular here
    // 
    if (!FS_LoadArchive(arc, 0, NULL, 0, NULL, 0, CustomRom_ReadCallback, CustomRom_WriteCallback))
    {
        OS_TPanic("error! FS_LoadArchive() failed.\n");
    }
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
    static FSArchive custom_rom;

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
               "test 1 : query default \"rom\" directories ... \n\n");

    DumpRomDir("rom:/");


    OS_TPrintf("\n"
               "++++++++++++++++++++++++++++++++++++++++\n"
               "test 2 : query custom archive \"cst\" directories ... \n\n");

    CreateCustomArchive(&custom_rom, "cst");
    DumpRomDir("cst:/");

    OS_TPrintf("\n" "++++++++++++++++++++++++++++++++++++++++\n" "end\n\n");
    OS_Terminate();
}
