/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     sharedWram.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 9345 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MI_SHAREDWRAM_H_
#define TWL_MI_SHAREDWRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

//================================================================
// defines about register SCFG_MBKs
//================================================================
typedef enum
{
	MI_WRAM_A = 0,
	MI_WRAM_B = 1,
	MI_WRAM_C = 2
} MIWramPos;


#define MI_WRAM_A_MAX_NUM			4
#define MI_WRAM_B_MAX_NUM           8
#define MI_WRAM_C_MAX_NUM           8

#define MI_WRAM_A_SIZE              HW_WRAM_A_SIZE
#define MI_WRAM_B_SIZE              HW_WRAM_B_SIZE
#define MI_WRAM_C_SIZE              HW_WRAM_C_SIZE

#ifdef SDK_TWLHYB
#define MI_WRAM_ADDRESS_A0          0x03000000
#else
#define MI_WRAM_ADDRESS_A0          0x037c0000
#endif
#define MI_WRAM_ADDRESS_B0          HW_WRAM_B
#define MI_WRAM_ADDRESS_C0          HW_WRAM_C

#define MI_WRAM_A_SLOT_SIZE         0x10000
#define MI_WRAM_B_SLOT_SIZE         0x8000
#define MI_WRAM_C_SLOT_SIZE         0x8000

//---- master proc
typedef enum
{
	MI_WRAM_ARM9 = 0,
	MI_WRAM_ARM7 = 1,
	MI_WRAM_DSP  = 2, // only for WRAM_B and WRAM_C

	MI_WRAM_FREE  = 3
} MIWramProc;


#define MI_WRAM_MASTER_MASK_A		REG_MI_MBK_A0_M_MASK
#define MI_WRAM_MASTER_MASK_B       REG_MI_MBK_B0_M_MASK
#define MI_WRAM_MASTER_MASK_C       REG_MI_MBK_C0_M_MASK

//---- start offset
typedef enum
{
	MI_WRAM_OFFSET_0KB   = 0,
	MI_WRAM_OFFSET_32KB  = 1,
	MI_WRAM_OFFSET_64KB  = 2,
	MI_WRAM_OFFSET_96KB  = 3,
	MI_WRAM_OFFSET_128KB = 4,
	MI_WRAM_OFFSET_160KB = 5,
	MI_WRAM_OFFSET_192KB = 6,
	MI_WRAM_OFFSET_224KB = 7
} MIWramOffset;

#define MI_WRAM_OFFSET_MASK_A   	REG_MI_MBK_A0_OF_MASK
#define MI_WRAM_OFFSET_MASK_B   	REG_MI_MBK_B0_OF_MASK
#define MI_WRAM_OFFSET_MASK_C   	REG_MI_MBK_C0_OF_MASK
#define MI_WRAM_OFFSET_SHIFT_A   	REG_MI_MBK_A0_OF_SHIFT
#define MI_WRAM_OFFSET_SHIFT_B   	REG_MI_MBK_B0_OF_SHIFT
#define MI_WRAM_OFFSET_SHIFT_C   	REG_MI_MBK_C0_OF_SHIFT

//---- enable/disable
typedef enum
{
	MI_WRAM_ENABLE   = TRUE,
	MI_WRAM_DISABLE  = FALSE
} MIWramEnable;

#define MI_WRAM_ENABLE_MASK_A   	REG_MI_MBK_A0_E_MASK
#define MI_WRAM_ENABLE_MASK_B   	REG_MI_MBK_B0_E_MASK
#define MI_WRAM_ENABLE_MASK_C   	REG_MI_MBK_C0_E_MASK

//---- image type
typedef enum
{
	MI_WRAM_IMAGE_32KB  = 0, // only for WRAM_B and WRAM_C
	MI_WRAM_IMAGE_64KB  = 1,
	MI_WRAM_IMAGE_128KB = 2,
	MI_WRAM_IMAGE_256KB = 3
} MIWramImage;


//---- size
typedef enum
{
	MI_WRAM_SIZE_0KB   = 0,
	MI_WRAM_SIZE_32KB  = 1,
	MI_WRAM_SIZE_64KB  = 2,
	MI_WRAM_SIZE_96KB  = 3,
	MI_WRAM_SIZE_128KB = 4,
	MI_WRAM_SIZE_160KB = 5,
	MI_WRAM_SIZE_192KB = 6,
	MI_WRAM_SIZE_224KB = 7,
	MI_WRAM_SIZE_256KB = 8
} MIWramSize;

