/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_process.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-01-08#$
  $Rev: 11275 $
  $Author: kakemizu_hironori $
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>

#include "dsp_coff.h"


/*---------------------------------------------------------------------------*/
/* Constants */

#define DSP_DPRINTF(...)    ((void) 0)
//#define DSP_DPRINTF OS_TPrintf

// Section enumeration callback in the process image
typedef BOOL (*DSPSectionEnumCallback)(DSPProcessContext *, const COFFFileHeader *, const COFFSectionTable *);


/*---------------------------------------------------------------------------*/
/* Variables */

// Process context currently running
static DSPProcessContext   *DSPiCurrentComponent = NULL;
static PMExitCallbackInfo   DSPiShutdownCallbackInfo[1];
static BOOL                 DSPiShutdownCallbackIsRegistered = FALSE;
static PMSleepCallbackInfo  DSPiPreSleepCallbackInfo[1];
static BOOL                 DSPiPreSleepCallbackIsRegistered = FALSE;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_EnumSections

  Description:  Runs the predetermined processes by enumerating sections in the process image

  Arguments:    context: DSPProcessContext structure passed to the callback function
                callback: Callback called for each section

  Returns:      TRUE if all sections are enumerated; FALSE if ended prematurely.
 *---------------------------------------------------------------------------*/
BOOL DSP_EnumSections(DSPProcessContext *context, DSPSectionEnumCallback callback);

/*---------------------------------------------------------------------------*
  Name:         DSP_StopDSPComponent

  Description:  Prepares to end DSP.
                Currently, only when stopping the DMA of DSP

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StopDSPComponent(void)
{
    DSPProcessContext  *context = DSPiCurrentComponent;
    context->hookFactors = 0;
    DSP_SendData(DSP_PIPE_COMMAND_REGISTER, DSP_PIPE_FLAG_EXIT_OS);
    (void)DSP_RecvData(DSP_PIPE_COMMAND_REGISTER);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ShutdownCallback

  Description:  Forced system shutdown termination callback.

  Arguments:    name: Process name
                       Search for processes registered last if NULL is specified

  Returns:      DSPProcessContext structure that matches the specified name
 *---------------------------------------------------------------------------*/
