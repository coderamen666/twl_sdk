/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_archive_fatfs.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL

#include <nitro/fs.h>
#include <nitro/os/ARM9/tcm.h>
#include <nitro/os/ARM9/cache.h>
#include <nitro/math/math.h>
#include <nitro/std/string.h>
#include <nitro/gx/gx_vramcnt.h>
#include <nitro/spi/ARM9/pm.h>
#include <twl/fatfs.h>
#include <twl/mi/common/sharedWram.h>

#include "../include/util.h"
#include "../include/command.h"
#include "../include/rom.h"


#if defined(FS_IMPLEMENT)


#include <twl/ltdmain_begin.h>


/*---------------------------------------------------------------------------*/
/* Constants */

// Characters that cannot be used in entry names of paths
extern const u16  *FSiPathInvalidCharactersW;
const u16  *FSiPathInvalidCharactersW = L":*?\"<>|";


/*---------------------------------------------------------------------------*/
/* Variables */

static int                      FSiSwitchableWramSlots = 0;

static FSFATFSArchiveContext    FSiFATFSDriveDefault[FS_MOUNTDRIVE_MAX] ATTRIBUTE_ALIGN(32);
static FATFSRequestBuffer       FSiFATFSAsyncRequestDefault[FS_MOUNTDRIVE_MAX] ATTRIBUTE_ALIGN(32);
static u8                       FSiTemporaryBufferDefault[FS_TEMPORARY_BUFFER_MAX] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
 * Implementation related to customize
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FSiFATFSDrive

  Description:  Context array that manages the archive of a FAT base.
                Ordinarily, an array size for the quantity of FS_MOUNTDRIVE_MAX is required, and in default, static variables in the LTDMAIN segment are used.
                
                With applications built using a special memory arrangement, it is possible to set the proper buffer by changing this variable internally by calling the FSi_SetupFATBuffers function that was overridden.
                
                

  Arguments:    None.

  Returns:      Buffer for the amount of the FS_MOUNTDRIVE_MAX of the FSFATFSArchiveContext structure.
 *---------------------------------------------------------------------------*/
FSFATFSArchiveContext *FSiFATFSDrive = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSiFATFSAsyncRequest

  Description:  Command buffer that is used in asynchronous processing by the FAT base archive.
                Ordinarily, an array size for the quantity of FS_MOUNTDRIVE_MAX is required, and in default, static variables in the LTDMAIN segment are used.
                
                With applications built using a special memory arrangement, it is possible to set the proper buffer by changing this variable internally by calling the FSi_SetupFATBuffers function that was overridden.
                
                

  Arguments:    None.

  Returns:      Static command buffer of FS_TEMPORARY_BUFFER_MAX bytes.
 *---------------------------------------------------------------------------*/
FATFSRequestBuffer *FSiFATFSAsyncRequest = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSiTemporaryBuffer

  Description:  Pointer to the static temporary buffer for issuing read/write commands.
                It must be memory that can be referenced by both ARM9/ARM7, and by default a static variable in the LTDMAIN segment is used.
                
                For applications constructed using a special memory layout, you can change these variables to set the appropriate buffer before calling the FS_Init function.
                
                

  Arguments:    None.

  Returns:      Static command buffer of FS_TEMPORARY_BUFFER_MAX bytes.
 *---------------------------------------------------------------------------*/
