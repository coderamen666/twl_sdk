/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg_common.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CTRDG_COMMON_H_
#define NITRO_CTRDG_COMMON_H_


#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/mi.h>
#include <nitro/os.h>
#include <nitro/pxi.h>


#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------

// Definitions Related to PXI Communications Protocol
#define CTRDG_PXI_COMMAND_MASK              0x0000003f  // Command part for start word
#define CTRDG_PXI_COMMAND_SHIFT             0
#define CTRDG_PXI_COMMAND_PARAM_MASK        0x01ffffc0  // Parameter part for start word
#define CTRDG_PXI_COMMAND_PARAM_SHIFT       6
#define CTRDG_PXI_FIXLEN_DATA_MASK          0x03ffffff  // Data part for fixed-length continuous packets
#define CTRDG_PXI_FIXLEN_DATA_SHIFT         0
#define CTRDG_PXI_FLEXLEN_DATA_MASK         0x01ffffff  // Data part for variable-length continuous packets
#define CTRDG_PXI_FLEXLEN_DATA_SHIFT        0
#define CTRDG_PXI_FLEXLEN_CONTINUOUS_BIT    0x02000000  // Transferring variable-length continuous packets
#define CTRDG_PXI_PACKET_MAX                4   // Largest continuous number of packets

// Commands issued from ARM9 to the ARM7 via PXI
#define CTRDG_PXI_COMMAND_INIT_MODULE_INFO  0x0001
#define CTRDG_PXI_COMMAND_TERMINATE         0x0002
#define CTRDG_PXI_COMMAND_SET_PHI           0x0003

#define CTRDG_PXI_COMMAND_IMI_PACKET_SIZE   2

// Commands issued from ARM7 to the ARM9 via PXI
#define CTRDG_PXI_COMMAND_PULLED_OUT        0x0011
#define CTRDG_PXI_COMMAND_SET_PHI_RESULT    0x0012


#ifdef SDK_ARM9
#define CTRDG_LOCKED_BY_MYPROC_FLAG     OS_MAINP_LOCKED_FLAG
#else
#define CTRDG_LOCKED_BY_MYPROC_FLAG     OS_SUBP_LOCKED_FLAG
#endif

#define CTRDG_SYSROM9_NINLOGO_ADR       0xffff0020

#define CTRDG_AGB_ROMHEADER_SIZE        0xc0
#define CTRDG_AGB_NINLOGO_SIZE          0x9c

#define CTRDG_IS_ROM_CODE               0x96


//---- PHI Output Control
typedef enum
{
    CTRDG_PHI_CLOCK_LOW = MIi_PHI_CLOCK_LOW,    // Low level
    CTRDG_PHI_CLOCK_4MHZ = MIi_PHI_CLOCK_4MHZ,  //  4.19 MHz
    CTRDG_PHI_CLOCK_8MHZ = MIi_PHI_CLOCK_8MHZ,  //  8.38 MHz
    CTRDG_PHI_CLOCK_16MHZ = MIi_PHI_CLOCK_16MHZ // 16.76 MHz
}
CTRDGPhiClock;

//----------------------------------------------------------------------------

// Game Pak header

typedef struct
{
    u32     startAddress;              // Start address (for AGB)
    u8      nintendoLogo[0x9c];        // NINTENDO logo data (for AGB)

    char    titleName[12];             // Soft title name (for AGB)
    u32     gameCode;                  // Game code (for AGB)
    u16     makerCode;                 // Maker code (for AGB)

    u8      isRomCode;                 // Is ROM code

    u8      machineCode;               // Machine code (for AGB)
    u8      deviceType;                // Device type (for AGB)

    u8      exLsiID[3];                // Extended LSI-ID

    u8      reserved_A[4];             // Reserved A (Set 0)
    u8      softVersion;               // Soft version (for AGB)
    u8      complement;                // Complement (for AGB)

    u16     moduleID;                  // Module ID
}
CTRDGHeader;

// Peripheral devices ID

typedef struct
{
    union
    {
        struct
        {
            u8      bitID;             // Bit ID
            u8      numberID:5;        // Number ID
            u8:     2;
            u8      disableExLsiID:1;  // Invalidate expanded LSI-ID region
        };
        u16     raw;
    };
}
CTRDGModuleID;

// Peripheral devices information

