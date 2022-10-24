/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - include
  File:     api.h

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
#if	!defined(NITRO_FS_API_H_)
#define NITRO_FS_API_H_


#include <nitro/fs/archive.h>
#include <nitro/fs/file.h>
#include <nitro/fs/romfat.h>
#include <nitro/fs/overlay.h>
#include <nitro/std.h>

#ifdef SDK_TWL
#include <twl/fatfs.h>
#endif // SDK_TWL

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_Init

  Description:  Initializes the FS library.

  Arguments:    default_dma_no: The DMA number to use internally, as necessary. (0-3)
                                 Add MI_DMA_USING_NEW when using the new DMA, and specify MI_DMA_NOT_USE when not using DMA.
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_Init(u32 default_dma_no);

/*---------------------------------------------------------------------------*
  Name:         FS_InitFatDriver

  Description:  Initializes the FAT driver.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_TWL)
void FS_InitFatDriver(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_IsAvailable

  Description:  Determines if the FS library has already been initialized.

  Arguments:    None.

  Returns:      TRUE if the FS_Init function has already been called.
 *---------------------------------------------------------------------------*/
BOOL    FS_IsAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         FS_End

  Description:  Unmounts all archives and shuts down the FS library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_End(void);

/*---------------------------------------------------------------------------*
  Name:         FS_GetDefaultDMA

  Description:  Gets the DMA channel currently configured for FS library use.

  Arguments:    None.

  Returns:      current DMA channel
 *---------------------------------------------------------------------------*/
u32     FS_GetDefaultDMA(void);

/*---------------------------------------------------------------------------*
  Name:         FS_SetDefaultDMA

  Description:  Sets the DMA channel for the FS library to use.

  Arguments:    dma_no: The DMA number to use internally, as necessary. (0-3)
                            Add MI_DMA_USING_NEW when using the new DMA, and specify MI_DMA_NOT_USE when not using DMA.
                            

  Returns:      The DMA channel that had previously been set.
 *---------------------------------------------------------------------------*/
u32     FS_SetDefaultDMA(u32 dma_no);

/*---------------------------------------------------------------------------*
  Name:         FS_SetCurrentDirectory

  Description:  Changes the current directory

  Arguments:    path: Path name

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_SetCurrentDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromMemory

  Description:  Temporarily generates file that maps memory region.

  Arguments:    file: FSFile structure that stores a file handle
                buf:              Memory that is target of READ and WRITE
                size:            Byte size of buf

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFileFromMemory(FSFile *file, void *buf, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromRom

  Description:  Temporarily generates a file that maps a specified CARD-ROM region.

  Arguments:    file: FSFile structure that stores a file handle
                offset: Offset from the start of the CARD-ROM region to be read
                size: Size in bytes from the offset in the target region

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFileFromRom(FSFile *file, u32 offset, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenTopLevelDirectory

  Description:  Opens the special high-level directory that can enumerate the archive name

  Arguments:    dir:           FSFile structure that stores directory handle

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenTopLevelDirectory(FSFile *dir);

/*---------------------------------------------------------------------------*
  Name:         FS_TryLoadTable

  Description:  Preloads a "rom" archive's table data.

  Arguments:    mem: Buffer to store the table data
                                 If NULL, simply calculate and return the size.
                size: The size of mem

  Returns:      The number of bytes required to preload a "rom" archive's FAT and FNT data.
 *---------------------------------------------------------------------------*/
