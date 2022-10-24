/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     rfat.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_FS_RFAT_H_
#define NITRO_FS_RFAT_H_


#include <nitro/fs/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

//#define FS_DISABLE_OLDPROTOTYPES

#define FS_COMMAND_ASYNC_BEGIN  (FSCommandType)(FS_COMMAND_READFILE)
#define FS_COMMAND_ASYNC_END    (FSCommandType)(FS_COMMAND_WRITEFILE + 1)
#define FS_COMMAND_SYNC_BEGIN   (FSCommandType)(FS_COMMAND_SEEKDIR)
#define FS_COMMAND_SYNC_END     (FSCommandType)(FS_COMMAND_CLOSEFILE + 1)
#define FS_COMMAND_STATUS_BEGIN (FSCommandType)(FS_COMMAND_ACTIVATE)
#define FS_COMMAND_STATUS_END   (FSCommandType)(FS_COMMAND_RESUME + 1)

// Numbers omitted for compatibility
#define FS_ARCHIVE_FLAG_IS_SYNC     0x00000100
#define FS_ARCHIVE_FLAG_TABLE_LOAD  0x00000004
#define FS_ARCHIVE_FLAG_OBSOLETE    0x00000104

// User procedure setting flag.
// This is specified as the 'flags' argument for FS_SetArchiveProc().
// Enable bits only for items that request callbacks.

// Asynchronous commands
#define	FS_ARCHIVE_PROC_READFILE		(1 << FS_COMMAND_READFILE)
#define	FS_ARCHIVE_PROC_WRITEFILE		(1 << FS_COMMAND_WRITEFILE)
// All asynchronous commands
#define	FS_ARCHIVE_PROC_ASYNC	\
	(FS_ARCHIVE_PROC_READFILE | FS_ARCHIVE_PROC_WRITEFILE)

// Synchronous commands
#define	FS_ARCHIVE_PROC_SEEKDIR			(1 << FS_COMMAND_SEEKDIR)
#define	FS_ARCHIVE_PROC_READDIR			(1 << FS_COMMAND_READDIR)
#define	FS_ARCHIVE_PROC_FINDPATH		(1 << FS_COMMAND_FINDPATH)
#define	FS_ARCHIVE_PROC_GETPATH			(1 << FS_COMMAND_GETPATH)
#define	FS_ARCHIVE_PROC_OPENFILEFAST	(1 << FS_COMMAND_OPENFILEFAST)
#define	FS_ARCHIVE_PROC_OPENFILEDIRECT	(1 << FS_COMMAND_OPENFILEDIRECT)
#define	FS_ARCHIVE_PROC_CLOSEFILE		(1 << FS_COMMAND_CLOSEFILE)
// All synchronous commands
#define	FS_ARCHIVE_PROC_SYNC	\
	(FS_ARCHIVE_PROC_SEEKDIR | FS_ARCHIVE_PROC_READDIR |	\
	 FS_ARCHIVE_PROC_FINDPATH | FS_ARCHIVE_PROC_GETPATH |	\
	FS_ARCHIVE_PROC_OPENFILEFAST | FS_ARCHIVE_PROC_OPENFILEDIRECT | FS_ARCHIVE_PROC_CLOSEFILE)

// Messages when status changes
#define	FS_ARCHIVE_PROC_ACTIVATE		(1 << FS_COMMAND_ACTIVATE)
#define	FS_ARCHIVE_PROC_IDLE			(1 << FS_COMMAND_IDLE)
#define	FS_ARCHIVE_PROC_SUSPENDING		(1 << FS_COMMAND_SUSPEND)
#define	FS_ARCHIVE_PROC_RESUME			(1 << FS_COMMAND_RESUME)
// All messages when status changes
#define	FS_ARCHIVE_PROC_STATUS	\
	(FS_ARCHIVE_PROC_ACTIVATE | FS_ARCHIVE_PROC_IDLE |	\
	 FS_ARCHIVE_PROC_SUSPENDING | FS_ARCHIVE_PROC_RESUME)

