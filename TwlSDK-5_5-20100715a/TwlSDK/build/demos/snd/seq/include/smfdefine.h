/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - seq
  File:     smfdefine.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef SMFDEFINE_H_
#define SMFDEFINE_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

/*------------------------------------------------------------------*/
/*                                                                  */
/* MIDI data structure definition                                               */
/*                                                                  */
/*------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/* Definition used for MIDI standard                                               */
/*--------------------------------------------------------------------*/

/* Upper 4-bit definition */
#define MIDI_NOTEOFF            0x80   /* Note off */
#define MIDI_NOTEON             0x90   /* Note on */
#define MIDI_POLYPRESS          0xA0   /* Key polyphonic pressure */
#define MIDI_CONTROLCHANGE      0xB0   /* Control change */
#define MIDI_PROGRAMCHANGE      0xC0   /* Program change */
#define MIDI_CANNELPRESS        0xD0   /* Channel pressure */
#define MIDI_PITCHBEND          0xE0   /* Pitch bend */
#define MIDI_SYSTEMMESSAGE      0xF0   /* System message */

/* MIDI_SYSTEMMESSAGE */
#define MIDI_SYSX               0xF0   /* Exclusive message */
/* COMMON MESSAGE */
#define MIDI_MTC                0xF1   /* MIDI time code */
#define MIDI_SONG_POSITION      0xF2   /* Song position pointer */
#define MIDI_SONG_SELECT        0xF3   /* Song select */
#define MIDI_UNDEFINED_F4       0xF4   /* Undefined */
#define MIDI_UNDEFINED_F5       0xF5   /* Undefined */
#define MIDI_TUNE_REQUEST       0xF6   /* Tune request */
#define MIDI_END_OF_SYSX        0xF7   /* End of exclusive */
/* REALTIME MESSAGE */
#define MIDI_TIMING_CLOCK       0xF8   /* Timing clock */
#define MIDI_UNDEFINED_F9       0xF9   /* Undefined */
#define MIDI_START              0xFA   /* Start */
#define MIDI_CONTINUE           0xFB   /* Continue */
#define MIDI_STOP               0xFC   /* Stop */
#define MIDI_UNDEFINED_FD       0xFD   /* Undefined */
#define MIDI_ACTIVESENSING      0xFE   /* Active sensing */
#define MIDI_SYSTEMRESET        0xFF   /* System reset */

