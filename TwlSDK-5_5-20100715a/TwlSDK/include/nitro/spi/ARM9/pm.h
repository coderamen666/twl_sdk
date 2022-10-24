/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM - include
  File:     pm.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_PM_ARM9_H_
#define NITRO_PM_ARM9_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/misc.h>
#include <nitro/types.h>
#else
#include <twl/misc.h>
#include <twl/types.h>
#endif

#include <nitro/spi/common/pm_common.h>
#include <nitro/spi/common/type.h>
#include <nitro/pxi/common/fifo.h>

//================================================================
//---- callback for async function
typedef void (*PMCallback) (u32 result, void *arg);

//================================================================
//     parameters for PM API
//================================================================
//---- LCD target
typedef enum
{
    PM_LCD_TOP = 0,
    PM_LCD_BOTTOM = 1,
    PM_LCD_ALL = 2
}
PMLCDTarget;

//---- back light
typedef enum
{
    PM_BACKLIGHT_OFF = 0,
    PM_BACKLIGHT_ON = 1
}
PMBackLightSwitch;

//---- battery status (VDET terminal voltage)
typedef enum
{
    PM_BATTERY_HIGH = 0,
    PM_BATTERY_LOW = 1
}
PMBattery;

//---- programable gain amp
typedef enum
{
    PM_AMP_OFF = 0,
    PM_AMP_ON = 1
}
PMAmpSwitch;

//---- gain of programable gain amp
typedef enum
{
    PM_AMPGAIN_20 = 0,
    PM_AMPGAIN_40 = 1,
    PM_AMPGAIN_80 = 2,
    PM_AMPGAIN_160 = 3,
    PM_AMPGAIN_DEFAULT = PM_AMPGAIN_40
}
PMAmpGain;

#define PM_GAINAMP_DEFAULT  PM_AMPGAIN_DEFAULT    // for compatible

//---- LCD power
typedef enum
{
    PM_LCD_POWER_OFF = 0,
    PM_LCD_POWER_ON = 1
}
PMLCDPower;

#ifdef SDK_TWL
//---- ExitFactor
typedef enum
{
    PM_EXIT_FACTOR_NONE    = 0,
    PM_EXIT_FACTOR_PWSW    = 1,
    PM_EXIT_FACTOR_BATTERY = 2,
    PM_EXIT_FACTOR_USER    = 3
}
PMExitFactor;
#endif

//---- callback type
// general purpose
typedef void (*PMGenCallback) (void *);
// sleep
#define PMSleepCallback PMGenCallback
#ifdef SDK_TWL
// exit
#define PMExitCallback PMGenCallback
// batteryLow
#define PMBatteryLowCallback PMGenCallback
#endif

//---- callback info type
// general purpose
typedef struct PMiGenCallbackInfo PMGenCallbackInfo;
struct PMiGenCallbackInfo
{
    PMGenCallback      callback;
    void*              arg;
    int                priority;
    PMGenCallbackInfo* next;
};

// sleep
#define PMSleepCallbackInfo PMGenCallbackInfo
#ifdef SDK_TWL
// exit
#define PMExitCallbackInfo PMGenCallbackInfo
// batteryLow
#define PMBatteryLowCallbackInfo PMGenCallbackInfo
#endif

//---- callback priority
#define PM_CALLBACK_PRIORITY_SYSMIN   (-65535)
#define PM_CALLBACK_PRIORITY_MIN      (-255)
#define PM_CALLBACK_PRIORITY_MAX      255
#define PM_CALLBACK_PRIORITY_SYSMAX   65535

#define PM_CALLBACK_PRIORITY_WM             1000
#define PM_CALLBACK_PRIORITY_DSP            1010
#define PM_CALLBACK_PRIORITY_CAMERA         1020
#define PM_CALLBACK_PRIORITY_NWM            PM_CALLBACK_PRIORITY_SYSMAX
#define PM_CALLBACK_PRIORITY_FS             PM_CALLBACK_PRIORITY_SYSMAX -1
#define PM_CALLBACK_PRIORITY_SNDEX          PM_CALLBACK_PRIORITY_SYSMAX -2

//================================================================================
//           INITIALIZE
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_Init

  Description:  initialize PM

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    PM_Init(void);