// All messages
#define	FS_ARCHIVE_PROC_ALL	(~0)


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef FSResult (*FS_ARCHIVE_PROC_FUNC) (struct FSFile *, FSCommandType);
typedef FSResult (*FS_ARCHIVE_READ_FUNC) (struct FSArchive *p, void *dst, u32 pos, u32 size);
typedef FSResult (*FS_ARCHIVE_WRITE_FUNC) (struct FSArchive *p, const void *src, u32 pos, u32 size);

typedef struct FSArchiveFAT
{
    u32     top;
    u32     bottom;
}
FSArchiveFAT;

typedef struct FSArchiveFNT
{
    u32     start;
    u16     index;
    u16     parent;
}
FSArchiveFNT;

#define FS_ROMFAT_CONTEXT_DEFINITION()      \
{                                           \
    u32                     base;           \
    u32                     fat;            \
    u32                     fat_size;       \
    u32                     fnt;            \
    u32                     fnt_size;       \
    u32                     fat_bak;        \
    u32                     fnt_bak;        \
    void                   *load_mem;       \
    FS_ARCHIVE_READ_FUNC    read_func;      \
    FS_ARCHIVE_WRITE_FUNC   write_func;     \
    u8                      reserved3[4];   \
    FS_ARCHIVE_PROC_FUNC    proc;           \
    u32                     proc_flag;      \
}

typedef struct FSROMFATArchiveContext
FS_ROMFAT_CONTEXT_DEFINITION()
FSROMFATArchiveContext;

#ifdef FS_DISABLE_OLDPROTOTYPES
#undef FS_ROMFAT_CONTEXT_DEFINITION
#define FS_ROMFAT_CONTEXT_DEFINITION() { u8 reserved[52]; } obsolete
#endif // FS_DISABLE_OLDPROTOTYPES

// for FS_COMMAND_SEEKDIR
typedef struct
{
    FSDirPos pos;
}
FSSeekDirInfo;

// for FS_COMMAND_READDIR
typedef struct
{
    FSDirEntry *p_entry;
    BOOL    skip_string;
}
FSReadDirInfo;

// for FS_COMMAND_FINDPATH
typedef struct
{
    FSDirPos pos;
    const char *path;
    BOOL    find_directory;
    union
    {
        FSFileID *file;
        FSDirPos *dir;
    }
    result;
}
FSFindPathInfo;

// for FS_COMMAND_GETPATH
typedef struct
{
    u8     *buf;
    u32     buf_len;
    u16     total_len;
    u16     dir_id;
}
FSGetPathInfo;

// for FS_COMMAND_OPENFILEFAST
typedef struct
{
    FSFileID id;
}
FSOpenFileFastInfo;

// for FS_COMMAND_OPENFILEDIRECT
typedef struct
{
    u32     top;
    u32     bottom;
    u32     index;
}
FSOpenFileDirectInfo;

// for FS_COMMAND_CLOSEFILE
typedef struct
{
    u32     reserved;
}
FSCloseFileInfo;

// for FS_COMMAND_READFILE
typedef struct
{
    void   *dst;
    u32     len_org;
    u32     len;
}
FSReadFileInfo;

// for FS_COMMAND_WRITEFILE
typedef struct
{
    const void *src;
    u32     len_org;
    u32     len;
}
FSWriteFileInfo;

typedef union FSROMFATCommandInfo
{
    FSReadFileInfo          readfile;
    FSWriteFileInfo         writefile;
    FSSeekDirInfo           seekdir;
    FSReadDirInfo           readdir;
    FSFindPathInfo          findpath;
    FSGetPathInfo           getpath;
    FSOpenFileFastInfo      openfilefast;
    FSOpenFileDirectInfo    openfiledirect;
    FSCloseFileInfo         closefile;
}
FSROMFATCommandInfo;

typedef struct FSROMFATFileProperty
{
    u32                 own_id;
    u32                 top;
    u32                 bottom;
    u32                 pos;
}
FSROMFATFileProperty;

typedef struct FSROMFATDirProperty
{
    FSDirPos            pos;
    u32                 parent;
}
FSROMFATDirProperty;

