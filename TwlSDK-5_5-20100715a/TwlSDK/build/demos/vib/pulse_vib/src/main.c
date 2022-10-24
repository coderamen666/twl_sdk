/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - VIB - pulse_vib
  File:     data.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: $
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "char_data.h"

/*-----------------------------------------------------------------------*
                    Type, Constant
 *-----------------------------------------------------------------------*/

// #define USE_BACKUP

// Number of pulse state save slot
#define SAVE_SLOT_NUM   8

// Current scene
typedef enum
{
    MM_SET_STATE,
    MM_SAVE
}
MenuMode;

typedef struct
{
    VIBPulseState pulse_state[SAVE_SLOT_NUM];
    u32     check_sum;
}
PulseStateBackup;

#define KEY_NUM             14
#define KEYREPEAT_FIRST     25         // Key repeat default interval
#define KEYREPEAT_INTERVAL  5          // Key repeat interval

/*-----------------------------------------------------------------------*
                    Function prototypes
 *-----------------------------------------------------------------------*/

//--- Auto Function Prototype --- Don't comment here
void    NitroMain();
void    read_key_data(void);
static void go_sleep_if_cover_fold(void);
static void VBlankIntr(void);
static void set_pulse_state(void);
static void save_and_load_pulse_state(void);
static void write_pulse_state(VIBPulseState * state);
static u32 *get_state_value_from_line(VIBPulseState * state, s32 line);
static void write_save_words(void);
static void print_message(char *str);
static void print_usage(MenuMode mode);
static void set_first_state(void);
static void read_tp(void);
static void cartridge_pullout_callback(void);
static u32 calc_sum(void *addr, u32 size);
static BOOL start_pulse_vib(VIBPulseState * state);
static void stop_pulse_vib(void);
//--- End of Auto Function Prototype

/*-----------------------------------------------------------------------*
                    Variables
 *-----------------------------------------------------------------------*/

// Key information
static u16 Trg;
static u16 Cont;
static u16 Rpt;

// Touch Panel information
static TPData tp_last;
static TPData tp_pre;
static TPData tp_trg;
static TPData tp_uptrg;

// Pulse vibration save slot
static VIBPulseState save_slot[SAVE_SLOT_NUM];

static s32 line;                       // Line number in menu
static MenuMode menu_mode;             // Current scene
static VIBPulseState pulse_state;      // Current pulse state
static s32 cartridge_pullout_flag;     // If TRUE, Game Pak is pulled out

static u16 backup_lockid;
static BOOL backup_is_enable;

/*-----------------------------------------------------------------------*
                    Global Function Definitions
 *-----------------------------------------------------------------------*/

void NitroMain()
{
    OS_Init();
    GX_Init();

    GX_SetPower(GX_POWER_ALL);

    //---- Set V-Blank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- Clear VRAM
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- Clear OAM and palette
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    // Initialize debug print
    dp_init();

    //---- V-Blank interrupt on
    (void)GX_VBlankIntr(TRUE);

    read_key_data();

    {                                  // Initialize Touch Panel
        TPCalibrateParam calibrate;

        TP_Init();
        (void)TP_GetUserInfo(&calibrate);
        TP_SetCalibrateParam(&calibrate);
    }

    dp_write("NRC PULSE TEST  " __DATE__, LCD_TOP, 1, 1);
    print_usage(MM_SET_STATE);

    backup_lockid = (u16)OS_GetLockID();
    backup_is_enable = FALSE;
#ifdef USE_BACKUP
    CARD_LockBackup(backup_lockid);
    (void)CARD_IdentifyBackup(CARD_BACKUP_TYPE_EEPROM_4KBITS);
    CARD_UnlockBackup(backup_lockid);
#endif

    // Initialize pulse vib
    cartridge_pullout_flag = FALSE;


    if (!VIB_Init())
    {
        dp_set_pltt(1);
        dp_write("NRC NOT INSERTED", LCD_TOP, 1, 20);
        dp_set_pltt(0);
    }
    VIB_SetCartridgePulloutCallback(cartridge_pullout_callback);

    // Initialize variables
    set_first_state();

    //---- Begin display
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();

    while (1)
    {
        //---- Wait for V-Blank interrupt
        OS_WaitVBlankIntr();

        // Main process
        dp_clear_scr(LCD_BOTTOM);

        //---- Display message if Game Pak pullout
        if (cartridge_pullout_flag)
        {
            dp_set_pltt(1);
            dp_write("NRC PULLOUT!!", LCD_TOP, 1, 20);
            dp_set_pltt(0);
        }

        //---- Read pad data
        read_key_data();

        //---- Read Touch Panel data
        read_tp();

        {                              // Display count up
            static u8 count;
            dp_write_byte(count++, LCD_TOP, 29, 1);
        }

        //---- Close cover to sleep
        go_sleep_if_cover_fold();

        switch (menu_mode)
        {
        case MM_SET_STATE:
            set_pulse_state();
            break;
        case MM_SAVE:
            save_and_load_pulse_state();
            break;
        }
    }
}