// Convert between size and enum number.
// Notice: Size does not contain KB.
//         i.e. To get enum of 64KB, do MI_WRAM_SIZE_TO_ENUM(64), not (64KB) or (65536).
//         i.e. The value of MI_WRAM_ENUM_TO_SIZE(MI_WRAM_SIZE_64KB) is 64, not 65536(64KB).
#define MI_WRAM_SIZE_TO_ENUM(size)   ((MIWramSize)((int)(size)>>5))
#define MI_WRAM_ENUM_TO_SIZE(n)      ((int)((n)<<5))

#define MI_WRAM_ERROR_LOCKED    -1

//================================================================================
//
//================================================================================
#ifdef SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         MIi_SetWramBank_A / MIi_SetWramBank_B / MIi_SetWramBank_C

  Description:  set up shared WRAM bank

  Arguments:    num    : slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7
                proc   : master select
                             MI_WRAM_ARM9 : ARM9
                             MI_WRAM_ARM7 : ARM7
                             MI_WRAM_DSP  : DSP   (only for WRAM_B and WRAM_C)
                offset : offset of WRAM allocation
                             (for WRAM_A)
                             MI_WRAM_OFFSET_0KB    :   0KB from top
                             MI_WRAM_OFFSET_64KB   :  64KB from top
                             MI_WRAM_OFFSET_128KB  : 128KB from top
                             MI_WRAM_OFFSET_192KB  : 192KB from top

                             (for WRAM_B and WRAM_C)
                             MI_WRAM_OFFSET_0KB    :   0KB from top
                             MI_WRAM_OFFSET_32KB   :  32KB from top
                             MI_WRAM_OFFSET_64KB   :  64KB from top
                             MI_WRAM_OFFSET_96KB   :  96KB from top
                             MI_WRAM_OFFSET_128KB  : 128KB from top
                             MI_WRAM_OFFSET_160KB  : 160KB from top
                             MI_WRAM_OFFSET_192KB  : 192KB from top
                             MI_WRAM_OFFSET_224KB  : 224KB from top

                enable : enable flag
                             MI_WRAM_ENABLE  : enable
                             MI_WRAM_DISABLE : disable

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetWramBank_A( int num, MIWramProc proc, MIWramOffset offset, MIWramEnable enable );
void MIi_SetWramBank_B( int num, MIWramProc proc, MIWramOffset offset, MIWramEnable enable );
void MIi_SetWramBank_C( int num, MIWramProc proc, MIWramOffset offset, MIWramEnable enable );
void MIi_SetWramBank( MIWramPos wram, int num, MIWramProc proc, MIWramOffset offset, MIWramEnable enable );
//---- for convenience
void MIi_SetWramBankMaster_A( int num, MIWramProc proc );
void MIi_SetWramBankMaster_B( int num, MIWramProc proc );
void MIi_SetWramBankMaster_C( int num, MIWramProc proc );
void MIi_SetWramBankMaster( MIWramPos wram, int num, MIWramProc proc );
//- - - - - - - - - 
void MIi_SetWramBankEnable_A( int num, MIWramEnable enable );
void MIi_SetWramBankEnable_B( int num, MIWramEnable enable );
void MIi_SetWramBankEnable_C( int num, MIWramEnable enable );
void MIi_SetWramBankEnable( MIWramPos wram, int num, MIWramEnable enable );
#endif

/*---------------------------------------------------------------------------*
  Name:         MIi_GetWramBank_A / MIi_SetWramBank_B / MIi_SetWramBank_C

  Description:  get WRAM bank setting

  Arguments:    num    ; slot num

  Returns:      value of setting
 *---------------------------------------------------------------------------*/
vu8 MIi_GetWramBank_A( int num );
vu8 MIi_GetWramBank_B( int num );
vu8 MIi_GetWramBank_C( int num );
vu8 MIi_GetWramBank( MIWramPos wram, int num );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramBankMaster_A / MI_GetWramBankMaster_B / MI_GetWramBankMaster_C

  Description:  get master which is set to shared WRAM bank

  Arguments:    num    ; slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      master select
                             MI_WRAM_ARM9 : ARM9
                             MI_WRAM_ARM7 : ARM7
                             MI_WRAM_DSP  : DSP   (only for WRAM_B and WRAM_C)
 *---------------------------------------------------------------------------*/