/* Control Change */
/* Continuous cc */
#define MIDI_CC_BANKSELECT_MSB  0x00   /*   0: Bank select */
#define MIDI_CC_BANKSELECT_LSB  0x20   /*  32: Bank select */
#define MIDI_CC_MODURATION      0x01   /*   1: Modulation wheel */
#define MIDI_CC_BREATH_CONTROL  0x02   /*   2: Breath control */
#define MIDI_CC_FOOT_CONTROL    0x04   /*   4: Foot control */
#define MIDI_CC_PORTAMENT_TIME  0x05   /*   5: Portamento time */
#define MIDI_CC_VOLUME          0x07   /*   7: Volume */
#define MIDI_CC_BALANCE         0x08   /*   8: Balance */
#define MIDI_CC_PAN             0x0A   /*  10: Pan */
#define MIDI_CC_EXPRESSION      0x0B   /*  11: Expression */
#define MIDI_CC_GENERAL1        0x10   /*  16: General purpose 1 */
#define MIDI_CC_GENERAL2        0x11   /*  17: General purpose 2 */
#define MIDI_CC_GENERAL3        0x12   /*  18: General purpose 3 */
#define MIDI_CC_GENERAL4        0x13   /*  19: General purpose 4 */
/* switched cc */
#define MIDI_CC_HOLD1           0x40   /*  64: Hold (damper) */
#define MIDI_CC_PORTAMENT       0x41   /*  65: Portamento switch */
#define MIDI_CC_SOSTENUTO       0x42   /*  66: Sostenuto */
#define MIDI_CC_SOFT_PEDAL      0x43   /*  67: Soft pedal */
#define MIDI_CC_LEGATO          0x44   /*  68: Legato foot switch */
#define MIDI_CC_HOLD2           0x45   /*  69: Hold (freeze) */
/* Sound controller cc */
#define MIDI_CC_SOUND_CONTROL1  0x46   /*  70: Sound controller 1 */
#define MIDI_CC_SOUND_CONTROL2  0x47   /*  71: Sound controller 2 */
#define MIDI_CC_SOUND_CONTROL3  0x48   /*  72: Sound controller 3 */
#define MIDI_CC_SOUND_CONTROL4  0x49   /*  73: Sound controller 4 */
#define MIDI_CC_SOUND_CONTROL5  0x4A   /*  74: Sound controller 5 */
#define MIDI_CC_SOUND_CONTROL6  0x4B   /*  75: Sound controller 6 */
#define MIDI_CC_SOUND_CONTROL7  0x4C   /*  76: Sound controller 7 */
#define MIDI_CC_SOUND_CONTROL8  0x4D   /*  77: Sound controller 8 */
#define MIDI_CC_SOUND_CONTROL9  0x4E   /*  78: Sound controller 9 */
#define MIDI_CC_SOUND_CONTROL10 0x4F   /*  79: Sound controller 10 */
#define MIDI_CC_SOUND_VARIATION 0x46   /*  70: Sound variation */
#define MIDI_CC_RESONANCE       0x47   /*  71: Resonance */
#define MIDI_CC_RELEASE_TIME    0x48   /*  72: Release time */
#define MIDI_CC_ATTACK_TIME     0x49   /*  73: Attack time */
#define MIDI_CC_BRIGHTNESS      0x4A   /*  74: Filter cut-off */
#define MIDI_CC_DECAY_TIME      0x4B   /*  75: Decay time */
#define MIDI_CC_VIBRATO_RATE    0x4C   /*  76: Vibrato rate (speed) */
#define MIDI_CC_VIBRATO_DEPTH   0x4D   /*  77: Vibrato depth */
#define MIDI_CC_VIBRATO_DELAY   0x4E   /*  78: Vibrato delay */
/* Portament_control */
#define MIDI_CC_PORTAMENT_CTRL  0x54   /*  84: Portamento control */
/* Effect controller cc */
#define MIDI_CC_EFFECT_CONTROL1 0x0C   /*  12: Effect controller 1 */
#define MIDI_CC_EFFECT_CONTROL2 0x0D   /*  13: Effect controller 2 */
#define MIDI_CC_REVERB_SEND     0x5B   /*  91: Reverb send level */
#define MIDI_CC_TREMOLO_DEPTH   0x5C   /*  92: Tremolo depth */
#define MIDI_CC_CHORUS_SEND     0x5D   /*  93: Chorus send level */
#define MIDI_CC_DELAY_SEND      0x5E   /*  94: Delay send level */
#define MIDI_CC_PHASER_DEPTH    0x5F   /*  95: Phaser depth */
/* Parameter control */
#define MIDI_CC_DATA_ENTRY_MSB  0x06   /*   6: Data entry */
#define MIDI_CC_DATA_ENTRY_LSB  0x26   /*  38: Data entry */
#define MIDI_CC_DATA_INCREMENT  0x60   /*  96: Increment */
#define MIDI_CC_DATA_DECREMENT  0x61   /*  97: Decrement */
#define MIDI_CC_RPN_MSB         0x65   /* 101: RPN MSB */
#define MIDI_CC_RPN_LSB         0x64   /* 100: RPN LSB */
#define MIDI_CC_NRPN_MSB        0x63   /*  99: NRPN MSB */
#define MIDI_CC_NRPN_LSB        0x62   /*  98: NRPN LSB */
/* Mode message */
#define MIDI_CC_ALL_SOUND_OFF   0x78   /* 120: All sound off */
#define MIDI_CC_RESET_ALL_CTRL  0x79   /* 121: Reset all controllers */
#define MIDI_CC_ROCAL_CONTROL   0x7A   /* 122: Local control */
#define MIDI_CC_ALL_NOTE_OFF    0x7B   /* 123: All notes off */
#define MIDI_CC_OMNI_ON         0x7C   /* 124: Omni-mode on */
#define MIDI_CC_OMNI_OFF        0x7D   /* 125: Omni-mode off */
#define MIDI_CC_MONO_MODE       0x7E   /* 126: Mono-mode on */
#define MIDI_CC_POLY_MODE       0x7F   /* 127: Poly-mode on */
/* RPN */
#define MIDI_RPN_PITCHBEND_SENS 0x0000 /* RPN: Pitch bend sensitivity */
#define MIDI_RPN_FINE_TUNE      0x0001 /* RPN: Fine tune */
#define MIDI_RPN_COASE_TUNE     0x0002 /* RPN: Coarse tune */
#define MIDI_RPN_TUNING_PROGRAM 0x0003 /* RPN: Tuning program select */
#define MIDI_RPN_TUNING_BANK    0x0004 /* RPN: Tuning bank select */
#define MIDI_RPN_MODURATION_SENS 0x0005 /* RPN: Modulation sensitivity */