u32     FS_TryLoadTable(void *mem, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_GetTableSize

  Description:  Gets a "ROM" archive's table size.

  Arguments:    None.

  Returns:      The number of bytes required to preload a "rom" archive's FAT and FNT data.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 FS_GetTableSize(void)
{
    return FS_TryLoadTable(NULL, 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadTable

  Description:  Preloads a "rom" archive's table data.

  Arguments:    mem: Buffer to store the table data
                size: The size of mem

  Returns:      TRUE if data was preloaded successfully.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_LoadTable(void *mem, u32 size)
{
    return (FS_TryLoadTable(mem, size) <= size);
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadTable

  Description:  Releases the table preloaded for a "rom" archive.

  Arguments:    None.

  Returns:      Pointer to the buffer assigned for the table.
 *---------------------------------------------------------------------------*/
SDK_INLINE void *FS_UnloadTable(void)
{
    return FS_UnloadArchiveTables(FS_FindArchive("rom", 3));
}

/*---------------------------------------------------------------------------*
  Name:         FS_ForceToEnableLatencyEmulation

  Description:  Uses a driver to emulate the random wait times that occur when accessing a degraded NAND device.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ForceToEnableLatencyEmulation(void);


/*---------------------------------------------------------------------------*
 * Internal functions
 *---------------------------------------------------------------------------*/

#define FS_TMPBUF_LENGTH        2048
#define FS_MOUNTDRIVE_MAX       OS_MOUNT_INFO_MAX
#define FS_TEMPORARY_BUFFER_MAX (FS_TMPBUF_LENGTH * FS_MOUNTDRIVE_MAX)

#ifdef SDK_TWL
typedef struct FSFATFSArchiveContext
{
    FSArchive       arc[1];
    char            fullpath[2][FATFS_PATH_MAX];
    u8             *tmpbuf;
    FATFSDriveResource  resource[1];
}
FSFATFSArchiveContext;

typedef struct FSFATFSArchiveWork
{
    u8 tmpbuf[FS_TMPBUF_LENGTH];
    FSFATFSArchiveContext context;
    int slot;
}
FSFATFSArchiveWork;


/*---------------------------------------------------------------------------*
  Name:         FSiFATFSDrive

  Description:  Context array that manages the archive of a FAT base
                Ordinarily, an array size for the quantity of FS_MOUNTDRIVE_MAX is required, and in default, static variables in the LTDMAIN segment are used.
                With applications built using a special memory arrangment, it is possible to set the proper buffer by changing this variable internally by calling the FSi_SetupFATBuffers function that was over-ridden.
                
                
                

  Arguments:    None.

  Returns:      Buffer for the amount of the FS_MOUNTDRIVE_MAX of the FSFATFSArchiveContext structure
 *---------------------------------------------------------------------------*/
extern FSFATFSArchiveContext *FSiFATFSDrive;

/*---------------------------------------------------------------------------*
  Name:         FSiFATFSAsyncRequest

  Description:  Command buffer that is used in asynchronous processing by the FAT base archive.
                Ordinarily, an array size for the quantity of FS_MOUNTDRIVE_MAX is required, and in default, static variables in the LTDMAIN segment are used.
                With applications built using a special memory arrangment, it is possible to set the proper buffer by changing this variable internally by calling the FSi_SetupFATBuffers function that was over-ridden.
                
                
                

  Arguments:    None.

  Returns:      Static command buffer of FS_TEMPORARY_BUFFER_MAX bytes.
 *---------------------------------------------------------------------------*/
extern FATFSRequestBuffer *FSiFATFSAsyncRequest;

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSpecialArchive

  Description:  Directly mounts a special archive

  Arguments:    param: Parameter that specifies the mounting target
                arcname: Archive name to mount
                          "otherPub" or "otherPrv" can be specified; when NULL is specified, unmount the previous archive
                          
                pWork: Work region that is used for mounting
                          It is necessary to retain while mounted

  Returns:      FS_RESULT_SUCCESS if processing was successful.
 *---------------------------------------------------------------------------*/
FSResult FSi_MountSpecialArchive(u64 param, const char *arcname, FSFATFSArchiveWork* pWork);

/*---------------------------------------------------------------------------*
  Name:         FSi_FormatSpecialArchive

  Description:  Formats content of special archive that satisfies the following conditions
                  - Currently mounted
                  - Hold your own ownership rights (dataPub, dataPrv, share*)

  Arguments:    path: Path name that includes the archive name

  Returns:      FS_RESULT_SUCCESS if processing was successful.
 *---------------------------------------------------------------------------*/
FSResult FSi_FormatSpecialArchive(const char *path);

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         FSiTemporaryBuffer

  Description:  Pointer to the static temporary buffer for issuing read/write commands.
                It must be memory that can be referenced by both ARM9/ARM7, and by default a static variable in the LTDMAIN segment is used.
                For applications constructed using a special memory layout, you can change these variables to set the appropriate buffer before calling the FS_Init function.
                
                
                

  Arguments:    None.

  Returns:      Static command buffer of FS_TEMPORARY_BUFFER_MAX bytes.
 *---------------------------------------------------------------------------*/
extern u8 *FSiTemporaryBuffer/* [FS_TEMPORARY_BUFFER_MAX] ATTRIBUTE_ALIGN(32)*/;

/*---------------------------------------------------------------------------*
  Name:         FSi_SetupFATBuffers

  Description:  Initializes various buffers needed by the FAT base archive.
                With applications built using a special memory arrangment, it is possible to independently set the various buffers by over-riding this variable and suppressing the required memory size.
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_SetupFATBuffers(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_OverrideRomArchive

  Description:  Replaces the ROM archive content as necessary.

  Arguments:    arc              ROM archive structure that should be mounted.

  Returns:      It is necessary to return TRUE if replacing with a different implementation without using the standard ROM archive on the card.
                
 *---------------------------------------------------------------------------*/
BOOL FSi_OverrideRomArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FSi_IsValidAddressForARM7

  Description:  Determines whether this is a buffer accessible from ARM7.
                With applications built using a special memory layout, it is possible to redefine the proper determination routine by overriding this function.
                
                

  Arguments:    buffer     Buffer to be determined
                length: Buffer length

  Returns:      TRUE if it is a buffer accessible from ARM7.
 *---------------------------------------------------------------------------*/
BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FSi_SetSwitchableWramSlots

  Description:  Specifies WRAM slot that the FS library can switch to ARM7 depending on the circumstances.

  Arguments:    bitsB: WRAM-B slot bit collection
                bitsC: Bitset of WRAM-C slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_SetSwitchableWramSlots(int bitsB, int bitsC);

/*---------------------------------------------------------------------------*
  Name:         FSi_UnmountRomAndCloseNANDSRL

  Description:  Disables the ROM archive for a NAND application and closes the SRL file.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_UnmountRomAndCloseNANDSRL(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringSjisToUnicode

  Description:  Converts a Shift JIS string into a Unicode string.
                When the path name being handled is clearly only in ASCII code, and the mutual conversion of Unicode and ShiftJIS can be simplified, it is possible to prevent the standard processes of the STD library from linking by overwriting this function.
                
                
                

  Arguments:    dst:               Conversion destination buffer
                                  The storage process is ignored if NULL is specified.
                dst_len:           Pointer which stores and passes the maximum number of characters for the conversion destination buffer, then receives the number of characters that were actually stored.
                                  Ignored when NULL is given.
                                  
                src:               Conversion source buffer
                src_len           Pointer which stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                                  The end-of-string position takes priority over this specification.
                                  When either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                                  
                callback:          The callback to be called if there are any characters that can't be converted.
                                  When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback);

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringUnicodeToSjis

  Description:  Converts a Unicode character string into a ShiftJIS character string.
                When the path name being handled is clearly only in ASCII code, and the mutual conversion of Unicode and ShiftJIS can be simplified, it is possible to prevent the standard processes of the STD library from linking by overwriting this function.
                
                
                

  Arguments:    dst:               Conversion destination buffer
                                  The storage process is ignored if NULL is specified.
                dst_len:           Pointer which stores and passes the maximum number of characters for the conversion destination buffer, then receives the number of characters that were actually stored.
                                  Ignored when NULL is given.
                                  
                src:               Conversion source buffer
                src_len           Pointer which stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                                  The end-of-string position takes priority over this specification.
                                  When either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                                  
                callback: The callback to be called if there are any characters that can't be converted.
                                  When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeBuffer

  Description:  Gets temporary buffer for Unicode conversion.
                The FS library is used for conversion of Shift_JIS.

  Arguments:    src: Shift_JIS string needed in Unicode conversion or NULL

  Returns:      String buffer converted to UTF16-LE if necessary
 *---------------------------------------------------------------------------*/
u16* FSi_GetUnicodeBuffer(const char *src);

/*---------------------------------------------------------------------------*
  Name:         FSi_ReleaseUnicodeBuffer

  Description:  Deallocates the temporary buffer for Unicode conversion

  Arguments:    buf: Buffer allocated by the FSi_GetUnicodeBuffer function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ReleaseUnicodeBuffer(const void *buf);


/*---------------------------------------------------------------------------*
 * Deprecated Functions
 *---------------------------------------------------------------------------*/

#define	FS_DMA_NOT_USE	MI_DMA_NOT_USE

/*---------------------------------------------------------------------------*
  Name:         FS_ChangeDir

  Description:  Changes the current directory

  Arguments:    path: Path name

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_ChangeDir(const char *path);


#ifdef SDK_ARM7
#define FS_CreateReadServerThread(priority) (void)CARD_SetThreadPriority(priority)
#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_API_H_ */