/*-----------------------------------------------------------------------*
                    Local Function Definition
 *-----------------------------------------------------------------------*/

static void go_sleep_if_cover_fold(void)
{

    // If wake up at Game Pak pullout, sleep again
    while (PAD_DetectFold())
    {
        stop_pulse_vib();
        PM_GoSleepMode(PM_TRIGGER_COVER_OPEN | PM_TRIGGER_CARTRIDGE, 0, 0);
    }
}

static void VBlankIntr(void)
{
    dp_flush();                        // Flush debug print

    //---- Check V-Blank interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

// Process of state change scene
static void set_pulse_state(void)
{

    if (Trg & PAD_KEY_UP)
    {                                  // Select parameter
        if (line == 0)
        {
            line = (s32)(pulse_state.pulse_num * 2 - 1);
        }
        else
        {
            line--;
        }
    }
    else if (Trg & PAD_KEY_DOWN)
    {                                  // Select parameter
        if (line == pulse_state.pulse_num * 2 - 1)
        {
            line = 0;
        }
        else
        {
            line++;
        }
    }
    else if ((Cont & PAD_KEY_LEFT) && (Cont & PAD_BUTTON_R))
    {                                  // Increase value very fast
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num > 101)
        {
            *num -= 100;
        }
        else
        {
            *num = 1;
        }
    }
    else if ((Cont & PAD_KEY_RIGHT) && (Cont & PAD_BUTTON_R))
    {                                  // Decrease value very fast
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num < 99899)
        {
            *num += 100;
        }
        else
        {
            *num = 99999;
        }
    }
    else if ((Cont & PAD_KEY_LEFT) && (Cont & PAD_BUTTON_X))
    {                                  // Increase value fast
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num > 11)
        {
            *num -= 10;
        }
        else
        {
            *num = 1;
        }
    }
    else if ((Cont & PAD_KEY_RIGHT) && (Cont & PAD_BUTTON_X))
    {                                  // Decrease value fast
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num < 99989)
        {
            *num += 10;
        }
        else
        {
            *num = 99999;
        }
    }
    else if ((Trg & PAD_KEY_LEFT) || (Rpt & PAD_KEY_LEFT))
    {                                  // Increase value
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num != 1)
        {
            (*num)--;
        }
    }
    else if ((Trg & PAD_KEY_RIGHT) || (Rpt & PAD_KEY_RIGHT))
    {                                  // Decrease value
        u32    *num = get_state_value_from_line(&pulse_state, line);

        if (*num != 99999)
        {
            (*num)++;
        }
    }
    else if (Trg & PAD_BUTTON_Y)
    {                                  // Change pulse num
        u32     old_pulse_num = pulse_state.pulse_num;

        if (pulse_state.pulse_num == VIB_PULSE_NUM_MAX)
        {
            pulse_state.pulse_num = 1;
        }
        else
        {
            pulse_state.pulse_num++;
        }
        if (line == old_pulse_num * 2 - 1)
        {
            line = (s32)(pulse_state.pulse_num * 2 - 1);
        }
        else if (line >= pulse_state.pulse_num * 2 - 1)
        {
            line = 0;
        }
    }

    if (line == pulse_state.pulse_num * 2 - 1)
    {                                  // Display cursor
        dp_write("@", LCD_BOTTOM, 2, 13);
    }
    else
    {
        dp_write("@", LCD_BOTTOM, 2, line + 1);
    }

    write_pulse_state(&pulse_state);   // Display current pulse state

    if ((Trg & PAD_BUTTON_A) || (tp_trg.touch && tp_trg.x > 128))
    {                                  // Start pulse vibration
        print_message("START PULSE VIB");
        (void)start_pulse_vib(&pulse_state);
    }
    else if (tp_trg.touch && tp_trg.x <= 128)
    {                                  // Start one-time pulse vibration
        VIBPulseState state_temp = pulse_state;
        print_message("START ONE TIME PULSE VIB");
        state_temp.repeat_num = 1;
        (void)start_pulse_vib(&state_temp);
    }
    else if (Trg & PAD_BUTTON_B || tp_uptrg.touch)
    {                                  // Stop pulse vibration
        print_message("STOP PULSE VIB");
        stop_pulse_vib();
    }
    if (Trg & PAD_BUTTON_START)
    {                                  // Move scene to save/load pulse state
        stop_pulse_vib();
        dp_clear(LCD_TOP, 0, 22, 32);
        dp_clear(LCD_TOP, 0, 23, 32);
        print_usage(MM_SAVE);

        // Read from backup
#ifdef USE_BACKUP
        CARD_LockBackup(backup_lockid);
        {
            PulseStateBackup backup;

            if (CARD_ReadEeprom(0, &backup, sizeof(PulseStateBackup)))
            {
                u32     check_sum = calc_sum(backup.pulse_state, sizeof(backup.pulse_state));
                if (check_sum == backup.check_sum)
                {
                    MI_CpuCopyFast(backup.pulse_state, save_slot, sizeof(backup.pulse_state));
                    OS_PutString("Read data from EEPROM.\n");
                    print_message("LOAD DATA FROM BACKUP");
                }
                else
                {
                    OS_PutString("Read data checksum error.\n");
                }
                backup_is_enable = TRUE;
            }
        }
        CARD_UnlockBackup(backup_lockid);
#endif

        menu_mode = MM_SAVE;
        line = 0;
    }
}

