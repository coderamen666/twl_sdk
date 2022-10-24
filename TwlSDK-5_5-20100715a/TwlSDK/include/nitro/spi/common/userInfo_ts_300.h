/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - include
  File:     userInfo_ts_300.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_SPI_COMMON_USERINFO_TS_300_H_
#define NITRO_SPI_COMMON_USERINFO_TS_300_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
// Configuration setting data format version
#define NVRAM_CONFIG_DATA_VERSION           5
#define NVRAM_CONFIG_DATA_EX_VERSION        1

// Various setting values within config
#define NVRAM_CONFIG_NICKNAME_LENGTH        10  // Nickname Length
#define NVRAM_CONFIG_COMMENT_LENGTH         26  // Comment Length
#define NVRAM_CONFIG_FAVORITE_COLOR_MAX     16  // Maximum number of favorite colors

// IPL2 type
#define NVRAM_CONFIG_IPL2_TYPE_ADDRESS      0x001d
#define NVRAM_CONFIG_IPL2_TYPE_SIZE         1
#define NVRAM_CONFIG_IPL2_TYPE_NORMAL       0xff
#define NVRAM_CONFIG_IPL2_TYPE_EX_MASK      0x50

// Language setting code
#define NVRAM_CONFIG_LANG_JAPANESE          0   // Japanese
#define NVRAM_CONFIG_LANG_ENGLISH           1   // English
#define NVRAM_CONFIG_LANG_FRENCH            2   // French
#define NVRAM_CONFIG_LANG_GERMAN            3   // German
#define NVRAM_CONFIG_LANG_ITALIAN           4   // Italian
#define NVRAM_CONFIG_LANG_SPANISH           5   // Spanish
#define NVRAM_CONFIG_LANG_CHINESE           6   // Chinese
#define NVRAM_CONFIG_LANG_HANGUL            7   // Korean
#define NVRAM_CONFIG_LANG_CODE_MAX          8   // Number of types of language setting codes

// Bitmap of languages supported in the normal version
#define NVRAM_CONFIG_LANG_BITMAP_NORMAL     ( (0x0001 << NVRAM_CONFIG_LANG_JAPANESE ) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_ENGLISH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_FRENCH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_GERMAN) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_ITALIAN) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_SPANISH) )

// Bitmap of languages supported in the Chinese version
#define NVRAM_CONFIG_LANG_BITMAP_CHINESE    ( (0x0001 << NVRAM_CONFIG_LANG_ENGLISH ) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_FRENCH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_GERMAN) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_ITALIAN) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_SPANISH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_CHINESE) )


// Bitmap of languages supported in the Korean version
#define NVRAM_CONFIG_LANG_BITMAP_HANGUL     ( (0x0001 << NVRAM_CONFIG_LANG_JAPANESE ) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_ENGLISH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_FRENCH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_GERMAN) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_SPANISH) \
                                            | (0x0001 << NVRAM_CONFIG_LANG_HANGUL) )

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Birthday data (2 bytes)
typedef struct NVRAMConfigDate
{
    u8      month;                     // Month: 01 to 12
    u8      day;                       // Day: 01 to 31

}
NVRAMConfigDate;

// Nickname (22 bytes)
typedef struct NVRAMConfigNickname
{
    u16     str[NVRAM_CONFIG_NICKNAME_LENGTH];  // Nickname (Maximum of 10 characters in Unicode (UTF16), no terminating code)
    u8      length;                    // Number of characters
    u8      rsv;

}
NVRAMConfigNickname;

// Comment (54 bytes)
typedef struct NVRAMConfigComment
{
    u16     str[NVRAM_CONFIG_COMMENT_LENGTH];   //Comment (A maximum of 26 characters in Unicode (UTF16), no terminating code)
    u8      length;                    // Number of characters
    u8      rsv;

}
NVRAMConfigComment;

// Owner Information (80 bytes)
typedef struct NVRAMConfigOwnerInfo
{
    u8      favoriteColor:4;           // Favorite color
    u8      rsv:4;                     // Reserved
    NVRAMConfigDate birthday;          // Birthdate
    u8      pad;
    NVRAMConfigNickname nickname;      // Nickname
    NVRAMConfigComment comment;        // Comment

}
NVRAMConfigOwnerInfo;