typedef union FSROMFATProperty
{
    FSROMFATFileProperty    file;
    FSROMFATDirProperty     dir;
}
FSROMFATProperty;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveBase

  Description:  Gets the base offset for a ROMFAT archive.

  Arguments:    arc: ROMFAT archive

  Returns:      Base offset for the ROMFAT archive.
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveBase(const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFAT

  Description:  Gets the FAT offset for a ROMFAT archive.

  Arguments:    arc: ROMFAT archive

  Returns:      FAT offset for the ROMFAT archive.
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveFAT(const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFNT

  Description:  Gets the FNT offset for a ROMFAT archive.

  Arguments:    arc: ROMFAT archive

  Returns:      FNT offset for the ROMFAT archive.
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveFNT(const struct FSArchive *arc);

/* Obtain specified position offset from the archive's base */
/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveOffset

  Description:  Gets the offset for the specified position from a ROMFAT archive's base.

  Arguments:    arc: ROMFAT archive

  Returns:      Offset to the specified position with the base added.
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveOffset(const struct FSArchive *arc, u32 pos);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveTableLoaded

  Description:  Determines whether a ROMFAT archive has finished preloading the table.

  Arguments:    arc: ROMFAT archive

  Returns:      TRUE if the table has been preloaded.
 *---------------------------------------------------------------------------*/
BOOL    FS_IsArchiveTableLoaded(volatile const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchive

  Description:  Loads the ROMFAT archive into the file system.

  Arguments:    arc: ROMFAT archive
                base: An arbitrary u32 value that can be used independently
                fat: Starting offset of the FAT table
                fat_size: Size of the FAT table
                fnt: Starting offset of the FNT table
                fnt_size: Size of the FNT table
                read_func: Read access callback
                write_func: Write access callback

  Returns:      TRUE if the ROMFAT archive is loaded correctly.
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadArchive(struct FSArchive *arc, u32 base,
                       u32 fat, u32 fat_size, u32 fnt, u32 fnt_size,
                       FS_ARCHIVE_READ_FUNC read_func, FS_ARCHIVE_WRITE_FUNC write_func);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchive

  Description:  Unloads a ROMFAT archive from the file system.

  Arguments:    arc: ROMFAT archive

  Returns:      TRUE if the ROMFAT archive is unloaded correctly.
 *---------------------------------------------------------------------------*/
BOOL    FS_UnloadArchive(struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchiveTables

  Description:  Preloads the FAT and FNT of an FS_UnloadArchive archive in memory.
                This will load the data and return the required size only when it is less than the specified size.

  Arguments:    arc: ROMFAT archive
                mem: Buffer to store the table data
                size: The size of mem

  Returns:      Always returns the total table size.
 *---------------------------------------------------------------------------*/
u32     FS_LoadArchiveTables(struct FSArchive *arc, void *mem, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchiveTables

  Description:  Releases preloaded memory for a ROMFAT archive.

  Arguments:    arc: ROMFAT archive

  Returns:      The buffer provided by the user to serve as preloaded memory.
 *---------------------------------------------------------------------------*/
void   *FS_UnloadArchiveTables(struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_SetArchiveProc

  Description:  Sets the user procedure for a ROMFAT archive.
                If proc is NULL or flags is 0, this will simply disable the user procedure.
                

  Arguments:    arc: ROMFAT archive
                proc: User procedure
                flags: Bitset of commands with procedure hooks

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_SetArchiveProc(struct FSArchive *arc, FS_ARCHIVE_PROC_FUNC proc, u32 flags);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageTop

  Description:  Gets the offset to the start of a file opened from a ROMFAT archive.
                

  Arguments:    file: File handle

  Returns:      The offset to the start of the file in the archive.
 *---------------------------------------------------------------------------*/
u32     FS_GetFileImageTop(const struct FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageBottom

  Description:  Gets the offset to the end of a file opened from a ROMFAT archive.
                

  Arguments:    file: File handle

  Returns:      The offset to the end of the file in the archive.
 *---------------------------------------------------------------------------*/
u32     FS_GetFileImageBottom(const struct FSFile *file);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_RFAT_H_ */