// Process of save/load state scene
static void save_and_load_pulse_state(void)
{

    if (Trg & PAD_KEY_UP)
    {                                  // Choose save/load slot
        if (line == 0)
        {
            line = SAVE_SLOT_NUM;
        }
        else
        {
            line--;
        }
    }
    else if (Trg & PAD_KEY_DOWN)
    {                                  // Choose save/load slot
        if (line == SAVE_SLOT_NUM)
        {
            line = 0;
        }
        else
        {
            line++;
        }
    }
    {
        VIBPulseState *selected_state;

        if (line == 0)
        {
            selected_state = &pulse_state;
        }
        else
        {
            selected_state = &save_slot[line - 1];
        }

        if (Trg & PAD_BUTTON_SELECT)
        {                              // Load pulse state
            if (line != 0)
            {
                if (backup_is_enable)
                {
                    print_message("LOAD DATA FROM BACKUP");
                    dp_write_dec(line - 1, LCD_TOP, 23, 22);
                }
                else
                {
                    print_message("LOAD DATA FROM SAVE");
                    dp_write_dec(line - 1, LCD_TOP, 21, 22);
                }
                pulse_state = *selected_state;
            }
            stop_pulse_vib();
            menu_mode = MM_SET_STATE;
            print_usage(MM_SET_STATE);
            line = 0;
            return;
        }
        else if (Trg & PAD_BUTTON_START)
        {                              // Save pulse state
            stop_pulse_vib();
            dp_clear(LCD_TOP, 0, 22, 32);
            dp_clear(LCD_TOP, 0, 23, 32);
            print_usage(MM_SET_STATE);

            if (line != 0)
            {
                *selected_state = pulse_state;

                if (backup_is_enable)
                {
                    PulseStateBackup backup;

                    MI_CpuCopyFast(save_slot, backup.pulse_state, sizeof(backup.pulse_state));
                    backup.check_sum = calc_sum(backup.pulse_state, sizeof(backup.pulse_state));

                    CARD_LockBackup(backup_lockid);
                    if (CARD_WriteAndVerifyEeprom(0, &backup, sizeof(PulseStateBackup)))
                    {
                        OS_PutString("Write data to EEPROM.\n");
                    }
                    CARD_UnlockBackup(backup_lockid);
                    print_message("SAVE CURRENT DATA TO BACKUP");
                    dp_write_dec(line - 1, LCD_TOP, 29, 22);
                }
                else
                {
                    print_message("SAVE CURRENT DATA TO SAVE");
                    dp_write_dec(line - 1, LCD_TOP, 27, 22);
                }
            }

            menu_mode = MM_SET_STATE;
            line = 0;
            return;
        }

        if ((Trg & PAD_BUTTON_A) || (tp_trg.touch && tp_trg.x > 128))
        {                              // Start pulse vibration
            if (line == 0)
            {
                print_message("START PULSE VIB");
            }
            else
            {
                print_message("START PULSE VIB SAVE");
                dp_write_dec(line - 1, LCD_TOP, 22, 22);
            }
            (void)start_pulse_vib(selected_state);
        }
        else if (tp_trg.touch && tp_trg.x <= 128)
        {                              // Start one-time pulse vibration
            VIBPulseState state_temp = *selected_state;
            if (line == 0)
            {
                print_message("START ONE TIME PULSE VIB");
            }
            else
            {
                print_message("START ONE TIME PULSE VIB SAVE");
                dp_write_dec(line - 1, LCD_TOP, 31, 22);
            }
            state_temp.repeat_num = 1;
            (void)start_pulse_vib(&state_temp);
        }
        else if (Trg & PAD_BUTTON_B || (tp_uptrg.touch && tp_uptrg.x > 128))
        {                              // Stop pulse vibration
            print_message("STOP PULSE VIB");
#ifdef USE_VIB
            stop_pulse_vib();
#endif
        }

        write_pulse_state(selected_state);      // Display current pulse state
    }
    dp_write("@", LCD_BOTTOM, 20, line + 1);    // Display cursor
    write_save_words();                // Display save slot words
}

