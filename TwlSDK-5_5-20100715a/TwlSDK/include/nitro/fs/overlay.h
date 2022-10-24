/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - include
  File:     overlay.h

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


#if	!defined(NITRO_FS_OVERLAY_H_)
#define NITRO_FS_OVERLAY_H_


#include <nitro/fs/file.h>
#include <nitro/mi.h>
#include <nitro/card/rom.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Overlay ID
typedef u32 FSOverlayID;
#define	FS_EXTERN_OVERLAY(name)     extern u32 SDK_OVERLAY_ ## name ## _ID[1]
#define	FS_OVERLAY_ID(name)         ((u32)&(SDK_OVERLAY_ ## name ## _ID))
#define	FS_EXTERN_LTDOVERLAY(name)  extern u32 SDK_LTDOVERLAY_ ## name ## _ID[1]
#define	FS_LTDOVERLAY_ID(name)      ((u32)&(SDK_LTDOVERLAY_ ## name ## _ID))

typedef void (*FSOverlayInitFunc) (void);

// Structure with overlay header information
typedef struct FSOverlayInfoHeader
{
// Protected:
    u32     id;
    u8     *ram_address;
    u32     ram_size;
    u32     bss_size;
    FSOverlayInitFunc *sinit_init;
    FSOverlayInitFunc *sinit_init_end;
    u32     file_id;
    u32     compressed:24;
    u32     flag:8;
}
FSOverlayInfoHeader;

// Structure with overlay information
typedef struct FSOverlayInfo
{
// Private:
    FSOverlayInfoHeader header;
    MIProcessor         target;
    CARDRomRegion       file_pos;
}
FSOverlayInfo;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_InitOverlay

  Description:  Initializes overlay management information.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_InitOverlay(void);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadOverlayInfo

  Description:  Load the information of specified overlay module.

  Arguments:    p_ovi:       pointer to destination FSOverlayInfo
                target:      ARM9 or ARM7
                id:          overlay-id (FS_OVERLAY_ID(overlay-name))

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadOverlayInfo(FSOverlayInfo *p_ovi, MIProcessor target, FSOverlayID id);

/*---------------------------------------------------------------------------*
  Name:         FS_GetOverlayTotalSize

  Description:  Get total size of overlay module from FSOverlayInfo.
                (text section + data section + bss section)

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      total size of overlay module
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 FS_GetOverlayTotalSize(const FSOverlayInfo *p_ovi)
{
    return (u32)(p_ovi->header.ram_size + p_ovi->header.bss_size);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetOverlayImageSize

  Description:  Get image size of overlay module from FSOverlayInfo.
                (text section + data section)

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      Image size of overlay module.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 FS_GetOverlayImageSize(const FSOverlayInfo *p_ovi)
{
    return (u32)(p_ovi->header.ram_size);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetOverlayAddress

  Description:  Get start-address of overlay module from FSOverlayInfo.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      Start-address of overlay module.
 *---------------------------------------------------------------------------*/
SDK_INLINE void *FS_GetOverlayAddress(const FSOverlayInfo *p_ovi)
{
    return p_ovi->header.ram_address;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetOverlayFileID

  Description:  Get file-id which contains an image of overlay module.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      File-id of overlay.
 *---------------------------------------------------------------------------*/
FSFileID FS_GetOverlayFileID(const FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_ClearOverlayImage

  Description:  Zero out the memory where FS_GetOverlayAddress() points,
                and invalidate its region of cache.
                This function is called in FS_LoadOverlayImage().

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_ClearOverlayImage(FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadOverlayImage

  Description:  Load the image of overlay module without 'static initializer'.
                By this call, the memory where FS_GetOverlayAddress() points
                is set to rare initial status.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadOverlayImage(FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadOverlayImageAsync

  Description:  Load the image of overlay module without 'static initializer'.
                By this call, the memory where FS_GetOverlayAddress() points
                is set to rare initial status.

  Arguments:    p_ovi:       pointer to FSOverlayInfo
                p_file:      pointer to FSFile for asynchronous reading

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadOverlayImageAsync(FSOverlayInfo *p_ovi, FSFile *p_file);

/*---------------------------------------------------------------------------*
  Name:         FS_StartOverlay

  Description:  Execute 'static initializer'.
                This function needs:
                    the memory where FS_GetOverlayAddress() points is
                    set by data which FS_LoadOverlayImage() makes.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_StartOverlay(FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_EndOverlay

  Description:  Execute 'static unintinalizer'.
                By this function:
                    all the destructors in specified overlay module
                    are removed from global destructor chain and executed.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_EndOverlay(FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadOverlayImage

  Description:  Unload overlay module.

  Arguments:    p_ovi:       pointer to FSOverlayInfo

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_UnloadOverlayImage(FSOverlayInfo *p_ovi);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadOverlay

  Description:  Load overlay module and initialize.

  Arguments:    target:      ARM9 or ARM7
                id:          overlay-id (FS_OVERLAY_ID(overlay-name))

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadOverlay(MIProcessor target, FSOverlayID id);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadOverlay

  Description:  Unload overlay module and clean up.

  Arguments:    target:      ARM9 or ARM7
                id:          overlay-id (FS_OVERLAY_ID(overlay-name))

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    FS_UnloadOverlay(MIProcessor target, FSOverlayID id);

/*---------------------------------------------------------------------------*
  Name:         FS_AttachOverlayTable

  Description:  Attach the overlayinfo-table (OVT) to file-system.
                After setting, FS_LoadOverlayInfo()
                loads overlay-info from specified memory.

  Arguments:    target:      ARM9 or ARM7
                ptr:         pointer to buffer which contains OVT.
                            If ptr is NULL, reset to default(from CARD).
                len:         length of OVT.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_AttachOverlayTable(MIProcessor target, const void *ptr, u32 len);


/* Obsolete */

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterOverlayToDebugger

  Description:  Register specified overlay to debugger so that the debugger can trace
                its runtime information.
                (obsolete)

  Arguments:    p_ovi:       ignored
                p_file:      ignored

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_RegisterOverlayToDebugger(const FSOverlayInfo *p_ovi, const FSFile *p_file)
{
    (void)p_ovi;
    (void)p_file;
}


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_OVERLAY_H_ */
