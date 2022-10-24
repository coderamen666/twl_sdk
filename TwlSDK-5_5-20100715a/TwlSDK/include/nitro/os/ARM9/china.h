/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     china.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-24#$
  $Rev: 11186 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_ARM9_CHINA_H_
#define NITRO_OS_ARM9_CHINA_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#define OS_BURY_STRING_FORCHINA         "[SDK+NINTENDO:FORCHINA]"
#define OS_BUSY_STRING_LEN_FORCHINA     23

typedef enum
{
    OS_CHINA_ISBN_DISP,
    OS_CHINA_ISBN_NO_DISP,
    OS_CHINA_ISBN_CHECK_ROM
}OSChinaIsbn;

/*---------------------------------------------------------------------------*
  Name:         OS_InitChina

  Description:  Initializes SDK's OS library.
                Exclusively for applications for the China region.
                Use this function instead of the OS_Init function.

  Arguments:    isbn: String array of the ISBN (and other information).
                            {
                                char    ISBN[ 17 ] ,
                                char    Joint registration code  [ 12 ] ,
                                char    (China-specific information displayed on left side) [ 4 ],
                                char    (China-specific information displayed on right side) [ 3 ],
                            }

                param: Behavior when displaying ISBNs.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_InitChina(const char **isbn, OSChinaIsbn param);

/*---------------------------------------------------------------------------*
  Name:         OS_ShowAttentionChina

  Description:  Displays the logos and warning screens for China, for the prescribed lengths of time.
                This function can be redefined in the user application.

  Arguments:    isbn: String array of the ISBN (and other information).
                param: Behavior when displaying ISBNs.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_ShowAttentionChina(const char **isbn, OSChinaIsbn param);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_OS_ARM9_CHINA_H_ */

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