// Display pulse state
static void write_pulse_state(VIBPulseState * state)
{

    s32     i;

    // Display on/off time length
    for (i = 0; i < state->pulse_num; i++)
    {
        double  time;
        char    buf[32];

        time = (double)state->on_time[i];
        time = time / 10;

        (void)snprintf(buf, 32, "%6.1f", time);

        if (state->on_time[i] > VIB_ON_TIME_MAX)
            dp_set_pltt(1);
        dp_write(buf, LCD_BOTTOM, 9, i * 2 + 1);
        if (state->on_time[i] > VIB_ON_TIME_MAX)
            dp_set_pltt(0);

        time = (double)state->off_time[i];
        time = time / 10;

        (void)snprintf(buf, 32, "%6.1f", time);

        if (state->on_time[i] > state->off_time[i])
            dp_set_pltt(1);
        dp_write(buf, LCD_BOTTOM, 9, i * 2 + 2);
        if (state->on_time[i] > state->off_time[i])
            dp_set_pltt(0);

        dp_write(" ON  :", LCD_BOTTOM, 3, i * 2 + 1);
        dp_write("OFF  :", LCD_BOTTOM, 3, i * 2 + 2);
        dp_write_dec(i + 1, LCD_BOTTOM, 7, i * 2 + 1);
        dp_write_dec(i + 1, LCD_BOTTOM, 7, i * 2 + 2);
        dp_write("ms", LCD_BOTTOM, 15, i * 2 + 1);
        dp_write("ms", LCD_BOTTOM, 15, i * 2 + 2);
    }
    dp_clear(LCD_BOTTOM, 0, (s32)state->pulse_num * 2, 32);

    // Display rest time
    {
        double  time;
        char    buf[32];

        time = (double)state->rest_time;
        time = time / 10;

        (void)snprintf(buf, 32, "%6.1f", time);

        dp_write(" REST:", LCD_BOTTOM, 3, 13);
        if (state->rest_time < VIB_REST_TIME_MIN)
            dp_set_pltt(1);
        dp_write(buf, LCD_BOTTOM, 9, 13);
        if (state->rest_time < VIB_REST_TIME_MIN)
            dp_set_pltt(0);
        dp_write("ms", LCD_BOTTOM, 15, 13);
    }

    // Display pulse num
    dp_write_dec((s32)state->pulse_num, LCD_BOTTOM, 13, 16);
    dp_write("PULSE NUM:", LCD_BOTTOM, 2, 16);
}

// Get pointer to pointed parameter by cursor
static u32 *get_state_value_from_line(VIBPulseState * state, s32 line)
{

    if (line == pulse_state.pulse_num * 2 - 1)
    {
        return &(state->rest_time);
    }
    else
    {
        if (line & 1)
        {
            return &(state->off_time[line >> 1]);
        }
        else
        {
            return &(state->on_time[line >> 1]);
        }
    }
}

