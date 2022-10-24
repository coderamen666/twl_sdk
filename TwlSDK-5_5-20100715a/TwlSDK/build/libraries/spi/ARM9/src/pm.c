/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM
  File:     pm.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-10-01#$
  $Rev: 11078 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro/spi/ARM9/pm.h>
#include <nitro/pxi.h>
#include <nitro/gx.h>
#include <nitro/spi/common/config.h>
#include <nitro/ctrdg.h>
#include <nitro/mb.h>
#ifdef SDK_TWL
#include "../../os/common/include/application_jump_private.h"
#endif

//---------------- For IS-TWL-DEBUGGER
#ifdef          SDK_LINK_ISTD
# pragma warn_extracomma off
# include       <istdbglib.h>          // Has extra comma in enum
# pragma warn_extracomma reset
#endif

//---- PM work area
typedef struct
{
	//---- Lock flag
    BOOL    lock;

	//---- Callback
    PMCallback callback;
    void   *callbackArg;

	//---- Free work area
    void   *work;
}
PMiWork;

#define PMi_LCD_WAIT_SYS_CYCLES  0x360000
#define PMi_PXI_WAIT_TICK        10

//---- Compare method
#define PMi_COMPARE_GT           0    // Compare by '>'
#define PMi_COMPARE_GE           1    // Compare by '>='

//----------------
inline u32 PMi_MakeData1(u32 bit, u32 seq, u32 command, u32 data)
{
    return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((command) << 8) | ((data) & 0xff);
}

//----------------
inline u32 PMi_MakeData2(u32 bit, u32 seq, u32 data)
{
    return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((data) & 0xffff);
}

static u32 PMi_TryToSendPxiData(u32* sendData, int num, u16* retValue, PMCallback callback, void* arg);
static void PMi_TryToSendPxiDataTillSuccess(u32* sendData, int num);

static u32 PMi_ForceToPowerOff(void);
static void PMi_CallPostExitCallbackAndReset(BOOL isExit);

//---------------- Wait unlock
static void PMi_WaitBusy(void);

//---------------- Dummy callback
void    PMi_DummyCallback(u32 result, void *arg);

//---------------- Callback list operation
static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info, int priority, int method);
static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info);
static void PMi_ClearList(PMGenCallbackInfo **listp);
static void PMi_ExecuteList(PMGenCallbackInfo *listp);

#ifdef SDK_TWL
static void PMi_FinalizeDebugger(void);
#include <twl/ltdmain_begin.h>
static void PMi_ProceedToExit(PMExitFactor factor);
static void PMi_ClearPreExitCallback(void);
static void PMi_ClearPostExitCallback(void);
#include <twl/ltdmain_end.h>
#endif

static void PMi_LCDOnAvoidReset(void);

static void PMi_WaitVBlank(void);

static PMiWork PMi_Work;

static volatile BOOL PMi_SleepEndFlag;

//---- V-Blank count LCD turned off
static u32 PMi_LCDCount;
static u32 PMi_DispOffCount;

//---- Callback list
static PMSleepCallbackInfo        *PMi_PreSleepCallbackList = NULL;
static PMSleepCallbackInfo        *PMi_PostSleepCallbackList = NULL;
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static PMExitCallbackInfo         *PMi_PreExitCallbackList = NULL;
static PMExitCallbackInfo         *PMi_PostExitCallbackList = NULL;
static PMBatteryLowCallbackInfo    PMi_BatteryLowCallbackInfo = {NULL,NULL,NULL};
#include <twl/ltdmain_end.h>
#endif

//---- Mic power config
static u32 PMi_SetAmp(PMAmpSwitch status);
static PMAmpSwitch sAmpSwitch = PM_AMP_OFF;

//---- Auto reset/shutdown flag
#ifdef SDK_TWL
static BOOL PMi_AutoExitFlag = TRUE;
#ifndef SDK_FINALROM
static BOOL PMi_ExitSequenceFlag = FALSE;
#endif
static PMExitFactor PMi_ExitFactor = PM_EXIT_FACTOR_NONE;
#endif
static u32 PMi_PreDmaCnt[4];

#define PMi_WAITBUSY_METHOD_CPUMODE   (1<<1)
#define PMi_WAITBUSY_METHOD_CPSR      (1<<2)
#define PMi_WAITBUSY_METHOD_IME       (1<<3)
static BOOL PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;

//================================================================
//      INTERNAL FUNCTIONS
//================================================================

extern void PXIi_HandlerRecvFifoNotEmpty(void);
//----------------------------------------------------------------
//  PMi_WaitBusy
//
//       Wait while locked
//
static void PMi_WaitBusy(void)
{
    volatile BOOL *p = &PMi_Work.lock;

    while (*p)
    {
        if ( (    PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPUMODE && OS_GetProcMode() == OS_PROCMODE_IRQ )
             || ( PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPSR && OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE )
             || ( PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_IME && OS_GetIrq() == OS_IME_DISABLE ) )
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}

//----------------------------------------------------------------
//  PMi_DummyCallback
//
static void PMi_DummyCallback(u32 result, void *arg)
{
	if ( arg )
	{
		*(u32 *)arg = result;
	}
}

//----------------------------------------------------------------
//   PMi_CallCallbackAndUnlock
//
static void PMi_CallCallbackAndUnlock(u32 result)
{
    PMCallback callback = PMi_Work.callback;
    void       *arg     = PMi_Work.callbackArg;

	PMi_Work.lock = FALSE;

	//---- Call callback
    if (callback)
    {
        PMi_Work.callback = NULL;
        (callback) (result, arg);
    }
}

//----------------------------------------------------------------
//  PMi_WaitVBlankCount
//
static void PMi_WaitVBlank(void)
{
    vu32    vcount = OS_GetVBlankCount();
    while (vcount == OS_GetVBlankCount())
    {
    }
}

//================================================================
//      INIT
//================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_Init

  Description:  Initializes PM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_Init(void)
{
	static u16 PMi_IsInit = FALSE;

    if (PMi_IsInit)
    {
        return;
    }
    PMi_IsInit = TRUE;

    //---- Initialize work area
    PMi_Work.lock = FALSE;
    PMi_Work.callback = NULL;

#ifdef SDK_TWL
    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_NONE;
#endif

    //---- Wait for till ARM7 PXI library start
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_PM, PXI_PROC_ARM7))
    {
		SVC_WaitByLoop(100);
    }

    //---- Set receive callback
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_PM, PMi_CommonCallback);

    //---- Init LCD count
    PMi_LCDCount = PMi_DispOffCount = OS_GetVBlankCount();
}