MIWramProc MI_GetWramBankMaster_A( int num );
MIWramProc MI_GetWramBankMaster_B( int num );
MIWramProc MI_GetWramBankMaster_C( int num );
MIWramProc MI_GetWramBankMaster( MIWramPos wram, int num );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramBankOffset_A / MI_GetWramBankOffset_B / MI_GetWramBankOffset_C

  Description:  get offset which is set to shared WRAM bank

  Arguments:    num    ; slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      offset : offset of WRAM allocation
                (for WRAM_A)
                             MI_WRAM_OFFSET_0KB    :   0KB from top
                             MI_WRAM_OFFSET_64KB   :  64KB from top
                             MI_WRAM_OFFSET_128KB  : 128KB from top
                             MI_WRAM_OFFSET_192KB  : 192KB from top

                (for WRAM_B and WRAM_C)
                             MI_WRAM_OFFSET_0KB    :   0KB from top
                             MI_WRAM_OFFSET_32KB   :  32KB from top
                             MI_WRAM_OFFSET_64KB   :  64KB from top
                             MI_WRAM_OFFSET_96KB   :  96KB from top
                             MI_WRAM_OFFSET_128KB  : 128KB from top
                             MI_WRAM_OFFSET_160KB  : 160KB from top
                             MI_WRAM_OFFSET_192KB  : 192KB from top
                             MI_WRAM_OFFSET_224KB  : 224KB from top

 *---------------------------------------------------------------------------*/
MIWramOffset MI_GetWramBankOffset_A( int num );
MIWramOffset MI_GetWramBankOffset_B( int num );
MIWramOffset MI_GetWramBankOffset_C( int num );
MIWramOffset MI_GetWramBankOffset( MIWramPos wram, int num );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramBankEnable_A / MI_GetWramBankEnable_B /MI_GetWramBankEnable_C

  Description:  get enable/disable which is set to shared WRAM bank

  Arguments:    num    ; slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      enable flag
                             MI_WRAM_ENABLE  : enable
                             MI_WRAM_DISABLE : disable

 *---------------------------------------------------------------------------*/
MIWramEnable MI_GetWramBankEnable_A( int num );
MIWramEnable MI_GetWramBankEnable_B( int num );
MIWramEnable MI_GetWramBankEnable_C( int num );
MIWramEnable MI_GetWramBankEnable( MIWramPos wram, int num );

//================================================================================
// low level functions of wram maps
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_SetWramMap_A / MIi_SetWramMap_B / MIi_SetWramMap_C

  Description:  set WRAM map

  Arguments:    start : start address
                        (for WRAM_A)
                        0x03000000 - 0x03FF0000 (64KB step)
                        (for WRAM_B and WRAM_C)
                        0x03000000 - 0x03FF0000 (32KB step)

                end   : end address
                        (for WRAM_A)
                        0x02FFFFFF - 0x03FFFFFF (64KB step)
                        (for WRAM_B and WRAM_C)
                        0x02FFFFFF - 0x03FFFFFF (32KB step)

                image : MI_WRAM_IMAGE_32KB     (only for WRAM_B and WRAM_C)
                        MI_WRAM_IMAGE_64KB
                        MI_WRAM_IMAGE_128KB
                        MI_WRAM_IMAGE_256KB

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetWramMap_A( u32 start, u32 end, MIWramImage image );
void MIi_SetWramMap_B( u32 start, u32 end, MIWramImage image );
void MIi_SetWramMap_C( u32 start, u32 end, MIWramImage image );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramMapStart_A / MI_GetWramMapStart_B / MI_GetWramMapStart_C

  Description:  get start address of shared WRAM

  Arguments:    None

  Return:    	start : start address
                        (for WRAM_A)
                        0x03000000 - 0x03FF0000 (64KB step)
                        (for WRAM_B and WRAM_C)
                        0x03000000 - 0x03FF0000 (32KB step)
 *---------------------------------------------------------------------------*/