/*--------------------------------------------------------------------*/
/* Definitions for standard MIDI File format                        */
/*--------------------------------------------------------------------*/

#define SMF_FORMAT0             0      /* SMF format0 */
#define SMF_FORMAT1             1      /* SMF format1 */
#define SMF_FORMAT2             2      /* SMF format2 */

#define SMF_NOTEOFF             0x80   /* Note off */
#define SMF_NOTEON              0x90   /* Note on */
#define SMF_POLYPRESS           0xA0   /* Key polyphonic pressure */
#define SMF_CONTROLCHANGE       0xB0   /* Control change */
#define SMF_PROGRAMCHANGE       0xC0   /* Program change */
#define SMF_CANNELPRESS         0xD0   /* Channel pressure */
#define SMF_PITCHBEND           0xE0   /* Pitch bend */
#define SMF_SYSX                0xF0   /* Exclusive message */
#define SMF_MTC                 0xF1   /* MIDI time code */
#define SMF_SONGPOSITION        0xF2   /* Song position pointer */
#define SMF_SONGSELECT          0xF3   /* Song select */
#define SMF_UNDEFINED_F4        0xF4   /* Undefined */
#define SMF_UNDEFINED_F5        0xF5   /* Undefined */
#define SMF_TUNEREQUEST         0xF6   /* Tune request */
#define SMF_ENDOFSYSX           0xF7   /* End of exclusive */
#define SMF_TIMINGCLOCK         0xF8   /* Timing clock */
#define SMF_UNDEFINED_F9        0xF9   /* Undefined */
#define SMF_START               0xFA   /* Start */
#define SMF_CONTINUE            0xFB   /* Continue */
#define SMF_STOP                0xFC   /* Stop */
#define SMF_UNDEFINED_FD        0xFD   /* Undefined */
#define SMF_ACTIVESENSING       0xFE   /* Active sensing */
#define SMF_META                0xFF   /* Meta-event */

#define SMF_META_SEQ_NUM        0x00   /* Sequence number */
#define SMF_META_TEXT           0x01   /* Text event */
#define SMF_META_COPYRIGHT      0x02   /* Display copyrights */
#define SMF_META_SEQ_NAME       0x03   /* Sequence name / track name */
#define SMF_META_INST_NAME      0x04   /* +Instrument name */
#define SMF_META_LYRIC          0x05   /* Lyrics */
#define SMF_META_MARKER         0x06   /* Marker */
#define SMF_META_QUE            0x07   /* Queue point */
#define SMF_META_PREFIX         0x20   /* Channel prefix */
#define SMF_META_ENDOFTRACK     0x2F   /* End of track */
#define SMF_META_TEMPO          0x51   /* Set tempo */
#define SMF_META_SMPTE          0x54   /* SMPTE offset */
#define SMF_META_BEAT           0x58   /* Tempo / Metronome settings */
#define SMF_META_KEY            0x59   /* Pitch */
#define SMF_META_OTHER          0x7F   /* Sequencer specific meta event */


#ifdef __cplusplus
extern "C" {
#endif
/*------------------------------------------------------------------*/
/* Structure Definitions                                                       */
/*------------------------------------------------------------------*/

/* SMF chunk tag structure */
typedef struct tSMFChunkTag
{
    char    chunkType[4];              /* Chunk type string (MThd, MTrk) */
    unsigned long length;              /* Data length of the chunk */
}
SMFChunkTag;

/* Header chunk structure. */
typedef struct tSMFHeaderChunk
{
    unsigned short format;             /* File format */
    unsigned short ntracks;            /* Track count */
    unsigned short division;           /* Quarter note resolution. */
}
SMFHeaderChunk;


#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif // SMFDEFINE_H_
