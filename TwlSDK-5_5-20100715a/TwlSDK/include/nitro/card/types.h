/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     types.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-26#$
  $Rev: 10827 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_TYPES_H_
#define NITRO_CARD_TYPES_H_


#include <nitro/misc.h>
#include <nitro/types.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Data structure for a ROM region
typedef struct CARDRomRegion
{
    u32     offset;
    u32     length;
}
CARDRomRegion;

// ROM header structure
typedef struct CARDRomHeader
{

    // 0x000-0x020 [System reserve area]
    char    game_name[12];          // Software title name
    u32     game_code;              // Game code
    u16     maker_code;             // Maker code
    u8      product_id;             // System code
    u8      device_type;            // Device type
    u8      device_size;            // Device capacity
    u8      reserved_A[9];          // System reserve A
    u8      game_version;           // Software version
    u8      property;               // Internal use flag
    // 0x020-0x040 [parameter for resident modules]
    void   *main_rom_offset;        // ARM9 transfer source ROM offset
    void   *main_entry_address;     // ARM9 execution start address (not yet implemented)
    void   *main_ram_address;       // ARM9 transfer destination RAM offset
    u32     main_size;              // ARM9 transfer size
    void   *sub_rom_offset;         // ARM7 transfer source ROM offset
    void   *sub_entry_address;      // ARM7 execution start address (not yet implemented)
    void   *sub_ram_address;        // ARM7 transfer destination RAM offset
    u32     sub_size;               // ARM7 transfer size

    // 0x040-0x050 [parameters for the file table]
    CARDRomRegion fnt;              // Filename table
    CARDRomRegion fat;              // File allocation table

    // 0x050-0x060 [parameters for the overlay header table]
    CARDRomRegion main_ovt;         // ARM9 overlay header table
    CARDRomRegion sub_ovt;          // ARM7 overlay header table

    // 0x060-0x070 [System reserve area]
    u8      rom_param_A[8];         // Mask ROM control parameter A
    u32     banner_offset;          // Banner file ROM offset
    u16     secure_crc;             // Secure area CRC
    u8      rom_param_B[2];         // Mask ROM control parameter B

    // 0x070-0x078 [Auto load parameters]
    void   *main_autoload_done;     // ARM9 auto load hook address
    void   *sub_autoload_done;      // ARM7 auto load hook address

    // 0x078-0x0C0 [System reserve area]
    u8      rom_param_C[8];         // Mask ROM control parameter C
    u32     rom_size;               // Application final ROM offset
    u32     header_size;            // ROM header size
    u32     main_module_param_offset;  // Offset for table of ARM9 module parameters
    u32     sub_module_param_offset;   // Offset for table of ARM7 module parameters

    u16     normal_area_rom_offset; // undeveloped
    u16     twl_ltd_area_rom_offset;// undeveloped
    u8      reserved_B[0x2C];       // System reserve B

    // 0x0C0-0x160 [System reserve area]
    u8      logo_data[0x9C];        // NINTENDO logo image data
    u16     logo_crc;               // NINTENDO logo CRC
    u16     header_crc;             // ROM internal registration data CRC

}
CARDRomHeader;

typedef CARDRomHeader CARDRomHeaderNTR;

// Extended TWL ROM header data
typedef struct CARDRomHeaderTWL
{
    CARDRomHeaderNTR    ntr;
    u8                  debugger_reserved[0x20];    // Reserved for the debugger
    u8                  config1[0x34];              // Group of flags used internally
    // 0x1B4 - accessControl
    struct {
        u32     :5;
        u32     game_card_on :1;            // Power to the Game Card is on with a NAND application (normal mode)
		u32     :2;
        u32     game_card_nitro_mode :1;    // Access to NTR-compatible regions on the Game Card with a NAND application
		u32     :2;
        u32     photo_access_read :1;       // "photo:" archive read-access control
        u32     photo_access_write :1;      // "photo:" archive write-access control
        u32     sdmc_access_read :1;        // "sdmc:" archive read-access control
        u32     sdmc_access_write :1;       // "sdmc:" archive write-access control
        u32     backup_access_read :1;      // CARD-backup read-access control
        u32     backup_access_write :1;     // CARD-backup write-access control
        u32     :0;
    }access_control;
    u8                  reserved_0x1B8[8];          // Reserved (all 0's)
    u32                 main_ltd_rom_offset;
    u8                  reserved_0x1C4[4];          // Reserved (all 0's)
    void               *main_ltd_ram_address;
    u32                 main_ltd_size;
    u32                 sub_ltd_rom_offset;
    u8                  reserved_0x1D4[4];          // Reserved (all 0's)
    void               *sub_ltd_ram_address;
    u32                 sub_ltd_size;
    CARDRomRegion       digest_area_ntr;
    CARDRomRegion       digest_area_ltd;
    CARDRomRegion       digest_tabel1;
    CARDRomRegion       digest_tabel2;
    u32                 digest_table1_size;
    u32                 digest_table2_sectors;
    u8                  config2[0xF8];              // Group of flags used internally
    u8                  main_static_digest[0x14];
    u8                  sub_static_digest[0x14];
    u8                  digest_tabel2_digest[0x14];
    u8                  banner_digest[0x14];
    u8                  main_ltd_static_digest[0x14];
    u8                  sub_ltd_static_digest[0x14];
}
CARDRomHeaderTWL;


SDK_COMPILER_ASSERT(sizeof(CARDRomHeader) == 0x160);
SDK_COMPILER_ASSERT(sizeof(CARDRomHeaderTWL) == 0x378);


/*---------------------------------------------------------------------------*/
/* Constants */

// Size of the ROM area
#define CARD_ROM_PAGE_SIZE	512

// Size of the DS Download Play signature data that is sometimes attached to the end of NTR regions (CARDRomHeader.rom_size)
// 
#define CARD_ROM_DOWNLOAD_SIGNATURE_SIZE 136

// Processing result for functions in the CARD library
typedef enum CARDResult
{
    CARD_RESULT_SUCCESS = 0,
    CARD_RESULT_FAILURE,
    CARD_RESULT_INVALID_PARAM,
    CARD_RESULT_UNSUPPORTED,
    CARD_RESULT_TIMEOUT,
    CARD_RESULT_ERROR,
    CARD_RESULT_NO_RESPONSE,
    CARD_RESULT_CANCELED
}
CARDResult;

#define CARD_ROM_HEADER_EXE_NTR_OFF 0x01
#define CARD_ROM_HEADER_EXE_TWL_ON  0x02


/*---------------------------------------------------------------------------*/
/* Types */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsExecutableOnNTR

  Description:  Checks the flag that determines whether a program can start in DS mode.

  Arguments:    header: The program's ROM header data, which must be checked

  Returns:      TRUE if the ROM header belongs to a program that can be started in DS mode.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsExecutableOnNTR(const CARDRomHeader *header)
{
    return ((header->product_id & CARD_ROM_HEADER_EXE_NTR_OFF) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsExecutableOnTWL

  Description:  Checks the flag that determines whether a program can start in TWL mode.

  Arguments:    header: The program's ROM header data, which must be checked

  Returns:      TRUE if the ROM header belongs to a program that can be started in TWL mode.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsExecutableOnTWL(const CARDRomHeader *header)
{
    return ((header->product_id & CARD_ROM_HEADER_EXE_TWL_ON) != 0);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_TYPES_H_