u32 MI_GetWramMapStart_A(void);
u32 MI_GetWramMapStart_B(void);
u32 MI_GetWramMapStart_C(void);
u32 MI_GetWramMapStart( MIWramPos wram );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramMapEnd_A / MI_GetWramMapEnd_B / MI_GetWramMapEnd_C

  Description:  get end address of shared WRAM

  Arguments:    None

  Returns:      end   : end address
                        (for WRAM_A)
                        0x02FFFFFF - 0x03FFFFFF (64KB step)
                        (for WRAM_B and WRAM_C)
                        0x02FFFFFF - 0x03FF0000 (32KB step)
 *---------------------------------------------------------------------------*/
u32 MI_GetWramMapEnd_A(void);
u32 MI_GetWramMapEnd_B(void);
u32 MI_GetWramMapEnd_C(void);
u32 MI_GetWramMapEnd( MIWramPos wram );

/*---------------------------------------------------------------------------*
  Name:         MI_GetWramMapImage_A / MI_GetWramMapImage_B / MI_GetWramMapImage_C

  Description:  get image setting of shared WRAM

  Arguments:    None

  Returns:      image : MI_WRAM_IMAGE_32KB     (only for WRAM_B and WRAM_C)
                        MI_WRAM_IMAGE_64KB
                        MI_WRAM_IMAGE_128KB
                        MI_WRAM_IMAGE_256KB
 *---------------------------------------------------------------------------*/
MIWramImage MI_GetWramMapImage_A(void);
MIWramImage MI_GetWramMapImage_B(void);
MIWramImage MI_GetWramMapImage_C(void);
MIWramImage MI_GetWramMapImage( MIWramPos wram );

/*---------------------------------------------------------------------------*
  Name:         MI_IsWramSlotLocked_A / MI_IsWramSlotLocked_B / MI_IsWramSlotLocked_C

  Description:  check if WRAM slot locked 

  Arguments:    num : slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      TRUE  : locked
                FALSE : not locked
 *---------------------------------------------------------------------------*/
BOOL MI_IsWramSlotLocked_A( int num );
BOOL MI_IsWramSlotLocked_B( int num );
BOOL MI_IsWramSlotLocked_C( int num );
BOOL MI_IsWramSlotLocked( MIWramPos wram, int num );

/*---------------------------------------------------------------------------*
  Name:         MIi_AddressToWramSlot

  Description:  convert address to slot number of WRAM-B/C

  Arguments:    address : address of WRAM-B/C
                type    : destination to store WRAM type

  Returns:      slot number of WRAM-B/C if specified address is valid
                -1 means failure
 *---------------------------------------------------------------------------*/
SDK_INLINE int MIi_AddressToWramSlot(const void *address, MIWramPos *type)
{
    int     retval = -1;
    static const u32 wramBCtop = HW_WRAM_C;
    static const u32 wramBCbototm = HW_WRAM_B_END;
    SDK_COMPILER_ASSERT(HW_WRAM_C < HW_WRAM_B);
    SDK_COMPILER_ASSERT(wramBCbototm - wramBCtop == HW_WRAM_B_SIZE + HW_WRAM_C_SIZE);
    u32     slot = (u32)(((u32)address - wramBCtop) / MI_WRAM_B_SLOT_SIZE);
    if (slot < MI_WRAM_C_MAX_NUM)
    {
        if (type)
        {
            *type = MI_WRAM_C;
        }
        retval = (int)slot;
    }
    else if (slot < MI_WRAM_B_MAX_NUM + MI_WRAM_C_MAX_NUM)
    {
        if (type)
        {
            *type = MI_WRAM_B;
        }
        retval = (int)(slot - MI_WRAM_C_MAX_NUM);
    }
    return retval;
}

//================================================================================
// WRAM manager
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_InitWramManager

  Description:  initialize the WRAM manager on ARM9

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_InitWramManager(void);

/*---------------------------------------------------------------------------*
  Name:         MI_AllocWram

  Description:  allocate WRAM slot

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                size   : size to allocate
                             (for WRAM_A)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_64KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_256KB : 256KB

                             (for WRAM_B and WRAM_C)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_32KB  :  16KB
                                MI_WRAM_SIZE_64KB  :  32KB
                                MI_WRAM_SIZE_96KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_160KB : 160KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_224KB : 224KB
                                MI_WRAM_SIZE_256KB : 256KB
                proc   : processor
                              	MI_WRAM_ARM9 : ARM9
                                MI_WRAM_ARM7 : ARM7
                                MI_WRAM_DSP  : DSP

  Returns:      top address of allocated WRAM
                0 means failure
 *---------------------------------------------------------------------------*/