// Display save slot words
static void write_save_words(void)
{

    dp_write("CURRENT", LCD_BOTTOM, 22, 1);
    dp_write("SAVE 0 ", LCD_BOTTOM, 22, 2);
    dp_write("SAVE 1 ", LCD_BOTTOM, 22, 3);
    dp_write("SAVE 2 ", LCD_BOTTOM, 22, 4);
    dp_write("SAVE 3 ", LCD_BOTTOM, 22, 5);
    dp_write("SAVE 4 ", LCD_BOTTOM, 22, 6);
    dp_write("SAVE 5 ", LCD_BOTTOM, 22, 7);
    dp_write("SAVE 6 ", LCD_BOTTOM, 22, 8);
    dp_write("SAVE 7 ", LCD_BOTTOM, 22, 9);
}

// Display message text
static void print_message(char *str)
{
    dp_clear(LCD_TOP, 0, 22, 32);
    dp_clear(LCD_TOP, 0, 23, 32);
    dp_write(str, LCD_TOP, 1, 22);
}

// Display usage
static void print_usage(MenuMode mode)
{

    dp_clear(LCD_TOP, 0, 3, 32 * 10);
    dp_write("A: START PULSE VIB", LCD_TOP, 1, 3);
    dp_write("B: STOP PULSE VIB", LCD_TOP, 1, 4);

    switch (mode)
    {
    case MM_SET_STATE:
        dp_write("Y: CHANGE PULSE NUM", LCD_TOP, 1, 5);
        dp_write("X: PUSH TO SPEEDUP LEFT/RIGHT", LCD_TOP, 1, 6);
        dp_write("R: PUSH TO SPEED+++ LEFT/RIGHT", LCD_TOP, 1, 7);
        dp_write("   UP/DOWN: CHOOSE PARAM", LCD_TOP, 1, 8);
        dp_write("LEFT/RIGHT: CHANGE PARAM NUM", LCD_TOP, 1, 9);
        dp_write("     START: ENTER SAVE MENU", LCD_TOP, 1, 10);
        break;
    case MM_SAVE:
        dp_write("UP/DOWN: CHOOSE SAVE SLOT", LCD_TOP, 1, 5);
        dp_write("  START: SAVE DATA AND EXIT", LCD_TOP, 1, 6);
        dp_write(" SELECT: LOAD DATA AND EXIT", LCD_TOP, 1, 7);
        break;
    }

    dp_write("TOUCH RIGHT SIDE:", LCD_TOP, 1, 12);
    dp_write("     CONTINUOUS PULSE VIB", LCD_TOP, 1, 13);
    dp_write("TOUCH LEFT SIDE:", LCD_TOP, 1, 14);
    dp_write("     ONE TIME PULSE VIB", LCD_TOP, 1, 15);
}

// Initialize variables
static void set_first_state(void)
{

    u32     i;

    pulse_state.pulse_num = 2;
    pulse_state.rest_time = 500;
    pulse_state.repeat_num = 0;

    pulse_state.on_time[0] = 15;
    pulse_state.on_time[1] = 15;
    pulse_state.on_time[2] = 15;
    pulse_state.on_time[3] = 15;
    pulse_state.on_time[4] = 15;
    pulse_state.on_time[5] = 15;

    pulse_state.off_time[0] = 15;
    pulse_state.off_time[1] = 15;
    pulse_state.off_time[2] = 15;
    pulse_state.off_time[3] = 15;
    pulse_state.off_time[4] = 15;
    pulse_state.off_time[5] = 15;

    MI_CpuClearFast(save_slot, sizeof(save_slot));

    for (i = 0; i < SAVE_SLOT_NUM; i++)
    {
        save_slot[i].pulse_num = VIB_PULSE_NUM_MAX;
        save_slot[i].rest_time = 100;
        save_slot[i].repeat_num = 0;

        save_slot[i].on_time[0] = 10;
        save_slot[i].on_time[1] = 10;
        save_slot[i].on_time[2] = 10;
        save_slot[i].on_time[3] = 10;
        save_slot[i].on_time[4] = 10;
        save_slot[i].on_time[5] = 10;

        save_slot[i].off_time[0] = 10;
        save_slot[i].off_time[1] = 10;
        save_slot[i].off_time[2] = 10;
        save_slot[i].off_time[3] = 10;
        save_slot[i].off_time[4] = 10;
        save_slot[i].off_time[5] = 10;
    }
}