//================================================================================
//           SEND COMMAND TO ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SendUtilityCommandAsync / PM_SendUtilityCommand

  Description:  send utility command to ARM7

  Arguments:    number    : action number
                parameter : parameter for utility command
                retValue  : buffer to store result
                callback  : callback function
                arg       : argument of callback

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy because other device or other PM function uses SPI
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32     PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16* retValue, PMCallback callback, void *arg);
u32     PM_SendUtilityCommand(u32 number, u16 parameter, u16* retValue);

/*---------------------------------------------------------------------------*
  Name:         PM_SetBackLightAsync / PM_SetBackLight

  Description:  change backlight switch

  Arguments:    targer   : target LCD.
                           PM_LCD_TOP    : top LCD 
                           PM_LCD_BOTTOM : bottom LCD 
                           PM_LCD_ALL    : top and bottom LCD 
                sw       : switch of top LCD.
                           PM_BACKLIGHT_OFF : off
                           PM_BACKLIGHT_ON  : on
                callback : callback function
                arg      : callback argument

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_INVALID_COMMAND : bad status given
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_SetBackLightAsync(PMLCDTarget target, PMBackLightSwitch sw, PMCallback callback, void *arg);
u32     PM_SetBackLight(PMLCDTarget target, PMBackLightSwitch status);

//================================================================================
//             RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_ForceToPowerOffAsync / PM_ForceToPowerOff

  Description:  force to turn off main power

  Arguments:    callback : callback function
                arg      : callback argument

  Returns:      (PM_ForceToPowerOffAsync)
                result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI

                (PM_ForceToPowerOff)
                PM_RESULT_SUCCESS  : success to exec command

                If success, maybe you cannot do anything because power if off.
 *---------------------------------------------------------------------------*/
u32     PM_ForceToPowerOffAsync(PMCallback callback, void *arg);
u32     PM_ForceToPowerOff(void);

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpAsync / PM_SetAmp

  Description:  switch amp

  Arguments:    sw       : switch of programable gain amp
                          PM_AMP_ON   : on
                          PM_AMP_OFF  : off
                callback : callback function
                arg      : callback argument

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_SetAmpAsync(PMAmpSwitch sw, PMCallback callback, void *arg);
u32     PM_SetAmp(PMAmpSwitch sw);

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainAsync / PM_SetAmpGain

  Description:  change amp gain

  Arguments:    gain      : gain
                          PM_AMPGAIN_20   : gain=20
                          PM_AMPGAIN_40   : gain=40
                          PM_AMPGAIN_80   : gain=80
                          PM_AMPGAIN_160  : gain=160
                callback : callback function
                arg      : callback argument

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_SetAmpGainAsync(PMAmpGain gain, PMCallback callback, void *arg);
u32     PM_SetAmpGain(PMAmpGain gain);

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainLevelAsync / PM_SetAmpGainLevel

  Description:  change amp gain

  Arguments:    level    : gain. 0 - 119 (0dB - 59.5dB)
                           (119 is defined as PM_AMPGAIN_LEVEL_MAX)

                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_SetAmpGainLevelAsync(u8 level, PMCallback callback, void *arg);
u32 PM_SetAmpGainLevel(u8 level);
#endif

//================================================================================
//           GET STATUS FROM ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_GetBattery

  Description:  get battery status

  Arguments:    batteryBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY    : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_GetBattery(PMBattery *batteryBuf);