u32 MI_AllocWram( MIWramPos wram, MIWramSize size, MIWramProc proc );
#define MI_AllocWram_A(size, proc)     MI_AllocWram( MI_WRAM_A, size, proc )
#define MI_AllocWram_B(size, proc)     MI_AllocWram( MI_WRAM_B, size, proc )
#define MI_AllocWram_C(size, proc)     MI_AllocWram( MI_WRAM_C, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_AllocWramSlot

  Description:  allocate specified WRAM slot

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                slot   : wram slot
                proc   : processor
                              	MI_WRAM_ARM9 : ARM9
                                MI_WRAM_ARM7 : ARM7
                                MI_WRAM_DSP  : DSP

  Returns:      top address of allocated WRAM
                0 means failure
 *---------------------------------------------------------------------------*/
u32 MI_AllocWramSlot( MIWramPos wram, int slot, MIWramSize size, MIWramProc proc );
#define MI_AllocWramSlot_A(slot, size, proc)     MI_AllocWramSlot( MI_WRAM_A, slot, size, proc )
#define MI_AllocWramSlot_B(slot, size, proc)     MI_AllocWramSlot( MI_WRAM_B, slot, size, proc )
#define MI_AllocWramSlot_C(slot, size, proc)     MI_AllocWramSlot( MI_WRAM_C, slot, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_FreeWram

  Description:  release WRAM

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      number of blocks set to be free.
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_FreeWram( MIWramPos wram, MIWramProc proc );
#define MI_FreeWram_A(proc)     MI_FreeWram( MI_WRAM_A, proc )
#define MI_FreeWram_B(proc)     MI_FreeWram( MI_WRAM_B, proc )
#define MI_FreeWram_C(proc)     MI_FreeWram( MI_WRAM_C, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_FreeWramSlot

  Description:  release WRAM

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                slot   : slot number
                             (for WRAM_A)
                                0-3
                             (for WRAM_B and WRAM_C)
                                0-7
                size   :
                             (for WRAM_A)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_64KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_256KB : 256KB

                             (for WRAM_B and WRAM_C)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_32KB  :  16KB
                                MI_WRAM_SIZE_64KB  :  32KB
                                MI_WRAM_SIZE_96KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_160KB : 160KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_224KB : 224KB
                                MI_WRAM_SIZE_256KB : 256KB

                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      number of blocks set to be free.
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_FreeWramSlot( MIWramPos wram, int slot, MIWramSize size, MIWramProc proc );
#define MI_FreeWramSlot_A(slot, size, proc)     MI_FreeWramSlot( MI_WRAM_A, slot, size, proc )
#define MI_FreeWramSlot_B(slot, size, proc)     MI_FreeWramSlot( MI_WRAM_B, slot, size, proc )
#define MI_FreeWramSlot_C(slot, size, proc)     MI_FreeWramSlot( MI_WRAM_C, slot, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_SwitchWram

  Description:  switch WRAM master

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

                newProc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      number of changed slots
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_SwitchWram( MIWramPos wram, MIWramProc proc, MIWramProc newProc );
#define MI_SwitchWram_A(proc, newProc)		MI_SwitchWram( MI_WRAM_A, proc, newProc )
#define MI_SwitchWram_B(proc, newProc)		MI_SwitchWram( MI_WRAM_B, proc, newProc )
#define MI_SwitchWram_C(proc, newProc)		MI_SwitchWram( MI_WRAM_C, proc, newProc )

