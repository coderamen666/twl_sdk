/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - tp
  File:     tp.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/spi/ARM9/tp.h>
#include <nitro/spi/common/config.h>
#include "spi.h"

#define TP_RAW_MAX  0x1000
#define TP_CALIBRATE_DOT_INV_SCALE_SHIFT    (28 - TP_CALIBRATE_DOT_SCALE_SHIFT)
#define TP_CALIBRATE_DOT2ORIGIN_SCALE_SHIFT (TP_CALIBRATE_DOT_SCALE_SHIFT - TP_CALIBRATE_ORIGIN_SCALE_SHIFT)

/*===========================================================================*
    Static function prototype definition
 *===========================================================================*/

static void TPi_TpCallback(PXIFifoTag tag, u32 data, BOOL err);

/*---------------------------------------------------------------------------*
    Static variable definitions
 *---------------------------------------------------------------------------*/
typedef struct
{
    s32     x0;                        // X coordinate intercept
    s32     xDotSize;                  // X dot width
    s32     xDotSizeInv;               // X denominator

    s32     y0;                        // Y coordinate intercept
    s32     yDotSize;                  // Y dot width
    s32     yDotSizeInv;               // Y denominator

}
TPiCalibrateParam;

#pragma  warn_padding off
static struct
{
    TPRecvCallback callback;           // User callback function called when TP value obtained
    TPData  buf;                       // TP value receive buffer when TP value is obtained once
    u16     index;                     // Latest buffer index during auto-sampling
    u16     frequence;                 // Auto-sampling frequency in a single frame
    TPData *samplingBufs;              // Pointer to the TP value buffer during auto-sampling
    u16     bufSize;                   // TP buffer size during auto-sampling
    // PADDING 2 BYTE
    TPiCalibrateParam calibrate;       // Calibration parameters
    u16     calibrate_flg;             // calibration flag

    vu16    state;                     // Touch panel status
    vu16    err_flg;                   // Error flag
    vu16    command_flg;               // Flag while request command is executing
}
tpState;
#pragma  warn_padding reset


/*---------------------------------------------------------------------------*
    Inline sub-routine definition
    
    The response from ARM7 for these instructions are returned via the PXI library.
    To obtain the response from the ARM7, specify the callback for the "PXI_FIFO_TAG_TOUCHPANEL" tag to PXI.
    
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TPi_SamplingNow

  Description:  Samples touch panel once.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL TPi_SamplingNow(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT
                               | SPI_PXI_END_BIT
                               | (0 << SPI_PXI_INDEX_SHIFT)
                               | (SPI_PXI_COMMAND_TP_SAMPLING << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         TPi_AutoSamplingOn

  Description:  Begins auto-sampling the touch panel.

  Arguments:    vCount: V-Count at which to carry out sampling.
                            If sampling is done multiple times per each single frame, the single frame is divided in time starting here.
                            
                frequency - Frequency of sampling for 1 frame.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL TPi_AutoSamplingOn(u16 vCount, u8 frequency)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT
                               | (0 << SPI_PXI_INDEX_SHIFT)
                               | (SPI_PXI_COMMAND_TP_AUTO_ON << 8) | (u32)frequency, 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_END_BIT | (1 << SPI_PXI_INDEX_SHIFT) | (u32)vCount, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         TPi_AutoSamplingOff

  Description:  Stop auto-sampling the touch panel

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL TPi_AutoSamplingOff(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT
                               | SPI_PXI_END_BIT
                               | (0 << SPI_PXI_INDEX_SHIFT)
                               | (SPI_PXI_COMMAND_TP_AUTO_OFF << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         TPi_SetupStability

  Description:  Sets stability determination parameters for sampling

  Arguments:    range - For continuous sampling, error for which detected voltage is considered stable.
                        The detection value is 12 bits, 0 - 4095.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL TPi_SetupStability(u16 range)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT
                               | SPI_PXI_END_BIT
                               | (0 << SPI_PXI_INDEX_SHIFT)
                               | (SPI_PXI_COMMAND_TP_SETUP_STABILITY << 8) | (u32)range, 0))
    {
        return FALSE;
    }

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         TPi_CopyTpFromSystemWork

  Description:  Takes the touch screen values data written to shared memory from ARM7, and reads them out to a separate region.
                

  Returns:      result - Stores the latest touch panel value that was read.
 *---------------------------------------------------------------------------*/
