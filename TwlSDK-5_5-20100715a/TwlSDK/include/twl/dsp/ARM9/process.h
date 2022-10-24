/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_process.h

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-21#$
  $Rev: 9018 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_PROCESS_H_
#define TWL_DSP_PROCESS_H_


/*---------------------------------------------------------------------------*
 * Only TWL systems can use this module.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL


#include <twl/dsp.h>
#include <twl/dsp/common/byteaccess.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// WRAM-B/C slot size.
#define DSP_WRAM_SLOT_SIZE (32 * 1024)

// Bit definition macros used for hook registration.
#define DSP_HOOK_SEMAPHORE_(id) (1 << (id))
#define DSP_HOOK_REPLY_(id)     (1 << (16 + (id)))
#define DSP_HOOK_MAX            (16 + 3)

// Flag that determines whether to use SDK-specific synchronous processing at startup.
#define DSP_PROCESS_FLAG_USING_OS   0x0001

/*---------------------------------------------------------------------------*/
/* Declarations */

// Hook function prototype for DSP interrupts
typedef void (*DSPHookFunction)(void *userdata);

// This structure holds process memory information.
// DSP processes are loaded with the following procedure.
//   
//       (1) List the section tables for a process image and calculate the segment placement state (segment map) to utilize.
//   
//       (2) Select the actual DSP segment from one of the WRAM-B/C slots that the DSP is allowed to use and then fix the table (slot map) that maps with each segment.
//       
//   (3) Re-list the process image's section tables and then actually place the process image in WRAM-B/C.
//       
typedef struct DSPProcessContext
{
    // Unidirectional list and process name to support multi-processes.
    struct DSPProcessContext   *next;
    char                        name[15 + 1];
    // The file handle that stores the process image, and usable WRAM.
    // This is used temporarily only during load processing.
    FSFile                     *image;
    u16                         slotB;
    u16                         slotC;
    int                         flags;
    // The segment map held by the process on the DSP.
    // This is calculated by the DSP_MapProcessSegment function.
    int                         segmentCode;
    int                         segmentData;
    // The slot map assigned to each segment.
    // The mapper is placed by the DSP_StartupProcess function.
    int                         slotOfSegmentCode[8];
    int                         slotOfSegmentData[8];
    // Hook functions registered for the various factors that lead to DSP interrupts.
    // The DSP_SetProcessHook function registers these separately for each factor.
    int                         hookFactors;
    DSPHookFunction             hookFunction[DSP_HOOK_MAX];
    void                       *hookUserdata[DSP_HOOK_MAX];
    int                         hookGroup[DSP_HOOK_MAX];
}
DSPProcessContext;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_StopDSPComponent

  Description:  Prepares to shut down the DSP.
                Currently, this only stops DMAs for the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StopDSPComponent(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_InitProcessContext

  Description:  Initializes a process information structure.

  Arguments:    context: DSPProcessContext structure
                name: The process name or NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitProcessContext(DSPProcessContext *context, const char *name);

/*---------------------------------------------------------------------------*
  Name:         DSP_ExecuteProcess

  Description:  Loads and starts a DSP process image.

  Arguments:    context: A DSPProcessContext structure to use for state management.
                          This will be accessed by the system until the process is destroyed.
                image: File handle that points to a process image.
                          This will only be accessed within this function.
                slotB: WRAM-B that was allowed to be used for code memory.
                slotC: WRAM-C that was allowed to be used for data memory.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_ExecuteProcess(DSPProcessContext *context, FSFile *image, int slotB, int slotC);

/*---------------------------------------------------------------------------*
  Name:         DSP_QuitProcess

  Description:  Shuts down a DSP process and releases memory.

  Arguments:    context: A DSPProcessContext structure to use for state management.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_QuitProcess(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_FindProcess

  Description:  Finds a running process.

  Arguments:    name: The process name.
                       If NULL is specified, this will find the last process that was registered.

  Returns:      A DSPProcessContext structure that matches the specified name.
 *---------------------------------------------------------------------------*/
DSPProcessContext* DSP_FindProcess(const char *name);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetProcessSlotFromSegment

  Description:  Gets the WRAM slot number that corresponds to the specified segment number.

  Arguments:    context: DSPProcessContext structure
                wram: MI_WRAM_B/MI_WRAM_C.
                segment: Segment number.

  Returns:      The WRAM slot number that corresponds to the specified segment number.
 *---------------------------------------------------------------------------*/
SDK_INLINE int DSP_GetProcessSlotFromSegment(const DSPProcessContext *context, MIWramPos wram, int segment)
{
    return (wram == MI_WRAM_B) ? context->slotOfSegmentCode[segment] : context->slotOfSegmentData[segment];
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ConvertProcessAddressFromDSP

  Description:  Converts an address in DSP memory space to a WRAM address.

  Arguments:    context: DSPProcessContext structure
                wram: MI_WRAM_B/MI_WRAM_C.
                address: An address in DSP memory space. (in units of words in DSP memory space)

  Returns:      The WRAM address that corresponds to the specified DSP address.
 *---------------------------------------------------------------------------*/
SDK_INLINE void* DSP_ConvertProcessAddressFromDSP(const DSPProcessContext *context, MIWramPos wram, int address)
{
    int     segment = (address / (DSP_WRAM_SLOT_SIZE/2));
    int     mod = (address - segment * (DSP_WRAM_SLOT_SIZE/2));
    int     slot = DSP_GetProcessSlotFromSegment(context, wram, segment);
    SDK_ASSERT((slot >= 0) && (slot < MI_WRAM_B_MAX_NUM));
    return (u8*)MI_GetWramMapStart(wram) + slot * DSP_WRAM_SLOT_SIZE + mod * 2;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_AttachProcessMemory

  Description:  Assigns contiguous memory space to unused process regions.

  Arguments:    context: DSPProcessContext structure
                wram: The memory space to assign. (MI_WRAM_B/MI_WRAM_C)
                slots: WRAM slots to assign.

  Returns:      The starting address of the assigned DSP memory space or 0.
 *---------------------------------------------------------------------------*/
u32 DSP_AttachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots);

/*---------------------------------------------------------------------------*
  Name:         DSP_DetachProcessMemory

  Description:  Releases the WRAM slots assigned to used process regions.

  Arguments:    context: DSPProcessContext structure
                slots: Assigned WRAM slots.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DetachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots);

/*---------------------------------------------------------------------------*
  Name:         DSP_SwitchProcessMemory

  Description:  Switches the DSP address management being used by the specified process.

  Arguments:    context: DSPProcessContext structure
                wram: The memory space to switch. (MI_WRAM_B/MI_WRAM_C)
                address: The starting address to switch. (in units of words in DSP memory space)
                length: The memory size to switch. (in units of words in DSP memory space)
                to: The new master processor.

  Returns:      TRUE if all slots are switched successfully.
 *---------------------------------------------------------------------------*/
BOOL DSP_SwitchProcessMemory(DSPProcessContext *context, MIWramPos wram, u32 address, u32 length, MIWramProc to);

/*---------------------------------------------------------------------------*
  Name:         DSP_SetProcessHook

  Description:  Sets a hook for a factor that causes the DSP to interrupt a process.

  Arguments:    context: DSPProcessContext structure
                factors: Bitset to specify the interrupt causes.
                function: The hook function that should be called.
                userdata: Arbitrary user-defined argument to pass to the hook function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetProcessHook(DSPProcessContext *context, int factors, DSPHookFunction function, void *userdata);

/*---------------------------------------------------------------------------*
  Name:         DSPi_CreateMemoryFile

  Description:  Converts a static DSP process image into a memory file.

  Arguments:    memfile: The FSFile structure to turn into a memory file.
                image: A buffer storing the DSP process image.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSPi_CreateMemoryFile(FSFile *memfile, const void *image)
{
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    FS_InitFile(memfile);
    return FS_CreateFileFromMemory(memfile, (void *)image, (u32)(16 * 1024 * 1024));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadProcessPipe

  Description:  Receives data from the pipe associated with the designated process port.

  Arguments:    context: DSPProcessContext structure
                port: The port to receive data from.
                buffer: The received data.
                length: The received data size. (in bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadProcessPipe(DSPProcessContext *context, int port, void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_WriteProcessPipe

  Description:  Sends data to the pipe associated with the designated process port.

  Arguments:    context: DSPProcessContext structure
                port: The port to send data to.
                buffer: The data to send.
                length: The size of the data to send. (in bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WriteProcessPipe(DSPProcessContext *context, int port, const void *buffer, u32 length);

#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * The following is a discontinued candidate interface that is not thought to be used at the present.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_MapProcessSegment

  Description:  Calculates the segment map for a process to own.

  Arguments:    context: DSPProcessContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MapProcessSegment(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadProcessImage

  Description:  Loads the specified process image.
                The segment map must already be calculated.

  Arguments:    context: DSPProcessContext structure

  Returns:      TRUE if all images were loaded successfully.
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadProcessImage(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_StartupProcess

  Description:  Calculates the segment map for a process image and loads it into WRAM.
                This is a combination of the DSP_MapProcessSegment and DSP_LoadProcessImage functions.

  Arguments:    context: DSPProcessContext structure
                image: File handle that indicates a process image.
                             This will only be accessed within this function.
                slotB: WRAM-B that was allowed to be used for code memory.
                slotC: WRAM-C that was allowed to be used for data memory.
                slotMapper: The algorithm to assign WRAM slots to segments.
                             If NULL is specified, the appropriate default processing will be selected.

  Returns:      TRUE if all images were loaded successfully.
 *---------------------------------------------------------------------------*/
BOOL DSP_StartupProcess(DSPProcessContext *context, FSFile *image,
                        int slotB, int slotC, BOOL (*slotMapper)(DSPProcessContext *, int, int));


#endif


#ifdef __cplusplus
} // extern "C"
#endif


#endif // SDK_TWL


/*---------------------------------------------------------------------------*/
#endif // TWL_DSP_PROCESS_H_