/*---------------------------------------------------------------------------*
  Name:         MI_SwitchWramSlot

  Description:  switch WRAM master

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

                slot   : slot number
                             (for WRAM_A)
                                0-3
                             (for WRAM_B and WRAM_C)
                                0-7
                size   :
                             (for WRAM_A)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_64KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_256KB : 256KB

                             (for WRAM_B and WRAM_C)
                                MI_WRAM_SIZE_0KB   :   0KB
                                MI_WRAM_SIZE_32KB  :  16KB
                                MI_WRAM_SIZE_64KB  :  32KB
                                MI_WRAM_SIZE_96KB  :  64KB
                                MI_WRAM_SIZE_128KB : 128KB
                                MI_WRAM_SIZE_160KB : 160KB
                                MI_WRAM_SIZE_192KB : 192KB
                                MI_WRAM_SIZE_224KB : 224KB
                                MI_WRAM_SIZE_256KB : 256KB

                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

                newProc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      number of changed slots
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_SwitchWramSlot( MIWramPos wram, int slot, MIWramSize size, MIWramProc proc, MIWramProc newProc );
#define MI_SwitchWramSlot_A(slot, size, proc, newProc)	MI_SwitchWramSlot(MI_WRAM_A, slot, size, proc, newProc)
#define MI_SwitchWramSlot_B(slot, size, proc, newProc)	MI_SwitchWramSlot(MI_WRAM_B, slot, size, proc, newProc)
#define MI_SwitchWramSlot_C(slot, size, proc, newProc)	MI_SwitchWramSlot(MI_WRAM_C, slot, size, proc, newProc)

/*---------------------------------------------------------------------------*
  Name:         MI_ReserveWram

  Description:  reserve WRAM

  Arguments:    wram   : wram position
                                 MI_WRAM_A : WRAM A
                                 MI_WRAM_B : WRAM B
                                 MI_WRAM_C : WRAM C
                size   : size to reserve
                                 MI_WRAM_SIZE_0KB   :   0KB
                                 MI_WRAM_SIZE_32KB  :  16KB
                                 MI_WRAM_SIZE_64KB  :  32KB
                                 MI_WRAM_SIZE_96KB  :  64KB
                                 MI_WRAM_SIZE_128KB : 128KB
                                 MI_WRAM_SIZE_160KB : 160KB
                                 MI_WRAM_SIZE_192KB : 192KB
                                 MI_WRAM_SIZE_224KB : 224KB
                                 MI_WRAM_SIZE_256KB : 256KB
                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      top address of reserved WRAM
                0 means failure
 *---------------------------------------------------------------------------*/
u32  MI_ReserveWram( MIWramPos wram, MIWramSize size, MIWramProc proc );
#define MI_ReserveWram_A(size, proc)   MI_ReserveWram( MI_WRAM_A, size, proc )
#define MI_ReserveWram_B(size, proc)   MI_ReserveWram( MI_WRAM_B, size, proc )
#define MI_ReserveWram_C(size, proc)   MI_ReserveWram( MI_WRAM_C, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_ReserveWramSlot

  Description:  reserve specified WRAM slot

  Arguments:    wram   : wram position
                                 MI_WRAM_A : WRAM A
                                 MI_WRAM_B : WRAM B
                                 MI_WRAM_C : WRAM C
                slot   : wram slot
                size   : size to reserve
                                 MI_WRAM_SIZE_0KB   :   0KB
                                 MI_WRAM_SIZE_32KB  :  16KB
                                 MI_WRAM_SIZE_64KB  :  32KB
                                 MI_WRAM_SIZE_96KB  :  64KB
                                 MI_WRAM_SIZE_128KB : 128KB
                                 MI_WRAM_SIZE_160KB : 160KB
                                 MI_WRAM_SIZE_192KB : 192KB
                                 MI_WRAM_SIZE_224KB : 224KB
                                 MI_WRAM_SIZE_256KB : 256KB
                proc   : processor
                                 MI_WRAM_ARM9 : ARM9
                                 MI_WRAM_ARM7 : ARM7
                                 MI_WRAM_DSP  : DSP

  Returns:      top address of reserved WRAM
                0 means failure
 *---------------------------------------------------------------------------*/
u32  MI_ReserveWramSlot( MIWramPos wram, int slot, MIWramSize size, MIWramProc proc );
#define MI_ReserveWramSlot_A(slot, size, proc)   MI_ReserveWramSlot( MI_WRAM_A, slot, size, proc )
#define MI_ReserveWramSlot_B(slot, size, proc)   MI_ReserveWramSlot( MI_WRAM_B, slot, size, proc )
#define MI_ReserveWramSlot_C(slot, size, proc)   MI_ReserveWramSlot( MI_WRAM_C, slot, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_CancelWram

  Description:  cancel reserved WRAM

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                proc   : processor
                                MI_WRAM_ARM9 : ARM9
                                MI_WRAM_ARM7 : ARM7
                                MI_WRAM_DSP  : DSP

  Returns:      nunmber of canceled blocks
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_CancelWram( MIWramPos wram, MIWramProc proc );
#define MI_CancelWram_A(proc)     MI_CancelWram( MI_WRAM_A, proc )
#define MI_CancelWram_B(proc)     MI_CancelWram( MI_WRAM_B, proc )
#define MI_CancelWram_C(proc)     MI_CancelWram( MI_WRAM_C, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_CancelWramSlot

  Description:  cancel reserved WRAM

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                slot   :								
                size   :								
                proc   : processor
                                MI_WRAM_ARM9 : ARM9
                                MI_WRAM_ARM7 : ARM7
                                MI_WRAM_DSP  : DSP

  Returns:      nunmber of canceled blocks
                -1 means failure
 *---------------------------------------------------------------------------*/