u8 *FSiTemporaryBuffer = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSi_SetupFATBuffers

  Description:  Initializes various buffers needed by the FAT base archive.
                With applications built using a special memory arrangement, it is possible to independently set the various buffers by overriding this variable and suppressing the required memory size.
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void FSi_SetupFATBuffers(void)
{
    FSiFATFSDrive = FSiFATFSDriveDefault;
    FSiFATFSAsyncRequest = FSiFATFSAsyncRequestDefault;
    FSiTemporaryBuffer = FSiTemporaryBufferDefault;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsValidAddressForARM7

  Description:  Determines whether this is a buffer accessible from ARM7.
                With applications built using a special memory layout, it is possible to redefine the proper determination routine by overriding this function.
                
                

  Arguments:    buffer: Buffer to be determined
                length: Buffer length

  Returns:      TRUE if it is a buffer accessible from ARM7.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length)
 __attribute__((never_inline))
{
    u32     addr = (u32)buffer;
    u32     dtcm = OS_GetDTCMAddress();
    if ((addr + length > dtcm) && (addr < dtcm + HW_DTCM_SIZE))
    {
        return FALSE;
    }
    if ((addr >= HW_TWL_MAIN_MEM) && (addr + length <= HW_TWL_MAIN_MEM_END))
    {
        return TRUE;
    }
    if ((addr >= HW_EXT_WRAM_ARM7) && (addr + length <= GX_GetSizeOfARM7()))
    {
        return TRUE;
    }
    // Allow if it is a switchable WRAM not overlapping the slot boundary
    if ((addr >= HW_WRAM_AREA) && (addr <= HW_WRAM_AREA_END) &&
        (MATH_ROUNDDOWN(addr, MI_WRAM_B_SLOT_SIZE) + MI_WRAM_B_SLOT_SIZE ==
         MATH_ROUNDUP(addr + length, MI_WRAM_B_SLOT_SIZE)))
    {
        MIWramPos   type;
        int         slot = MIi_AddressToWramSlot(buffer, &type);
        if (slot >= 0)
        {
            if ((((FSiSwitchableWramSlots >> ((type == MI_WRAM_B) ? 0 : 8)) & (1 << slot)) != 0) &&
                // If success is not always possible by switching, the application guarantees it
                ((MI_GetWramBankMaster(type, slot)  == MI_WRAM_ARM7) ||
                 (MI_SwitchWramSlot(type, slot, MI_WRAM_SIZE_32KB,
                                    MI_GetWramBankMaster(type, slot), MI_WRAM_ARM7) >= 0)))
            {
                
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SetSwitchableWramSlots

  Description:  Specifies WRAM slot that the FS library can switch to ARM7 depending on the circumstances.

  Arguments:    bitsB: WRAM-B slot bit collection
                bitsC: Bitset of WRAM-C slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_SetSwitchableWramSlots(int bitsB, int bitsC)
{
    FSiSwitchableWramSlots = (bitsB << 0) | (bitsC << 8);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_AllocUnicodeFullPath

  Description:  Gets full path linking the archive name and relative path.

  Arguments:    context: FSFATFSArchiveContext structure
                relpath: Relative path under the archive

  Returns:      Buffer that stores the full Unicode path that begins from the archive name.
 *---------------------------------------------------------------------------*/
static u16* FSi_AllocUnicodeFullPath(FSFATFSArchiveContext *context, const char *relpath)
{
    u16        *dst = FSi_GetUnicodeBuffer(NULL);
    {
        int     pos = 0;
        int     dstlen;
        // First link the archive name and the relative path
        dstlen = FS_ARCHIVE_FULLPATH_MAX - pos - 2;
        (void)FSi_ConvertStringSjisToUnicode(&dst[pos], &dstlen, FS_GetArchiveName(context->arc), NULL, NULL);
        pos += dstlen;
        dst[pos++] = L':';
        dst[pos++] = L'/';
        dstlen = FS_ARCHIVE_FULLPATH_MAX - pos;
        (void)FSi_ConvertStringSjisToUnicode(&dst[pos], &dstlen, relpath, NULL, NULL);
        pos += dstlen;
        // Remove trailing forward slashes ('/')
        if ((pos > 0) && ((dst[pos - 1] == L'\\') || (dst[pos - 1] == L'/')))
        {
            --pos;
        }
        dst[pos] = L'\0';
    }
    return dst;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DostimeToFstime

  Description:  Converts FAT DOS time stamp to an FSDateTime format.

  Arguments:    dst: FSDateTime structure that stores the conversion results
                src: DOS time stamp that is the conversion source

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_DostimeToFstime(FSDateTime *dst, u32 src)
{
    dst->year = FATFS_DOSTIME_TO_YEAR(src);
    dst->month = FATFS_DOSTIME_TO_MON(src);
    dst->day = FATFS_DOSTIME_TO_MDAY(src);
    dst->hour = FATFS_DOSTIME_TO_HOUR(src);
    dst->minute = FATFS_DOSTIME_TO_MIN(src);
    dst->second = FATFS_DOSTIME_TO_SEC(src);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FstimeToDostime

  Description:  Converts FSDateTime to a FAT DOS time stamp format.

  Arguments:    dst: DOS time stamp that stores the conversion results
                src: FSDateTime structure that is the conversion source

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_FstimeToDostime(u32 *dst, const FSDateTime *src)
{
    *dst = FATFS_DATETIME_TO_DOSTIME(src->year, src->month, src->day,
                                     src->hour, src->minute, src->second);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_CheckFstime

  Description:  Checks the FSDateTime.

  Arguments:    src: FSDateTime structure

  Returns:      TRUE or FALSE.
 *---------------------------------------------------------------------------*/
static BOOL FSi_CheckFstime( const FSDateTime *fstime)
{
    if( fstime->month / 13 != 0) { return FALSE;}
//    if( fstime->day   / 32 != 0) { return FALSE;}
    if( fstime->hour  / 24 != 0) { return FALSE;}
    if( fstime->minute/ 60 != 0) { return FALSE;}
    if( fstime->second/ 61 != 0) { return FALSE;}
    return( TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeSpanExcluding

  Description:  Gets the string length not including specified characters.

  Arguments:    src: Scanning target string
                pattern: Characters to be searched for

  Returns:      String length, not including specified characters.
 *---------------------------------------------------------------------------*/
static int FSi_GetUnicodeSpanExcluding(const u16 *src, const u16 *pattern)
{
    int     pos = 0;
    BOOL    found = FALSE;
    for (; src[pos]; ++pos)
    {
        int     i;
        for (i = 0; pattern[i]; ++i)
        {
            if (src[pos] == pattern[i])
            {
                found = TRUE;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_UsingInvalidCharacterW

  Description:  Determines whether an inappropriate character is being used as the path name.
                However, the "arcname:/<***>" is allowed as a special path.

  Arguments:    path: Full path string targeted for scanning

  Returns:      TRUE if using an inappropriate character.
 *---------------------------------------------------------------------------*/
static BOOL FSi_UsingInvalidCharacterW(const u16 *path)
{
    BOOL        retval = FALSE;
    const u16  *list = FSiPathInvalidCharactersW;
    if (list && *list)
    {
        BOOL    foundLT = FALSE;
        int     pos = 0;
        // Skip archive name
        while (path[pos] && (path[pos] != L':'))
        {
            ++pos;
        }
        // Skip archive separation
        pos += (path[pos] == L':');
        pos += (path[pos] == L'/');
        // Search for prohibited characters considering special paths
        if (path[pos] == L'<')
        {
            foundLT = TRUE;
            ++pos;
        }
        // Confirm whether prohibited characters are being used
        while (path[pos])
        {
            pos += FSi_GetUnicodeSpanExcluding(&path[pos], list);
            if (path[pos])
            {
                if (foundLT && (path[pos] == L'>') && (path[pos + 1] == L'\0'))
                {
                    foundLT = FALSE;
                    pos += 1;
                }
                else
                {
                    retval = TRUE;
                    break;
                }
            }
        }
        retval |= foundLT;
        // Warn if using prohibited characters
        if (retval)
        {
            static BOOL logOnce = FALSE;
            if (!logOnce)
            {
                OS_TWarning("specified path includes invalid character '%c'\n", (char)path[pos]);
                logOnce = TRUE;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertError

  Description:  Converts FATFS library error codes to FS library error codes.

  Arguments:    error: FATFS library error codes

  Returns:      FS library error codes.
 *---------------------------------------------------------------------------*/
FSResult FSi_ConvertError(u32 error)
{
    if (error == FATFS_RESULT_SUCCESS)
    {
        return FS_RESULT_SUCCESS;
    }
    else if (error == FATFS_RESULT_BUSY)
    {
        return FS_RESULT_BUSY;
    }
    else if (error == FATFS_RESULT_FAILURE)
    {
        return FS_RESULT_FAILURE;
    }
    else if (error == FATFS_RESULT_UNSUPPORTED)
    {
        return FS_RESULT_UNSUPPORTED;
    }
    else if (error == FATFS_RESULT_INVALIDPARAM)
    {
        return FS_RESULT_INVALID_PARAMETER;
    }
    else if (error == FATFS_RESULT_ALREADYDONE)
    {
        return FS_RESULT_ALREADY_DONE;
    }
    else if (error == FATFS_RESULT_PERMISSIONDENIDED)
    {
        return FS_RESULT_PERMISSION_DENIED;
    }
    else if (error == FATFS_RESULT_NOMORERESOURCE)
    {
        return FS_RESULT_NO_MORE_RESOURCE;
    }
    else if (error == FATFS_RESULT_MEDIAFATAL)
    {
        return FS_RESULT_MEDIA_FATAL;
    }
    else if (error == FATFS_RESULT_NOENTRY)
    {
        return FS_RESULT_NO_ENTRY;
    }
    else if (error == FATFS_RESULT_MEDIANOTHING)
    {
        return FS_RESULT_MEDIA_NOTHING;
    }
    else if (error == FATFS_RESULT_MEDIAUNKNOWN)
    {
        return FS_RESULT_MEDIA_UNKNOWN;
    }
    else if (error == FATFS_RESULT_BADFORMAT)
    {
        return FS_RESULT_BAD_FORMAT;
    }
    else if (error == FATFS_RESULT_CANCELED)
    {
        return FS_RESULT_CANCELED;
    }
    else
    {
        return FS_RESULT_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFSAsyncDone

  Description:  Notifies completion of FATFS asynchronous process.

  Arguments:    buffer: Request structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_FATFSAsyncDone(FATFSRequestBuffer *buffer)
{
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)buffer->userdata;
    FS_NotifyArchiveAsyncEnd(context->arc, FSi_ConvertError(buffer->header.result));
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetArchiveCaps

  Description:  The FS_COMMAND_GETARCHIVECAPS command.

  Arguments:    arc: Calling archive
                caps: Location to save the device capability flag

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetArchiveCaps(FSArchive *arc, u32 *caps)
{
    (void)arc;
    *caps = FS_ARCHIVE_CAPS_UNICODE;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetPathInfo

  Description:  The FS_COMMAND_GETPATHINFO command.

  Arguments:    arc: Calling archive
                baseid: Base directory ID (0 for the root)
                relpath: Path
                info: Location to save file information

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetPathInfo(FSArchive *arc, u32 baseid, const char *relpath, FSPathInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileInfoW          stat[1];
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    (void)baseid;
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_GetFileInfoW(path, stat))
    {
        info->attributes = stat->attributes;
        if ((stat->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
        {
            info->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
        }
        info->filesize = stat->length;
        FSi_DostimeToFstime(&info->atime, stat->dos_atime);
        FSi_DostimeToFstime(&info->mtime, stat->dos_mtime);
        FSi_DostimeToFstime(&info->ctime, stat->dos_ctime);
        info->id = FS_INVALID_FILE_ID;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetPathInfo

  Description:  The FS_COMMAND_SETPATHINFO command.

  Arguments:    arc: Calling archive
                baseid: Base directory ID (0 for the root)
                relpath: Path
                info: Storage source of file information

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetPathInfo(FSArchive *arc, u32 baseid, const char *relpath, FSPathInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileInfoW          stat[1];
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    (void)baseid;
    FSi_FstimeToDostime(&stat->dos_atime, &info->atime);
    FSi_FstimeToDostime(&stat->dos_mtime, &info->mtime);
    FSi_FstimeToDostime(&stat->dos_ctime, &info->ctime);
    stat->attributes = (info->attributes & (FATFS_ATTRIBUTE_DOS_MASK | FATFS_PROPERTY_CTRL_MASK));
    info->attributes &= ~FATFS_PROPERTY_CTRL_MASK;
    if (((stat->attributes & FATFS_PROPERTY_CTRL_MASK) != 0) && !FSi_CheckFstime(&info->mtime))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_SetFileInfoW(path, stat))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CreateFile

  Description:  The FATFS_COMMAND_CREATE_FILE command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CreateFile(FSArchive *arc, u32 baseid, const char *relpath, u32 permit)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    permitstring[16];
    char                   *s = permitstring;
    BOOL                    tranc = TRUE;
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((permit & FS_PERMIT_R) != 0)
    {
        *s++ = 'r';
    }
    if ((permit & FS_PERMIT_W) != 0)
    {
        *s++ = 'w';
    }
    *s++ = '\0';
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_CreateFileW(path, tranc, permitstring))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_DeleteFile

  Description:  The FATFS_COMMAND_DELETE_FILE command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_DeleteFile(FSArchive *arc, u32 baseid, const char *relpath)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_DeleteFileW(path))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_RenameFile

  Description:  The FATFS_COMMAND_RENAME_FILE command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_RenameFile(FSArchive *arc, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *src = FSi_AllocUnicodeFullPath(context, relpath_src);
    u16                    *dst = FSi_AllocUnicodeFullPath(context, relpath_dst);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(src))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(dst))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_RenameFileW(src, dst))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(src);
    FSi_ReleaseUnicodeBuffer(dst);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid_src;
    (void)baseid_dst;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CreateDirectory

  Description:  The FATFS_COMMAND_CREATE_DIRECTORY command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CreateDirectory(FSArchive *arc, u32 baseid, const char *relpath, u32 permit)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    permitstring[16];
    char                   *s = permitstring;
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((permit & FS_PERMIT_R) != 0)
    {
        *s++ = 'r';
    }
    if ((permit & FS_PERMIT_W) != 0)
    {
        *s++ = 'w';
    }
    *s++ = '\0';
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_CreateDirectoryW(path, permitstring))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_DeleteDirectory

  Description:  The FATFS_COMMAND_DELETE_DIRECTORY command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_DeleteDirectory(FSArchive *arc, u32 baseid, const char *relpath)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_DeleteDirectoryW(path))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_RenameDirectory

  Description:  The FATFS_COMMAND_RENAME_DIRECTORY command.

  Arguments:    

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_RenameDirectory(FSArchive *arc, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *src = FSi_AllocUnicodeFullPath(context, relpath_src);
    u16                    *dst = FSi_AllocUnicodeFullPath(context, relpath_dst);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(src))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(dst))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_RenameDirectoryW(src, dst))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(src);
    FSi_ReleaseUnicodeBuffer(dst);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid_src;
    (void)baseid_dst;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetArchiveResource

  Description:  The FATFS_COMMAND_GETARCHIVERESOURCE command.

  Arguments:    arc: Calling archive
                resource: Storage destination of resource information

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetArchiveResource(FSArchive *arc, FSArchiveResource *resource)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, "/");
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_GetDriveResourceW(path, context->resource))
    {
        resource->totalSize = context->resource->totalSize;
        resource->availableSize = context->resource->availableSize;
        resource->maxFileHandles = context->resource->maxFileHandles;
        resource->currentFileHandles = context->resource->currentFileHandles;
        resource->maxDirectoryHandles = context->resource->maxDirectoryHandles;
        resource->currentDirectoryHandles = context->resource->currentDirectoryHandles;
        // For FAT archives
        resource->bytesPerSector = context->resource->bytesPerSector;
        resource->sectorsPerCluster = context->resource->sectorsPerCluster;
        resource->totalClusters = context->resource->totalClusters;
        resource->availableClusters = context->resource->availableClusters;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_OpenFile

  Description:  The FS_COMMAND_OPENFILE command.

  Arguments:    arc: Calling archive
                file: Target file
                baseid: Base directory (0 for the root)
                path: File path
                mode: Access mode

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_OpenFile(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSFileHandle handle = FATFS_INVALID_HANDLE;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    modestring[16];
    char                   *s = modestring;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((mode & FS_FILEMODE_R) != 0)
    {
        *s++ = 'r';
        if ((mode & FS_FILEMODE_W) != 0)
        {
            *s++ = '+';
        }
    }
    else if ((mode & FS_FILEMODE_W) != 0)
    {
        *s++ = 'w';
    }
    if ((mode & FS_FILEMODE_L) != 0)
    {
        *s++ = 'l';
    }
    *s++ = '\0';
    {
        const u16  *unipath = NULL;
        u16        *fpath = NULL;
        // Store Unicode as it is if the Unicode version structure was passed
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            unipath = (const u16 *)path;
        }
        // If not, store as Shift_JIS version
        // (However, this is a tentative implementation; in the future, nothing other than the Unicode version will be passed to archives that declare CAPS_UNICODE.)
        //  
        else
        {
            fpath = FSi_AllocUnicodeFullPath(context, path);
            unipath = fpath;
        }
        if (FSi_UsingInvalidCharacterW(unipath))
        {
            result = FS_RESULT_INVALID_PARAMETER;
        }
        else
        {
            handle = FATFS_OpenFileW(unipath, modestring);
            if (handle != FATFS_INVALID_HANDLE)
            {
                FS_SetFileHandle(file, arc, (void*)handle);
                result = FS_RESULT_SUCCESS;
            }
            else
            {
                result = FSi_ConvertError(tls->result);
            }
        }
        FSi_ReleaseUnicodeBuffer(fpath);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CloseFile

  Description:  The FS_COMMAND_CLOSEFILE command.

  Arguments:    arc: Calling archive
                file: Target file

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CloseFile(FSArchive *arc, FSFile *file)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_CloseFile(handle))
    {
        FS_DetachHandle(file);
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_ReadFile

  Description:  The FS_COMMAND_READFILE command.

  Arguments:    arc: Calling archive
                file: Target file
                buffer: Memory to transfer to
                length: Transfer size

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_ReadFile(FSArchive *arc, FSFile *file, void *buffer, u32 *length)
{
    FSResult                result = FS_RESULT_SUCCESS;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileHandle         handle = (FATFSFileHandle)FS_GetFileUserData(file);
    u32                     rest = *length;
    BOOL                    async = ((file->stat & FS_FILE_STATUS_BLOCKING) == 0);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    while (rest > 0)
    {
        void   *dst = buffer;
        u32     len = rest;
        int     read;
        // If the ARM7 cannot directly access the specified memory, substitute it with a temporary buffer
        if (!FSi_IsValidAddressForARM7(buffer, rest))
        {
            dst = context->tmpbuf;
            len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
        }
        // If the specified memory is not 32-byte aligned, substitute it with a temporary buffer
        else if ((((u32)buffer | len) & 31) != 0)
        {
            // No special adjustment is needed if the next transfer is the last
            if (len <= FS_TMPBUF_LENGTH)
            {
                dst = context->tmpbuf;
                len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
            }
            // If the start of the buffer is not aligned, align it with the next transfer
            else if (((u32)buffer & 31) != 0)
            {
                dst = context->tmpbuf;
                len = MATH_ROUNDUP((u32)buffer, 32) - (u32)buffer;
            }
            // If only the transfer size is not aligned, truncate the end and transfer directly
            else
            {
                len = MATH_ROUNDDOWN(len, 32);
            }
        }
        // Even if a temporary buffer is not used, write back a dirty cache for main memory
        // 
        if ((dst == buffer) &&
            (((u32)dst >= HW_TWL_MAIN_MEM) && ((u32)dst + len <= HW_TWL_MAIN_MEM_END)))
        {
            DC_FlushRange(dst, len);
        }
        // If an asynchronous process was requested and an asynchronous process is actually possible
        // Set a request buffer here
        async &= ((dst == buffer) && (len == rest));
        if (async)
        {
            FATFSi_SetRequestBuffer(&FSiFATFSAsyncRequest[context - FSiFATFSDrive], FSi_FATFSAsyncDone, context);
        }
        // Actually call
        read = FATFS_ReadFile(handle, dst, (int)len);
        if (async)
        {
            rest -= len;
            result = FS_RESULT_PROC_ASYNC;
            break;
        }
        // If switched with the temporary buffer, copy here
        if ((dst != buffer) && (read > 0))
        {
            DC_InvalidateRange(dst, (u32)read);
            MI_CpuCopy8(dst, buffer, (u32)read);
        }
        buffer = (u8 *)buffer + read;
        rest -= read;
        if (read != len)
        {
            result = FSi_ConvertError(tls->result);
            break;
        }
    }
    *length -= rest;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_WriteFile

  Description:  The FS_COMMAND_WRITEFILE command.

  Arguments:    arc: Calling archive
                file: Target file
                buffer: Memory to transfer from
                length: Transfer size

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_WriteFile(FSArchive *arc, FSFile *file, const void *buffer, u32 *length)
{
    FSResult                result = FS_RESULT_SUCCESS;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileHandle         handle = (FATFSFileHandle)FS_GetFileUserData(file);
    u32                     rest = *length;
    BOOL                    async = ((file->stat & FS_FILE_STATUS_BLOCKING) == 0);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    while (rest > 0)
    {
        // If the ARM7 cannot directly access the specified memory, substitute it with a temporary buffer
        const void *dst = buffer;
        u32         len = rest;
        int         written;
        if (!FSi_IsValidAddressForARM7(buffer, rest) || ((((u32)buffer | len) & 31) != 0))
        {
            dst = context->tmpbuf;
            len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
        }
        // If the specified memory is not 32-byte aligned, substitute it with a temporary buffer
        else if ((((u32)buffer | len) & 31) != 0)
        {
            // No special adjustment is needed if the next transfer is the last
            if (len <= FS_TMPBUF_LENGTH)
            {
                dst = context->tmpbuf;
                len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
            }
            // If the start of the buffer is not aligned, align it with the next transfer
            else if (((u32)buffer & 31) != 0)
            {
                dst = context->tmpbuf;
                len = MATH_ROUNDUP((u32)buffer, 32) - (u32)buffer;
            }
            // If only the transfer size is not aligned, truncate the end and transfer directly
            else
            {
                len = MATH_ROUNDDOWN(len, 32);
            }
        }
        // If a temporary buffer is used, copy data that should be written
        if (dst != buffer)
        {
            MI_CpuCopy8(buffer, context->tmpbuf, len);
        }
        // If the buffer is in main memory, data write-back is necessary to make it visible from the ARM7
        if (((u32)dst >= HW_TWL_MAIN_MEM) && ((u32)dst + len <= HW_TWL_MAIN_MEM_END))
        {
            DC_StoreRange(dst, len);
        }
        // If an asynchronous process was requested and an asynchronous process is actually possible
        // Set a request buffer here
        async &= ((dst == buffer) && (len == rest));
        if (async)
        {
            FATFSi_SetRequestBuffer(&FSiFATFSAsyncRequest[context - FSiFATFSDrive], FSi_FATFSAsyncDone, context);
        }
        // Actually call
        written = FATFS_WriteFile(handle, dst, (int)len);
        if (async)
        {
            rest -= len;
            result = FS_RESULT_PROC_ASYNC;
            break;
        }
        buffer = (u8 *)buffer + written;
        rest -= written;
        if (written != len)
        {
            result = FSi_ConvertError(tls->result);
            break;
        }
    }
    *length -= rest;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetSeekCache

  Description:  The FS_COMMAND_SETSEEKCACHE command.

  Arguments:    arc: Calling archive
                file: Target file
                buf: Cache buffer
                buf_size: Cache buffer size

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetSeekCache(FSArchive *arc, FSFile *file, void* buf, u32 buf_size)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    
    if ((buf != NULL) &&
        (((u32)buf < HW_TWL_MAIN_MEM) || (((u32)buf + buf_size) > HW_TWL_MAIN_MEM_END)))
    {
        // A buffer that is not in main memory causes a parameter error
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if ((buf != NULL) && (buf_size < 32))
    {
        // Fail if memory size is insufficient to share with the ARM7
        result = FS_RESULT_FAILURE;
    }
    else if ((((u32)buf & 31) != 0) && ((buf_size - (32 - ((u32)buf & 31))) < 32))
    {
        // Also fail if it is too small after being 32-byte aligned
        result = FS_RESULT_FAILURE;
    }
    else
    {
        void   *cache;
        u32     cache_size;
        
        if (buf != NULL) {
            // Flush this because it may have existed in the cache before being used by this function
            DC_FlushRange(buf, buf_size);
        }
        
        // Extract a region that is 32-byte aligned to allow sharing with the ARM7
        if (((u32)buf & 31) != 0) {
            cache = (void *)((u32)buf + (32 - ((u32)buf & 31)));    // 32-byte boundary
            cache_size = buf_size - (32 - ((u32)buf & 31));
        } else {
            cache = buf;
            cache_size = buf_size;
        }
        cache_size = cache_size & ~31;                              // Reduce to 32 bytes

        if( FATFS_SetSeekCache(handle, cache, cache_size) != FALSE)
        {
            result = FS_RESULT_SUCCESS;
        }
        else
        {
            result = FSi_ConvertError(tls->result);
        }
    }
    (void)arc;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SeekFile

  Description:  The FS_COMMAND_SEEKFILE command.

  Arguments:    arc: Calling archive
                file: Target file
                offset: Displacement and moved-to position
                from: Starting point to seek from

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SeekFile(FSArchive *arc, FSFile *file, int *offset, FSSeekFileMode from)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSSeekMode   mode = FATFS_SEEK_CUR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (from == FS_SEEK_SET)
    {
        mode = FATFS_SEEK_SET;
    }
    else if (from == FS_SEEK_CUR)
    {
        mode = FATFS_SEEK_CUR;
    }
    else if (from == FS_SEEK_END)
    {
        mode = FATFS_SEEK_END;
    }
    *offset = FATFS_SeekFile(handle, *offset, mode);
    if (*offset >= 0)
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetFileLength

  Description:  The FS_COMMAND_GETFILELENGTH command.

  Arguments:    arc: Calling archive
                file: Target file
                length: Location to save the obtained size

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetFileLength(FSArchive *arc, FSFile *file, u32 *length)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    int             n = FATFS_GetFileLength(handle);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (n >= 0)
    {
        *length = (u32)n;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetFileLength

  Description:  The FS_COMMAND_SETFILELENGTH command.

  Arguments:    arc: Calling archive
                file: Target file
                length: File size to be set

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetFileLength(FSArchive *arc, FSFile *file, u32 length)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_SetFileLength(handle, (int)length))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetFilePosition

  Description:  The FS_COMMAND_GETFILEPOSITION command.

  Arguments:    arc: Calling archive
                file: Target file
                position: Location to store the obtained position

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetFilePosition(FSArchive *arc, FSFile *file, u32 *position)
{
    *position = 0;
    return FSi_FATFS_SeekFile(arc, file, (int*)position, FS_SEEK_CUR);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_FlushFile

  Description:  The FS_COMMAND_FLUSHFILE command.

  Arguments:    arc: Calling archive
                file: Target file

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_FlushFile(FSArchive *arc, FSFile *file)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_FlushFile(handle))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_OpenDirectory

  Description:  The FS_COMMAND_OPENDIRECTORY command.

  Arguments:    arc: Calling archive
                file: Target file
                baseid: Base directory ID (0 for the root)
                path: Path
                mode: Access mode

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_OpenDirectory(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSDirectoryHandle    handle = FATFS_INVALID_HANDLE;
    char                    modestring[16];
    char                   *s = modestring;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((mode & FS_FILEMODE_R) != 0)
    {
        *s++ = 'r';
    }
    if ((mode & FS_FILEMODE_W) != 0)
    {
        *s++ = 'w';
    }
    if ((mode & FS_DIRMODE_SHORTNAME_ONLY) != 0)
    {
        *s++ = 's';
    }
    *s++ = '\0';
    {
        u16    *unipath = NULL;
        u16    *fpath = NULL;
        // Store Unicode as it is if the Unicode version structure was passed
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            unipath = (u16 *)path;
        }
        // If not, store as Shift_JIS version
        // (However, this is a tentative implementation; in the future, nothing other than the Unicode version will be passed to archives that declare CAPS_UNICODE.)
        //  
        else
        {
            fpath = FSi_AllocUnicodeFullPath(context, path);
            unipath = fpath;
        }
        // Disable last special wildcard specifications
        // Because the path always normalizes and passes the FS library, the possibility of changes to the buffer content is guaranteed.
        // 
        if (*unipath)
        {
            int     pos;
            for (pos = 0; unipath[pos]; ++pos)
            {
            }
            for (--pos; (pos > 0) && (unipath[pos] == L'*'); --pos)
            {
            }
            if (unipath[pos] != L'/')
            {
                unipath[++pos] = L'/';
            }
            unipath[++pos] = L'*';
            unipath[++pos] = L'\0';
        }
        handle = FATFS_OpenDirectoryW(unipath, modestring);
        if (handle != FATFS_INVALID_HANDLE)
        {
            FS_SetDirectoryHandle(file, arc, (void*)handle);
            result = FS_RESULT_SUCCESS;
        }
        else
        {
            result = FSi_ConvertError(tls->result);
        }
        FSi_ReleaseUnicodeBuffer(fpath);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CloseDirectory

  Description:  The FS_COMMAND_CLOSEDIRECTORY command.

  Arguments:    arc: Calling archive
                file: Target file

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CloseDirectory(FSArchive *arc, FSFile *file)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSDirectoryHandle    handle = (FATFSDirectoryHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_CloseDirectory(handle))
    {
        FS_DetachHandle(file);
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_ReadDirectory

  Description:  The FS_COMMAND_READDIR command.

  Arguments:    arc: Calling archive
                file: Target file
                info: Location to store the information

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_ReadDirectory(FSArchive *arc, FSFile *file, FSDirectoryEntryInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSDirectoryHandle    handle = (FATFSDirectoryHandle)FS_GetFileUserData(file);
    FATFSFileInfoW          tmp[1];
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_ReadDirectoryW(handle, tmp))
    {
        // Store Unicode as is if the Unicode version structure was passed
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            FSDirectoryEntryInfoW  *infow = (FSDirectoryEntryInfoW *)info;
            infow->shortname_length = (u32)STD_GetStringLength(tmp->shortname);
            (void)STD_CopyLString(infow->shortname, tmp->shortname, sizeof(infow->shortname));
            {
                int     n;
                for (n = 0; (n < FS_ENTRY_LONGNAME_MAX - 1) && tmp->longname[n]; ++n)
                {
                    infow->longname[n] = tmp->longname[n];
                }
                infow->longname[n] = L'\0';
                infow->longname_length = (u32)n;
            }
            infow->attributes = tmp->attributes;
            if ((tmp->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
            {
                infow->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
            }
            infow->filesize = tmp->length;
            FSi_DostimeToFstime(&infow->atime, tmp->dos_atime);
            FSi_DostimeToFstime(&infow->mtime, tmp->dos_mtime);
            FSi_DostimeToFstime(&infow->ctime, tmp->dos_ctime);
        }
        // If not, store as Shift_JIS version
        // (However, this is a tentative implementation; in the future, nothing other than the Unicode version will be passed to archives that declare CAPS_UNICODE.)
        //  
        else
        {
            FSDirectoryEntryInfo   *infoa = (FSDirectoryEntryInfo *)info;
            infoa->shortname_length = (u32)STD_GetStringLength(tmp->shortname);
            (void)STD_CopyLString(infoa->shortname, tmp->shortname, sizeof(infoa->shortname));
            {
                int     dstlen = sizeof(infoa->longname) - 1;
                int     srclen = 0;
                (void)FSi_ConvertStringUnicodeToSjis(infoa->longname, &dstlen, tmp->longname, NULL, NULL);
                infoa->longname[dstlen] = L'\0';
                infoa->longname_length = (u32)dstlen;
            }
            infoa->attributes = tmp->attributes;
            if ((tmp->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
            {
                infoa->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
            }
            infoa->filesize = tmp->length;
            FSi_DostimeToFstime(&infoa->atime, tmp->dos_atime);
            FSi_DostimeToFstime(&infoa->mtime, tmp->dos_mtime);
            FSi_DostimeToFstime(&infoa->ctime, tmp->dos_ctime);
        }
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

static PMSleepCallbackInfo  FSiPreSleepInfo;
static PMExitCallbackInfo   FSiPostExitInfo;
static void FSi_SleepProcedure(void *arg)
{
    (void)arg;
    (void)FATFS_FlushAll();
}

static void FSi_ShutdownProcedure(void *arg)
{
    (void)arg;
    (void)FATFS_UnmountAll();
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountFATFSArchive

  Description:  Mounts the FATFS archive.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_MountFATFSArchive(FSFATFSArchiveContext *context, const char *arcname, u8 *tmpbuf)
{
    static const FSArchiveInterface FSiArchiveFATFSInterface =
    {
        // Commands that are compatible with the old specifications
        FSi_FATFS_ReadFile,
        FSi_FATFS_WriteFile,
        NULL,               // SeekDirectory
        FSi_FATFS_ReadDirectory,
        NULL,               // FindPath
        NULL,               // GetPath
        NULL,               // OpenFileFast
        NULL,               // OpenFileDirect
        FSi_FATFS_CloseFile,
        NULL,               // Activate
        NULL,               // Idle
        NULL,               // Suspend
        NULL,               // Resume
        // New commands that are compatible with the old specifications
        FSi_FATFS_OpenFile,
        FSi_FATFS_SeekFile,
        FSi_FATFS_GetFileLength,
        FSi_FATFS_GetFilePosition,
        // Commands extended by the new specifications (UNSUPPORTED if NULL)
        NULL,               // Mount
        NULL,               // Unmount
        FSi_FATFS_GetArchiveCaps,
        FSi_FATFS_CreateFile,
        FSi_FATFS_DeleteFile,
        FSi_FATFS_RenameFile,
        FSi_FATFS_GetPathInfo,
        FSi_FATFS_SetPathInfo,
        FSi_FATFS_CreateDirectory,
        FSi_FATFS_DeleteDirectory,
        FSi_FATFS_RenameDirectory,
        FSi_FATFS_GetArchiveResource,
        NULL,               // 29UL
        FSi_FATFS_FlushFile,
        FSi_FATFS_SetFileLength,
        FSi_FATFS_OpenDirectory,
        FSi_FATFS_CloseDirectory,
        FSi_FATFS_SetSeekCache,
    };
    u32     arcnamelen = (u32)STD_GetStringLength(arcname);
    context->tmpbuf = tmpbuf;
    FS_InitArchive(context->arc);
    if (FS_RegisterArchiveName(context->arc, arcname, arcnamelen))
    {
        (void)FS_MountArchive(context->arc, context, &FSiArchiveFATFSInterface, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountDefaultArchives

  Description:  Mounts the default archive by referencing a startup argument given to IPL.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_MountDefaultArchives(void)
{
    // Wait until FATFS mounts various physical media
    FATFS_Init();
    FSi_SetupFATBuffers();
    // Set the callback to set flash before sleep and before shutdown
    FSiPreSleepInfo.callback = FSi_SleepProcedure;
    FSiPostExitInfo.callback = FSi_ShutdownProcedure;
    PMi_InsertPreSleepCallbackEx(&FSiPreSleepInfo, PM_CALLBACK_PRIORITY_FS);
    PMi_InsertPostExitCallbackEx(&FSiPostExitInfo, PM_CALLBACK_PRIORITY_FS);
    // Mount to FS only the physical drive that actually has access rights
    if (FSiFATFSDrive && FSiTemporaryBuffer)
    {
        int                 index = 0;
        static const int    max = FS_MOUNTDRIVE_MAX;
        const char         *arcname = FATFSi_GetArcnameList();
        while ((index < max) && *arcname)
        {
            FSi_MountFATFSArchive(&FSiFATFSDrive[index], arcname, &FSiTemporaryBuffer[FS_TMPBUF_LENGTH * index]);
            arcname += STD_GetStringLength(arcname) + 1;
            ++index;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSpecialArchive

  Description:  Directly mounts a special archive.

  Arguments:    param: Mounting target
                arcname: Archive name to mount
                          "otherPub" or "otherPrv" can be specified; when NULL is specified, unmount the previous archive
                          
                pWork: Work region used for mounting
                          It is necessary to retain while mounted

  Returns:      FS_RESULT_SUCCESS if processing was successful.
 *---------------------------------------------------------------------------*/
FSResult FSi_MountSpecialArchive(u64 param, const char *arcname, FSFATFSArchiveWork* pWork)
{
    FSResult    result = FS_RESULT_ERROR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (!FATFS_MountSpecial(param, arcname, &pWork->slot))
    {
        result = FSi_ConvertError(tls->result);
    }
    else
    {
        if (arcname && *arcname)
        {
            FSi_MountFATFSArchive(&pWork->context, arcname, pWork->tmpbuf);
        }
        else
        {
            (void)FS_UnmountArchive(pWork->context.arc);
            (void)FS_ReleaseArchiveName(pWork->context.arc);
        }
        result = FS_RESULT_SUCCESS;
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FormatSpecialArchive

  Description:  Formats content of special archive that satisfies the following conditions.
                  - Currently mounted
                  - Hold your own ownership rights (dataPub, dataPrv, share*)

  Arguments:    path: Path name that includes the archive name

  Returns:      FS_RESULT_SUCCESS if processing was successful.
 *---------------------------------------------------------------------------*/
FSResult FSi_FormatSpecialArchive(const char *path)
{
    FSResult    result = FS_RESULT_ERROR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (!FATFS_FormatSpecial(path))
    {
        result = FSi_ConvertError(tls->result);
    }
    else
    {
        result = FS_RESULT_SUCCESS;
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}


#include <twl/ltdmain_end.h>


#endif /* FS_IMPLEMENT */

#endif /* SDK_TWL */

/*---------------------------------------------------------------------------*
  Name:         FS_ForceToEnableLatencyEmulation

  Description:  Pseudo-reproduction of random driver latency generated when accessing a degraded NAND device.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ForceToEnableLatencyEmulation(void)
{
#if defined(SDK_TWL)
    if (OS_IsRunOnTwl())
    {
        (void)FATFS_SetLatencyEmulation(TRUE);
    }
#endif
}
