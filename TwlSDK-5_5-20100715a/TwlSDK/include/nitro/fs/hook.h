/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     hook.h

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


#ifndef NITRO_FS_HOOK_H_
#define NITRO_FS_HOOK_H_


#include <nitro/fs/archive.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

typedef u32 FSEvent;
#define FS_EVENT_NONE           0x00000000
#define FS_EVENT_MEDIA_REMOVED  0x00000001
#define FS_EVENT_MEDIA_INSERTED 0x00000002


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef void (*FSEventFunction)(void *userdata, FSEvent event, void *argument);
typedef struct FSEventHook
{
    struct FSEventHook *next;
    FSEventFunction     callback;
    void               *userdata;
    FSArchive          *arc;
}
FSEventHook;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterEventHook

  Description:  Registers hooks for internal archive events.

  Arguments:    arc: The archive for which to register
                hook: The FSEventHook structure to register
                callback: The callback function to invoke when an event occurs
                userdata: Arbitrary user-defined data (NULL when this is not required)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_RegisterEventHook(const char *arc, FSEventHook *hook, FSEventFunction callback, void *userdata);

/*---------------------------------------------------------------------------*
  Name:         FS_UnregisterEventHook

  Description:  Unregisters hooks for registered internal archived events.

  Arguments:    hook: A registered FSEventHook structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_UnregisterEventHook(FSEventHook *hook);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_HOOK_H_ */