/*---------------------------------------------------------------------------*
  Name:         PM_GetBatteryLevel

  Description:  get battery level

  Arguments:    levelBuf  : buffer to store result
                     the value range is from PM_BATTERY_LEVEL_MIN to PM_BATTERY_LEVEL_MAX.

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
                PM_RESULT_ERROR    : cannot use this function (running NITRO mode)
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
u32     PM_GetBatteryLevel(PMBatteryLevel *levelBuf);
#else
static inline u32 PM_GetBatteryLevel(PMBatteryLevel *levelBuf)
{
#pragma unused(levelBuf)
    return PM_RESULT_ERROR;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetACAdapter

  Description:  check if AC power adaptor is connected

  Arguments:    isConnectedBuf  : buffer to store result
                       TRUE  ... connected
                       FALSE ... not connected

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
                PM_RESULT_ERROR    : cannot use this function (running NITRO mode)
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
u32     PM_GetACAdapter(BOOL *isConnectedBuf);
#else
static inline u32     PM_GetACAdapter(BOOL *isConnectedBuf)
{
#pragma unused(isConnectedBuf)
    return PM_RESULT_ERROR;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetBackLight

  Description:  get backlight status

  Arguments:    top    : buffer to set result of top LCD
                bottom : buffer to set result of bottom LCD

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_GetBackLight(PMBackLightSwitch *top, PMBackLightSwitch *bottom);

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmp

  Description:  get status of amp switch

  Arguments:    swBuf  : buffer to store result

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_GetAmp(PMAmpSwitch *swBuf);

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGain

  Description:  get status of amp gain

  Arguments:    gainBuf  : buffer to store result

  Returns:      result. 
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_GetAmpGain(PMAmpGain *gainBuf);

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGainLevel

  Description:  get level of amp gain

  Arguments:    levelBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetAmpGainLevel(u8 *levelBuf);
#endif

//================================================================================
//           WIRELESS LED
//================================================================================
#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PMi_SetWirelessLED

  Description:  get status of amp gain

  Arguments:    sw : PM_WIRELESS_LED_ON  ... ON
                     PM_WIRELESS_LED_OFF ... OFF

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_ERROR    : some error occurred in ARM7
 *---------------------------------------------------------------------------*/
u32 PMi_SetWirelessLED( PMWirelessLEDStatus sw );
#endif  //ifdef SDK_TWL

//================================================================================
//           SLEEP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_GoSleepMode

  Description:  go to be in sleep mode

  Arguments:    trigger    : factors to return from being on sleep
                logic      : key logic to key interrupt
                             PM_PAD_LOGIC_AND : occur interrupt at all specified button pushed
                             PM_PAD_LOGIC_OR  : occur interrupt at one of specified buttons pushed
                keyPattern : keys to wakeup 

  Returns:      None
 *---------------------------------------------------------------------------*/
void    PM_GoSleepMode(PMWakeUpTrigger trigger, PMLogic logic, u16 keyPattern);