// Alarm clock data for IPL (6 bytes)
typedef struct NVRAMConfigAlarm
{
    u8      hour;                      // Alarm Hour: 00 - 23
    u8      minute;                    // Alarm Minute: 00 - 59
    u8      second;                    // Alarm Seconds: 00 - 59
    u8      pad;
    u16     enableWeek:7;              // Flag for which days of the week the alarm is enabled (bit0: Sunday, bit1: Monday, etc..."1" means "enabled")
    u16     alarmOn:1;                 // Alarm ON/OFF (0: OFF, 1: ON)
    u16     rsv:8;                     // Reserved

}
NVRAMConfigAlarm;

// Touch Panel Calibration Data (12 bytes)
typedef struct NVRAMConfigTpCalibData
{
    u16     raw_x1;                    // TP-obtained x value of the first calibration point
    u16     raw_y1;                    // TP-obtained y value of the first calibration point
    u8      dx1;                       // LCD x coordinate of the first calibration point
    u8      dy1;                       // LCD y coordinate of the first calibration point
    u16     raw_x2;                    // TP-obtained x value of the second calibration point
    u16     raw_y2;                    // TP-obtained y value of the second calibration point
    u8      dx2;                       // LCD x coordinate of the second calibration point
    u8      dy2;                       // LCD y coordinate of the second calibration point

}
NVRAMConfigTpCalibData;

// Option information (12 bytes)
typedef struct NVRAMConfigOption
{
    u16     language:3;                // Language code
    u16     agbLcd:1;                  // Start up on which LCD when booting in AGB mode? (0:TOP,1:BOTTOM)
    u16     detectPullOutCardFlag:1;   // Flag that indicates that the card has been pulled out
    u16     detectPullOutCtrdgFlag:1;  // Flag that indicates that the Game Pak has been pulled out
    u16     autoBootFlag:1;            // Whether or not to automatically start up without stopping at the menu in the startup sequence.
    u16     rsv:4;                     // Reserved
    u16     input_favoriteColor:1;     // Was a favorite color input?
    u16     input_tp:1;                // Has the Touch Screen been calibrated? ( " )
    u16     input_language:1;          // Was a language input?         (0: Not configured, 1: Configured)
    u16     input_rtc:1;               // Was the RTC configured?          (       "       )
    u16     input_nickname:1;          // Was a nickname input? (       "       )
    u8      timezone;                  // Time Zone (Currently reserved)
    u8      rtcClockAdjust;            // RTC Clock adjustment value
    s64     rtcOffset;                 // Offset value when configuring the RTC (each time the user changes the configuration of the RTC, this fluctuates in response to that value)

}
NVRAMConfigOption;


// Various types of configuration data (112 bytes)
typedef struct NVRAMConfigData
{
    u8      version;                   // Flash storage data format version
    u8      pad;
    NVRAMConfigOwnerInfo owner;        // Owner Information
    NVRAMConfigAlarm alarm;            // Alarm clock data for IPL
    NVRAMConfigTpCalibData tp;         // Touch Panel Calibration Data
    NVRAMConfigOption option;          // Options

}
NVRAMConfigData;

// Format when saving NVRAM for various types of configuration data (116 bytes)
typedef struct NVRAMConfig
{
    NVRAMConfigData ncd;               // Various types of configuration data
    u16     saveCount;                 // Counts 0x00-0x7f by looping; valid if the count is new data.
    u16     crc16;                     // 16-bit CRC of configuration settings data

}
NVRAMConfig;

// Each type of configuration extension data (138 bytes)
typedef struct NVRAMConfigDataEx
{
    u8      version;                   // Version type of extension configuration data.
    u8      language;                  // Extended language code. (Values extended past NVRAM_CONFIG_LANG_CHINESE are saved.)
    u16     valid_language_bitmap;     // Bitmap that shows valid language codes
    u8      padding[256 - sizeof(NVRAMConfigData) - 4 - 4 - 2];
    // Padding so that the size of NVRAMConfigEx equals 1 page of NVRAM (256 bytes)
}
NVRAMConfigDataEx;

// Format when saving the NVRAM for each type of extended NITRO configuration data (256 bytes)
typedef struct NVRAMConfigEx
{
    NVRAMConfigData ncd;               // Various types of configuration data
    u16     saveCount;                 // Counts 0x00-0x7f by looping; valid if the count is new data.
    u16     crc16;                     // 16-bit CRC of configuration settings data
    NVRAMConfigDataEx ncd_ex;          // Extended configuration settings data
    u16     crc16_ex;                  // 16-bit CRC of extended configuration settings data

}
NVRAMConfigEx;


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_SPI_COMMON_USERINFO_TS_300_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