typedef struct
{
    CTRDGModuleID moduleID;            // Module ID
    u8      exLsiID[3];                // Extended LSI-ID
    u8      isAgbCartridge:1;          // Is AgbCartridge
    u8      detectPullOut:1;           // Detect pulled-out Game Pak
    u8:     0;
    u16     makerCode;                 // Maker code (for AGB)
    u32     gameCode;                  // Game code (for AGB)
}
CTRDGModuleInfo;


//---- Callback type for pulled-out Game Pak
#ifdef SDK_ARM9
typedef BOOL (*CTRDGPulledOutCallback) (void);
#endif
/*---------------------------------------------------------------------------*

  Description:  About the CTRDG library

                ARM7 periodically checks to see if the Game Pak has been removed.
                However, this cannot be done if the ARM9 does not release its access rights to the cartridge bus, so do not keep a lock for more than 10 frames.
                
                

                Internal processing locks the Game Pak, so control will not return until locks set by other processors are released.
                
                

                However, if it has already been locked by this processor, you may call this without unlocking.
                

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsInitialized

  Description:  Checks whether the ctrdg library is initialized
                (for internal use)

  Arguments:    None.

  Returns:      TRUE ... initialized already
                FALSE... not initialized
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWLLTD
BOOL CTRDGi_IsInitialized(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Init

  Description:  Initializes the Game Pak.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWLLTD
void    CTRDG_Init(void);
#else // SDK_TWLLTD
SDK_INLINE void CTRDG_Init(void)
{
#ifdef SDK_ARM9
    static BOOL isInitialized;

    if (isInitialized)
    {
        return;
    }
    isInitialized = TRUE;

    {
        CTRDGModuleInfo *cip = (CTRDGModuleInfo *)HW_CTRDG_MODULE_INFO_BUF;

        cip->moduleID.raw = 0xFFFF;
        cip->exLsiID[0] = 0xFF;
        cip->exLsiID[1] = 0xFF;
        cip->exLsiID[2] = 0xFF;
        cip->isAgbCartridge = FALSE;
        cip->detectPullOut = FALSE;
        cip->makerCode = 0xFFFF;
        cip->gameCode = 0xFFFFFFFF;
    }
#endif // SDK_ARM9
}
#endif // SDK_TWLLTD

/*---------------------------------------------------------------------------*
  Name:         CTRDG_DummyInit

  Description:  Initialize for setting CTRDG blank.
                Called from the user-defined CTRDG_Init.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_ARM9) && !defined(SDK_TWLLTD)
void CTRDG_DummyInit(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsPulledOut

  Description:  Checks to see if the Game Pak has been removed.

                If the Game Pak was never inserted, the function returns FALSE.

                If no resistance adapter Game Pak has been installed in the IS-NITRO-DEBUGGER, be aware that a Game Pak removal may be detected even if a Game Pak was never inserted.
                
                

  Arguments:    None.

  Returns:      TRUE if Game Pak removal detected
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsPulledOut(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsExisting

  Description:  Checks whether there is a Game Pak.

  Arguments:    None.

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsExisting(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsBitID

  Description:  Whether bit ID peripheral exists in the Game Pak.


  Arguments:    bitID  bit ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsBitID(u8 bitID);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsNumberID

  Description:  Whether number ID peripheral exists in the Game Pak.

  Arguments:    numberID  number ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsNumberID(u8 numberID);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsAgbCartridge

  Description:  Checks whether there is an AGB Game Pak.

  Arguments:    None.

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsAgbCartridge(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsOptionCartridge

  Description:  Checks whether there is an option Game Pak.

  Arguments:    None.

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsOptionCartridge(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_GetAgbGameCode

  Description:  Gets the AGB Game Pak's game code.

  Arguments:    None.

  Returns:      Game code if it exists, FALSE if it doesn't exist
 *---------------------------------------------------------------------------*/