//================================================================================
//           CALLBACK for sleep mode
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetSleepCallbackInfo

  Description:  set up sleep callback info

  Arguments:    info     : callback info
                callback : callback to set
                arg      : arg to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void PM_SetSleepCallbackInfo(PMSleepCallbackInfo *info, PMSleepCallback callback, void *arg)
{
    info->callback = callback;
    info->arg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreSleepCallback

  Description:  append callback info to pre-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPreSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreSleepCallback

  Description:  prepend callback info to pre-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPreSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostSleepCallback

  Description:  append callback info to post-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void    PM_AppendPostSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostSleepCallback

  Description:  prepend callback info to post-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPostSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreSleepCallback

  Description:  insert callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InsertPreSleepCallback(PMSleepCallbackInfo *info, int priority);

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostSleepCallback

  Description:  insert callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InsertPostSleepCallback(PMSleepCallbackInfo *info, int priority);

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreSleepCallback

  Description:  delete callback info from pre-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePreSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostSleepCallback

  Description:  delete callback info from post-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void    PM_DeletePostSleepCallback(PMSleepCallbackInfo *info);

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPreSleepCallback

  Description:  clear pre-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPreSleepCallback(void);

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPostSleepCallback

  Description:  clear post-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPostSleepCallback(void);


//================================================================================
//             LCD
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetLCDPower

  Description:  turn on/off LCD power
                when undefined SDK_NO_THREAD (= thread is valid),
                Tick system and Alarm system are needed.

                this function is sync version.

  Arguments:    sw : switch
                   PM_LCD_POWER_ON  : on
                   PM_LCD_POWER_OFF : off

  Returns:      TRUE   if success
                FALSE  if fail. maybe interval of LCD off->on is too short.
 *---------------------------------------------------------------------------*/
BOOL    PM_SetLCDPower(PMLCDPower sw);

/*---------------------------------------------------------------------------*
  Name:         PM_GetLCDPower

  Description:  get status of LCD power

  Arguments:    None

  Returns:      status.
                PM_LCD_POWER_ON  : on
                PM_LCD_POWER_OFF : off
 *---------------------------------------------------------------------------*/
PMLCDPower PM_GetLCDPower(void);


//================================================================================
//             LED
//================================================================================
u32     PMi_SendLEDPatternCommandAsync(PMLEDPattern pattern, PMCallback callback, void *arg);
u32     PMi_SendLEDPatternCommand(PMLEDPattern pattern);
/*---------------------------------------------------------------------------*
  Name:         PM_SetLEDPatternAsync / PM_SetLEDPattern

  Description:  set up LED blink pattern

  Arguments:    pattern  : LED blink pattern
                callback : callback function
                arg      : callback argument

  Returns:      result of issueing command
                PM_RESULT_SUCCESS : success
                PM_RESULT_BUSY    : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
static inline u32 PM_SetLEDPatternAsync(PMLEDPattern pattern, PMCallback callback, void *arg)
{
    return PMi_SendLEDPatternCommandAsync(pattern, callback, arg);
}
static inline u32 PM_SetLEDPattern(PMLEDPattern pattern)
{
    return PMi_SendLEDPatternCommand(pattern);
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetLEDPatternAsync / PM_GetLEDPattern

  Description:  get current LED pattern

  Arguments:    patternBuf  : area to set LED status
                callback    : callback function
                arg         : callback argument

  Returns:      result of issueing command
                PM_RESULT_SUCCESS : success
                PM_RESULT_BUSY    : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32     PM_GetLEDPatternAsync(PMLEDPattern *patternBuf, PMCallback callback, void *arg);
u32     PM_GetLEDPattern(PMLEDPattern *patternBuf);

#ifdef SDK_TWL
//================================================================================
//             SHUTDOWN / RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetExitCallbackInfo

  Description:  set up exit callback info

  Arguments:    info     : callback info
                callback : callback to set
                arg      : arg to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void PM_SetExitCallbackInfo(PMExitCallbackInfo *info, PMExitCallback callback, void *arg)
{
    info->callback = callback;
    info->arg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         PM_ReadyToExit

  Description:  tell ARM7 that ARM9 is ready to exit

  Arguments:    None

  Returns:      None
                (This function will never return.)
 *---------------------------------------------------------------------------*/
void PM_ReadyToExit(void);

/*---------------------------------------------------------------------------*
  Name:         PM_GetExitFactor

  Description:  get a factor which caused to exit

  Arguments:    None

  Returns:      factor :  PM_EXIT_FACTOR_PWSW    ... pushed power switch
                          PM_EXIT_FACTOR_BATTERY ... battery low
                          PM_EXIT_FACTOR_USER    ... called PM_ForceToResetHardware
                          PM_EXIT_FACTOR_NONE    ... not set yet
 *---------------------------------------------------------------------------*/
PMExitFactor PM_GetExitFactor(void);

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreExitCallback

  Description:  append exit callback info to pre-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPreExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostExitCallback

  Description:  append exit callback info to post-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPostExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreExitCallback

  Description:  prepend exit callback info to pre-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPreExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostExitCallback

  Description:  prepend exit callback info to post-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPostExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreExitCallback

  Description:  insert an exit callback info to pre-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InsertPreExitCallback( PMExitCallbackInfo *info, int priority );

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostExitCallback

  Description:  insert an exit callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InsertPostExitCallback( PMExitCallbackInfo *info, int priority );

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreExitCallback

  Description:  delete exit callback info from pre-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePreExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostExitCallback

  Description:  delete exit callback info from post-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePostExitCallback( PMExitCallbackInfo *info );

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPreExitCallback

  Description:  clear exit pre-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPreExitCallback(void);

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPostExitCallback

  Description:  clear exit post-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPostExitCallback(void);

/*---------------------------------------------------------------------------*
  Name:         PM_SetAutoExit

  Description:  set flag to shutdown/reset (not) automatically after callback

  Arguments:    sw : TRUE  ... shutdown/reset after callback automatically
                     FALSE ... not shutdown/reset after callback

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_SetAutoExit( BOOL sw );

/*---------------------------------------------------------------------------*
  Name:         PM_GetAutoExit

  Description:  get the current setting of AutoExit

  Arguments:    None

  Returns:      TRUE  ... shutdown/reset after callback automatically
                FALSE ... not shutdown/reset after callback
 *---------------------------------------------------------------------------*/
BOOL PM_GetAutoExit(void);

//================================================================================
//             BATTERY CALLBACK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetBatteryLowCallback

  Description:  set battery low callback

  Arguments:    callback : callback called in detecting battery low
                arg      : callback argument 

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_SetBatteryLowCallback( PMBatteryLowCallback callback, void* arg );

//================================================================================
//             RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_ForceToResetHardware

  Description:  tell ARM7 to reset hardware

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ForceToResetHardware(void);

#endif

//================================================================================
//   DUMP (for DEBUG)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_DumpSleepCallback

  Description:  dump sleep callbacks (for debug)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
void PM_DumpSleepCallback(void);
#else
#define PM_DumpSleepCallback()    ((void)0)
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_DumpExitCallback

  Description:  dump exit callbacks (for debug)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#ifndef SDK_FINALROM
void PM_DumpExitCallback(void);
#else
#define PM_DumpExitCallback()    ((void)0)
#endif
#endif

//================================================================
//         internal functions
//================================================================
//
// these PMi_ functions are internal function.
// don't use these functions
//
//---- sound power
typedef enum
{
    PM_SOUND_POWER_OFF = 0,
    PM_SOUND_POWER_ON = 1
}
PMSoundPowerSwitch;

//---- sound volume control
typedef enum
{
    PM_SOUND_VOLUME_OFF = 0,
    PM_SOUND_VOLUME_ON = 1
}
PMSoundVolumeSwitch;

//---------------- set LCD and LED
BOOL    PMi_SetLCDPower(PMLCDPower sw, PMLEDStatus led, BOOL skip, BOOL isSync);

//---------------- change sound power switch
u32     PMi_SetSoundPowerAsync(PMSoundPowerSwitch sw, PMCallback callback, void *arg);
u32     PMi_SetSoundPower(PMSoundPowerSwitch sw);

//---------------- change sound volume control switch
u32     PMi_SetSoundVolumeAsync(PMSoundVolumeSwitch sw, PMCallback callback, void *arg);
u32     PMi_SetSoundVolume(PMSoundVolumeSwitch sw);

//---------------- get status of sound power switch
u32     PMi_GetSoundPower(PMSoundPowerSwitch *swBuf);

//---------------- get status of sound volume control switch
u32     PMi_GetSoundVolume(PMSoundVolumeSwitch *swBuf);

//---------------- send data to ARM7
void    PMi_SendPxiData(u32 data);

//---------------- callback
void    PMi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err);

