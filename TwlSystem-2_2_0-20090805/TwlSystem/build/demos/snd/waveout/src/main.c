/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - snd - waveout
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// USAGE:
//  A: play wave
//  B: stop wave
//  L: record wave
//  Up or Down    : change play speed
//  Left or Right : change panning
//---------------------------------------------------------------------------

#include <nitro.h>
#include <nnsys/snd.h>

#define SAMPLING_RATE 16000
#define SAMPLING_TIME     5 /* sec */

#define CHANNEL_NO 7

#define WAVE_SAMPLE ( SAMPLING_RATE * SAMPLING_TIME )

#define     RETRY_MAX_COUNT     8

static s16 sWaveBuffer[ WAVE_SAMPLE ] ATTRIBUTE_ALIGN(32);
static MICAutoParam sMicParam;
static NNSSndWaveOutHandle sWaveOutHandle;

static u16 Cont;
static u16 Trg;

static void VBlankIntr( void );
static void MicCallback( MICResult /*result*/, void* /*arg*/ );
#ifdef SDK_TWL
static SNDEXFrequency   GetI2SFrequency(void);
#endif
    
//----------------------------------------------------------------
//   NitroMain
//
void NitroMain()
{
    OS_Init();
    GX_Init();
    
    // VBlank settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask( OS_IE_V_BLANK );
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

#ifdef SDK_TWL
    SNDEX_Init();
#endif
    MIC_Init();
    
#ifdef  SDK_TS
    // Initialize PMIC
    PM_Init();
    // AMP on
    (void)PM_SetAmp( PM_AMP_ON );
#if ( SDK_TS_VERSION >= 100 )
    // Adjust AMP gain
    (void)PM_SetAmpGain( PM_AMPGAIN_80 );
#endif
#if ( SDK_TS_VERSION == 100 )
    // Turn off LCD backlight to deal with noise.
    (void)PM_SetBackLight( PM_LCD_ALL, PM_BACKLIGHT_OFF );
#endif
#endif
    
    // Initialize sound
    NNS_SndInit();
    
    // Initialize waveout
    sWaveOutHandle = NNS_SndWaveOutAllocChannel( CHANNEL_NO );
    if ( sWaveOutHandle == NNS_SND_WAVEOUT_INVALID_HANDLE ) {
        OS_Printf("NNS_SndWaveOutAllocChannel is Failed\n");
    }
    
    // Initialize mic parameter
    sMicParam.type = MIC_SAMPLING_TYPE_SIGNED_12BIT;
    sMicParam.buffer = sWaveBuffer;
    sMicParam.size = sizeof( sWaveBuffer );
#ifdef SDK_TWL    
    if ((OS_IsRunOnTwl() == TRUE) && (GetI2SFrequency() == SNDEX_FREQUENCY_47610))
    {
        sMicParam.rate  =   MIC_SAMPLING_RATE_23810;
    }
    else
    {
        sMicParam.rate  =   MIC_SAMPLING_RATE_16360;
    }
#else
    sMicParam.rate = MIC_SAMPLING_RATE_16K;
#endif
    sMicParam.loop_enable = FALSE;
    sMicParam.full_callback = MicCallback;
    sMicParam.full_arg = NULL;
    
    // dummy pad read
    Cont = PAD_Read();
    
    // print usage
    OS_Printf("=================================\n");
    OS_Printf("USAGE:\n");
    OS_Printf(" A : play recorded sound\n");
    OS_Printf(" B : stop sound\n");
    OS_Printf(" L : record sound [ %d sec ]\n", SAMPLING_TIME );
    OS_Printf(" Up or Down    : change play speed\n");
    OS_Printf(" Left or Right : change pan\n");
    OS_Printf("=================================\n");
    
    //================ Main Loop
    while(1)
    {
        u16 ReadData;
        
        SVC_WaitVBlankIntr();
        
        ReadData = PAD_Read();
        Trg  = (u16)(ReadData & (ReadData ^ Cont));
        Cont = ReadData;
        
        if ( Trg & PAD_BUTTON_A ) {
            // play wave
            (void)NNS_SndWaveOutStart(
                sWaveOutHandle,
                NNS_SND_WAVE_FORMAT_PCM16,
                sWaveBuffer,
                FALSE,
                0,
                WAVE_SAMPLE,
                (int)( HW_CPU_CLOCK_ARM7 / sMicParam.rate ),
                127,
                32768,
                64
            );
        }
        if ( Trg & PAD_BUTTON_B ) {
            // stop wave
            NNS_SndWaveOutStop( sWaveOutHandle );
        }
        if ( Trg & PAD_BUTTON_L ) {
            // record wave
            OS_Printf("Recording start ...");
#ifdef SDK_TWL
            (void)MIC_StartLimitedSampling( & sMicParam );
#else
            (void)MIC_StartAutoSampling( & sMicParam );
#endif
        }
        
        if ( Cont & PAD_KEY_UP ) {
            NNS_SndWaveOutSetSpeed( sWaveOutHandle, 32768 * 2 );
        }
        else if ( Cont & PAD_KEY_DOWN ){
            NNS_SndWaveOutSetSpeed( sWaveOutHandle, 32768 / 2 );            
        }
        else {
            NNS_SndWaveOutSetSpeed( sWaveOutHandle, 32768 );            
        }
        
        if ( Cont & PAD_KEY_LEFT ) {
            NNS_SndWaveOutSetPan( sWaveOutHandle, 0 );
        }
        else if ( Cont & PAD_KEY_RIGHT ){
            NNS_SndWaveOutSetPan( sWaveOutHandle, 127 );            
        }
        else {
            NNS_SndWaveOutSetPan( sWaveOutHandle, 64 );            
        }
        
        //---- framework
        NNS_SndMain();
    }
}

static void MicCallback( MICResult result, void* arg )
{
#pragma unused( result, arg )    
    OS_Printf( " Done\n" );
}

//---------------------------------------------------------------------------
// VBlank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call 'OS_SetIrqCheckFlag' to check a VBlank interrupt.
//---------------------------------------------------------------------------
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );                   // Checking VBlank interrupt
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         GetI2SFrequency

  Description:  Gets the I2S operating frequency.

  Arguments:    None.

  Returns:      SDNEXFrequency  -   Returns the I2S operating frequency.
 *---------------------------------------------------------------------------*/
static SNDEXFrequency
GetI2SFrequency(void)
{
    SNDEXResult     result;
    SNDEXFrequency  freq;
    s32             retry   =   0;

    while (TRUE)
    {
        result  =   SNDEX_GetI2SFrequency(&freq);
        switch (result)
        {
        case SNDEX_RESULT_SUCCESS:          // Succeeded
            return freq;
        case SNDEX_RESULT_EXCLUSIVE:        // exclusion error
        case SNDEX_RESULT_PXI_SEND_ERROR:   // PXI queue is full
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("Retry count overflow.\n");
            break;
        case SNDEX_RESULT_BEFORE_INIT:      // pre-initialization
        case SNDEX_RESULT_ILLEGAL_STATE:    // abnormal state
            OS_TWarning("Illegal state to get I2S frequency.\n");
            break;
        case SNDEX_RESULT_FATAL_ERROR:      // fatal error
        default:
            OS_Panic("Fatal error (%d).\n", result);
            /* Never return */
        }
    }
    return SNDEX_FREQUENCY_32730;
}
#endif