//================================================================
//      PXI CALLBACK
//================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_CommonCallback

  Description:  Callback to receive data from PXI.

  Arguments:    tag: Tag from PXI (unused)
                data: Data from PXI
                err: Error bit

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    u16     command;
    u16     pxiResult;
	BOOL    callCallback = TRUE;

    command = (u16)((data & SPI_PXI_RESULT_COMMAND_MASK) >> SPI_PXI_RESULT_COMMAND_SHIFT);
    pxiResult = (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

    //---- Check PXI communication error
    if (err)
    {
        switch ( command )
        {
        case SPI_PXI_COMMAND_PM_SLEEP_START:
        case SPI_PXI_COMMAND_PM_UTILITY:
            pxiResult = PM_RESULT_BUSY;
            break;

        default:
            pxiResult = PM_RESULT_ERROR;
        }

        PMi_CallCallbackAndUnlock(pxiResult);
        return;
    }

	switch( command )
	{
	    case SPI_PXI_COMMAND_PM_SLEEP_START:
            // Do nothing
            break;

            //---- Result for utility
		case SPI_PXI_COMMAND_PM_UTILITY:
            if (PMi_Work.work)
            {
                *(u16*)PMi_Work.work = (u16)pxiResult;
            }
            pxiResult = (u16)PM_RESULT_SUCCESS;
			break;

			//---- Sync with ARM7
		case SPI_PXI_COMMAND_PM_SYNC:
			pxiResult = (u16)PM_RESULT_SUCCESS;
            break;

			//---- End of sleep
		case SPI_PXI_COMMAND_PM_SLEEP_END:
			PMi_SleepEndFlag = TRUE;
			break;

#ifdef SDK_TWL
			// Send notification of power-down/reset from ARM7
		case SPI_PXI_COMMAND_PM_NOTIFY:
			switch( pxiResult )
			{
				case PM_NOTIFY_POWER_SWITCH:
					OS_TPrintf("[ARM9] Pushed power button.\n" );
					PMi_ProceedToExit(PM_EXIT_FACTOR_PWSW);
                    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
					break;

				case PM_NOTIFY_SHUTDOWN:
					OS_TPrintf("[ARM9] Shutdown\n" );
					// Do nothing
					break;

				case PM_NOTIFY_RESET_HARDWARE:
					OS_TPrintf("[ARM9] Reset Hardware\n" );
					// Do nothing
					break;
				case PM_NOTIFY_BATTERY_LOW:
					OS_TPrintf("[ARM9] Battery low\n" );
					if ( PMi_BatteryLowCallbackInfo.callback )
					{
						(PMi_BatteryLowCallbackInfo.callback)(PMi_BatteryLowCallbackInfo.arg);
					}
					break;
				case PM_NOTIFY_BATTERY_EMPTY:
					OS_TPrintf("[ARM9] Battery empty\n" );
					PMi_ProceedToExit(PM_EXIT_FACTOR_BATTERY);
                    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
					break;
				default:
					OS_TPrintf("[ARM9] unknown %x\n", pxiResult );
					break;
			}

			callCallback = FALSE;
			break;

#endif	/* SDK_TWL */
	}

	if ( callCallback )
	{
		PMi_CallCallbackAndUnlock(pxiResult);
	}
}

//================================================================================
//           SEND COMMAND TO ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_TryToSendPxiData

  Description:  Checks lock and sends command data.

  Arguments:    sendData: Command data array to send
                num: Number of data in array
                retValue: Return value (when success command)
                callback: Callback called in finishing proc
                arg: Callback argument

  Returns:      PM_BUSY: Busy (locked).
                PM_SUCCESS: Sent command data.
 *---------------------------------------------------------------------------*/
static u32 PMi_TryToSendPxiData(u32* sendData, int num, u16* retValue, PMCallback callback, void* arg)
{
    int n;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- Check lock
    if (PMi_Work.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PM_BUSY;
    }
    PMi_Work.lock = TRUE;

    //---- Set callback
    PMi_Work.work = (void*)retValue;
    PMi_Work.callback = callback;
    PMi_Work.callbackArg = arg;

    //---- Send command to ARM7
    for( n=0; n<num; n++ )
    {
        PMi_SendPxiData( sendData[n] );
    }

    (void)OS_RestoreInterrupts(enabled);
    return PM_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_TryToSendPxiDataTillSuccess

  Description:  Sends data until success.

  Arguments:    sendData: Command data array to send
                num: Number of data in array

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define PMi_UNUSED_RESULT 0xffff0000  //Value that should never be returned
void PMi_TryToSendPxiDataTillSuccess(u32* sendData, int num)
{
    volatile u32 result;
    while(1)
    {
        result = PMi_UNUSED_RESULT;
        while( PMi_TryToSendPxiData( sendData, num, NULL, PMi_DummyCallback, (void*)&result) != PM_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

        //---- Wait for finishing command
        while(result == PMi_UNUSED_RESULT)
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
        if ( result == SPI_PXI_RESULT_SUCCESS )
        {
            break;
        }

        //---- Wait
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SendSleepStart

  Description:  Sends SLEEP START command to ARM7.

  Arguments:    trigger: Factors to wake up | backlight status to recover
                keyIntrData: Key pattern and logic in waking up by key interrupt

  Returns:      Result of issuing command.
                Always return PM_RESULT_SUCCESS.
 *---------------------------------------------------------------------------*/
u32 PMi_SendSleepStart(u16 trigger, u16 keyIntrData)
{
    u32 sendData[2];

    //---- Send SYNC
    sendData[0] =  PMi_MakeData1(SPI_PXI_START_BIT | SPI_PXI_END_BIT, 0, SPI_PXI_COMMAND_PM_SYNC, 0);
    PMi_TryToSendPxiDataTillSuccess( sendData, 1 );

    //---- Turn LCD off
    while( PMi_SetLCDPower(PM_LCD_POWER_OFF, PM_LED_BLINK_LOW, FALSE, TRUE) != TRUE )
    {
        // No need to insert spinwait (because wait in PMi_SetLCDPower)
    }

    //---- Send SLEEP_START
    sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT, 0, SPI_PXI_COMMAND_PM_SLEEP_START, trigger);
    sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT,   1, keyIntrData);
    PMi_TryToSendPxiDataTillSuccess( sendData, 2 );

    return PM_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SendUtilityCommandAsync / PM_SendUtilityCommand

  Description:  Sends utility command to ARM7.

  Arguments:    number: Action number (PM_UTIL_xxx in spi/common/pm_common.h)
                parameter: Parameter for utility command
                retValue: Return value (when success command)
                callback: Callback called in finishing proc
                arg: Callback argument

  Returns:      result of issuing command
                PM_RESULT_BUSY: Busy because other PM function uses SPI.
                PM_RESULT_SUCCESS: Success.
 *---------------------------------------------------------------------------*/
u32 PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16* retValue, PMCallback callback, void *arg)
{
    u32 sendData[2];

    sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT, 0, SPI_PXI_COMMAND_PM_UTILITY, number);
    sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT, 1, parameter);

    return PMi_TryToSendPxiData( sendData, 2, retValue, callback, arg );
}