//---------------- send sleep command / dispose sleep end
u32     PMi_SendSleepStart(u16 trigger, u16 keyIntrData);
u32     PMi_DisposeSleepEnd(void);

//---------------- set LED status
u32     PMi_SetLEDAsync(PMLEDStatus status, PMCallback callback, void *arg);
u32     PMi_SetLED(PMLEDStatus status);

//---------------- get counter value LCD turned off
u32     PMi_GetLCDOffCount(void);

#ifdef SDK_TWL
//---------------- execute callbacks registerd to info lists
void    PMi_ExecuteAllListsOfExitCallback(void);
void    PMi_ExecutePreExitCallbackList(void);
void    PMi_ExecutePostExitCallbackList(void);
#endif

#ifndef SDK_FINALROM
//---------------- read register
u32     PMi_ReadRegisterAsync(u16 registerAddr, u16 *buffer, PMCallback callback, void *arg);
u32     PMi_ReadRegister(u16 registerAddr, u16 *buffer);

//---------------- write register
u32     PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg);
u32     PMi_WriteRegister(u16 registerAddr, u16 data);
#endif

//---------------- set sleep callback for system
void PMi_InsertPreSleepCallbackEx( PMSleepCallbackInfo *info, int priority );
void PMi_InsertPostSleepCallbackEx( PMSleepCallbackInfo *info, int priority );

#ifdef SDK_TWL
//---------------- set exit callback for system
void PMi_InsertPreExitCallbackEx( PMExitCallbackInfo *info, int priority );
void PMi_InsertPostExitCallbackEx( PMExitCallbackInfo *info, int priority );
#endif

//---------------- record disp off counter
void PMi_SetDispOffCount(void);

#ifdef SDK_TWL
//---------------- try to lock
BOOL PMi_TryLockForReset(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PM_ARM9_H_ */
#endif