// Read Touch Panel data
static void read_tp(void)
{

    TPData  tp_raw;

    tp_pre = tp_last;

    while (TP_RequestRawSampling(&tp_raw) != 0)
    {
    }
    TP_GetCalibratedPoint(&tp_last, &tp_raw);

    if (tp_last.validity != TP_VALIDITY_VALID)
    {
        tp_last = tp_pre;
        tp_last.touch = 0;
    }

    if (tp_last.touch && (!tp_pre.touch))
    {
        tp_trg = tp_last;
    }
    else
    {
        tp_trg.touch = FALSE;
    }

    if ((!tp_last.touch) && tp_pre.touch)
    {
        tp_uptrg = tp_pre;
    }
    else
    {
        tp_uptrg.touch = FALSE;
    }
}

// If Game Pak pulled out, display caution and set flag
static void cartridge_pullout_callback(void)
{

    cartridge_pullout_flag = TRUE;
}

static u32 calc_sum(void *addr, u32 size)
{

    u32     i;
    u32     times = size / 4;
    u32    *num_by_4byte = addr;
    u32     sum = 0;

    for (i = 0; i < times; i++)
    {
        sum += *num_by_4byte;
        num_by_4byte++;
    }

    return sum;
}

static BOOL start_pulse_vib(VIBPulseState * state)
{
    int     i;

    /* Does the ON time exceed VIB_ON_TIME_MAX? */
    for (i = 0; i < state->pulse_num; i++)
    {
        if (state->on_time[i] > VIB_ON_TIME_MAX)
        {
            VIB_StopPulse();
            dp_set_pltt(1);
            dp_write("ON time[", LCD_TOP, 1, 22);
            dp_write_dec(i + 1, LCD_TOP, 9, 22);
            dp_write("] is over", LCD_TOP, 10, 22);
            dp_write("VIB_ON_TIME_MAX", LCD_TOP, 1, 23);
            dp_set_pltt(0);
            return FALSE;
        }
    }
    /* Does the OFF time exceed the previous ON time? */
    for (i = 0; i < state->pulse_num - 1; i++)
    {
        if (state->on_time[i] > state->off_time[i])
        {
            VIB_StopPulse();
            dp_set_pltt(1);
            dp_write("ON time[", LCD_TOP, 1, 22);
            dp_write_dec(i + 1, LCD_TOP, 9, 22);
            dp_write("] is over OFF time[", LCD_TOP, 10, 22);
            dp_write_dec(i + 1, LCD_TOP, 29, 22);
            dp_write("]", LCD_TOP, 30, 22);
            dp_set_pltt(0);
            return FALSE;
        }
    }
    /* Is the REST time less than VIB_REST_TIME_MIN? */
    if (state->rest_time < VIB_REST_TIME_MIN)
    {
        VIB_StopPulse();
        dp_set_pltt(1);
        dp_write("REST time is less than", LCD_TOP, 1, 22);
        dp_write("VIB_REST_TIME_MIN", LCD_TOP, 1, 23);
        dp_set_pltt(0);
        return FALSE;
    }

    VIB_StartPulse(state);
    return TRUE;
}

static void stop_pulse_vib(void)
{

    VIB_StopPulse();
}

void read_key_data(void)
{

    int     i;
    u16     keyData;
    u16     bit;
    static u8 keyrep_count[16];

    //---- Load pad data
    keyData = PAD_Read();
    Trg = (u16)(keyData & (keyData ^ Cont));
    Cont = keyData;
    Rpt = 0;
    for (i = 0; i < KEY_NUM; ++i)
    {
        if (!(Cont & (bit = (u16)(PAD_BUTTON_A << i))))
        {
            keyrep_count[i] = 0;
            continue;
        }
        keyrep_count[i]++;
        if (keyrep_count[i] == KEYREPEAT_FIRST)
        {
            Rpt |= bit;
            continue;
        }
        if (keyrep_count[i] == KEYREPEAT_FIRST + KEYREPEAT_INTERVAL)
        {
            Rpt |= bit;
            keyrep_count[i] = KEYREPEAT_FIRST;
        }
    }
}