//---------------- Sync version
u32 PM_SendUtilityCommand(u32 number, u16 parameter, u16* retValue)
{
    u32     commandResult;
    u32     sendResult = PM_SendUtilityCommandAsync(number, parameter, retValue, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

//================================================================================
//           ACTION ABOUT PMIC
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetLEDAsync / PMi_SetLED

  Description:  Changes LED status.

  Arguments:    status: PM_LED_ON: On
                        PM_LED_BLINK_HIGH: Blinking in high speed
                        PM_LED_BLINK_LOW: Blinking in low speed
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PMi_SetLEDAsync(PMLEDStatus status, PMCallback callback, void *arg)
{
    u32     command;

    switch (status)
    {
    case PM_LED_ON:
        command = PM_UTIL_LED_ON;
        break;
    case PM_LED_BLINK_HIGH:
        command = PM_UTIL_LED_BLINK_HIGH_SPEED;
        break;
    case PM_LED_BLINK_LOW:
        command = PM_UTIL_LED_BLINK_LOW_SPEED;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- Sync version
u32 PMi_SetLED(PMLEDStatus status)
{
    u32     commandResult;
    u32     sendResult = PMi_SetLEDAsync(status, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetBackLightAsync / PM_SetBackLight

  Description:  Changes backlight switch.

  Arguments:    targer: Target LCD
                           PM_LCD_TOP: Top LCD
                           PM_LCD_BOTTOM: Bottom LCD
                           PM_LCD_ALL: Top and bottom LCD
                sw: Switch of top LCD
                           PM_BACKLIGHT_OFF: Off
                           PM_BACKLIGHT_ON: On
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_INVALID_COMMAND: Bad status given.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_SetBackLightAsync(PMLCDTarget target, PMBackLightSwitch sw, PMCallback callback, void *arg)
{
    u32     command = 0;

    if (target == PM_LCD_TOP)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD2_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD2_BACKLIGHT_OFF;
        }
    }
    else if (target == PM_LCD_BOTTOM)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD1_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD1_BACKLIGHT_OFF;
        }
    }
    else if (target == PM_LCD_ALL)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD12_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD12_BACKLIGHT_OFF;
        }
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- Sync version
u32 PM_SetBackLight(PMLCDTarget target, PMBackLightSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PM_SetBackLightAsync(target, sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SetSoundPowerAsync / PMi_SetSoundPower

  Description:  Changes sound power switch.

  Arguments:    sw: Switch of sound power
                          PM_SOUND_POWER_OFF: Off
                          PM_SOUND_POWER_ON: On
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_INVALID_COMMAND: Bad status given.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PMi_SetSoundPowerAsync(PMSoundPowerSwitch sw, PMCallback callback, void *arg)
{
    u32     command;

    switch (sw)
    {
    case PM_SOUND_POWER_ON:
        command = PM_UTIL_SOUND_POWER_ON;
        break;
    case PM_SOUND_POWER_OFF:
        command = PM_UTIL_SOUND_POWER_OFF;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- Sync version
u32 PMi_SetSoundPower(PMSoundPowerSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PMi_SetSoundPowerAsync(sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SetSoundVolumeAsync / PMi_SetSoundVolume

  Description:  Changes sound volume control switch.

  Arguments:    sw: Switch of sound volume control
                          PM_SOUND_VOLUME_ON: On
                          PM_SOUND_VOLUME_OFF: Off
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_INVALID_COMMAND: Bad status given.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PMi_SetSoundVolumeAsync(PMSoundVolumeSwitch sw, PMCallback callback, void *arg)
{
    u32     command;

    switch (sw)
    {
    case PM_SOUND_VOLUME_ON:
        command = PM_UTIL_SOUND_VOL_CTRL_ON;
        break;
    case PM_SOUND_VOLUME_OFF:
        command = PM_UTIL_SOUND_VOL_CTRL_OFF;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- Sync version
u32 PMi_SetSoundVolume(PMSoundVolumeSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PMi_SetSoundVolumeAsync(sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PM_ForceToPowerOffAsync / PMi_ForceToPowerOff / PM_ForceToPowerOff

  Description:  Forces to turn off main power.

  Arguments:    callback: Callback function
                arg: Callback argument

  Returns:      (PM_ForceToPowerOffAsync / PMi_ForceToPowerOff)
                Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.

                (PM_ForceToPowerOff)
                PM_RESULT_SUCCESS: Success to exec command

                If success, you may not be able to do anything because power is off.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_ForceToPowerOffAsync(PMCallback callback, void *arg)
{
#ifdef SDK_TWL
	//---- Set exit factors
	PMi_ExitFactor = PM_EXIT_FACTOR_USER;
#endif

	//---- Force to set LCD on avoid PMIC bug
	PMi_LCDOnAvoidReset();

#ifdef SDK_TWL
	if ( OS_IsRunOnTwl() )
	{
		//---- Call exit callbacks
		PMi_ExecuteList(PMi_PostExitCallbackList);
	}
#endif

    return PM_SendUtilityCommandAsync(PM_UTIL_FORCE_POWER_OFF, 0, NULL, callback, arg);
}

//---------------- Sync version
static u32 PMi_ForceToPowerOff(void)
{
    u32     commandResult;
    u32     sendResult = PM_ForceToPowerOffAsync(PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
        PMi_WaitBusy();
        PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;
        return commandResult;
    }
    return sendResult;
}
//---------------- Wait version
u32 PM_ForceToPowerOff(void)
{
    while( PMi_ForceToPowerOff() != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    (void)OS_DisableInterrupts();

    //---- Stop all DMA
    MI_StopAllDma();
#ifdef SDK_TWL
	if ( OS_IsRunOnTwl() )
	{
		MI_StopAllNDma();
	}
#endif

    //---- Loop intentionally
    while (1)
    {
        OS_Halt();
    }

    //---- Just avoid warning
    return PM_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpAsync / PM_SetAmp

  Description:  Switches amp.

  Arguments:    sw: Switch of programmable gain amp
                          PM_AMP_ON: On
                          PM_AMP_OFF: Off
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_SetAmpAsync(PMAmpSwitch status, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMP, (u16)status, NULL, callback, arg );
}

//---------------- Sync version
u32 PM_SetAmp(PMAmpSwitch status)
{
    //---- Remember mic power flag
    sAmpSwitch = status;
    return PMi_SetAmp(status);
}

//---------------- Sync version
static u32 PMi_SetAmp(PMAmpSwitch status)
{
    if (PM_GetLCDPower())
    {
		//---- Change amp status in case of LCD-ON only
		return PM_SendUtilityCommand(PM_UTIL_SET_AMP, (u16)status, NULL );
	}
	else
	{
		return PM_RESULT_SUCCESS;
	}
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainAsync / PM_SetAmpGain

  Description:  Changes amp gain.

  Arguments:    gain: Gain
                          PM_AMPGAIN_20: Gain=20
                          PM_AMPGAIN_40: Gain=40
                          PM_AMPGAIN_80: Gain=80
                          PM_AMPGAIN_160: Gain=160
                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_SetAmpGainAsync(PMAmpGain gain, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL, callback, arg );
}

//---------------- Sync version
u32 PM_SetAmpGain(PMAmpGain gain)
{
	return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL );
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainLevelAsync / PM_SetAmpGainLevel

  Description:  Changes amp gain.

  Arguments:    level: Gain. Range is 0-119 (0 dB - 59.5 dB).
                           (119 is defined as PM_AMPGAIN_LEVEL_MAX.)

                callback: Callback function
                arg: Callback argument

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_SetAmpGainLevelAsync(u8 level, PMCallback callback, void *arg)
{
	SDK_ASSERT( level <= PM_AMPGAIN_LEVEL_MAX );
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL, callback, arg );
}

//---------------- Sync version
u32 PM_SetAmpGainLevel(u8 level)
{
	SDK_ASSERT( level <= PM_AMPGAIN_LEVEL_MAX );
	return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL );
}
#endif

//================================================================================
//           GET STATUS FROM ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_GetBattery

  Description:  Gets battery status.

  Arguments:    batteryBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
u32 PM_GetBattery(PMBattery *batteryBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BATTERY, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (batteryBuf)
		{
			*batteryBuf = status? PM_BATTERY_LOW: PM_BATTERY_HIGH;
		}
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetBatteryLevel

  Description:  Gets battery level.

  Arguments:    levelBuf: Buffer to store result
                     The range of values is from PM_BATTERY_LEVEL_MIN to PM_BATTERY_LEVEL_MAX.

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
                PM_RESULT_ERROR: Cannot use this function (running in NITRO mode).
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static u32 PMi_GetBatteryLevelCore(PMBatteryLevel *levelBuf)
{
    u16 status;
    u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BATTERY_LEVEL, &status);

    if ( result == PM_RESULT_SUCCESS )
    {
        if (levelBuf)
        {
            *levelBuf = (PMBatteryLevel)status;
        }
    }
    return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetBatteryLevel(PMBatteryLevel *levelBuf)
{
	if ( OS_IsRunOnTwl() )
	{
		return PMi_GetBatteryLevelCore(levelBuf);
	}
	else
	{
		return PM_RESULT_ERROR;
	}
}

#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetACAdapter

  Description:  Checks whether AC power adaptor is connected.

  Arguments:    isConnectedBuf: Buffer to store result
                       TRUE: Connected.
                       FALSE: Not connected.

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
                PM_RESULT_ERROR: Cannot use this function (running in NITRO mode).
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL

#include <twl/ltdmain_begin.h>
static u32 PMi_GetACAdapterCore(BOOL *isConnectedBuf)
{
    u16 status;
    u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AC_ADAPTER, &status);

    if ( result == PM_RESULT_SUCCESS )
    {
        if (isConnectedBuf)
        {
            *isConnectedBuf = status? TRUE: FALSE;
        }
    }
    return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetACAdapter(BOOL *isConnectedBuf)
{
	if ( OS_IsRunOnTwl() )
	{
		return PMi_GetACAdapterCore(isConnectedBuf);
	}
	else
	{
		return PM_RESULT_ERROR;
	}
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetBackLight

  Description:  Gets backlight status.

  Arguments:    top: Buffer to set result of top LCD
                bottom: Buffer to set result of bottom LCD

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
u32 PM_GetBackLight(PMBackLightSwitch *top, PMBackLightSwitch *bottom)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BACKLIGHT, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (top)
		{
            *top = (status & PMIC_CTL_BKLT2) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;
		}
		if (bottom)
		{
            *bottom = (status & PMIC_CTL_BKLT1) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;		
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetSoundPower

  Description:  Gets status of sound power switch.

  Arguments:    swBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_ERROR: Some error occurred in ARM7.
 *---------------------------------------------------------------------------*/
u32 PMi_GetSoundPower(PMSoundPowerSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_SOUND_POWER, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
            *swBuf = status? PM_SOUND_POWER_ON : PM_SOUND_POWER_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetSoundVolume

  Description:  Gets status of sound volume control switch.

  Arguments:    swBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_ERROR: Some error occurred in ARM7.
 *---------------------------------------------------------------------------*/
u32 PMi_GetSoundVolume(PMSoundVolumeSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_SOUND_VOLUME, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
			*swBuf = status? PM_SOUND_VOLUME_ON : PM_SOUND_VOLUME_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmp

  Description:  Gets status of amp switch.

  Arguments:    swBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
u32 PM_GetAmp(PMAmpSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMP, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
			*swBuf = status? PM_AMP_ON: PM_AMP_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGain

  Description:  Gets status of amp gain.

  Arguments:    gainBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
u32 PM_GetAmpGain(PMAmpGain *gainBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMPGAIN, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (gainBuf)
		{
			*gainBuf = (PMAmpGain)status;
		}
	}
    return result;
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGainLevel

  Description:  Gets level of amp gain.

  Arguments:    levelBuf: Buffer to store result

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI.
 *---------------------------------------------------------------------------*/
u32 PM_GetAmpGainLevel(u8 *levelBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMPGAIN_LEVEL, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (levelBuf)
		{
			*levelBuf = (u8)status;
		}
	}
	return result;
}
#endif

//================================================================================
//           WIRELESS LED
//================================================================================
#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PMi_SetWirelessLED

  Description:  Gets status of amp gain.

  Arguments:    sw: PM_WIRELESS_LED_ON: ON
                     PM_WIRELESS_LED_OFF: OFF

  Returns:      Result.
                PM_RESULT_SUCCESS: Success to exec command.
                PM_RESULT_ERROR: Some error occurred in ARM7.
 *---------------------------------------------------------------------------*/
u32 PMi_SetWirelessLED( PMWirelessLEDStatus sw )
{
	return PM_SendUtilityCommand( PM_UTIL_WIRELESS_LED, (u16)sw, NULL );
}
#endif  //ifdef SDK_TWL

//================================================================================
//           SEND DATA TO ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SendPxiData

  Description:  Sends data via PXI.

  Arguments:    data: Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_SendPxiData(u32 data)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_PM, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        // Do nothing
    }
}

//================================================================================
//           SLEEP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_PreSleepForDma

  Description:  Gets DMA ready for sleep mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PreSleepForNdma(u32 i)
{
    vu32 *ndmaCntp = (vu32*)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
    if ( *ndmaCntp & MI_NDMA_IMM_MODE_ON )
    {
        // Normally when starting up a new DMA, the continuous mode setting is invalid
        MI_WaitNDma(i);
        return;
    }
    switch ( *ndmaCntp & MI_NDMA_TIMING_MASK )
    {
        case MI_NDMA_TIMING_CARD:
            // Card DMA is normally in continuous mode
            // DMA will not restart if there is a wait for the memory controller to stop
            while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
            {
            }
            break;
        case MI_NDMA_TIMING_DISP_MMEM:
        case MI_NDMA_TIMING_DISP:
            // The main memory display DMA and the display synch DMA stop after completion of one screen of DMA, even in continuous mode
            //  
            MI_WaitNDma(i);
        case MI_NDMA_TIMING_CAMERA:
            // Once the camera enters standby mode, the camera DMA does not complete even if execution waits, so it is forced to stop
            //  
            //   CAMERA_DmaPipeInfinity is continuous mode
            //   CAMERA_DmaRecv* are normal mode
            MI_StopNDma(i);
            break;
        default:
            // If not in continuous mode, wait for completion of DMA
            if ( ! (*ndmaCntp & MI_NDMA_CONTINUOUS_ON) )
            {
                MI_WaitNDma(i);
            }
            // If in continuous mode, force DMA to stop
            // MI_NDMA_TIMING_TIMER0 through MI_NDMA_TIMING_TIMER3 (these exist because if it is forced to resume, it will start from the beginning and not do what the user intended)
            // MI_NDMA_TIMING_V_BLANK (MI_DMA_CONTINUOUS_ON is not yet used with publicly disclosed functions)
            // MI_NDMA_TIMING_H_BLANK (MI_DMA_CONTINUOUS_ON is used with publicly disclosed functions, but if neglected, a buffer overrun will occur)
            // MI_NDMA_TIMING_GXFIFO (MI_DMA_CONTINUOUS_ON is not yet used with publicly disclosed functions)
            else
            {
                MI_StopNDma(i);
                SDK_WARNING( FALSE, "[ARM9] Force to stop NDMA%d before sleep.", i );
            }
            break;
    }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PreSleepForDma(void)
{
    u32 i;

    for (i=0; i<=MI_DMA_MAX_NUM; i++)
    {
        // Old DMA
        {
            vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);

            PMi_PreDmaCnt[i] = *dmaCntp;

            switch ( *dmaCntp & MI_DMA_TIMING_MASK )
            {
                case MI_DMA_TIMING_CARD:
                    // Card DMA is normally in continuous mode
                    // DMA will not restart if there is a wait for the memory controller to stop
                    while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
                    {
                    }
                    break;
                case MI_DMA_TIMING_DISP_MMEM:
                case MI_DMA_TIMING_DISP:
                    // The main memory display DMA and the display synch DMA stop after completion of one screen of DMA, even in continuous mode
                    //  
                    MI_WaitDma(i);
                    break;
                default:
                    // If not in continuous mode, wait for completion of DMA
                    if ( ! (*dmaCntp & MI_DMA_CONTINUOUS_ON) )
                    {
                        MI_WaitDma(i);
                    }
                    // If in continuous mode, force DMA to stop
                    // MI_DMA_TIMING_IMM (when used with MI_DMA_CONTINUOUS_ON, the CPU cannot operate and runs out of control)
                    // MI_DMA_TIMING_V_BLANK (MI_DMA_CONTINUOUS_ON is not yet used with publicly disclosed functions)
                    // MI_DMA_TIMING_H_BLANK (MI_DMA_CONTINUOUS_ON is used with publicly disclosed functions, but if neglected, a buffer overrun will occur)
                    // MI_DMA_TIMING_CARTRIDGE (Even if using MI_DMA_CONTINUOUS_ON, resetting is necessary because DMA requests during sleep are cancelled)
                    // MI_DMA_TIMING_GXFIFO (MI_DMA_CONTINUOUS_ON is not yet used with publicly disclosed functions)
                    else
                    {
                        MI_StopDma(i);
                        SDK_WARNING( FALSE, "[ARM9] Force to stop DMA%d before sleep.", i );
                    }
                    break;
            }
        }
#ifdef SDK_TWL
        // New DMA
        if (OS_IsRunOnTwl())
        {
            PMi_PreSleepForNdma(i);
        }
#endif // SDK_TWL
    }
}
/*---------------------------------------------------------------------------*
  Name:         PMi_PostSleepForDma

  Description:  Resumes DMA from sleep mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PostSleepForNdma(u32 i)
{
    vu32 *ndmaCntp = (vu32*)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
    if ( *ndmaCntp & MI_NDMA_IMM_MODE_ON )
    {
        // Normally when starting up a new DMA, the continuous mode setting is invalid
        return;
    }
    switch ( *ndmaCntp & MI_NDMA_TIMING_MASK )
    {
        case MI_NDMA_TIMING_V_BLANK:
        case MI_NDMA_TIMING_H_BLANK:
            // If in continuous mode, restart DMA (for testing)
            if ( *ndmaCntp & MI_NDMA_CONTINUOUS_ON )
            {
                *ndmaCntp |= MI_NDMA_ENABLE;
            }
            break;
    }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PostSleepForDma(void)
{
    u32 i;

    for (i=0; i<=MI_DMA_MAX_NUM; i++)
    {
        // Old DMA
        {
            vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);
            u32 preCnt = PMi_PreDmaCnt[i];

            switch ( preCnt & MI_DMA_TIMING_MASK )
            {
                case MI_DMA_TIMING_V_BLANK:
                case MI_DMA_TIMING_H_BLANK:
                case MI_DMA_TIMING_CARTRIDGE:
                    // If in continuous mode, restart DMA (for testing)
                    if ( preCnt & MI_DMA_CONTINUOUS_ON )
                    {
                        *dmaCntp = preCnt;
                    }
                    break;
            }
        }
#ifdef SDK_TWL
        // New DMA
        if (OS_IsRunOnTwl())
        {
            PMi_PostSleepForNdma(i);
        }
#endif // SDK_TWL
    }
}

/*---------------------------------------------------------------------------*
  Name:         PM_GoSleepMode

  Description:  Goes to be in sleep mode.

  Arguments:    trigger: Factors to return from being on sleep
                logic: Key logic to key interrupt
                             PM_PAD_LOGIC_AND: Occur interrupt at all specified button pushed
                             PM_PAD_LOGIC_OR: Occur interrupt at one of specified buttons pushed
                keyPattern: Keys to wakeup

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_GoSleepMode(PMWakeUpTrigger trigger, PMLogic logic, u16 keyPattern)
{
    BOOL    prepIrq;                   // IME
    OSIntrMode prepIntrMode;           // CPSR-IRQ
    OSIrqMask prepIntrMask;            // IE
    BOOL    powerOffFlag = FALSE;

    PMBackLightSwitch preTop;
    PMBackLightSwitch preBottom;

    u32     preGX;
    u32     preGXS;
    PMLCDPower preLCDPower;

    //---- Call pre-callbacks
    PMi_ExecuteList(PMi_PreSleepCallbackList);

    //---- Disable all interrupt
    prepIrq = OS_DisableIrq();
    prepIntrMode = OS_DisableInterrupts();
    prepIntrMask = OS_DisableIrqMask( OS_IE_MASK_ALL );

    //---- Interrupt setting
	// Enable PXI from ARM7 and TIMER0 (if needed)
    (void)OS_SetIrqMask( OS_IE_FIFO_RECV | (OS_IsTickAvailable()? OS_IE_TIMER0: 0 ) );
    (void)OS_RestoreInterrupts(prepIntrMode);
    (void)OS_EnableIrq();

    //---- Check card trigger
    if (trigger & PM_TRIGGER_CARD)
    {
		OSBootType type = OS_GetBootType();
		//---- If multiboot child or NAND application, ignore card check flag
		if ( type == OS_BOOTTYPE_DOWNLOAD_MB || type == OS_BOOTTYPE_NAND )
		{
            trigger &= ~PM_TRIGGER_CARD;
		}
    }

    //---- Check cartridge trigger
    if (trigger & PM_TRIGGER_CARTRIDGE)
    {
        //---- If running on TWL or cartridge does not exist, ignore cartridge check flag
        if ( OS_IsRunOnTwl() || !CTRDG_IsExisting() )
        {
            trigger &= ~PM_TRIGGER_CARTRIDGE;
        }
    }

    //---- Remember gx state
    preGX = reg_GX_DISPCNT;
    preGXS = reg_GXS_DB_DISPCNT;
    preLCDPower = PM_GetLCDPower();

    //---- Set backlight off
    while( PM_GetBackLight(&preTop, &preBottom) != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
    while( PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    //---- Wait a few frames after backlight off for avoiding appearance of afterimage
    PMi_WaitVBlank();

    reg_GX_DISPCNT = reg_GX_DISPCNT & ~REG_GX_DISPCNT_MODE_MASK;    // Main screen off
    GXS_DispOff();

    PMi_WaitVBlank();
    PMi_WaitVBlank(); // Twice

    PMi_PreSleepForDma();

    //---- Send SLEEP_START command to ARM7
	PMi_SleepEndFlag = FALSE;
    {
        u16     param = (u16)(trigger
							  | (preTop? PM_BACKLIGHT_RECOVER_TOP_ON: PM_BACKLIGHT_RECOVER_TOP_OFF)
							  | (preBottom? PM_BACKLIGHT_RECOVER_BOTTOM_ON: PM_BACKLIGHT_RECOVER_BOTTOM_OFF));

        (void)OS_SetIrqMask( OS_IE_FIFO_RECV );
        (void)PMi_SendSleepStart(param, (u16)(logic | keyPattern));
    }

    //==== Halt ================
	while( ! PMi_SleepEndFlag )
	{
		OS_Halt();
	}
    //==========================

    (void)OS_SetIrqMask( OS_IE_FIFO_RECV | (OS_IsTickAvailable()? OS_IE_TIMER0: 0 ) );

    //---- Check card remove
    if ((trigger & PM_TRIGGER_CARD) && (OS_GetRequestIrqMask() & OS_IE_CARD_IREQ))
    {
        powerOffFlag = TRUE;
    }

    //---- Turn LCD on and restore gx state
    if (!powerOffFlag)
    {
        if (preLCDPower == PM_LCD_POWER_ON)
        {
            while( PMi_SetLCDPower(PM_LCD_POWER_ON, PM_LED_ON, TRUE, TRUE) != TRUE )
            {
                // No need to insert spinwait (because wait in PMi_SetLCDPower)
            }
        }
        else
        {
            while( PMi_SetLED(PM_LED_ON) != PM_RESULT_SUCCESS )
            {
                OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
            }
        }

        reg_GX_DISPCNT = preGX;
        reg_GXS_DB_DISPCNT = preGXS;
    }
    
	//---- Wait while specified period for LCD (0x360000sysClock == about 106ms)
    OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

    //---- Restore all interrupts
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask(prepIntrMask);
    (void)OS_RestoreInterrupts(prepIntrMode);
    (void)OS_RestoreIrq(prepIrq);

    //---- Power off if needed
    if (powerOffFlag)
    {
        (void)PM_ForceToPowerOff();
    }

    //---- Call post-callbacks
    PMi_ExecuteList(PMi_PostSleepCallbackList);
}

//================================================================================
//             LCD
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetLCDPower

  Description:  Turns LCD power on/off.

  Arguments:    sw: Switch
                   PM_LCD_POWER_ON: On
                   PM_LCD_POWER_ON: Off
                led: LED status
                   PM_LED_NONE: No touch
                   PM_LED_ON
                   PM_LED_BLINK_HIGH
                   PM_LED_BLINK_LOW
                skip: Whether skip wait
                   TRUE: Skip
                   FALSE: Wait
                isSync: Whether this function is sync version
                   TRUE: Sync
                   FALSE: Async

  Returns:      TRUE if successful.
                FALSE if failed.
 *---------------------------------------------------------------------------*/
#define PMi_WAIT_FRAME_AFTER_LCDOFF  7
#define PMi_WAIT_FRAME_AFTER_GXDISP  2

BOOL PMi_SetLCDPower(PMLCDPower sw, PMLEDStatus led, BOOL skip, BOOL isSync)
{
    switch (sw)
    {
    case PM_LCD_POWER_ON:
        // Compare with counter in which LCD power was turned off.
        //     The reason for waiting 100 ms: The interval switching LCD power
        //     from OFF to ON must be more than 100 ms. If shorter than that,
        //     PMIC will be shut down and never recover itself.
        if (!skip && OS_GetVBlankCount() - PMi_LCDCount <= PMi_WAIT_FRAME_AFTER_LCDOFF)
        {
            return FALSE;
        }

        //---- LED
        if (led != PM_LED_NONE)
        {
            if (isSync)
            {
                while( PMi_SetLED(led) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
            else
            {
                while( PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
        }

        (void)GXi_PowerLCD(TRUE);

		//---- Recover mic power flag
		while( PMi_SetAmp(sAmpSwitch) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
        break;

    case PM_LCD_POWER_OFF:
		//---- Force to turn mic power off
        while( PMi_SetAmp(PM_AMP_OFF) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

        //---- Need 2 V-Blanks after GX_DispOff()
        if ( OS_GetVBlankCount() - PMi_DispOffCount <= PMi_WAIT_FRAME_AFTER_GXDISP )
        {
            PMi_WaitVBlank();
            PMi_WaitVBlank(); // Twice
        }

        //---- LCD power off
        (void)GXi_PowerLCD(FALSE);

        //---- Remember LCD off timing
        PMi_LCDCount = OS_GetVBlankCount();

        //---- LED
        if (led != PM_LED_NONE)
        {
            if (isSync)
            {
                while( PMi_SetLED(led) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
            else
            {
                while( PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
        }
        break;

    default:
        // Do nothing
        break;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetLCDPower

  Description:  Turns LCD power on/off.
                When undefined SDK_NO_THREAD (= thread is valid),
                Tick system and Alarm system are needed.

                This function is the synchronous version.

  Arguments:    sw: Switch
                   PM_LCD_POWER_ON: On
                   PM_LCD_POWER_OFF: Off

  Returns:      TRUE if successful.
                FALSE if failed. Perhaps interval of LCD off->on is too short.
 *---------------------------------------------------------------------------*/
BOOL PM_SetLCDPower(PMLCDPower sw)
{
    if (sw != PM_LCD_POWER_ON)
    {
        sw = PM_LCD_POWER_OFF;
        if (GX_IsDispOn()) // To turn the LCD OFF, first make very sure to set the GX_DispOff status
        {
            GX_DispOff();
        }
    }
    return PMi_SetLCDPower(sw, PM_LED_NONE /* No touch */ , FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetLCDPower

  Description:  Gets status of LCD power.

  Arguments:    None.

  Returns:      Status.
                PM_LCD_POWER_ON: On
                PM_LCD_POWER_OFF: Off
 *---------------------------------------------------------------------------*/
PMLCDPower PM_GetLCDPower(void)
{
    return (reg_GX_POWCNT & REG_GX_POWCNT_LCD_MASK) ? PM_LCD_POWER_ON : PM_LCD_POWER_OFF;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetLCDOffCount

  Description:  Gets counter value LCD turned off.

  Arguments:    None.

  Returns:      Counter value.
 *---------------------------------------------------------------------------*/
u32 PMi_GetLCDOffCount(void)
{
    return PMi_LCDCount;
}


//================================================================================
//             LED
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SendLEDPatternCommandAsync

  Description:  Sets up LED blink pattern.

  Arguments:    pattern: LED blink pattern
                callback: Callback function
                arg: Callback argument

  Returns:      Result of issuing command.
                PM_RESULT_BUSY: Busy.
                PM_RESULT_SUCCESS: Success.
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PMi_SendLEDPatternCommandAsync(PMLEDPattern pattern, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_BLINK, pattern, NULL, callback, arg);	
}

//---------------- Sync version
u32 PMi_SendLEDPatternCommand(PMLEDPattern pattern)
{
	return PM_SendUtilityCommand(PM_UTIL_SET_BLINK, pattern, NULL);	
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetLEDPattern

  Description:  Gets current LED blink pattern.

  Arguments:    pattern: LED blink pattern
                callback: Callback function
                arg: Callback argument

  Returns:      Result of issuing command.
                PM_RESULT_SUCCESS: Success
                PM_RESULT_BUSY: Busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- Async version
u32 PM_GetLEDPatternAsync(PMLEDPattern *patternBuf, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK, (u16*)&patternBuf, callback, arg);
}

//---------------- Sync version
u32 PM_GetLEDPattern(PMLEDPattern *patternBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (patternBuf)
		{
			*patternBuf = (PMLEDPattern)status;
		}
	}
    return result;
}

//================================================================================
//             CALLBACK LIST OPERATION (general)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_InsertList

  Description:  Inserts callback to callback info list as specified priority.
                Subroutine of PM_Add*CallbackInfo().

  Arguments:    listp: Callback info list pointer
                info: Callback info to delete
                priority: Priority
                method: Comparison operator

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info, int priority, int method)
{
	OSIntrMode intr;
	PMGenCallbackInfo *p;
	PMGenCallbackInfo *pre;

	if (!listp)
	{
		return;
	}

	info->priority = priority;

	intr = OS_DisableInterrupts();
	p = *listp;
	pre = NULL;

	while (p)
	{
		//---- Found the position to insert
		if (method == PMi_COMPARE_GT && p->priority > priority )
		{
			break;
		}
		if (method == PMi_COMPARE_GE && p->priority >= priority )
		{
			break;
		}

		pre = p;
		p = p->next;
	}

	if ( p )
	{
		info->next = p;
	}
	else
	{
		info->next = NULL;
	}


	if ( pre )
	{
		pre->next = info;
	}
	else
	{
		//---- Add to top
		*listp = info;
	}

    (void)OS_RestoreInterrupts(intr);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_DeleteList

  Description:  Deletes callback info from callback info list.
                Subroutine of PM_Delete*CallbackInfo().

  Arguments:    listp: Callback info list pointer
                info: Callback info to delete

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info)
{
    OSIntrMode  intr;
    PMGenCallbackInfo *p = *listp;
    PMGenCallbackInfo *pre;

    if (!listp)
    {
        return;
    }

    intr = OS_DisableInterrupts();
    pre = p = *listp;
    while (p)
    {
        //---- One to delete?
        if (p == info)
        {
            if (p == pre)
            {
                *listp = p->next;
            }
            else
            {
                pre->next = p->next;
            }
            break;
        }

        pre = p;
        p = p->next;
    }
    (void)OS_RestoreInterrupts(intr);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearList

  Description:  Clears callback list.

  Arguments:    listp: Callback info list pointer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_ClearList(PMGenCallbackInfo **listp)
{
	listp = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecuteList

  Description:  Executes each callback registered to info list.

  Arguments:    listp: Callback info list pointer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_ExecuteList(PMGenCallbackInfo *listp)
{
    while (listp)
    {
        (listp->callback) (listp->arg);

        listp = listp->next;
    }
}

//================================================================================
//             SLEEP CALLBACK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreSleepCallback

  Description:  Appends callback info to pre-callback info list.

  Arguments:    info: Callback info to append

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_AppendPreSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreSleepCallback

  Description:  Prepends callback info to pre-callback info list.

  Arguments:    info: Callback info to prepend

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_PrependPreSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostSleepCallback

  Description:  Appends callback info to post-callback info list.

  Arguments:    info: Callback info to append

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_AppendPostSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostSleepCallback

  Description:  Prepends callback info to post-callback info list.

  Arguments:    info: Callback info to prepend

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_PrependPostSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreSleepCallback

  Description:  Inserts callback info to post-callback info list.

  Arguments:    info: Callback info to add
                priority: Priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_InsertPreSleepCallbackEx(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PreSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreSleepCallback(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPreSleepCallbackEx(info, priority);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostSleepCallback

  Description:  Inserts callback info to post-callback info list.

  Arguments:    info: Callback info to add
                priority: Priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_InsertPostSleepCallbackEx(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PostSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostSleepCallback(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPostSleepCallbackEx(info, priority);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreSleepCallback

  Description:  Deletes callback info from pre-callback info list.

  Arguments:    info: Callback info to delete

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DeletePreSleepCallback(PMSleepCallbackInfo *info)
{
    PMi_DeleteList(&PMi_PreSleepCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostSleepCallback

  Description:  Deletes callback info from post-callback info list.

  Arguments:    info: Callback info to delete

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DeletePostSleepCallback(PMSleepCallbackInfo *info)
{
    PMi_DeleteList(&PMi_PostSleepCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPreSleepCallback

  Description:  Clears pre-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_ClearPreSleepCallback(void)
{
    PMi_ClearList(&PMi_PreSleepCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPostSleepCallback

  Description:  Clears post-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_ClearPostSleepCallback(void)
{
    PMi_ClearList(&PMi_PostSleepCallbackList);
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
//================================================================================
//             SHUTDOWN / RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_ProceedToExit

  Description:  Runs before shutdown or reset of hardware.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_ProceedToExit(PMExitFactor factor)
{
    //---- Check if reset already
    if ( ! PMi_TryLockForReset() )
    {
        return;
    }

	//---- Set exit factors
	PMi_ExitFactor = factor;

#ifndef SDK_FINALROM
	//---- Declare to enter exit sequence
	PMi_ExitSequenceFlag = TRUE;
#endif

	//---- Call exit callbacks
    PMi_ExecuteList(PMi_PreExitCallbackList);

	if ( PMi_AutoExitFlag )
	{
		//---- Tell ARM7 that ARM9 is ready to shut down
		PM_ReadyToExit();
		// This point will not be reached
	}
}

/*---------------------------------------------------------------------------*
  Name:         PM_ReadyToExit

  Description:  Tells ARM7 that ARM9 is ready to exit.

  Arguments:    None.

  Returns:      None.
                (This function will never return.)
 *---------------------------------------------------------------------------*/
void PM_ReadyToExit(void)
{
#ifndef SDK_FINALROM
	SDK_ASSERT( PMi_ExitSequenceFlag == TRUE );
#endif

    PMi_CallPostExitCallbackAndReset(TRUE);
    //This point will never be reached
}

/*---------------------------------------------------------------------------*
  Name:         PMi_FinalizeDebugger

  Description:  Finalizes the debugger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_FinalizeDebugger(void)
{
    //---- Synchronize with ARM7
    OSi_SetSyncValue( OSi_SYNCVAL_NOT_READY );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_SENDER, (void*)HW_INIT_LOCK_BUF );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_RECVER, (void*)HW_INIT_LOCK_BUF );

    (void)OS_DisableInterrupts();

#ifndef SDK_FINALROM
    //---- Finalize debugger for TS board
    if ( OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER )
    {
        _ISTDbgLib_OnBeforeResetHard();
    }
#endif

	//---- Synchronize with ARM7
	OSi_SetSyncValue( OSi_SYNCVAL_READY );
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetExitFactor

  Description:  Gets factors that caused exit.

  Arguments:    None.

  Returns:      factors:  PM_EXIT_FACTOR_PWSW: Power switch was pressed.
                          PM_EXIT_FACTOR_BATTERY: Battery low.
                          PM_EXIT_FACTOR_USER: Called PM_ForceToResetHardware.
                          PM_EXIT_FACTOR_NONE: Not set yet.
 *---------------------------------------------------------------------------*/
PMExitFactor PM_GetExitFactor(void)
{
	return PMi_ExitFactor;
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreExitCallback

  Description:  Appends exit callback info to pre-callback info list.

  Arguments:    info: Callback info to append

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_AppendPreExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostExitCallback

  Description:  Appends exit callback info to post-callback info list.

  Arguments:    info: Callback info to append

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_AppendPostExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreExitCallback

  Description:  Prepends exit callback info to pre-callback info list.

  Arguments:    info: Callback info to prepend

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_PrependPreExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostExitCallback

  Description:  Prepends exit callback info to post-callback info list.

  Arguments:    info: Callback info to prepend

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_PrependPostExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreExitCallback

  Description:  Inserts an exit callback info into pre-callback info list.

  Arguments:    info: Callback info to add
                priority: Priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_InsertPreExitCallbackEx( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PreExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreExitCallback( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPreExitCallbackEx( info, priority );
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostExitCallback

  Description:  Inserts an exit callback info into post-callback info list.

  Arguments:    info: Callback info to add
                priority: Priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_InsertPostExitCallbackEx( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PostExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostExitCallback( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPostExitCallbackEx( info, priority );
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreExitCallback

  Description:  Deletes exit callback info from pre-callback info list.

  Arguments:    info: Callback info to delete

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DeletePreExitCallback( PMExitCallbackInfo *info )
{
    PMi_DeleteList(&PMi_PreExitCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostExitCallback

  Description:  Deletes exit callback info from post-callback info list.

  Arguments:    info: Callback info to delete

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DeletePostExitCallback( PMExitCallbackInfo *info )
{
    PMi_DeleteList(&PMi_PostExitCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearPreExitCallback

  Description:  Clears the exit pre-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_ClearPreExitCallback(void)
{
    PMi_ClearList(&PMi_PreExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearPostExitCallback

  Description:  Clears the exit post-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_ClearPostExitCallback(void)
{
    PMi_ClearList(&PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecutePreExitCallbackList

  Description:  Executes the exit pre-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_ExecutePreExitCallbackList(void)
{
    PMi_ExecuteList(PMi_PreExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecutePostExitCallbackList

  Description:  Executes the exit post-callback info list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_ExecutePostExitCallbackList(void)
{
    PMi_ExecuteList(PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecuteAllListsOfExitCallback

  Description:  Executes all callbacks registered to info lists.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_ExecuteAllListsOfExitCallback(void)
{
    PMi_ExecuteList(PMi_PreExitCallbackList);
    PMi_ExecuteList(PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAutoExit

  Description:  Sets flag indicating whether to automatically shutdown/reset after callbacks.

  Arguments:    sw: TRUE: Shut down/reset after callback automatically
                     FALSE: Do not shut down/reset after callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_SetAutoExit( BOOL sw )
{
	PMi_AutoExitFlag = sw;
}
/*---------------------------------------------------------------------------*
  Name:         PM_GetAutoExit

  Description:  Gets the current setting of AutoExit.

  Arguments:    None.

  Returns:      TRUE: Shut down/reset after callback automatically
                FALSE: Do not shut down/reset after callback
 *---------------------------------------------------------------------------*/
BOOL PM_GetAutoExit(void)
{
	return PMi_AutoExitFlag;
}

//================================================================================
//             BATTERY CALLBACK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetBatteryLowCallback

  Description:  Sets the low battery callback.

  Arguments:    callback: Callback called when low battery is detected.
                arg: Callback argument 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_SetBatteryLowCallback( PMBatteryLowCallback callback, void* arg )
{
	PMi_BatteryLowCallbackInfo.callback = callback;
	PMi_BatteryLowCallbackInfo.arg = arg;
}

//================================================================================
//             RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_ForceToResetHardware

  Description:  Tells the ARM7 to reset hardware.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_ForceToResetHardware(void)
{
    //---- Set exit factors
    PMi_ExitFactor = PM_EXIT_FACTOR_USER;

    PMi_CallPostExitCallbackAndReset(FALSE);
    //This point will never be reached
}

/*---------------------------------------------------------------------------*
  Name:         PMi_CallPostExitCallbackAndReset

  Description:  Calls post exit callbacks and resets hardware.

  Arguments:    isExit: TRUE: EXIT (HARDWARERESET or SHUTDOWN)
                        FALSE: HARDWARERESET

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_CallPostExitCallbackAndReset(BOOL isExit)
{
    //---- Call exit callbacks
    PMi_ExecuteList(PMi_PostExitCallbackList);

    //---- Display off
    GX_DispOff();
    GXS_DispOff();

    //---- Wait 2 V-Blanks at least
    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
    {
        int n;
        for( n=0; n<3; n++ )
        {
            u32 count = OS_GetVBlankCount();
            while( count == OS_GetVBlankCount() )
            {
                OS_SpinWait(100);
            }
        }
    }

    //---- Send EXIT or RESET command
    while(1)
    {
        u16 result;
        u32 command = isExit? PM_UTIL_FORCE_EXIT: PM_UTIL_FORCE_RESET_HARDWARE;

        //---- Exit if 'forced' is specified
        if ( *(u32*)HW_RESET_LOCK_FLAG_BUF == PM_RESET_FLAG_FORCED )
        {
            command = PM_UTIL_FORCE_EXIT;

            //---- Clear launcher param
            ((LauncherParam*)HW_PARAM_LAUNCH_PARAM)->header.magicCode = 0;
        }

        PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE | PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
        while( PM_SendUtilityCommand(command, 0, &result) != PM_SUCCESS )
        {
            //---- Wait
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
            PMi_WaitBusy();
        }
        if ( result == SPI_PXI_RESULT_SUCCESS )
        {
            break;
        }

        //---- Wait
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    (void)OS_DisableInterrupts();

    //---- Stop all DMA
    MI_StopAllDma();
    MI_StopAllNDma();

    //---- Finalize debugger
    PMi_FinalizeDebugger();

    OSi_TerminateCore();
    //This point will never be reached
}
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         PMi_LCDOnAvoidReset

  Description:  Force to power on LCD to avoid the following.
                On USG, the operation of power off with LCD-ON may cause reset hardware not power off
                because of PMIC of the specified maker.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PMi_LCDOnAvoidReset(void)
{
    BOOL preMethod;

	//---- Wait while specified period for LCD (0x360000sysClock == about 106 ms)
	OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

	//---- Force to set LCD power on
    preMethod = PMi_WaitBusyMethod;
    PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE | PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
	if (PM_GetLCDPower() != PM_LCD_POWER_ON)
	{
		//---- Set backlight off (for avoiding appearance of afterimage)
        while( PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

		while (!PM_SetLCDPower(PM_LCD_POWER_ON))
		{
			OS_SpinWait(PMi_PXI_WAIT_TICK);
		}
	}
    PMi_WaitBusyMethod = preMethod;
}

#ifndef SDK_FINALROM
//================================================================================
//     DIRECT REGISTER OPERATION (for DEBUG)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_ReadRegisterAsync

  Description:  Sends read register command to ARM7.

  Arguments:    registerAddr: PMIC register number (0-3)
                buffer: Buffer to store register value
                callback: Callback function
                arg: Callback argument

  Returns:      Result of issuing command.
                PM_RESULT_BUSY: Busy.
                PM_RESULT_SUCCESS: Success.
 *---------------------------------------------------------------------------*/
u32 PMi_ReadRegisterAsync(u16 registerAddr, u16 *buffer, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_READREG, registerAddr, buffer, callback, arg);
}
u32 PMi_ReadRegister(u16 registerAddr, u16 *buffer)
{
	return PM_SendUtilityCommand(PMi_UTIL_READREG, registerAddr, buffer);
}
/*---------------------------------------------------------------------------*
  Name:         PMi_WriteRegisterAsync

  Description:  Sends write register command to ARM7.

  Arguments:    registerAddr: PMIC register number (0-3)
                data: Data written to PMIC register
                callback: Callback function
                arg: Callback argument

  Returns:      Result of issuing command.
                PM_RESULT_BUSY: Busy
                PM_RESULT_SUCCESS: Success
 *---------------------------------------------------------------------------*/
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_WRITEREG, (u16)((registerAddr<<8) | (data&0xff)), NULL, callback, arg);
}

u32 PMi_WriteRegister(u16 registerAddr, u16 data)
{
	return PM_SendUtilityCommand(PMi_UTIL_WRITEREG, (u16)((registerAddr<<8) | (data&0xff)), NULL);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PMi_SetDispOffCount

  Description:  Records disp from counter for internal use.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_SetDispOffCount(void)
{
	PMi_DispOffCount = OS_GetVBlankCount();
}

//================================================================================
//   DUMP (for DEBUG)
//================================================================================
#ifndef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         PM_DumpSleepCallback

  Description:  Dumps sleep callbacks (for debug).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DumpSleepCallback(void)
{
	PMGenCallbackInfo *p;

	p = PMi_PreSleepCallbackList;
	OS_TPrintf("----PreSleep Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}

	p = PMi_PostSleepCallbackList;
	OS_TPrintf("----PostSleep Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         PM_DumpExitCallback

  Description:  Dumps exit callbacks (for debug).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DumpExitCallback(void)
{
	PMGenCallbackInfo *p;

	p = PMi_PreExitCallbackList;
	OS_TPrintf("----PreExit Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}

	p = PMi_PostExitCallbackList;
	OS_TPrintf("----PostExit Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}
}
#include <twl/ltdmain_end.h>
#endif //ifdef SDK_TWL
#endif //ifndef SDK_FINALROM

//================================================================================
//   LOCK for RESET/EXIT
//================================================================================
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         PMi_TryLockForReset

  Description:  Tries to lock. This is for the forced reset and pushing battery button.

  Arguments:    None.

  Returns:      TRUE: Lock succeeded.
                FALSE: Lock failed.
 *---------------------------------------------------------------------------*/
static volatile BOOL isLockedReset = FALSE;
BOOL PMi_TryLockForReset(void)
{
    OSIntrMode e = OS_DisableInterrupts();

    //---- Try lock
    if ( isLockedReset )
    {
        (void)OS_RestoreInterrupts(e);
        return FALSE;
    }
    isLockedReset = TRUE;

    (void)OS_RestoreInterrupts(e);
    return TRUE;
}
#include <twl/ltdmain_end.h>
#endif // ifdef SDK_TWL
