/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - SubBanner
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2011-04-12#$
  $Rev: 11403 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/na.h>
#include <DEMO.h>

#define MENU_ELEMENT_NUM 4  // Number of menu items
#define MENU_TEXT_SIZE 8

#define COLOR_WHITE GX_RGBA(31, 31, 31, 1)
#define COLOR_RED   GX_RGBA(31,  0,  0, 1)
#define COLOR_GREEN GX_RGBA( 0, 31,  0, 1)
#define COLOR_BLUE  GX_RGBA( 0,  0, 31, 1)
#define COLOR_BLACK GX_RGBA( 0,  0,  0, 1)

// Menu element coordinates
typedef struct MenuPos
{
    BOOL   enable;
    int    x;
    int    y;
}MenuPos;

// Structure holding parameters that determine menu composition
typedef struct MenuParam
{
    int      num;
    GXRgb    normal_color;
    GXRgb    select_color;
    GXRgb    disable_color;
    u16      padding;
    MenuPos *pos;
    const char **str_elem;
}MenuParam;

static const char *pStrMenu[ MENU_ELEMENT_NUM ] = 
{
    "write sub banner file",
    "edit sub banner file",
    "delete sub banner file",
    "return to launcher",
};

static MenuPos menuPos[] =
{
    { TRUE,  3,   8 },
    { TRUE,  3,   9 },
    { TRUE,  3,   10 },
    { TRUE,  3,   11 },
};

static const MenuParam menuParam =
{
    MENU_ELEMENT_NUM,
    COLOR_BLACK,
    COLOR_GREEN,
    COLOR_RED,
    0,
    &menuPos[ 0 ],
    (const char **)&pStrMenu,
};

static void PrintBootType();
static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

#define MESSAGE_SIZE 50

static char message[MESSAGE_SIZE]="";

/*---------------------------------------------------------------------------*
  Name:         EditSubBanner

  Description:  Switches the sub-banner's RGB colors and flips it in the horizontal direction.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL EditSubBanner(NASubBanner* banner)
{
    int i;
    NASubBannerAnime* anime = &banner->anime;
    
    if (!NA_LoadSubBanner(banner))
    {
        return FALSE;
    }

    // Rotate the RGB values for the palette data
    for (i = 0; i < NA_SUBBANNER_PALETTE_NUM; i++)
    {
        anime->pltt[0][i] = (GXRgb)((GX_RGB_R_MASK & anime->pltt[0][i]) << 5 |  // R to G
                                    (GX_RGB_G_MASK & anime->pltt[0][i]) << 5 |  // G to B
                                    (GX_RGB_B_MASK & anime->pltt[0][i]) >> 10); // B to R
    }

    NA_MakeSubBannerHeader(banner);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    int cursor = 0;
    int i;

    OS_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();

    // Print usage
    PrintBootType();
    OS_TPrintf("=================================\n");
    OS_TPrintf("USAGE: SubBanner demo\n");
    OS_TPrintf(" UP DOWN : select menu\n");
    OS_TPrintf(" A       : execute menu\n");
    OS_TPrintf("=================================\n");

    for (;;)
    {
        DEMOReadKey();
        if( DEMO_IS_TRIG(PAD_KEY_DOWN) )
        {   // Move cursor
            if( ++cursor == MENU_ELEMENT_NUM )
            {
                cursor=0;
            }
        }
        if( DEMO_IS_TRIG(PAD_KEY_UP) )
        {
            if( --cursor & 0x80 )
            {
                cursor=MENU_ELEMENT_NUM - 1;
            }
        }

        // Render the menu
        {
            const MenuParam *param = &menuParam;

            DEMOFillRect(0, 0, 256, 192, COLOR_WHITE);
            DEMOSetBitmapGroundColor(COLOR_WHITE);
            DEMOSetBitmapTextColor( COLOR_BLUE );
            DEMODrawText( 1 * MENU_TEXT_SIZE, 0 * MENU_TEXT_SIZE, "SubBanner");

            for( i = 0; i < param->num; i++)
            {
                DEMOSetBitmapTextColor( i == cursor ? param->select_color : param->normal_color );
                DEMODrawText( param->pos[i].x * MENU_TEXT_SIZE, param->pos[i].y * MENU_TEXT_SIZE,
                            ( i == cursor ? "=>%s" : "  %s" ), param->str_elem[i] );
            }

            DEMOSetBitmapTextColor( COLOR_RED );
            DEMODrawText( 1 * MENU_TEXT_SIZE, 3 * MENU_TEXT_SIZE, message);
        }

        if( DEMO_IS_TRIG(PAD_BUTTON_A) )
        {    // Branch to menu items
             switch( cursor )
             {
                 NASubBanner subBanner;
                 FSFile file[1];
                 s32 readLen;
               case 0:
                 //Write the sub-banner
                 FS_InitFile(file);
                 if (FS_OpenFileEx(file, "rom:/sub_banner.bnr", FS_FILEMODE_R) )
                 {
                     readLen = FS_ReadFile(file, &subBanner, sizeof(NASubBanner));
                     (void)FS_CloseFile(file);
                     if( readLen == sizeof(NASubBanner) )
                     {
                         // Success
                         if ( NA_SaveSubBanner( &subBanner ) )
                         {
                             (void)STD_CopyLString( message, "write succeed.", MESSAGE_SIZE );
                         }
                         else
                         {
                             (void)STD_CopyLString( message, "write failed.", MESSAGE_SIZE );
                         }
                     }
                     else
                     {
                         (void)STD_CopyLString( message, "read rom failed.", MESSAGE_SIZE );
                     }
                 }
                 else
                 {
                     (void)STD_CopyLString( message, "open rom failed.", MESSAGE_SIZE );
                 }
                 break;
               case 1:
                 if (EditSubBanner( &subBanner ))
                 {   // Success
                     if ( NA_SaveSubBanner( &subBanner ) )
                     {
                         (void)STD_CopyLString( message, "edit succeed.", MESSAGE_SIZE );
                     }
                     else
                     {
                         (void)STD_CopyLString( message, "edit failed.", MESSAGE_SIZE );
                     }
                 }
                 else
                 {   // Failed
                     (void)STD_CopyLString( message, "load failed.", MESSAGE_SIZE );
                 }
                 break;
               case 2:
                 //Delete the sub-banner
                 if ( NA_DeleteSubBanner(&subBanner) )
                 {
                     (void)STD_CopyLString( message, "delete succeed", MESSAGE_SIZE );
                 }
                 else
                 {
                     (void)STD_CopyLString( message, "delete failed", MESSAGE_SIZE );
                 }
                 break;
               case 3:
                 (void)OS_JumpToSystemMenu();
                 //Restart
                 break;
             }
        }
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  Prints the BootType.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
      case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType:OS_BOOTTYPE_ROM\n"); break;
      case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType:OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  Configures display settings for console screen output.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // Initialize screen display
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  Initializes interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // Enable master interrupt flag
    (void)OS_EnableIrq();

    // Allow IRQ interrupts
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  Creates a heap and makes OS_Alloc usable.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // Initialize the main arena's allocation system
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // Create a heap in the main arena
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  Initializes the file system and makes the ROM accessible.
                The InitInteruptSystem function must have been called before this function is.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );
}
