/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tcl - demos.TWL - tcl-1
  File:     main.c

  Copyright 2008-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-10-15#$
  $Rev: 11095 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/tcl.h>
#include <twl/ssp/ARM9/jpegdec.h>
#include <twl/ssp/ARM9/exifdec.h>

#include "DEMO.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define WIDTH       256         // Image width
#define HEIGHT      192         // Image height

/*---------------------------------------------------------------------------*
    Internal Functions and Definitions
 *---------------------------------------------------------------------------*/
void VBlankIntr(void);
void Print(void);
void DownSampling( const u16* org_img, int org_wdt, int org_hgt,
                         u16* new_img, int new_wdt, int new_hgt );
int GetR ( u16 color );
int GetG ( u16 color );
int GetB ( u16 color );

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
// Buffer storing images
static u16 draw_buffer[ WIDTH * HEIGHT ] ATTRIBUTE_ALIGN(32);

int condition_id = 0;
int search = 0;
int num_pict = 0;
int draw_id = 0;

// Get the color value (from 0 to 31)
int GetR ( u16 color ) { return (color & GX_RGB_R_MASK) >> GX_RGB_R_SHIFT; }
int GetG ( u16 color ) { return (color & GX_RGB_G_MASK) >> GX_RGB_G_SHIFT; }
int GetB ( u16 color ) { return (color & GX_RGB_B_MASK) >> GX_RGB_B_SHIFT; }

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