u32     CTRDG_GetAgbGameCode(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_GetAgbMakerCode

  Description:  Gets the AGB Game Pak's maker code.

  Arguments:    None.

  Returns:      Maker code if it exists, FALSE if it doesn't exist
 *---------------------------------------------------------------------------*/
u16     CTRDG_GetAgbMakerCode(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsAgbCartridgePulledOut

  Description:  Get whether system has detected pulled-out AGB Game Pak.

  Arguments:    None.

  Returns:      TRUE if Game Pak removal detected
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsAgbCartridgePulledOut(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsOptionCartridgePulledOut

  Description:  get whether system has detected pulled-out option Game Pak.

  Arguments:    None.

  Returns:      TRUE if Game Pak removal detected
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsOptionCartridgePulledOut(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetPulledOutCallback

  Description:  Set user callback for being pulled-out Game Pak.

  Arguments:    callback: Callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void    CTRDG_SetPulledOutCallback(CTRDGPulledOutCallback callback);
#endif

/*---------------------------------------------------------------------------*
  Name:         CTRDG_TerminateForPulledOut

  Description:  Terminates for pulling out Game Pak.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void    CTRDG_TerminateForPulledOut(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SendToARM7

  Description:  Sends data to ARM7.

  Arguments:    arg: data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDG_SendToARM7(void *arg);

//================================================================================
//       ACCESS DATA IN CARTRIDGE
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CTRDG_DmaCopy16 / 32

  Description:  Reads Game Pak data via DMA.

  Arguments:    dmaNo: DMA No.
                src: source address (in Game Pak)
                dest: destination address (in memory)
                size: forwarding size

  Returns:      TRUE if success.
                FALSE if fail. (no Game Pak)
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size);
BOOL    CTRDG_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CpuCopy8 / 16 / 32

  Description:  Reads Game Pak data by CPU access.

  Arguments:    src: source address (in Game Pak)
                dest: destination address (in memory)
                size: forwarding size

  Returns:      TRUE if success.
                FALSE if fail. (no Game Pak)
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_CpuCopy8(const void *src, void *dest, u32 size);
BOOL    CTRDG_CpuCopy16(const void *src, void *dest, u32 size);
BOOL    CTRDG_CpuCopy32(const void *src, void *dest, u32 size);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Read8 / 16 / 32

  Description:  Reads Game Pak data by CPU access.

  Arguments:    address: source address (in Game Pak)
                rdata: address to store read data

  Returns:      TRUE if success.
                FALSE if fail. (no Game Pak)
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_Read8(const u8 *address, u8 *rdata);
BOOL    CTRDG_Read16(const u16 *address, u16 *rdata);
BOOL    CTRDG_Read32(const u32 *address, u32 *rdata);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Write8 / 16 / 32

  Description:  Writes data to Game Pak.

  Arguments:    address: destination address (in Game Pak)
                data: data to write

  Returns:      TRUE if success.
                FALSE if fail. (no Game Pak)
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_Write8(u8 *address, u8 data);
BOOL    CTRDG_Write16(u16 *address, u16 data);
BOOL    CTRDG_Write32(u32 *address, u32 data);


/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsEnabled

  Description:  Checks if CTRDG is accessible.

  Arguments:    None.

  Returns:      Returns Game Pak access permission.
 *---------------------------------------------------------------------------*/
BOOL    CTRDG_IsEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Enable

  Description:  Set Game Pak access permission mode.

  Arguments:    enable       permission mode to be set.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDG_Enable(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CheckEnabled

  Description:  Terminates program if CTRDG is not accessible.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDG_CheckEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CheckPulledOut

  Description:  Game Pak is pulled out.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDG_CheckPulledOut(void);


/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetPhiClock

  Description:  Sets ARM7 and ARM9 PHI output clock.

  Arguments:    clock:    Phi clock to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void    CTRDG_SetPhiClock(CTRDGPhiClock clock);
#endif


/*---------------------------------------------------------------------------*
  Name:         CTRDG_GetPhiClock

  Description:  Gets ARM7 and ARM9 PHI output clock setting.

  Arguments:    None.

  Returns:      Phi clock setting
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
static inline CTRDGPhiClock CTRDG_GetPhiClock(void)
{
      return (CTRDGPhiClock)MIi_GetPhiClock();
}
#endif

//================================================================================
//       PRIVATE FUNCTION
//================================================================================
//---- Private function ( don't use these CTRDGi_* function )

// ROM access-cycle structure
typedef struct CTRDGRomCycle
{
    MICartridgeRomCycle1st c1;
    MICartridgeRomCycle2nd c2;

}
CTRDGRomCycle;

// Processor unit lock information structure
typedef struct CTRDGLockByProc
{
    BOOL    locked;
    OSIntrMode irq;

}
CTRDGLockByProc;

void    CTRDGi_InitCommon(void);
void    CTRDGi_InitModuleInfo(void);
void    CTRDGi_SendtoPxi(u32 data);
#if defined(SDK_ARM7)
BOOL    CTRDGi_LockByProcessor(u16 lockID, CTRDGLockByProc *info);
#else
void    CTRDGi_LockByProcessor(u16 lockID, CTRDGLockByProc *info);
#endif
void    CTRDGi_UnlockByProcessor(u16 lockID, CTRDGLockByProc *info);
void    CTRDGi_ChangeLatestAccessCycle(CTRDGRomCycle *r);
void    CTRDGi_RestoreAccessCycle(CTRDGRomCycle *r);

BOOL    CTRDGi_IsBitIDAtInit(u8 bitID);
BOOL    CTRDGi_IsNumberIDAtInit(u8 numberID);
BOOL    CTRDGi_IsAgbCartridgeAtInit(void);
u32     CTRDGi_GetAgbGameCodeAtInit(void);
u16     CTRDGi_GetAgbMakerCodeAtInit(void);

// Get the Game Pak header address
#define CTRDGi_GetHeaderAddr()          ((CTRDGHeader *)HW_CTRDG_ROM)

// Get the ROM image address of the Game Pak peripheral ID
#define CTRDGi_GetModuleIDImageAddr()   ((u16 *)(HW_CTRDG_ROM + 0x0001fffe))

// Get the address of the Game Pak peripheral information
#define CTRDGi_GetModuleInfoAddr()      ((CTRDGModuleInfo *)HW_CTRDG_MODULE_INFO_BUF)




#define CTRDGi_FORWARD_TYPE_DMA   0x00000000
#define CTRDGi_FORWARD_TYPE_CPU   0x00000001
#define CTRDGi_FORWARD_TYPE_MASK  0x00000001

#define CTRDGi_FORWARD_WIDTH_8    0x00000000
#define CTRDGi_FORWARD_WIDTH_16   0x00000010
#define CTRDGi_FORWARD_WIDTH_32   0x00000020
#define CTRDGi_FORWARD_WIDTH_MASK 0x00000030

#define CTRDGi_FORWARD_DMA16   ( CTRDGi_FORWARD_TYPE_DMA | CTRDGi_FORWARD_WIDTH_16 )
#define CTRDGi_FORWARD_DMA32   ( CTRDGi_FORWARD_TYPE_DMA | CTRDGi_FORWARD_WIDTH_32 )
#define CTRDGi_FORWARD_CPU8    ( CTRDGi_FORWARD_TYPE_CPU | CTRDGi_FORWARD_WIDTH_8  )
#define CTRDGi_FORWARD_CPU16   ( CTRDGi_FORWARD_TYPE_CPU | CTRDGi_FORWARD_WIDTH_16 )
#define CTRDGi_FORWARD_CPU32   ( CTRDGi_FORWARD_TYPE_CPU | CTRDGi_FORWARD_WIDTH_32 )

#define CTRDGi_ACCESS_DIR_WRITE   0x00000000
#define CTRDGi_ACCESS_DIR_READ    0x00000001
#define CTRDGi_ACCESS_DIR_MASK    0x00000001

#define CTRDGi_ACCESS_WIDTH_8     0x00000010
#define CTRDGi_ACCESS_WIDTH_16    0x00000020
#define CTRDGi_ACCESS_WIDTH_32    0x00000040
#define CTRDGi_ACCESS_WIDTH_MASK  0x000000f0

#define CTRDGi_ACCESS_WRITE8  ( CTRDGi_ACCESS_DIR_WRITE | CTRDGi_ACCESS_WIDTH_8  )
#define CTRDGi_ACCESS_WRITE16 ( CTRDGi_ACCESS_DIR_WRITE | CTRDGi_ACCESS_WIDTH_16 )
#define CTRDGi_ACCESS_WRITE32 ( CTRDGi_ACCESS_DIR_WRITE | CTRDGi_ACCESS_WIDTH_32 )
#define CTRDGi_ACCESS_READ8   ( CTRDGi_ACCESS_DIR_READ  | CTRDGi_ACCESS_WIDTH_8  )
#define CTRDGi_ACCESS_READ16  ( CTRDGi_ACCESS_DIR_READ  | CTRDGi_ACCESS_WIDTH_16 )
#define CTRDGi_ACCESS_READ32  ( CTRDGi_ACCESS_DIR_READ  | CTRDGi_ACCESS_WIDTH_32 )

//---- internal function
BOOL    CTRDGi_CopyCommon(u32 dmaNo, const void *src, void *dest, u32 size, u32 forwardType);

//---- internal function
BOOL    CTRDGi_AccessCommon(void *address, u32 data, void *rdata, u32 accessType);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_CTRDG_COMMON_H_ */
#endif