static inline void TPi_CopyTpFromSystemWork(TPData *result)
{
    SPITpData spi_tp;

    spi_tp.halfs[0] = *(u16 *)(&(OS_GetSystemWork()->touch_panel[0]));
    spi_tp.halfs[1] = *(u16 *)(&(OS_GetSystemWork()->touch_panel[2]));

    // Read from system area (2 byte access)
    result->x = (u16)spi_tp.e.x;
    result->y = (u16)spi_tp.e.y;
    result->touch = (u8)spi_tp.e.touch;
    result->validity = (u8)spi_tp.e.validity;
}


/*---------------------------------------------------------------------------*
  Name:         TPi_ErrorAtPxi

  Description:  Processing for an error in PXI communication with ARM7.
                If a callback is set, this returns the TP_RESULT_PXI_BUSY callback 
                
                
  Arguments:    command - Request type
                
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void TPi_ErrorAtPxi(TPRequestCommand command)
{

    tpState.err_flg |= (1 << command);

    if (tpState.callback)
    {
        (tpState.callback) (command, TP_RESULT_PXI_BUSY, 0);
    }
}


/*===========================================================================*
    Function definition
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         TPi_TpCallback

  Description:  This is the function called when PXI receives a touch screen-related message from the ARM7.
                
                This saves the touch screen values from ARM7, and if a callback function is set, it also calls the user callback. 
                
                

  Arguments:    tag -  Tag so PXI can distinguish type
                data - Message data from ARM7
                err -  PXI transfer error flag
                
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TPi_TpCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)

    u16     result;
    u16     command;

    result = (u16)(data & SPI_PXI_DATA_MASK);
    command = (u16)((result & 0x7f00) >> 8);

    // PXI transfer error
    if (err)
    {
        TPi_ErrorAtPxi((TPRequestCommand)command);
        return;
    }

    if (command == SPI_PXI_COMMAND_TP_AUTO_SAMPLING)
    {
        // Notification that auto-sampling execution has completed
//        tpState.index = (u16) (result & 0x00FF);
        tpState.index++;
        if (tpState.index >= tpState.bufSize)
        {
            tpState.index = 0;
        }

        // Save touch panel values from the shared area
        TPi_CopyTpFromSystemWork(&tpState.samplingBufs[tpState.index]);

        if (tpState.callback)
        {
            (tpState.callback) ((TPRequestCommand)command, TP_RESULT_SUCCESS, (u8)tpState.index);
        }
        return;
    }

    if (!(data & SPI_PXI_END_BIT))
    {
        return;
    }

    SDK_ASSERT(result & 0x8000);

    switch ((u8)(result & 0x00ff))
    {
    case SPI_PXI_RESULT_SUCCESS:
        // Process when successful
        switch (command)
        {
        case SPI_PXI_COMMAND_TP_SAMPLING:
            // Obtain sampling result
            TPi_CopyTpFromSystemWork(&tpState.buf);
            tpState.state = TP_STATE_READY;
            break;

        case SPI_PXI_COMMAND_TP_AUTO_ON:
            tpState.state = TP_STATE_AUTO_SAMPLING;
            break;

        case SPI_PXI_COMMAND_TP_AUTO_OFF:
            tpState.state = TP_STATE_READY;
            break;
        }

        // Turn off busy flag
        tpState.command_flg &= ~(1 << command);

        // callback call
        if (tpState.callback)
        {
            (tpState.callback) ((TPRequestCommand)command, TP_RESULT_SUCCESS, 0);
        }
        break;

    case SPI_PXI_RESULT_EXCLUSIVE:
        result = TP_RESULT_EXCLUSIVE;
        goto common;

    case SPI_PXI_RESULT_INVALID_PARAMETER:
        result = TP_RESULT_INVALID_PARAMETER;
        goto common;

    case SPI_PXI_RESULT_ILLEGAL_STATUS:
        result = TP_RESULT_ILLEGAL_STATUS;

      common:
        // Error processing
        // Set error flag
        tpState.err_flg |= (1 << command);
        tpState.command_flg &= ~(1 << command);

        if (tpState.callback)
        {
            (tpState.callback) ((TPRequestCommand)command, (TPRequestResult)(result & 0xff), 0);
        }
        break;

    case SPI_PXI_RESULT_INVALID_COMMAND:
    default:
        // Abnormal end
//        OS_TPrintf("result=%x\n",result);
        OS_TPanic("Get illegal TP command from ARM7!\n");
        return;
    }
}




/*---------------------------------------------------------------------------*
  Name:         TP_Init

  Description:  Touch panel library initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_Init(void)
{
    static u16 initial = FALSE;

    if (initial)
    {
        return;
    }
    initial = TRUE;

    //****************************************************************
    // Initialize TP.
    //****************************************************************
    PXI_Init();

    tpState.index = 0;
    tpState.callback = NULL;
    tpState.samplingBufs = NULL;
    tpState.state = TP_STATE_READY;
    tpState.calibrate_flg = FALSE;
    tpState.command_flg = 0;
    tpState.err_flg = 0;

    // 2003/05/18 Add by terui.
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_TOUCHPANEL, PXI_PROC_ARM7))
    {
    }

    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TOUCHPANEL, TPi_TpCallback);
}


/*---------------------------------------------------------------------------*
  Name:         TP_GetUserInfo

  Description:  This function gets calibration parameters from the system's internal flash memory.
                

  Returns:      param - Pointer to area for obtaining parameters
                BOOL  - TRUE if the values were successfully acquired
                        Returns FALSE if valid values could not be found.
 *---------------------------------------------------------------------------*/