/*---------------------------------------------------------------------------*
  Name:         Print

  Description:  Renders debug information.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Print(void)
{
    const char* condition_name[] = { "All Picture", "Favorite1", "Favorite2", "Favorite3", "FavoriteAll" };

    ClearScreen();

    PutScreen(0, 0, 0xff, "TCL-1 Search Picture Demo");

    PutScreen(2, 4, 0xff, "TCL Condition : %s", condition_name[condition_id] );
    PutScreen(2, 5, 0xff, "Draw Index    : %d", draw_id );

    PutScreen(2, 7, 0xff, "Num Pictures  : %d", num_pict );

    PutScreen(1, 12, 0xff, "A Button > Draw Picture" );
    PutScreen(1, 13, 0xff, "B Button > Draw Thumbnail" );
    PutScreen(1, 14, 0xff, "X Button > Select Condition" );
    PutScreen(1, 15, 0xff, "Y Button > Change Search Method" );
    PutScreen(1, 16, 0xff, "R Button > Delete Picture" );
    PutScreen(1, 17, 0xff, "L Button > Change FavoriteType" );

    if( search )
    {
        PutScreen(1, 2, 0xff, "Iterate Search" );
    }
    else
    {
        PutScreen(1, 2, 0xff, "Random Search" );
        PutScreen(1, 18, 0xff, "Left  Key > Select Draw Index" );
        PutScreen(1, 19, 0xff, "Right Key > Select Draw Index" );
    }
}

/*---------------------------------------------------------------------------*
  Name:         DownSampling

  Description:  Size reduction.

  Arguments:    org_img: Original image
                org_wdt: Original image width
                org_hgt: Original image height
                new_img: New image
                new_wdt: New image width
                new_hgt: New image height

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DownSampling( const u16* org_img, int org_wdt, int org_hgt,
                         u16* new_img, int new_wdt, int new_hgt )
{
    int h, w, i = 0;

    if( org_wdt == TCL_JPEG_WIDTH )
    {
        // Normal photograph
        for ( h = 0; h < new_hgt; ++h )
        {
            for ( w = 0; w < new_wdt; ++w )
            {
                new_img[ h * new_wdt + w ] = org_img[ ( h * org_hgt / new_hgt ) * org_wdt +
                                                      w * org_wdt / new_wdt ];
            }
        }
    }
    else
    {
        // Thumbnail
        u16* thumb_img = &new_img[ ( HEIGHT - SSP_JPEG_THUMBNAIL_HEIGHT ) / 2 * WIDTH +
                                   ( WIDTH - SSP_JPEG_THUMBNAIL_WIDTH ) / 2 ];

        for ( h = 0; h < org_hgt; ++h )
        {
            for ( w = 0; w < org_wdt; ++w )
            {
                thumb_img[ h * new_wdt + w ] = org_img[ i ];
                ++i;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OSHeapHandle handle;
    char  pict_path[TCL_PATH_LEN];
    void* nstart;

    void* pTableBuf;
    void* pWorkBuf;

    FSResult fs_result;
    TCLResult result;

    // TCL accessor
    TCLAccessor accessor;

    // Search condition object
    TCLSearchObject search_obj;
    TCLSortType sort_type = TCL_SORT_TYPE_DATE;

    // Initialization process
    DEMOInitCommon();
    DEMOInitVRAM();

    FS_Init( FS_DMA_NOT_USE );

    InitScreen();
    ClearScreen();

    // GX-related settings
    GX_SetBankForBG( GX_VRAM_BG_128_A );

    GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BG0_AS_2D );

    GX_SetVisiblePlane( GX_PLANEMASK_BG3 );
    G2_SetBG3Priority( 0 );
    G2_BG3Mosaic( FALSE );

    G2_SetBG3ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000 );

    MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );
    GX_LoadBG3Bmp( draw_buffer, 0, WIDTH * HEIGHT * sizeof(u16) );

    // Create heap
    nstart = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1 );
    OS_SetMainArenaLo( nstart );

    handle = OS_CreateHeap( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi() );
    if (handle < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    handle = OS_SetCurrentHeap( OS_ARENA_MAIN, handle );

    // TCL initialization
    // 32-byte alignment necessary
    pTableBuf = OS_Alloc( TCL_GetTableBufferSize() );
    pWorkBuf = OS_Alloc( TCL_GetWorkBufferSize() );
    if( pTableBuf == NULL || pWorkBuf == NULL )
    {
        OS_Panic("Cannot allocate memory!");
    }

    result = TCL_LoadTable( &accessor,
                            pTableBuf,
                            TCL_GetTableBufferSize(),
                            pWorkBuf,
                            TCL_GetWorkBufferSize(),
                            &fs_result );

#if 1

    // End the following processes when loading of the management file fails

    switch ( result )
    {
      case TCL_RESULT_SUCCESS: break; // Success
      case TCL_RESULT_ERROR_EXIST_OTHER_FILE:
      case TCL_RESULT_ERROR_ALREADY_MANAGED:
        // Error indicating no obstacle to getting a photo
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );
        break;
      default:
        // For other errors, prompt Nintendo DSi camera startup and end the process
        //OS_TPrintf ("appropriate text here")

        OS_Panic("failed TCL_LoadTable : %d\n", result );
        break;
    }

#else

    // Try recovering if loading of the management file failed

    switch( result )
    {
    case TCL_RESULT_SUCCESS: break; // Success
    case TCL_RESULT_ERROR_EXIST_OTHER_FILE:
    case TCL_RESULT_ERROR_ALREADY_MANAGED:
        // Error indicating no obstacle to getting a photo
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );
        break;
    default:
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );

        // The management file is recreated because read processing cannot be carried out
        result = TCL_CreateTable( &accessor,
                                  pTableBuf,
                                  TCL_GetTableBufferSize(),
                                  pWorkBuf,
                                  TCL_GetWorkBufferSize(),
                                  &fs_result );

        if( result != TCL_RESULT_SUCCESS )
        {
            OS_Panic("failed TCL_CreateTable : %d\n", result );
        }
        break;
    }

#endif

    // Initialize SearchObject
    TCL_InitSearchObject( &search_obj, TCL_SEARCH_CONDITION_PICTURE );

    // How many photos in all?
    num_pict = TCL_GetNumPictures( &accessor, &search_obj );

    // Sort the table as conditions indicate
    TCL_SortTable( &accessor,  sort_type );

    Print();

    DEMOStartDisplay();

    // Updates text display
    UpdateScreen();

    while (1)
    {
        // Reading pad information and controls
        DEMOReadKey();

        if( num_pict != 0 && DEMO_IS_TRIG( PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_R | PAD_BUTTON_L ) )
        {
            BOOL success_flag = FALSE;

            // Display thumbnails when the B Button is pressed
            BOOL thumbnail_flag = DEMO_IS_TRIG( PAD_BUTTON_B );

            BOOL delete_flag = DEMO_IS_TRIG( PAD_BUTTON_R );

            BOOL favorite_flag = DEMO_IS_TRIG( PAD_BUTTON_L );

            const TCLPictureInfo* pPicInfo = NULL;
            const TCLMakerNote* pMakerNote = NULL;

            MI_CpuClear( pict_path, TCL_PATH_LEN );

            if( search )
            {
                do
                {
                    draw_id = search_obj.startIdx;

                    // Get photo information
                    result = TCL_SearchNextPictureInfo( &accessor,
                                                        &pPicInfo,
                                                        &search_obj );

                    if( result == TCL_RESULT_ERROR_NO_FIND_PICTURE )
                    {
                        // Initialize SearchObject as one full search has been completed
                        TCL_InitSearchObject( &search_obj, search_obj.condition );
                        OS_TPrintf("finished TCL_SearchNextPicturePath\n" );
                    }
                } while( result == TCL_RESULT_ERROR_NO_FIND_PICTURE );
            }
            else
            {
                result = TCL_SearchPictureInfoByIndex( &accessor,
                                                       &pPicInfo,
                                                       &search_obj,
                                                       draw_id );
            }

            if( result != TCL_RESULT_SUCCESS )
            {
                OS_TPrintf("failed TCL_Search* : %d\n", result );
            }
            else if( delete_flag )
            {
                TCLPictureInfo* pGetPicInfo = NULL;
                FSResult result;

                // For the test, get the picture's path and then get TCLPictureInfo again.
                ( void ) TCL_PrintPicturePath( pict_path, TCL_PATH_LEN, pPicInfo );
                if(TCL_GetPictureInfoFromPath( &accessor, &pGetPicInfo, (const char*)pict_path, (u32)STD_StrLen(pict_path) ) == FALSE)
                {
                    OS_TPrintf("failed TCL_PrintPictureInfo\n");
                }
                else
                {
                    // Allow picture data to be deleted only if it was created by one of your game titles.
                    // You may also restrict this to only items created by this application.
                    if(TCL_DeletePicture(&accessor, pGetPicInfo, &result) != TCL_RESULT_SUCCESS)
                        OS_TPrintf("failed TCL_DeletePicture\n");
                    
                    search_obj.condition = TCL_SEARCH_CONDITION_PICTURE;
                    sort_type = TCL_SORT_TYPE_DATE;

                    switch( condition_id )
                    {
                    case 0: break;
                    case 1:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_1;
                        sort_type = TCL_SORT_TYPE_FAVORITE_1;
                        break;
                    case 2:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_2;
                        sort_type = TCL_SORT_TYPE_FAVORITE_2;
                        break;
                    case 3:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_3;
                        sort_type = TCL_SORT_TYPE_FAVORITE_3;
                        break;
                    case 4:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_ALL;
                        sort_type = TCL_SORT_TYPE_FAVORITE_ALL;
                        break;
                    }

                    if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
                    num_pict = TCL_GetNumPictures( &accessor, &search_obj );
                    TCL_SortTable( &accessor, sort_type );
                    draw_id = 0;

                    Print();
                }
            }
            else if( favorite_flag )
            {
                TCLPictureInfo* pChangePicInfo = (TCLPictureInfo*)pPicInfo;
                TCLFavoriteType nextFavoriteType;
                FSResult result;

                switch ( pChangePicInfo->favoriteType )
                {
                case TCL_FAVORITE_TYPE_NONE:
                    nextFavoriteType = TCL_FAVORITE_TYPE_1;
                    break;
                case TCL_FAVORITE_TYPE_1:
                    nextFavoriteType = TCL_FAVORITE_TYPE_2;
                    break;
                case TCL_FAVORITE_TYPE_2:
                    nextFavoriteType = TCL_FAVORITE_TYPE_3;
                    break;
                case TCL_FAVORITE_TYPE_3:
                default:
                    nextFavoriteType = TCL_FAVORITE_TYPE_NONE;
                    break;
                }

                OS_TPrintf("change favoriteType %d -> %d\n", pChangePicInfo->favoriteType, nextFavoriteType);

                if(TCL_ChangePictureFavoriteType(&accessor, pChangePicInfo, nextFavoriteType, &result) != TCL_RESULT_SUCCESS)
                    OS_TPrintf("failed TCL_ChangePictureFavoriteType\n");

                search_obj.condition = TCL_SEARCH_CONDITION_PICTURE;
                sort_type = TCL_SORT_TYPE_DATE;

                switch( condition_id )
                {
                case 0: break;
                case 1:
                    search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_1;
                    sort_type = TCL_SORT_TYPE_FAVORITE_1;
                    break;
                case 2:
                    search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_2;
                    sort_type = TCL_SORT_TYPE_FAVORITE_2;
                    break;
                case 3:
                    search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_3;
                    sort_type = TCL_SORT_TYPE_FAVORITE_3;
                    break;
                case 4:
                    search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_ALL;
                    sort_type = TCL_SORT_TYPE_FAVORITE_ALL;
                    break;
                }

                if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
                num_pict = TCL_GetNumPictures( &accessor, &search_obj );
                TCL_SortTable( &accessor, sort_type );
                draw_id = 0;

                Print();
            }
            else
            {
                FSFile file;
                u32 size;
                s32 read_size;
                s16 width, height;
                TCLResult decodeResult;
                u32 decodeOptiion;

                // JPEG buffer
                u8*  pJpegBuf = NULL;
                u16* pDecodeTmpBuf = NULL;

                if( thumbnail_flag )
                {
                    width  = SSP_JPEG_THUMBNAIL_WIDTH;
                    height = SSP_JPEG_THUMBNAIL_HEIGHT;
                    decodeOptiion = SSP_JPEG_THUMBNAIL | SSP_JPEG_RGB555;
                }
                else
                {
                    width  = TCL_JPEG_WIDTH;
                    height = TCL_JPEG_HEIGHT;
                    decodeOptiion = SSP_JPEG_RGB555;
                }

                // Get the photo path
                ( void ) TCL_PrintPicturePath( pict_path, TCL_PATH_LEN, pPicInfo );

                // Get photo
                FS_InitFile( &file );
                if( FS_OpenFileEx( &file, pict_path, FS_FILEMODE_R ) )
                {
                    pJpegBuf = (u8*)OS_Alloc( TCL_MAX_JPEG_SIZE );

                    // Allocate the necessary memory for whether it is a normal photograph or a thumbnail
                    pDecodeTmpBuf = (u16*)OS_Alloc( width * height * sizeof(u16) );

                    if( pJpegBuf == NULL || pDecodeTmpBuf == NULL )
                    {
                        OS_Panic("Cannot allocate memory!");
                    }

                    size = FS_GetFileLength( &file );
                    read_size = FS_ReadFile( &file, pJpegBuf, (s32)size );

                    if( read_size > 0 )
                    {
                        // JPEG decoding
                        decodeResult = TCL_DecodePicture( pJpegBuf,
                                                          size,
                                                          (u8*)pDecodeTmpBuf,
                                                          width, height,
                                                          decodeOptiion );

                        if( decodeResult == TCL_RESULT_SUCCESS )
                        {
                            // Get manufacturer's notes
                            pMakerNote = ( TCLMakerNote* ) SSP_GetJpegDecoderMakerNoteAddrEx( SSP_MAKERNOTE_PHOTO );

                            // Display if image type is same
                            if( TCL_IsSameImageType( pPicInfo, pMakerNote ) != FALSE )
                            {
                                MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );

                                // Shrink to image size
                                DownSampling( pDecodeTmpBuf, width, height,
                                              draw_buffer, WIDTH, HEIGHT );

                                success_flag = TRUE;
                            }
                            else OS_TPrintf("Not Same Image Type\n");
                        }
                        else OS_TPrintf("failed JPEG Decode %d\n", decodeResult);
                    }
                    else OS_TPrintf("failed FS_ReadFile\n");

                    ( void ) FS_CloseFile( &file );

                    OS_Free( pJpegBuf );
                    OS_Free( pDecodeTmpBuf );
                }
                else OS_TPrintf("failed FS_Open\n");
            }

            if( !success_flag ) MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );
            DC_FlushRange( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );

            Print();
        }
        else if( DEMO_IS_TRIG( PAD_BUTTON_X ) )
        {
            // Modify search conditions
            if( ++condition_id > 4 ) condition_id = 0;

            search_obj.condition = TCL_SEARCH_CONDITION_PICTURE;
            sort_type = TCL_SORT_TYPE_DATE;

            switch( condition_id )
            {
            case 0: break;
            case 1:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_1;
                sort_type = TCL_SORT_TYPE_FAVORITE_1;
                break;
            case 2:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_2;
                sort_type = TCL_SORT_TYPE_FAVORITE_2;
                break;
            case 3:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_3;
                sort_type = TCL_SORT_TYPE_FAVORITE_3;
                break;
            case 4:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_ALL;
                sort_type = TCL_SORT_TYPE_FAVORITE_ALL;
                break;
            }

            if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
            num_pict = TCL_GetNumPictures( &accessor, &search_obj );
            TCL_SortTable( &accessor, sort_type );
            draw_id = 0;

            Print();
        }
        else if( DEMO_IS_TRIG( PAD_BUTTON_Y ) )
        {
            // Modify search method
            search ^= 1;
            draw_id = 0;

            if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
            Print();
        }
        else if( num_pict != 0 && !search )
        {
            // Modify ID of displayed photo
            if( DEMO_IS_TRIG( PAD_KEY_RIGHT ) )
            {
                if( ++draw_id >= num_pict ) draw_id = 0;
                Print();
            }
            else if( DEMO_IS_TRIG( PAD_KEY_LEFT ) )
            {
                if( --draw_id < 0 ) draw_id = num_pict - 1;
                Print();
            }
        }

        OS_WaitVBlankIntr();

        // Updates text display
        UpdateScreen();

        GX_LoadBG3Bmp( draw_buffer, 0, WIDTH * HEIGHT * sizeof(u16) );
    }

    OS_Free( pTableBuf );
    OS_Free( pWorkBuf );

    OS_Terminate();
}
