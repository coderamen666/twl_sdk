/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WM - demos - wireless-all
  File:     common.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: common.h,v $
  Revision 1.2  2006/04/10 13:37:12  yosizaki
  Cleaned up source code indent.

  Revision 1.1  2006/04/10 13:07:32  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef WM_DEMO_WIRELESS_ALL_COMMON_H_
#define WM_DEMO_WIRELESS_ALL_COMMON_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "mbp.h"
#include "wh.h"
#include "wfs.h"


/*****************************************************************************/
/* declaration */

/* Key input structure used for data sharing */
typedef struct
{
    u32     count;                     /* Frame info */
    u16     key;                       /* Key information */
    u16     padding;
}
GShareData;

SDK_COMPILER_ASSERT(sizeof(GShareData) <= WH_DS_DATA_SIZE);


/*****************************************************************************/
/* Constants */

/* Palette color for text */
enum
{
    PLTT_BLACK = 0,
    PLTT_BLUE = 1,
    PLTT_RED = 2,
    PLTT_PURPLE = 3,
    PLTT_GREEN = 4,
    PLTT_CYAN = 5,
    PLTT_YELLOW = 6,
    PLTT_WHITE = 7,

    PLTT_LINK_ICON                     /* Palette for the wireless link strength display icon */
};


/*****************************************************************************/
/* Functions */

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         InitCommon

  Description:  Common initialization functions

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    InitCommon(void);

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_PRESS

  Description:  Determines own key status.

  Arguments:    The key flag for determination.

  Returns:      TRUE if the specified key is being pressed; FALSE otherwise.
                
 *---------------------------------------------------------------------------*/
BOOL    IS_PAD_PRESS(u16 flag);

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_TRIGGER

  Description:  Determines own key trigger status.

  Arguments:    The key flag for determination.

  Returns:      TRUE if specified key trigger is enabled; FALSE if not enabled.
                
 *---------------------------------------------------------------------------*/
BOOL    IS_PAD_TRIGGER(u16 flag);

/*---------------------------------------------------------------------------*
  Name:         GetCurrentInput

  Description:  Gets current key input of the specified player.

  Arguments:    aid -- The player acquiring the key input.

  Returns:      Returns a valid address if the latest data sharing succeeds; otherwise, returns NULL.
                
 *---------------------------------------------------------------------------*/
GShareData *GetCurrentInput(int aid);

/*---------------------------------------------------------------------------*
  Name:         GetCurrentChannel

  Description:  Gets the currently selected wireless channel.

  Arguments:    None.

  Returns:      Returns the currently selected wireless channel.
 *---------------------------------------------------------------------------*/
u16     GetCurrentChannel(void);

/*---------------------------------------------------------------------------*
  Name:         SetCurrentChannel

  Description:  Sets the current wireless channel.

  Arguments:    channel -- Channel to be set.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    SetCurrentChannel(u16 channel);

/*---------------------------------------------------------------------------*
  Name:         LoadLinkIcon

  Description:  Loads the link icon into VRAM.

  Arguments:    id -- The load target's character ID.
                palette -- The load target's palette.
                level -- The link strength.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    LoadLinkIcon(int id, int palette, int level);

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Draws the background text.

  Arguments:    x -- The rendering x grid position.
                y -- The rendering y grid position.
                palette -- The palette index.
                format -- The format string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PrintString(int x, int y, int palette, const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         WaitNextFrame

  Description:  Initializes rendering while waiting for the next picture frame.

  Arguments:    None.

  Returns:      Returns TRUE when key input is updated.
 *---------------------------------------------------------------------------*/
BOOL    WaitNextFrame(void);

/*---------------------------------------------------------------------------*
  Name:         WaitWHState

  Description:  Waits until entering the specified WH state.

  Arguments:    state -- The state whose transition is awaited.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WaitWHState(int state);

/*---------------------------------------------------------------------------*
  Name:         MeasureChannel

  Description:  Measures the wireless use rates for each useable channel.
                Blocks and displays internally until completed.

  Arguments:    None.

  Returns:      The channel with the lowest rate of wireless use.
 *---------------------------------------------------------------------------*/
u16     MeasureChannel(void);

/*---------------------------------------------------------------------------*
  Name:         ExecuteDownloadServer

  Description:  Runs DS Download Play parent process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    ExecuteDownloadServer(void);

/*---------------------------------------------------------------------------*
  Name:         StartWirelessParent

  Description:  Launches data sharing parent mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    StartWirelessParent(void);

/*---------------------------------------------------------------------------*
  Name:         StartWirelessChild

  Description:  Launches data sharing child mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    StartWirelessChild(void);

/*---------------------------------------------------------------------------*
  Name:         ExecuteDataSharing

  Description:  Main process for data sharing.
                WFS process in background.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    ExecuteDataSharing(void);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // WM_DEMO_WIRELESS_ALL_COMMON_H_