int MI_CancelWramSlot( MIWramPos wram, int slot, MIWramSize size, MIWramProc proc );
#define MI_CancelWramSlot_A(slot, size, proc)   MI_CancelWramSlot( MI_WRAM_A, slot, size, proc )
#define MI_CancelWramSlot_B(slot, size, proc)   MI_CancelWramSlot( MI_WRAM_B, slot, size, proc )
#define MI_CancelWramSlot_C(slot, size, proc)   MI_CancelWramSlot( MI_WRAM_C, slot, size, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_LockWramSlot

  Description:  lock WRAM slot

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                slots  : slot to lock

  Returns:      0 if success
                -1 means filer
 *---------------------------------------------------------------------------*/
int MI_LockWramSlots( MIWramPos wram, u32 slots );
#define MI_LockWramSlots_A(slots)       MI_LockWramSlots( MI_WRAM_A, slots )
#define MI_LockWramSlots_B(slots)       MI_LockWramSlots( MI_WRAM_B, slots )
#define MI_LockWramSlots_C(slots)       MI_LockWramSlots( MI_WRAM_C, slots )

/*---------------------------------------------------------------------------*
  Name:         MI_UnlockWramSlot

  Description:  unlock WRAM slot

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C
                slots  : slot to unlock

  Returns:      0 if success
                -1 means filer
 *---------------------------------------------------------------------------*/
int MI_UnlockWramSlots( MIWramPos wram, u32 slots );
#define MI_UnlockWramSlots_A(slots)       MI_UnlockWramSlots( MI_WRAM_A, slots )
#define MI_UnlockWramSlots_B(slots)       MI_UnlockWramSlots( MI_WRAM_B, slots )
#define MI_UnlockWramSlots_C(slots)       MI_UnlockWramSlots( MI_WRAM_C, slots )

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_GetWramReservation

  Description:  check WRAM reservation

  Arguments:    wram   : wram position
                             MI_WRAM_A : WRAM A
                             MI_WRAM_B : WRAM B
                             MI_WRAM_C : WRAM C

                num    ; slot num
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      	         MI_WRAM_ARM9 : ARM9
                    	     MI_WRAM_ARM7 : ARM7
                             MI_WRAM_DSP  : DSP   (only for WRAM_B and WRAM_C)
 *---------------------------------------------------------------------------*/
MIWramProc MI_GetWramReservation( MIWramPos wram, int slot );
#define MI_GetWramReservation_A(slot)   MI_GetWramReservation( MI_WRAM_A, slot )
#define MI_GetWramReservation_B(slot)   MI_GetWramReservation( MI_WRAM_B, slot )
#define MI_GetWramReservation_C(slot)   MI_GetWramReservation( MI_WRAM_C, slot )


/*---------------------------------------------------------------------------*
  Name:         MI_GetAllocatableWramSlot

  Description:  get allocatable WRAM slots by specifying the processor

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

                proc   : processor
                                MI_WRAM_ARM9 : ARM9
                                MI_WRAM_ARM7 : ARM7
                                MI_WRAM_DSP  : DSP

  Returns:      bit=1 slot is allocatable for the processor.
                bit=0 slot is used by other processors, so cannot allocate.
                  WRAM_A : (1<<3)=slot3, (1<<2)=slot2, ... , (1<<0)=slot0
                  WRAM_B : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0
                  WRAM_C : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0

 *---------------------------------------------------------------------------*/
u8 MI_GetAllocatableWramSlot( MIWramPos wram, MIWramProc proc );
#define MI_GetAllocatableWramSlot_A(proc)   MI_GetAllocatableWramSlot( MI_WRAM_A, proc )
#define MI_GetAllocatableWramSlot_B(proc)   MI_GetAllocatableWramSlot( MI_WRAM_B, proc )
#define MI_GetAllocatableWramSlot_C(proc)   MI_GetAllocatableWramSlot( MI_WRAM_C, proc )