static void DSPi_ShutdownCallback(void *arg)
{
    (void)arg;
    for (;;)
    {
        DSPProcessContext  *context = DSP_FindProcess(NULL);
        if (!context)
        {
            break;
        }
//        OS_TWarning("force-exit %s component.\n", context->name);
        DSP_QuitProcess(context);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PreSleepCallback

  Description:  Sleep warning callback.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PreSleepCallback(void *arg)
{
#pragma unused( arg )
    OS_TPanic("Could not sleep while DSP is processing.\n");
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_MasterInterrupts

  Description:  DSP interrupt process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_MasterInterrupts(void)
{
    DSPProcessContext  *context = DSPiCurrentComponent;
    if (context)
    {
        // An interrupt cause flag (IF) is only generated to the OS for the instance that either of the DSP interrupt causes first occurs. If there is the possibility that new DSP interrupt causes can occur while multiple DSP interrupt causes are being sequentially processed, it is necessary to repeat at that time without relying on the interrupt handler.
        // 
        // 
        // 
        // 
        for (;;)
        {
            // Batch determine messages that can be causes of DSP interrupts
            int     ready = (reg_DSP_SEM | (((reg_DSP_PSTS >> REG_DSP_PSTS_RRI0_SHIFT) & 7) << 16));
            int     factors = (ready & context->hookFactors);
            if (factors == 0)
            {
                break;
            }
            else
            {
                // Process corresponding hook process in order
                // If group-registered, notify together one time only
                while (factors != 0)
                {
                    int     index = (int)MATH_CTZ((u32)factors);
                    factors &= ~context->hookGroup[index];
                    (*context->hookFunction[index])(context->hookUserdata[index]);
                }
            }
        }
    }
    OS_SetIrqCheckFlag(OS_IE_DSP);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ReadProcessImage

  Description:  Read data from the process image.

  Arguments:    context: DSPProcessContext structure
                offset: Offset in image
                buffer: The buffer to transfer to
                length: Transfer size

  Returns:      TRUE, if specified size is correctly read
 *---------------------------------------------------------------------------*/
static BOOL DSPi_ReadProcessImage(DSPProcessContext *context, int offset, void *buffer, int length)
{
    return FS_SeekFile(context->image, offset, FS_SEEK_SET) &&
           (FS_ReadFile(context->image, buffer, length) == length);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_CommitWram

  Description:  Assign specified segment to specific processor.

  Arguments:    context: DSPProcessContext structure
                wram: Code/Data
                segment: Segment number to switch
                to: Processor to switch

  Returns:      TRUE, if all slots are correctly switched
 *---------------------------------------------------------------------------*/
static BOOL DSPi_CommitWram(DSPProcessContext *context, MIWramPos wram, int segment, MIWramProc to)
{
    BOOL    retval = FALSE;
    int     slot = DSP_GetProcessSlotFromSegment(context, wram, segment);
    // Disconnect from currently assigned processor
    if (!MI_IsWramSlotUsed(wram, slot) ||
        MI_FreeWramSlot(wram, slot, MI_WRAM_SIZE_32KB, MI_GetWramBankMaster(wram, slot)) > 0)
    {
        void   *physicalAddr;
            // Move to predetermined offset for each processor
        vu8 *bank = &((vu8*)((wram == MI_WRAM_B) ? REG_MBK_B0_ADDR  : REG_MBK_C0_ADDR))[slot];
        if (to == MI_WRAM_ARM9)
        {
            *bank = (u8)((*bank & ~MI_WRAM_OFFSET_MASK_B) | (slot << MI_WRAM_OFFSET_SHIFT_B));
        }
        else if (to == MI_WRAM_DSP)
        {
            *bank = (u8)((*bank & ~MI_WRAM_OFFSET_MASK_B) | (segment << MI_WRAM_OFFSET_SHIFT_B));
        }
        // Assign to specified processor
        physicalAddr = (void *)MI_AllocWramSlot(wram, slot, MI_WRAM_SIZE_32KB, to);
        if (physicalAddr != 0)
        {
            retval = TRUE;
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         DSPi_MapAndLoadProcessImageCallback

  Description:  Callback to map and load with 1 pass of the process image.

  Arguments:    context: DSPProcessContext structure
                header: COFF file header information
                section: Enumerated section

  Returns:      TRUE, if enumeration is continued; FALSE, if acceptable to end
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapAndLoadProcessImageCallback(DSPProcessContext *context,
                                                const COFFFileHeader *header,
                                                const COFFSectionTable *section)
{
    BOOL        retval = TRUE;

    // Because there is possibility that single sections can be located at a maximum of four locations for code and data.
    // 
    enum
    {
        placement_kind_max = 2, // { CODE, DATA }
        placement_code_page_max = 2,
        placement_data_page_max = 2,
        placement_max = placement_code_page_max + placement_data_page_max
    };
    MIWramPos   wrams[placement_max];
    int         addrs[placement_max];
    BOOL        nolds[placement_max];
    int         placement = 0;

    // Determine the page number to locate using the suffix of the section name
    // If the name field directly represents the string, it is located in only one position, but if it is a long name that must be referenced from the string table , it is possible to be located in multiple places.
    // 
    const char *name = (const char *)section->Name;
    char        longname[128];
    if (*(u32*)name == 0)
    {
        u32     stringtable = header->PointerToSymbolTable + 0x12 * header->NumberOfSymbols;
        if(!DSPi_ReadProcessImage(context,
                              (int)(stringtable + *(u32*)&section->Name[4]),
                              longname, sizeof(longname)))
        {
            retval = FALSE;
            return retval;
        }
        name = longname;
    }

    // Determine existence of section here that is a special landmark
    if (STD_CompareString(name, "SDK_USING_OS@d0") == 0)
    {
        context->flags |= DSP_PROCESS_FLAG_USING_OS;
    }

    // CODE section is WRAM-B
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        int         baseaddr = (int)(section->s_paddr * 2);
        BOOL        noload = ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_CODE_MEMORY) != 0);
        if (STD_StrStr(name, "@c0") != NULL)
        {
            wrams[placement] = MI_WRAM_B;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 0;
            nolds[placement] = noload;
            ++placement;
        }
        if (STD_StrStr(name, "@c1") != NULL)
        {
            wrams[placement] = MI_WRAM_B;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 1;
            nolds[placement] = noload;
            ++placement;
        }
    }

    // DATA section is WRAM-C
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        int         baseaddr = (int)(section->s_vaddr * 2);
        BOOL        noload = ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_DATA_MEMORY) != 0);
        if (STD_StrStr(name, "@d0") != NULL)
        {
            wrams[placement] = MI_WRAM_C;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 0;
            nolds[placement] = noload;
            ++placement;
        }
        if (STD_StrStr(name, "@d1") != NULL)
        {
            wrams[placement] = MI_WRAM_C;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 1;
            nolds[placement] = noload;
            ++placement;
        }
    }

    // Location destination information was listed up, so locate in each address
    {
        int     i;
        for (i = 0; i < placement; ++i)
        {
            MIWramPos   wram = wrams[i];
            int     dstofs = addrs[i];
            int     length = (int)section->s_size;
            int     srcofs = (int)section->s_scnptr;
            // Split load so as not to overlap the slot boundary
            while (length > 0)
            {
                // Clip the size at the slot boundary
                int     ceil = MATH_ROUNDUP(dstofs + 1, DSP_WRAM_SLOT_SIZE);
                int     curlen = MATH_MIN(length, ceil - dstofs);
                BOOL    newmapped = FALSE;
                // If the corresponding segment is not yet mapped, map and initialize here
                if (DSP_GetProcessSlotFromSegment(context, wram, dstofs / DSP_WRAM_SLOT_SIZE) == -1)
                {
                    int     segment = (dstofs / DSP_WRAM_SLOT_SIZE);
                    u16    *slots = (wram == MI_WRAM_B) ? &context->slotB : &context->slotC;
                    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
                    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
                    int     slot = (int)MATH_CountTrailingZeros((u32)*slots);
                    if (slot >= MI_WRAM_B_MAX_NUM)
                    {
                        retval = FALSE;
                        break;
                    }
                    else
                    {
                        newmapped = TRUE;
                        map[segment] = slot;
                        *slots &= ~(1 << slot);
                        *segbits |= (1 << segment);
                        if (!DSPi_CommitWram(context, wram, segment, MI_WRAM_ARM9))
                        {
                            retval = FALSE;
                            break;
                        }
                    }
                    MI_CpuFillFast(DSP_ConvertProcessAddressFromDSP(context, wram,
                                                                    segment * (DSP_WRAM_SLOT_SIZE / 2)),
                                   0, DSP_WRAM_SLOT_SIZE);
                }
                // If noload is specified, ignore
                if (nolds[i])
                {
                    DSP_DPRINTF("$%04X (noload)\n", dstofs);
                }
                else
                {
                    // Calculate the corresponding offset in this time's transfer range
                    u8     *dstbuf = (u8*)DSP_ConvertProcessAddressFromDSP(context, wram, dstofs / 2);
                    if (!DSPi_ReadProcessImage(context, srcofs, dstbuf, curlen))
                    {
                        retval = FALSE;
                        break;
                    }
                    DSP_DPRINTF("$%04X -> mem:%08X\n", dstofs, dstbuf);
                }
                srcofs += curlen;
                dstofs += curlen;
                length -= curlen;
            }
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_InitProcessContext

  Description:  Initializes the process information structure.

  Arguments:    context: DSPProcessContext structure
                name: Process name or NULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitProcessContext(DSPProcessContext *context, const char *name)
{
    int     i;
    int     segment;
    MI_CpuFill8(context, 0, sizeof(*context));
    context->next = NULL;
    context->flags = 0;
    (void)STD_CopyString(context->name, name ? name : "");
    context->image = NULL;
    // Make the segment map empty
    context->segmentCode = 0;
    context->segmentData = 0;
    // Make the slot map empty
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        context->slotOfSegmentCode[segment] = -1;
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        context->slotOfSegmentData[segment] = -1;
    }
    // Make the hook registration state empty
    context->hookFactors = 0;
    for (i = 0; i < DSP_HOOK_MAX; ++i)
    {
        context->hookFunction[i] = NULL;
        context->hookUserdata[i] = NULL;
        context->hookGroup[i] = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnumSections

  Description:  Runs the predetermined processes by enumerating sections in the process image.

  Arguments:    context: DSPProcessContext structure passed to the callback function
                callback: Callback called for each section

  Returns:      TRUE if all sections are enumerated; FALSE if ended prematurely.
 *---------------------------------------------------------------------------*/
BOOL DSP_EnumSections(DSPProcessContext *context, DSPSectionEnumCallback callback)
{
    BOOL            retval = FALSE;
    // Copy once to temporary, considering the memory alignment of the image buffer
    COFFFileHeader  header[1];
    if (DSPi_ReadProcessImage(context, 0, header, sizeof(header)))
    {
        int     base = (int)(sizeof(header) + header->SizeOfOptionalHeader);
        int     index;
        for (index = 0; index < header->NumberOfSections; ++index)
        {
            COFFSectionTable    section[1];
            if (!DSPi_ReadProcessImage(context, (int)(base + index * sizeof(section)), section, (int)sizeof(section)))
            {
                break;
            }
            // Sort only sections with a size over 1 and no BLOCK-HEADER attribute
            if (((section->s_flags & COFF_SECTION_ATTR_BLOCK_HEADER) == 0) && (section->s_size != 0))
            {
                if (callback && !(*callback)(context, header, section))
                {
                    break;
                }
            }
        }
        retval = (index >= header->NumberOfSections);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_AttachProcessMemory

  Description:  Assigns memory space continuously to unused regions of the process.

  Arguments:    context: DSPProcessContext structure
                wram: Memory space to assign (MI_WRAM_B/MI_WRAM_C)
                slots: WRAM slot to assign

  Returns:      Header address of the assigned DSP memory space, or 0
 *---------------------------------------------------------------------------*/
u32 DSP_AttachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots)
{
    u32     retval = 0;
    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
    // Search for continuous unusued regions only for the necessary amount
    int     need = (int)MATH_CountPopulation((u32)slots);
    u32     region = (u32)((1 << need) - 1);
    u32     space = (u32)(~*segbits & 0xFF);
    int     segment = 0;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        // There are various restrictions in the DSP page boundary (4 segments) so be careful to allocate without overlap
        // 
        if ((((segment ^ (segment + need - 1)) & 4) == 0) &&
            (((space >> segment) & region) == region))
        {
            // If there is an adequate unused region existing, assign slots from a lower order
            retval = (u32)(DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * segment);
            while (slots)
            {
                int     slot = (int)MATH_CountTrailingZeros((u32)slots);
                map[segment] = slot;
                slots &= ~(1 << slot);
                *segbits |= (1 << segment);
                segment += 1;
            }
            break;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DetachProcessMemory

  Description:  Deallocates the WRAM slots assigned to the regions used by the process.

  Arguments:    context: DSPProcessContext structure
                slots: Assigned WRAM slot

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DetachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots)
{
    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((((1 << segment) & *segbits) != 0) && (((1 << map[segment]) & slots) != 0)) 
        {
            *segbits &= ~(1 << segment);
            map[segment] = -1;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SwitchProcessMemory

  Description:  Switches DSP address management being used by a specified process.

  Arguments:    context: DSPProcessContext structure
                wram: Memory space to switch (MI_WRAM_B/MI_WRAM_C)
                address: Header address to switch (Word units of the DSP memory space)
                length: Memory size to switch (Word units of the DSP memory space)
                to: New master processor

  Returns:      TRUE, if all slots are correctly switched
 *---------------------------------------------------------------------------*/
BOOL DSP_SwitchProcessMemory(DSPProcessContext *context, MIWramPos wram, u32 address, u32 length, MIWramProc to)
{
    address = DSP_ADDR_TO_ARM(address);
    length = DSP_ADDR_TO_ARM(MATH_MAX(length, 1));
    {
        int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
        int     lower = (int)(address / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((address + length - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for (segment = lower; segment <= upper; ++segment)
        {
            if ((*segbits & (1 << segment)) != 0)
            {
                if (!DSPi_CommitWram(context, wram, segment, to))
                {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MapAndLoadProcessImage

  Description:  Maps and loads with one pass of the specified process image.
                Segment map does not need to be calculated

  Arguments:    context: DSPProcessContext structure
                image: File handle that specifies the process image
                          Referenced only in this function
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory

  Returns:      TRUE if all images are correctly loaded
 *---------------------------------------------------------------------------*/
static BOOL DSP_MapAndLoadProcessImage(DSPProcessContext *context, FSFile *image, int slotB, int slotC)
{
    BOOL    retval = FALSE;
    const u32   dspMemSize = DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * MI_WRAM_B_MAX_NUM;
    // Map WRAM while actually loading the process image
    context->image = image;
    context->slotB = (u16)slotB;
    context->slotC = (u16)slotC;

    if (DSP_EnumSections(context, DSPi_MapAndLoadProcessImageCallback))
    {
        DC_FlushRange((const void*)MI_GetWramMapStart_B(), MI_WRAM_B_SIZE);
        DC_FlushRange((const void*)MI_GetWramMapStart_C(), MI_WRAM_C_SIZE);
        // Switch all assigned WRAM slots to DSP
        if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_DSP) &&
            DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_DSP))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetProcessHook

  Description:  Sets the hook for DSP interrupt cause to the process.

  Arguments:    context: DSPProcessContext structure
                factors: Bit set of the specified interrupt cause
                           Bits 0 - 15 are semaphore; bits 16 - 18 are reply
                function: Hook function to call
                userdata: Arbitrary user-defined argument given to the hook function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetProcessHook(DSPProcessContext *context, int factors, DSPHookFunction function, void *userdata)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    int     i;
    for (i = 0; i < DSP_HOOK_MAX; ++i)
    {
        int     bit = (1 << i);
        if ((bit & factors) != 0)
        {
            context->hookFunction[i] = function;
            context->hookUserdata[i] = userdata;
            context->hookGroup[i] = factors;
        }
    }
    {
        // Change the interrupt cause
        int         modrep = (((factors >> 16) & 0x7) << REG_DSP_PCFG_PRIE0_SHIFT);
        int         modsem = ((factors >> 0) & 0xFFFF);
        int         currep = reg_DSP_PCFG;
        int         cursem = reg_DSP_PMASK;
        if (function != NULL)
        {
            reg_DSP_PCFG = (u16)(currep | modrep);
            reg_DSP_PMASK = (u16)(cursem & ~modsem);
            context->hookFactors |= factors;
        }
        else
        {
            reg_DSP_PCFG = (u16)(currep & ~modrep);
            reg_DSP_PMASK = (u16)(cursem | modsem);
            context->hookFactors &= ~factors;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPostStartProcess

  Description:  Hooks immediately after loading the DSP process image.
                It is necessary for the DSP component developer to start the debugger

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void DSP_HookPostStartProcess(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ExecuteProcess

  Description:  Loads the DSP process image and starts.

  Arguments:    context: DSPProcessContext structure used in status management
                          Referenced using the system until the process is destroyed
                image: File handle that specifies the process image
                          Referenced only in this function
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_ExecuteProcess(DSPProcessContext *context, FSFile *image, int slotB, int slotC)
{
    BOOL    retval = FALSE;
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    // Initialize DSP systems common to all components
    DSP_InitPipe();
    OS_SetIrqFunction(OS_IE_DSP, DSPi_MasterInterrupts);
    DSP_SetProcessHook(context,
                       DSP_HOOK_SEMAPHORE_(15) | DSP_HOOK_REPLY_(2),
                       (DSPHookFunction)DSP_HookPipeNotification, NULL);
    (void)OS_EnableIrqMask(OS_IE_DSP);
    // Map the process image to memory and load
    if (!DSP_MapAndLoadProcessImage(context, image, slotB, slotC))
    {
        OS_TWarning("you should check wram\n");
    }
    else
    {
        OSIntrMode  bak_cpsr = OS_DisableInterrupts();
        // Register the context to the process list
        DSPProcessContext  **pp = &DSPiCurrentComponent;
        for (pp = &DSPiCurrentComponent; *pp && (*pp != context); pp = &(*pp)->next)
        {
        }
        *pp = context;
        context->image = NULL;
        // Set the shutdown forced termination callback
        if (!DSPiShutdownCallbackIsRegistered)
        {
            PM_SetExitCallbackInfo(DSPiShutdownCallbackInfo, DSPi_ShutdownCallback, NULL);
            PMi_InsertPostExitCallbackEx(DSPiShutdownCallbackInfo, PM_CALLBACK_PRIORITY_DSP);
            DSPiShutdownCallbackIsRegistered = TRUE;
        }
        // Set the sleep warning callback
        if (!DSPiPreSleepCallbackIsRegistered)
        {
            PM_SetSleepCallbackInfo(DSPiPreSleepCallbackInfo, DSPi_PreSleepCallback, NULL);
            PMi_InsertPreSleepCallbackEx(DSPiPreSleepCallbackInfo, PM_CALLBACK_PRIORITY_DSP);
            DSPiPreSleepCallbackIsRegistered = TRUE;
        }
        // Set only the necessary interrupt cause as early as possible after starting up
        DSP_PowerOn();
        DSP_ResetOffEx((u16)(context->hookFactors >> 16));
        DSP_MaskSemaphore((u16)~(context->hookFactors >> 0));
        // If the ARM side is paused immediately after startup, it is possible to reload and trace the same component
        // 
        DSP_HookPostStartProcess();
        // For components that spontaneously send commands from the DSP side without waiting for command from the ARM9 processor, if reloaded by the DSP debugger, the command value will be initialized to 0 and unread bits will remain on the ARM9 processor.
        // 
        // 
        // 
        // This 0 is read and discarded in the library
        if ((context->flags & DSP_PROCESS_FLAG_USING_OS) != 0)
        {
            u16     id;
            for (id = 0; id < 3; ++id)
            {
                vu16    dummy;
                while (dummy = DSP_RecvDataCore(id), dummy != 1)
                {
                }
            }
        }
        // Just in case, check whether there is an interrupt that was not taken in the time from startup until setting
        DSPi_MasterInterrupts();
        retval = TRUE;
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_QuitProcess

  Description:  Deallocates memory by ending the DSP process.

  Arguments:    context: DSPProcessContext structure used in status management

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_QuitProcess(DSPProcessContext *context)
{
    OSIntrMode  bak_cpsr;
    
    // First, stop the TEAK DMA
    DSP_StopDSPComponent();
    
    bak_cpsr = OS_DisableInterrupts();
    // Stop the DSP
    DSP_ResetOn();
    DSP_PowerOff();
    // Invalidate interrupts
    (void)OS_DisableIrqMask(OS_IE_DSP);
    OS_SetIrqFunction(OS_IE_DSP, NULL);
    // Return all WRAM that were being used
    (void)MI_FreeWram(MI_WRAM_B, MI_WRAM_DSP);
    (void)MI_FreeWram(MI_WRAM_C, MI_WRAM_DSP);
    {
        // Release context from the processor list
        DSPProcessContext  **pp = &DSPiCurrentComponent;
        for (pp = &DSPiCurrentComponent; *pp; pp = &(*pp)->next)
        {
            if (*pp == context)
            {
                *pp = (*pp)->next;
                break;
            }
        }
        context->next = NULL;
    }
    (void)context;
    (void)OS_RestoreInterrupts(bak_cpsr);

    // Delete the warning callback when sleeping 
    PM_DeletePreSleepCallback(DSPiPreSleepCallbackInfo);
    DSPiPreSleepCallbackIsRegistered = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_FindProcess

  Description:  Search for the process being run.

  Arguments:    name: Process name
                       Search for processes registered last if NULL is specified

  Returns:      DSPProcessContext structure that matches the specified name
 *---------------------------------------------------------------------------*/
DSPProcessContext* DSP_FindProcess(const char *name)
{
    DSPProcessContext  *ptr = NULL;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    ptr = DSPiCurrentComponent;
    if (name)
    {
        for (; ptr && (STD_CompareString(ptr->name, name) != 0); ptr = ptr->next)
        {
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadProcessPipe

  Description:  Receives from pipe associated with predetermined port of the process.

  Arguments:    context: DSPProcessContext structure
                port: Reception source port
                buffer: Received data
                length: Received size (Byte Units)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadProcessPipe(DSPProcessContext *context, int port, void *buffer, u32 length)
{
    DSPPipe input[1];
    (void)DSP_LoadPipe(input, port, DSP_PIPE_INPUT);
    DSP_ReadPipe(input, buffer, (DSPByte)length);
    (void)context;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_WriteProcessPipe

  Description:  Sends to the pipe associated with predetermined port of the process.

  Arguments:    context: DSPProcessContext structure
                port: Reception destination port
                buffer: Send data
                length: Send size (Byte Units)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WriteProcessPipe(DSPProcessContext *context, int port, const void *buffer, u32 length)
{
    DSPPipe output[1];
    (void)DSP_LoadPipe(output, port, DSP_PIPE_OUTPUT);
    DSP_WritePipe(output, buffer, (DSPByte)length);
    (void)context;
}


#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * The following shows candidate interfaces for termination because they are considered not to be currently in use
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSegmentCallback

  Description:  Callback to calculate segment maps occupied by the process.

  Arguments:    context: DSPProcessContext structure
                header: COFF file header information
                section: Enumerated section

  Returns:      TRUE, if enumeration is continued; FALSE, if acceptable to end
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSegmentCallback(DSPProcessContext *context,
                                           const COFFFileHeader *header,
                                           const COFFSectionTable *section)
{
    (void)header;
    // TODO: You might achieve higher speeds by avoiding division
    if((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        u32     addr = DSP_ADDR_TO_ARM(section->s_paddr);
        int     lower = (int)(addr / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((addr + section->s_size - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for(segment = lower; segment <= upper; ++segment)
        {
            context->segmentCode |= (1 << segment);
        }
    }
    else if((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        u32     addr = DSP_ADDR_TO_ARM(section->s_vaddr);
        int     lower = (int)(addr / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((addr + section->s_size - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for(segment = lower; segment <= upper; ++segment)
        {
            context->segmentData |= (1 << segment);
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSlotDefault

  Description:  Initializes the slot map using empty numbers from the lower order.

  Arguments:    context: DSPProcessContext structure
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory

  Returns:      TRUE if the slot map that satisfies the conditions can be allocated.
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSlotDefault(DSPProcessContext *context, int slotB, int slotC)
{
    BOOL    retval = TRUE;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((context->segmentCode & (1 << segment)) != 0)
        {
            int     slot = (int)MATH_CountTrailingZeros((u32)slotB);
            if (slot >= MI_WRAM_B_MAX_NUM)
            {
                retval = FALSE;
                break;
            }
            context->slotOfSegmentCode[segment] = slot;
            slotB &= ~(1 << slot);
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if ((context->segmentData & (1 << segment)) != 0)
        {
            int     slot = (int)MATH_CountTrailingZeros((u32)slotC);
            if (slot >= MI_WRAM_C_MAX_NUM)
            {
                retval = FALSE;
                break;
            }
            context->slotOfSegmentData[segment] = slot;
            slotC &= ~(1 << slot);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsProcessMemoryReady

  Description:  Checks whether the WRAM slot that should be assigned to the process is being used.

  Arguments:    context: DSPProcessContext structure

  Returns:      TRUE, if all WRAM slots are not being used but are in a prepared state
 *---------------------------------------------------------------------------*/
static BOOL DSP_IsProcessMemoryReady(DSPProcessContext *context)
{
    BOOL    retval = TRUE;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((context->segmentCode & (1 << segment)) != 0)
        {
            int     slot = context->slotOfSegmentCode[segment];
            if (MI_IsWramSlotUsed(MI_WRAM_B, slot))
            {
                OS_TWarning("slot:%d for DSP:%05X is now used by someone.\n", slot, segment * DSP_WRAM_SLOT_SIZE);
                retval = FALSE;
                break;
            }
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if ((context->segmentData & (1 << segment)) != 0)
        {
            int     slot = context->slotOfSegmentData[segment];
            if (MI_IsWramSlotUsed(MI_WRAM_C, slot))
            {
                OS_TWarning("slot:%d for DSP:%05X is now used by someone.\n", slot, segment * DSP_WRAM_SLOT_SIZE);
                retval = FALSE;
                break;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_LoadProcessImageCallback

  Description:  Loads the process image using the group method.

  Arguments:    context: DSPProcessContext structure
                header: COFF file header information
                section: Enumerated section

  Returns:      TRUE, if enumeration is continued; FALSE, if acceptable to end
 *---------------------------------------------------------------------------*/
static BOOL DSPi_LoadProcessImageCallback(DSPProcessContext *context,
                                          const COFFFileHeader *header,
                                          const COFFSectionTable *section)
{
    BOOL        retval = TRUE;
    MIWramPos   wram = MI_WRAM_A;
    int         dstofs = 0;
    BOOL        noload = FALSE;
    (void)header;
    // CODE section is WRAM-B
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        wram = MI_WRAM_B;
        dstofs = (int)(section->s_paddr * 2);
        if ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_CODE_MEMORY) != 0)
        {
            noload = TRUE;
        }
    }
    // DATA section is WRAM-C
    else if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        wram = MI_WRAM_C;
        dstofs = (int)(section->s_vaddr * 2);
        if ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_DATA_MEMORY) != 0)
        {
            noload = TRUE;
        }
    }
    // Select the appropriate WRAM slot for each segment
    // (The constant "no WRAM" does not exist, so substitute WRAM-A)
    if (wram != MI_WRAM_A)
    {
        // If noload is specified, ignore
        if (noload)
        {
            DSP_DPRINTF("$%04X (noload)\n", dstofs);
        }
        else
        {
            // Split load so as not to overlap the slot boundary
            int     length = (int)section->s_size;
            int     srcofs = (int)section->s_scnptr;
            while (length > 0)
            {
                // Clip the size at the slot boundary
                int     ceil = MATH_ROUNDUP(dstofs + 1, DSP_WRAM_SLOT_SIZE);
                int     curlen = MATH_MIN(length, ceil - dstofs);
                // Calculate the corresponding offset in this time's transfer range
                u8     *dstbuf = (u8*)DSP_ConvertProcessAddressFromDSP(context, wram, dstofs/2);
                if (!DSPi_ReadProcessImage(context, srcofs, dstbuf, length))
                {
                    retval = FALSE;
                    break;
                }
                DSP_DPRINTF("$%04X -> mem:%08X\n", dstofs, dstbuf);
                srcofs += curlen;
                dstofs += curlen;
                length -= curlen;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MapProcessSegment

  Description:  Calculates segment maps occupied by the process.

  Arguments:    context: DSPProcessContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MapProcessSegment(DSPProcessContext *context)
{
    (void)DSP_EnumSections(context, DSPi_MapProcessSegmentCallback);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadProcessImage

  Description:  Loads the specified process image.
                The segment map must already be calculated.

  Arguments:    context: DSPProcessContext structure

  Returns:      TRUE if all images are correctly loaded
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadProcessImage(DSPProcessContext *context)
{
    BOOL    retval = FALSE;
    // Check whether it is really possible to use the WRAM slot that should be assigned to DSP
    if (DSP_IsProcessMemoryReady(context))
    {
        const u32   dspMemSize = DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * MI_WRAM_B_MAX_NUM;
        // Switch all assigned WRAM slots to ARM9
        if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_ARM9) &&
            DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_ARM9))
        {
            // Actually load the process image and flush to WRAM
            if (DSP_EnumSections(context, DSPi_LoadProcessImageCallback))
            {
                DC_FlushRange((const void*)MI_GetWramMapStart_B(), MI_WRAM_B_SIZE);
                DC_FlushRange((const void*)MI_GetWramMapStart_C(), MI_WRAM_C_SIZE);
                // Switch all assigned WRAM slots to DSP
                if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_DSP) &&
                    DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_DSP))
                {
                    retval = TRUE;
                }
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartupProcess

  Description:  Calculates the process image segment map and loads to WRAM.
                Version combining the DSP_MapProcessSegment and the DSP_LoadProcessImage functions.

  Arguments:    context: DSPProcessContext structure
                image: File handle that specifies the process image
                             Referenced only in this function
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory
                slotMapper: Algorithm that assigns WRAM slots to segments
                             If NULL is specified, the optimum default process is selected

  Returns:      TRUE if all images are correctly loaded
 *---------------------------------------------------------------------------*/
BOOL DSP_StartupProcess(DSPProcessContext *context, FSFile *image,
                        int slotB, int slotC, BOOL (*slotMapper)(DSPProcessContext *, int, int))
{
    BOOL    retval = FALSE;
    if (!slotMapper)
    {
        slotMapper = DSPi_MapProcessSlotDefault;
    }
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    context->image = image;
    DSP_MapProcessSegment(context);
    if (!(*slotMapper)(context, slotB, slotC) ||
        !DSP_LoadProcessImage(context))
    {
        OS_TWarning("you should check wram\n");
    }
    else
    {
        retval = TRUE;
    }
    context->image = NULL;
    return retval;
}


#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