BOOL TP_GetUserInfo(TPCalibrateParam *calibrate)
{
    NVRAMConfig *info = (NVRAMConfig *)(OS_GetSystemWork()->nvramUserInfo);
//    s16 x0, y0, xdot, ydot;
    u16     x1, y1, x2, y2, dx1, dy1, dx2, dy2;

    SDK_NULL_ASSERT(calibrate);

    x1 = info->ncd.tp.raw_x1;
    y1 = info->ncd.tp.raw_y1;
    dx1 = (u16)(info->ncd.tp.dx1);
    dy1 = (u16)(info->ncd.tp.dy1);
    x2 = info->ncd.tp.raw_x2;
    y2 = info->ncd.tp.raw_y2;
    dx2 = (u16)(info->ncd.tp.dx2);
    dy2 = (u16)(info->ncd.tp.dy2);

    /* For now, since there is no enable flag for the calibration value */
    if ((x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0) ||
        (TP_CalcCalibrateParam(calibrate, x1, y1, dx1, dy1, x2, y2, dx2, dy2) != 0))
    {
        // Since a CRC check on the data is not performed in the IPL, if values are abnormal, zero-clear all parameters and return TRUE.
        //  
        // In this case, the TP value always becomes (0,0) after calibration.
        calibrate->x0 = 0;
        calibrate->y0 = 0;
        calibrate->xDotSize = 0;
        calibrate->yDotSize = 0;
        return TRUE;
    }
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         TP_SetCalibrateParam

  Description:  Set parameters for calibration.
                After parameters are set with this function, the values are used for calibration in TP_WaitCalibratedResult(), TP_GetLatestCalibratedPointInAuto(), TP_GetCalibratedPoint(), and TP_GetUnCalibratedPoint().
                
                
                 At this point, the reciprocal of the dot size is calculated in advance.

  Arguments:    param - Pointer to the calibration parameters to set
                        If NULL is set, further coordinate conversions due to calibration will not be carried out.
                         (Default value: NULL)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_SetCalibrateParam(const TPCalibrateParam *param)
{
    OSIntrMode enabled;

    if (param == NULL)
    {
        tpState.calibrate_flg = FALSE;
        return;
    }

    // Calculate the reciprocals of xDotSize and yDotSize in advance
    enabled = OS_DisableInterrupts();

    if (param->xDotSize != 0)
    {
        CP_SetDiv32_32(0x10000000, (u32)param->xDotSize);

        tpState.calibrate.x0 = param->x0;
        tpState.calibrate.xDotSize = param->xDotSize;
        tpState.calibrate.xDotSizeInv = (s32)CP_GetDivResult32();
    }
    else
    {
        tpState.calibrate.x0 = 0;
        tpState.calibrate.xDotSize = 0;
        tpState.calibrate.xDotSizeInv = 0;
    }

    if (param->yDotSize != 0)
    {
        CP_SetDiv32_32(0x10000000, (u32)param->yDotSize);

        tpState.calibrate.y0 = param->y0;
        tpState.calibrate.yDotSize = param->yDotSize;
        tpState.calibrate.yDotSizeInv = (s32)CP_GetDivResult32();
    }
    else
    {
        tpState.calibrate.y0 = 0;
        tpState.calibrate.yDotSize = 0;
        tpState.calibrate.yDotSizeInv = 0;
    }

    (void)OS_RestoreInterrupts(enabled);

    tpState.calibrate_flg = TRUE;

}


/*---------------------------------------------------------------------------*
  Name:         TP_SetCallback

  Description:  Sets the user callback function that is invoked when processing results are returned from the Touch Screen.
                

  Arguments:    callback - User callback function pointer
                           If NULL, callback is not called (default:NULL)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_SetCallback(TPRecvCallback callback)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();
    tpState.callback = callback;
    (void)OS_RestoreInterrupts(enabled);
}



/*---------------------------------------------------------------------------*
  Name:         TP_RequestSamplingAsync

  Description:  Request a touch panel value from ARM7.
                After calling this function, you can use TP_WaitRawResult() or TP_WaitCalibratedResult() to read Touch Screen values.
                Cannot be used while auto sampling.
                
                
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_RequestSamplingAsync(void)
{
    OSIntrMode enabled;

    SDK_ASSERT(!(tpState.command_flg & TP_REQUEST_COMMAND_FLAG_SAMPLING));

    enabled = OS_DisableInterrupts();

    if (TPi_SamplingNow() == FALSE)
    {
        // Failure on PXI send
        (void)OS_RestoreInterrupts(enabled);
        TPi_ErrorAtPxi(TP_REQUEST_COMMAND_SAMPLING);
        return;
    }
    tpState.command_flg |= TP_REQUEST_COMMAND_FLAG_SAMPLING;
    tpState.err_flg &= ~TP_REQUEST_COMMAND_FLAG_SAMPLING;

    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         TP_WaitRawResult

  Description:  Waits for the ARM7 to set the touch screen values, then reads the uncalibrated raw values. 
                
                Data that is acquired here has already been subjected to chattering measures.

  Arguments:    None.

  Returns:      result:   Pointer to a variable that is used to acquire touch panel values
                         Takes uncalibrated values (0 - 4095) as x,y coordinates.
                u32: Returns 0 if data was obtained successfully and a non-zero value on failure.
                         
 *---------------------------------------------------------------------------*/
u32 TP_WaitRawResult(TPData *result)
{
    SDK_NULL_ASSERT(result);

    TP_WaitBusy(TP_REQUEST_COMMAND_FLAG_SAMPLING);

    if (tpState.err_flg & TP_REQUEST_COMMAND_FLAG_SAMPLING)
    {
        return 1;
    }

    *result = tpState.buf;
    return 0;
}



/*---------------------------------------------------------------------------*
  Name:         TP_GetCalibratedResult

  Description:  Assumes that the ARM7 has set the touch screen values, and reads values that correspond to calibrated screen coordinates.
                
                Data that is acquired here has already been subjected to chattering measures.

  Arguments:    result:   Pointer to a variable that is used to acquire touch panel values
                         The x, y coordinates will be the values corresponding to the screen coordinates.
                         If calibration parameters have not been set, this function obtains Touch Screen values in the range (0-4095).
                         
                         
  Returns:      u32: Returns 0 if data was obtained successfully and a non-zero value on failure.
                         
 *---------------------------------------------------------------------------*/
u32 TP_GetCalibratedResult(TPData *result)
{
    SDK_NULL_ASSERT(result);

    if (tpState.err_flg & TP_REQUEST_COMMAND_FLAG_SAMPLING)
    {
        return 1;
    }

    *result = tpState.buf;
    TP_GetCalibratedPoint(result, result);
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         TP_WaitCalibratedResult

  Description:  Waits for the ARM7 to set the touch screen values, and reads values that correspond to calibrated screen coordinates.
                
                Data that is acquired here has already been subjected to chattering measures.

  Arguments:    result:   Pointer to a variable that is used to acquire touch panel values
                         The x, y coordinates will be the values corresponding to the screen coordinates.
                         If calibration parameters have not been set, this function obtains Touch Screen values in the range (0-4095).
                         
                         
  Returns:      u32: Returns 0 if data was obtained successfully and a non-zero value on failure.
                         
 *---------------------------------------------------------------------------*/
u32 TP_WaitCalibratedResult(TPData *result)
{
    TP_WaitBusy(TP_REQUEST_COMMAND_FLAG_SAMPLING);
    return TP_GetCalibratedResult(result);
}


/*---------------------------------------------------------------------------*
  Name:         TP_RequestAutoSamplingStartAsync

  Description:  Sends a request to ARM7 to start auto sampling touch panel values.
                In one frame, data is sampled 'frequence' times at equal intervals, and the results are stored to an array specified by 'samplingBufs'.
                

  Arguments:    vcount:        Sets the VCOUNT value, which is used as the standard for auto sampling
                frequence:     Setting for the number of times sampling will be done in one frame
                samplingBufs:   Sets the area where auto-sampling data will be stored
                bufSize      - Sets the buffer size
                               The samplingBufs array is used as a ring buffer of size bufSize.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_RequestAutoSamplingStartAsync(u16 vcount, u16 frequence, TPData samplingBufs[], u16 bufSize)
{
    u32     i;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(samplingBufs);
    SDK_ASSERT(vcount < HW_LCD_LINES);
    SDK_ASSERT(frequence != 0 && frequence <= SPI_TP_SAMPLING_FREQUENCY_MAX);
    SDK_ASSERT(tpState.state == TP_STATE_READY);
    SDK_ASSERT(!(tpState.command_flg & TP_REQUEST_COMMAND_FLAG_AUTO_ON));
    SDK_ASSERT(bufSize != 0);

    tpState.samplingBufs = samplingBufs;
    tpState.index = 0;
    tpState.frequence = frequence;
    tpState.bufSize = bufSize;

    for (i = 0; i < bufSize; i++)
    {
        tpState.samplingBufs[i].touch = TP_TOUCH_OFF;
    }

    enabled = OS_DisableInterrupts();

    if ((u8)TPi_AutoSamplingOn(vcount, (u8)frequence) == FALSE)
    {
        // Failure on PXI send
        (void)OS_RestoreInterrupts(enabled);
        TPi_ErrorAtPxi(TP_REQUEST_COMMAND_AUTO_ON);
        return;
    }
    tpState.command_flg |= TP_REQUEST_COMMAND_FLAG_AUTO_ON;
    tpState.err_flg &= ~TP_REQUEST_COMMAND_FLAG_AUTO_ON;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         TP_RequestAutoSamplingStopAsync

  Description:  Sends a request to ARM7 to stop auto sampling touch panel values.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_RequestAutoSamplingStopAsync(void)
{
    OSIntrMode enabled;

    SDK_ASSERT(tpState.state == TP_STATE_AUTO_SAMPLING);
    SDK_ASSERT(!(tpState.command_flg & TP_REQUEST_COMMAND_FLAG_AUTO_OFF));

    enabled = OS_DisableInterrupts();

    if (TPi_AutoSamplingOff() == FALSE)
    {
        // Failure on PXI send
        (void)OS_RestoreInterrupts(enabled);
        TPi_ErrorAtPxi(TP_REQUEST_COMMAND_AUTO_OFF);
        return;
    }

    tpState.command_flg |= TP_REQUEST_COMMAND_FLAG_AUTO_OFF;
    tpState.err_flg &= ~TP_REQUEST_COMMAND_FLAG_AUTO_OFF;

    (void)OS_RestoreInterrupts(enabled);

}


/*---------------------------------------------------------------------------*
  Name:         TP_RequestSetStabilityAsync

  Description:  Sets touch panel anti-chattering parameters.
                Sets the number of times to retry sampling before values stabilize, and a range for determining whether values have stabilized.
                

  Arguments:    retry:   This argument is not used internally.
                range:   Range for determining whether the values have stabilized.
                         Coordinate values that do not converge within this range are invalid.
                         (Range: 0 to 4095, Default value: 20)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_RequestSetStabilityAsync(u8 retry, u16 range)
{
#pragma unused( retry )
    OSIntrMode enabled;

    SDK_ASSERT(range != 0);
    SDK_ASSERT(range < 255);

    enabled = OS_DisableInterrupts();

    if (TPi_SetupStability(range) == FALSE)
    {
        // Failure on PXI send
        (void)OS_RestoreInterrupts(enabled);
        TPi_ErrorAtPxi(TP_REQUEST_COMMAND_SET_STABILITY);
        return;
    }
    tpState.command_flg |= TP_REQUEST_COMMAND_FLAG_SET_STABILITY;
    tpState.err_flg &= ~TP_REQUEST_COMMAND_FLAG_SET_STABILITY;

    (void)OS_RestoreInterrupts(enabled);
}



/*---------------------------------------------------------------------------*
  Name:         TP_GetLatestRawPointInAuto

  Description:  Obtains the latest, valid touch panel values from the results of auto-sampling.
                The x and y coordinates of the values take the uncalibrated range (0-4095).

  Arguments:    None.

  Returns:      result - Pointer for obtaining the latest, valid touch panel value
 *---------------------------------------------------------------------------*/
void TP_GetLatestRawPointInAuto(TPData *result)
{
    s32     i, curr_index;
    TPData *tp;

    SDK_NULL_ASSERT(result);
    SDK_NULL_ASSERT(tpState.samplingBufs);

    result->validity = TP_VALIDITY_INVALID_XY;

    curr_index = tpState.index;

    // If the sampling buffer size is 1, return the current coordinate values unchanged
    if (tpState.frequence == 1 || tpState.bufSize == 1)
    {
        *result = tpState.samplingBufs[curr_index];
        return;
    }

    // Search for the most recent of the valid sampling values
    for (i = 0; i < tpState.frequence && i < tpState.bufSize - 1; i++)
    {
        s32     index;

        index = curr_index - i;
        if (index < 0)
        {
            index += tpState.bufSize;
        }

        tp = &tpState.samplingBufs[index];

        if (!tp->touch)
        {
            *result = *tp;
            return;
        }

        // If invalid data is present refer to the next-older sampling value
        if (result->validity & TP_VALIDITY_INVALID_X)
        {
            /* X-coordinate */
            if (!(tp->validity & TP_VALIDITY_INVALID_X))
            {
                result->x = tp->x;
                if (i != 0)
                {
                    result->validity &= ~TP_VALIDITY_INVALID_X;
                }
            }
        }

        if (result->validity & TP_VALIDITY_INVALID_Y)
        {
            /* Y-coordinate */
            if (!(tp->validity & TP_VALIDITY_INVALID_Y))
            {
                result->y = tp->y;
                if (i != 0)
                {
                    result->validity &= ~TP_VALIDITY_INVALID_Y;
                }
            }
        }

        if (result->validity == TP_VALIDITY_VALID)
        {
            result->touch = TP_TOUCH_ON;
            return;
        }
    }

    // When an invalid coordinate value is left, return the value that could be obtained.
    result->touch = TP_TOUCH_ON;
    return;
}



/*---------------------------------------------------------------------------*
  Name:         TP_GetLatestCalibratedPointInAuto

  Description:  Obtains the latest, valid touch panel values from the results of auto-sampling.
                The x and y coordinates of the values take the range of screen coordinates.

  Arguments:    None.

  Returns:      result - Pointer for obtaining the latest, valid touch panel value
 *---------------------------------------------------------------------------*/
void TP_GetLatestCalibratedPointInAuto(TPData *result)
{
    SDK_NULL_ASSERT(result);

    TP_GetLatestRawPointInAuto(result);

    TP_GetCalibratedPoint(result, result);

}


/*---------------------------------------------------------------------------*
  Name:         TP_GetLatestIndexInAuto

  Description:  Returns the index in the loop buffer (in which values are stored by auto-sampling) for the data that was last sampled.
                

  Arguments:    None.

  Returns:      u16 - Index to the data that was last sampled.
 *---------------------------------------------------------------------------*/
u16 TP_GetLatestIndexInAuto(void)
{
    return tpState.index;
}




/*---------------------------------------------------------------------------*
  Name:         TP_CalcCalibrateParam

  Description:  Calculates calibration parameters from the Touch Screen values and the screen coordinates for two points.
                

  Arguments:    raw_x1, raw_y1 - Point 1's touch panel values
                dx1, dy1       - Point 1's screen coordinates
                raw_x2, raw_y2 - Point 2's touch panel values
                dx2, dy2,      - Point 2's screen coordinates
                
  Returns:      calibrate      - Calibration parameters
                u32            - Returns zero for valid parameters and nonzero for invalid parameters.
                                 
                                 
 *---------------------------------------------------------------------------*/
u32 TP_CalcCalibrateParam(TPCalibrateParam *calibrate,
                          u16 raw_x1, u16 raw_y1,
                          u16 dx1, u16 dy1, u16 raw_x2, u16 raw_y2, u16 dx2, u16 dy2)
{
    s32     rx_width, dx_width, ry_width, dy_width;
    s32     tmp32;
    OSIntrMode enabled;

#define IN_S16_RANGE(x) ((x) < 0x8000 && (x) >= -0x8000)

    /*                                                                  */
    /* xDotSize = ((raw_x1 - raw_x2) << SCALE_SHIFT) / (dx1 - dx2)      */
    /* x0 = ((raw_x1 + raw_x2) - (dx1 + dx2) * xDotSize) / 2            */
    /* xDotSize = ((raw_x1 - raw_x2) << SCALE_SHIFT) / (dx1 - dx2)      */
    /* x0 = ((raw_x1 + raw_x2) - (dx1 + dx2) * xDotSize) / 2            */
    /*                                                                  */

    SDK_NULL_ASSERT(calibrate);
    // Check the coordinate range
    if (raw_x1 >= TP_RAW_MAX || raw_y1 >= TP_RAW_MAX || raw_x2 >= TP_RAW_MAX
        || raw_y2 >= TP_RAW_MAX)
    {
        return 1;
    }
    if (dx1 >= GX_LCD_SIZE_X || dx2 >= GX_LCD_SIZE_X || dy1 >= GX_LCD_SIZE_Y
        || dy2 >= GX_LCD_SIZE_Y)
    {
        return 1;
    }
    if (dx1 == dx2 || dy1 == dy2 || raw_x1 == raw_x2 || raw_y1 == raw_y2)
    {
        return 1;
    }

    rx_width = raw_x1 - raw_x2;
    dx_width = dx1 - dx2;

    enabled = OS_DisableInterrupts();

    // Calculate xDotSize
    CP_SetDiv32_32(((u32)rx_width) << TP_CALIBRATE_DOT_SCALE_SHIFT, (u32)dx_width);

    ry_width = raw_y1 - raw_y2;
    dy_width = dy1 - dy2;

    tmp32 = CP_GetDivResult32();
    CP_SetDiv32_32(((u32)ry_width) << TP_CALIBRATE_DOT_SCALE_SHIFT, (u32)dy_width);

    if (!IN_S16_RANGE(tmp32))
    {
        (void)OS_RestoreInterrupts(enabled);
        return 1;
    }
    calibrate->xDotSize = (s16)tmp32;
    tmp32 = (s16)((((s32)(raw_x1 + raw_x2) << TP_CALIBRATE_DOT_SCALE_SHIFT)
                   - ((s32)(dx1 + dx2) * calibrate->xDotSize)) >> (TP_CALIBRATE_DOT_SCALE_SHIFT -
                                                                   TP_CALIBRATE_ORIGIN_SCALE_SHIFT +
                                                                   1));
    if (!IN_S16_RANGE(tmp32))
    {
        (void)OS_RestoreInterrupts(enabled);
        return 1;
    }
    calibrate->x0 = (s16)tmp32;

    tmp32 = CP_GetDivResult32();
    (void)OS_RestoreInterrupts(enabled);

    if (!IN_S16_RANGE(tmp32))
    {
        return 1;
    }
    calibrate->yDotSize = (s16)tmp32;
    tmp32 = (s16)((((s32)(raw_y1 + raw_y2) << TP_CALIBRATE_DOT_SCALE_SHIFT)
                   - ((s32)(dy1 + dy2) * calibrate->yDotSize)) >> (TP_CALIBRATE_DOT_SCALE_SHIFT -
                                                                   TP_CALIBRATE_ORIGIN_SCALE_SHIFT +
                                                                   1));
    if (!IN_S16_RANGE(tmp32))
    {
        return 1;
    }
    calibrate->y0 = (s16)tmp32;

    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         TP_GetCalibratedPoint
  
  Description:  Convert a touch panel value to a screen coordinate
                Touch Screen values will be output unchanged when calibration parameters have not been set.
                It is OK to pass the same pointer to the disp and raw arguments.
                

  Arguments:    raw      - Pointer to the touch panel value that is the conversion source
                
  ReturnValue:  disp     - Pointer to the variable in which the value that has been converted to a screen coordinate was stored
 *---------------------------------------------------------------------------*/
void TP_GetCalibratedPoint(TPData *disp, const TPData *raw)
{
    TPiCalibrateParam *cal;

    enum
    { MAX_X = GX_LCD_SIZE_X - 1, MAX_Y = GX_LCD_SIZE_Y - 1 };

    // ----------------------------------------
    // dx = (raw_x * x0) / xDotSize
    // dy = (raw_y * y0) / yDotSize
    // ----------------------------------------

    SDK_NULL_ASSERT(disp);
    SDK_NULL_ASSERT(raw);
    SDK_NULL_ASSERT(tpState.calibrate_flg);

    if (!tpState.calibrate_flg)
    {
        *disp = *raw;
        return;
    }

    cal = &tpState.calibrate;

    disp->touch = raw->touch;
    disp->validity = raw->validity;

    if (raw->touch == 0)
    {
        disp->x = 0;
        disp->y = 0;
        return;
    }

    // X coordinate conversion
    // disp->x = (x - x0) / xDotSize;
    disp->x =
        (u16)((((u64)(raw->x << TP_CALIBRATE_ORIGIN_SCALE_SHIFT) -
                cal->x0) * cal->xDotSizeInv) >> (TP_CALIBRATE_DOT_INV_SCALE_SHIFT +
                                                 TP_CALIBRATE_ORIGIN_SCALE_SHIFT));

    if ((s16)disp->x < 0)
    {
        disp->x = 0;
    }
    else if ((s16)disp->x > MAX_X)
    {
        disp->x = MAX_X;
    }
    // Y coordinate conversion
    // disp->y = (y - y0) / yDotSize;
    disp->y =
        (u16)((((u64)(raw->y << TP_CALIBRATE_ORIGIN_SCALE_SHIFT) -
                cal->y0) * cal->yDotSizeInv) >> (TP_CALIBRATE_DOT_INV_SCALE_SHIFT +
                                                 TP_CALIBRATE_ORIGIN_SCALE_SHIFT));

    if ((s16)disp->y < 0)
    {
        disp->y = 0;
    }
    else if ((s16)disp->y > MAX_Y)
    {
        disp->y = MAX_Y;
    }
}

/*---------------------------------------------------------------------------*
  Name:         TP_GetUnCalibratedPoint

  Description:  Gets the inverse transformation results of a calibration
                Conversion from a screen coordinate to a touch panel value.
                Screen coordinates will be output unchanged when calibration parameters have not been set.
                

  Arguments:    dx, dy       - Screen XY coordinates.
                
  ReturnValue:  raw_x, raw_y - Pointer for returning the corresponding touch panel value
 *---------------------------------------------------------------------------*/
void TP_GetUnCalibratedPoint(u16 *raw_x, u16 *raw_y, u16 dx, u16 dy)
{
    TPiCalibrateParam *cal;

    // ----------------------------------------
    // raw_x = dx * xDotSize + x0;
    // raw_y = dy * yDotSize + y0;
    // ----------------------------------------

    SDK_NULL_ASSERT(raw_x);
    SDK_NULL_ASSERT(raw_y);
    SDK_ASSERT(tpState.calibrate_flg);

    if (!tpState.calibrate_flg)
    {
        *raw_x = dx;
        *raw_y = dy;
        return;
    }

    cal = &tpState.calibrate;

    *raw_x =
        (u16)((((s32)(dx * cal->xDotSize) >> TP_CALIBRATE_DOT2ORIGIN_SCALE_SHIFT) +
               (s32)cal->x0) >> TP_CALIBRATE_ORIGIN_SCALE_SHIFT);
    *raw_y =
        (u16)((((s32)(dy * cal->yDotSize) >> TP_CALIBRATE_DOT2ORIGIN_SCALE_SHIFT) +
               (s32)cal->y0) >> TP_CALIBRATE_ORIGIN_SCALE_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         TP_WaitBusy

  Description:  Waits until ARM7 completes a touch panel request

  Arguments:    command_flgs  - Type of request to the touch panel
                
  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_WaitBusy(TPRequestCommandFlag command_flgs)
{
#ifdef  SDK_DEBUG
    if (!(tpState.command_flg & command_flgs))
    {
        return;
    }
#endif
    // Fall into an infinite loop if interrupts are OFF
    SDK_ASSERT(OS_GetCpsrIrq() == OS_INTRMODE_IRQ_ENABLE);
    SDK_ASSERT(reg_OS_IME == 1);
    SDK_ASSERT(OS_GetIrqMask() & OS_IE_SPFIFO_RECV);

    // Place ASSERT statements before starting the loop.
    // Calls to this function must be prohibited while interrupts are disabled.
    // This is due to the fact that some conditions may influence whether the tpState flag has already been cleared.

    while (tpState.command_flg & command_flgs)
    {
        // Do nothing
    }

    return;
}

/*---------------------------------------------------------------------------*
  Name:         TP_WaitAllBusy

  Description:  Waits until ARM7 completes all of the touch panel requests

  Arguments:    None.
                
  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_WaitAllBusy(void)
{
#ifdef  SDK_DEBUG
    if (!tpState.command_flg)
    {
        return;
    }
#endif
    // Fall into an infinite loop if interrupts are OFF
    SDK_ASSERT(OS_GetCpsrIrq() == OS_INTRMODE_IRQ_ENABLE);
    SDK_ASSERT(reg_OS_IME == 1);
    SDK_ASSERT(OS_GetIrqMask() & OS_IE_SPFIFO_RECV);

    while (tpState.command_flg)
    {
        // Do nothing
    }

    return;
}


/*---------------------------------------------------------------------------*
  Name:         TP_CheckBusy

  Description:  Checks whether or not touch panel requests to ARM7 are busy.

  Arguments:    command_flgs  - Type of request to the touch panel
                
  Returns:      u32          - Returns zero if it is not busy and a non-zero value otherwise.
                               
 *---------------------------------------------------------------------------*/
u32 TP_CheckBusy(TPRequestCommandFlag command_flgs)
{
    return (u32)(tpState.command_flg & command_flgs);
}



/*---------------------------------------------------------------------------*
  Name:         TP_CheckError

  Description:  Checks whether or not touch panel request to ARM7 terminated with an error.

  Arguments:    command_flgs  - Type of request to the touch panel
                
  Returns:      u32           - Returns zero if an error has not occurred and a non-zero value otherwise.
                                
 *---------------------------------------------------------------------------*/
u32 TP_CheckError(TPRequestCommandFlag command_flgs)
{
    return (u32)(tpState.err_flg & command_flgs);
}