/*---------------------------------------------------------------------------*
  Name:         MI_GetFreeWramSlot

  Description:  get free WRAM slots.

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

  Returns:      bit=1 slot is free.
                bit=0 slot is not free, that is it is used or reserved by some processor.
                  WRAM_A : (1<<3)=slot3, (1<<2)=slot2, ... , (1<<0)=slot0
                  WRAM_B : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0
                  WRAM_C : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0

 *---------------------------------------------------------------------------*/
#define MI_GetFreeWramSlot(wram)     MI_GetAllocatableWramSlot( wram, MI_WRAM_FREE )
#define MI_GetFreeWramSlot_A()       MI_GetFreeWramSlot( MI_WRAM_A )
#define MI_GetFreeWramSlot_B()       MI_GetFreeWramSlot( MI_WRAM_B )
#define MI_GetFreeWramSlot_C()       MI_GetFreeWramSlot( MI_WRAM_C )

/*---------------------------------------------------------------------------*
  Name:         MI_GetUsedWramSlot

  Description:  get used WRAM slots.

  Arguments:    wram   : wram position
                                MI_WRAM_A : WRAM A
                                MI_WRAM_B : WRAM B
                                MI_WRAM_C : WRAM C

  Returns:      bit=1 slot is used by some processor.
                bit=0 slot is not used. (it may be reserved by some processor)
                  WRAM_A : (1<<3)=slot3, (1<<2)=slot2, ... , (1<<0)=slot0
                  WRAM_B : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0
                  WRAM_C : (1<<7)=slot7, (1<<6)=slot6, ... , (1<<0)=slot0

 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
u8 MI_GetUsedWramSlot( MIWramPos wram );
#define MI_GetUsedWramSlot_A()       MI_GetUsedWramSlot( MI_WRAM_A )
#define MI_GetUsedWramSlot_B()       MI_GetUsedWramSlot( MI_WRAM_B )
#define MI_GetUsedWramSlot_C()       MI_GetUsedWramSlot( MI_WRAM_C )
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_IsWramSlotUsed

  Description:  check if WRAM slot is used

  Arguments:    wram   : wram position
                             MI_WRAM_A : WRAM A
                             MI_WRAM_B : WRAM B
                             MI_WRAM_C : WRAM C

                slot   ; slot
                             WRAM_A:0-3  WRAM_B:0-7  WRAM_C:0-7

  Returns:      	         TRUE  ... used
                             FALSE ... not used
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
BOOL MI_IsWramSlotUsed( MIWramPos wram, int slot );
#define MI_IsWramSlotUsed_A(slot)      MI_IsWramSlotUsed( MI_WRAM_A, slot )
#define MI_IsWramSlotUsed_B(slot)      MI_IsWramSlotUsed( MI_WRAM_B, slot )
#define MI_IsWramSlotUsed_C(slot)      MI_IsWramSlotUsed( MI_WRAM_C, slot )
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_LockWramManager

  Description:  lock wram manager

  Arguments:    lockid : lockID

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_LockWramManager( u16* lockid );
/*---------------------------------------------------------------------------*
  Name:         MI_UnlockWramManager

  Description:  ullock wram manager

  Arguments:    lockid : lockID

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_UnlockWramManager( u16 lockid );
/*---------------------------------------------------------------------------*
  Name:         MI_TryLockWramManager

  Description:  try to lock wram manager

  Arguments:    lockid : lockID

  Returns:      >0 value          previous locked id
                OS_LOCK_SUCCESS   success to lock
 *---------------------------------------------------------------------------*/
s32 MI_TryLockWramManager( u16* lockid );

//================================================================================
//   for DEBUG
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DumpThreadList

  Description:  Dump wram Status for WRAM-A/B/C

  Arguments:    wram : wram position.
                MI_WRAM_A / MI_WRAM_B / MI_WRAM_C

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DumpWramList(MIWramPos wram);
#define MI_DumpWramList_A()     MI_DumpWramList( MI_WRAM_A )
#define MI_DumpWramList_B()     MI_DumpWramList( MI_WRAM_B )
#define MI_DumpWramList_C()     MI_DumpWramList( MI_WRAM_C )

/*---------------------------------------------------------------------------*
  Name:         MI_DumpWramListAll

  Description:  Dump all wram status

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DumpWramListAll(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_MI_SHAREDWRAM_H_ */
#endif
