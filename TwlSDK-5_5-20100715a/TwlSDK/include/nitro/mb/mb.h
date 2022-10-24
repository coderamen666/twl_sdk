/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-17#$
  $Rev: 8984 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MB_MB_H_
#define NITRO_MB_MB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/misc.h>
#include <nitro/fs/file.h>
#include <nitro/wm.h>


/* ---------------------------------------------------------------------

        Constants

   ---------------------------------------------------------------------*/

#define MB_MAX_CHILD                    WM_NUM_MAX_CHILD

/* Number of characters in file name */
#define MB_FILENAME_SIZE_MAX            (10)

/* Maximum number of files in parent device */
#define MB_MAX_FILE                     (16)

/* Size of multiboot working memory */
#define MB_SYSTEM_BUF_SIZE              (0xC000)

/* Possible size setting range for parent communication */
#define MB_COMM_PARENT_SEND_MIN         256
#define MB_COMM_PARENT_SEND_MAX         510

#define MB_COMM_PARENT_RECV_MIN         8
#define MB_COMM_PARENT_RECV_MAX         16

/* Minimum buffer size required for MB_ReadSegment() */
#define MB_SEGMENT_BUFFER_MIN           0x10000

/* MB API result type */
#define MB_SUCCESS                      MB_ERRCODE_SUCCESS      // Backwards compatibility

/* Definition value that specifies that the TGID value for MB_Init() should be automatically set */
#define	MB_TGID_AUTO	0x10000

/*
 * Parent device game information for multibooting
 */

#define MB_ICON_COLOR_NUM               16      // Number of icon colors
#define MB_ICON_PALETTE_SIZE            (MB_ICON_COLOR_NUM * 2) // Icon data size (16-bit color x number of colors)
#define MB_ICON_DATA_SIZE               512     // Icon data size (32x32 dots, 16 colors)
#define MB_GAME_NAME_LENGTH             48      // Game name length (2-byte units)   Note: Specify to fit within the number of characters noted on the left *and* within a width of 185 dots.
#define MB_GAME_INTRO_LENGTH            96      // Game description length (2-byte units)   Note: Specify to fit within the number of characters noted on the left *and* within a width of 199 dots times 2.
#define MB_USER_NAME_LENGTH             10      // User name length (2-byte units)
#define MB_MEMBER_MAX_NUM               15      // Maximum number of communicating members
#define MB_FILE_NO_MAX_NUM              64      // When MbGameInfo.fileNo is carried on the beacon it becomes 6-bit. Therefore the maximum number is 64.
#define MB_GAME_INFO_RECV_LIST_NUM      16      // Number of lists that receive game information on the child side.

/* Boot type */
#define MB_TYPE_ILLEGAL                 0       /* Illegal status */
#define MB_TYPE_NORMAL                  1       /* Boot from ROM */
#define MB_TYPE_MULTIBOOT               2       /* Multiboot child device */

#define MB_BSSDESC_SIZE                 (sizeof(MBParentBssDesc))
#define MB_DOWNLOAD_PARAMETER_SIZE      HW_DOWNLOAD_PARAMETER_SIZE

/* Parent download status */
typedef enum
{
    MB_COMM_PSTATE_NONE,
    MB_COMM_PSTATE_INIT_COMPLETE,
    MB_COMM_PSTATE_CONNECTED,
    MB_COMM_PSTATE_DISCONNECTED,
    MB_COMM_PSTATE_KICKED,
    MB_COMM_PSTATE_REQ_ACCEPTED,
    MB_COMM_PSTATE_SEND_PROCEED,
    MB_COMM_PSTATE_SEND_COMPLETE,
    MB_COMM_PSTATE_BOOT_REQUEST,
    MB_COMM_PSTATE_BOOT_STARTABLE,
    MB_COMM_PSTATE_REQUESTED,
    MB_COMM_PSTATE_MEMBER_FULL,
    MB_COMM_PSTATE_END,
    MB_COMM_PSTATE_ERROR,
    MB_COMM_PSTATE_WAIT_TO_SEND,

    /* Internally used enumerated values.
       Will not transition to these value's states. */
    MB_COMM_PSTATE_WM_EVENT = 0x80000000
}
MBCommPState;


/* Responses to connection request from child */
typedef enum
{
    MB_COMM_RESPONSE_REQUEST_KICK,     /* Reject child's entry request. */
    MB_COMM_RESPONSE_REQUEST_ACCEPT,   /* Accept child's entry request. */
    MB_COMM_RESPONSE_REQUEST_DOWNLOAD, /* Notify child of commencement of download. */
    MB_COMM_RESPONSE_REQUEST_BOOT      /* Notify child of commencement of boot. */
}
MBCommResponseRequestType;


/*  Parent event notification callback type */
typedef void (*MBCommPStateCallback) (u16 child_aid, u32 status, void *arg);


typedef struct
{
    u16     errcode;
}
MBErrorStatus;

typedef enum
{
    /* Value that indicates success */
    MB_ERRCODE_SUCCESS = 0,

    MB_ERRCODE_INVALID_PARAM,          /* Argument error */
    MB_ERRCODE_INVALID_STATE,          /* Inconsistent call state conditions */

    /* The following errors are only for children. */
    MB_ERRCODE_INVALID_DLFILEINFO,     /* Invalid download information */
    MB_ERRCODE_INVALID_BLOCK_NO,       /* Received block number is invalid */
    MB_ERRCODE_INVALID_BLOCK_NUM,      /* Number of blocks in received file is invalid */
    MB_ERRCODE_INVALID_FILE,           /* Received block from file that was not requested */
    MB_ERRCODE_INVALID_RECV_ADDR,      /* Receive address is invalid */

    /* The following errors are caused by WM errors. */
    MB_ERRCODE_WM_FAILURE,             /* Error in WM callback */

    /** The following are for when it is not possible to continue communications. (Must re-initialize WM) **/
    MB_ERRCODE_FATAL,

    MB_ERRCODE_MAX
}
MBErrCode;

/* ---------------------------------------------------------------------

        Structures

   ---------------------------------------------------------------------*/

/*
 * Structure with registration data for multiboot games
 */
typedef struct
{
    /* Program binary's file path */
    const char *romFilePathp;
    /* Pointer to game name string */
    u16    *gameNamep;
    /* Pointer to game content description string */
    u16    *gameIntroductionp;
    /* Icon character data file path */
    const char *iconCharPathp;
    /* Icon palette data file path */
    const char *iconPalettePathp;
    /* Game group ID */
    u32     ggid;
    /* Max number of players */
    u8      maxPlayerNum;
    u8      pad[3];
    /* User definition extended parameters (32 bytes) */
    u8      userParam[MB_DOWNLOAD_PARAMETER_SIZE];
}
MBGameRegistry;


/*
 * Icon data structure
 * An icon uses a 16-color palette and is 32x32 pixels.
 */
typedef struct
{
    /* Palette data (32 bytes) */
    u16     palette[MB_ICON_COLOR_NUM];
    u16     data[MB_ICON_DATA_SIZE / 2];
    /* Character data (512 bytes) */
}
MBIconInfo;                            /* 544 bytes */


/*
 * User information structure
 */
typedef struct
{
    /* Favorite color data (color set number) */
    u8      favoriteColor:4;
    /* Player number (Parent = #0, Children = #1-#15) */
    u8      playerNo:4;
    /* Nickname Length */
    u8      nameLength;
    /* Nickname */
    u16     name[MB_USER_NAME_LENGTH];
}
MBUserInfo;                            /* 22 bytes */


/*
 * Multiboot parent device information
 * (excluding gameInfo[128] from WMbssDesc)
 */
typedef struct
{
    u16     length;                    // 2
    u16     rssi;                      // 4
    u16     bssid[3];                  // 10
    u16     ssidLength;                // 12
    u8      ssid[32];                  // 44
    u16     capaInfo;                  // 46
    struct
    {
        u16     basic;                 // 48
        u16     support;               // 50
    }
    rateSet;
    u16     beaconPeriod;              // 52
    u16     dtimPeriod;                // 54
    u16     channel;                   // 56
    u16     cfpPeriod;                 // 58
    u16     cfpMaxDuration;            // 60
}
MBParentBssDesc;


/* Structure that holds parent information for a multiboot child */
typedef struct
{
    u16     boot_type;
    MBParentBssDesc parent_bss_desc;
}
MBParam;


/* Data type received when entry request comes from child */
typedef struct
{
    u32     ggid;                      //  4 bytes
    MBUserInfo userinfo;               // 22 bytes
    u16     version;                   //  2 bytes
    u8      fileid;                    //  1 byte
    u8      pad[3];                    // Total: 29 bytes
}
MBCommRequestData;


/* ---------------------------------------------------------------------

        Multiboot library (MB) API - For parent device

   ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MB_Init

  Description:  Initialize MB library

  Arguments:    work:     Pointer to memory that is allocated for managing internal MB status.
                         This size must be at least MB_SYSTEM_BUF_SIZE bytes or larger.
                          In the case of dynamic allocation, it is also possible to acquire the size with the MB_GetParentSystemBufSize function.
                         
                         
                         The allocated memory is used internally until the MB_End is called.
                         

                user:   - Pointer to the structure that holds user data.
                         Content pointed to by this pointer can only be referenced from inside the MB_Init function.
                         

                ggid:  - Specifies a GGID.

                tgid:   - Value of TGID (Use a value generated by a decision method that follows WM guidelines)
                         
                         When MB_TGID_AUTO is specified, an appropriate value is automatically set internally.
                         

                dma:  - DMA channel that is allocated for managing internal MB processes.
                         This DMA channel is used internally until the MB_End is called.
                          
  
  Returns:      Returns MB_SUCCESS if the initialization completes successfully.
 *---------------------------------------------------------------------------*/

int     MB_Init(void *work, const MBUserInfo *user, u32 ggid, u32 tgid, u32 dma);


/*---------------------------------------------------------------------------*
  Name:         MB_GetParentSystemBufSize

  Description:  Acquires the size of the work memory used by MB.

  Arguments:    None.
  
  Returns:      The size of the work memory used by MB.
 *---------------------------------------------------------------------------*/
int     MB_GetParentSystemBufSize(void);


/*---------------------------------------------------------------------------*
  Name:         MB_SetParentCommSize

  Description:  Sets the size of the parent's communication data that is used for MB communication.
                Use this between the time MB_Init is started and the time MB_StartParent is called. 
                Buffer size cannot be changed after communications begins.

  Arguments:    sendSize:   The size of send data that is sent from parent device to each child device.
  
  Returns:      If changing the size of the send/receive data succeeds, returns TRUE; otherwise returns FALSE.
                
 *---------------------------------------------------------------------------*/

BOOL    MB_SetParentCommSize(u16 sendSize);

/*---------------------------------------------------------------------------*
  Name:         MB_SetParentCommParam

  Description:  Sets the size of the parent's communication data that is used for MB communication, and the maximum number of children that can connect.
                Use this between the time MB_Init is started and the time MB_StartParent is called.
                 Buffer size cannot be changed after communications begins.

  Arguments:    sendSize:   The size of send data that is sent from parent device to each child device.
                maxChildren:   Maximum number of connected children
  
  Returns:      If changing the size of the send/receive data succeeds, returns TRUE; otherwise returns FALSE.
                
 *---------------------------------------------------------------------------*/

BOOL    MB_SetParentCommParam(u16 sendSize, u16 maxChildren);

/*---------------------------------------------------------------------------*
  Name:         MB_GetTgid

  Description:  Acquires the TGID used in WM as regards the MB library.

  Arguments:    None.
  
  Returns:      Returns the TGID used in WM.
 *---------------------------------------------------------------------------*/

u16     MB_GetTgid(void);


/*---------------------------------------------------------------------------*
  Name:         MB_End

  Description:  Shuts down the MB library.

  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/

void    MB_End(void);


/*---------------------------------------------------------------------------*
  Name:         MB_EndToIdle

  Description:  Shuts down the MB library.
                However the WM library will be kept in an idle state.

  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/

void    MB_EndToIdle(void);

/*---------------------------------------------------------------------------*
  Name:         MB_DisconnectChild

  Description:  Disconnects a child.

  Arguments:    aid:              aid of the child to disconnect. 
  
  Returns:      Returns MB_SUCCESS if start succeeds.
 *---------------------------------------------------------------------------*/

void    MB_DisconnectChild(u16 aid);


/*---------------------------------------------------------------------------*
  Name:         MB_StartParent

  Description:  Parent parameter setting & start

  Arguments:    channel:   Channel used for communicating as a parent 
  
  Returns:      Returns MB_SUCCESS if start succeeds.
 *---------------------------------------------------------------------------*/

int     MB_StartParent(int channel);


/*---------------------------------------------------------------------------*
  Name:         MB_StartParentFromIdle

  Description:  Parent parameter setting & start.
                However, the WM library must be in an idle state.

  Arguments:    channel:   Channel used for communicating as a parent 
  
  Returns:      Returns MB_SUCCESS if start succeeds.
 *---------------------------------------------------------------------------*/

int     MB_StartParentFromIdle(int channel);

/*---------------------------------------------------------------------------*
  Name:         MB_CommGetChildrenNumber

  Description:  Acquires the total number of children that are currently entered.

  Arguments:    None.

  Returns:      Returns the total number of children that the parent currently recognizes.
 *---------------------------------------------------------------------------*/

u8      MB_CommGetChildrenNumber(void);


/*---------------------------------------------------------------------------*
  Name:         MB_CommGetChildUser

  Description:  Acquires information belonging to a child that has a specified aid.

  Arguments:    child_aid:   aid that specifies the child for which status is being acquired 
  
  Returns:      Returns MB_SUCCESS if start succeeds.
 *---------------------------------------------------------------------------*/

const MBUserInfo *MB_CommGetChildUser(u16 child_aid);


/*---------------------------------------------------------------------------*
  Name:         MB_CommGetParentState

  Description:  Acquires the state of a parent with respect to a child that has a specified aid.

  Arguments:    child_aid:   aid that specifies the child for which status is being acquired 
  
  Returns:      Returns one of the states indicated by MBCommPState.
 *---------------------------------------------------------------------------*/

int     MB_CommGetParentState(u16 child);


/*---------------------------------------------------------------------------*
  Name:         MB_CommSetParentStateCallback

  Description:  Sets the parent state notification callback.

  Arguments:    callback:   Pointer to the callback function that notifies of the parent's state
  
  Returns:      None.
 *---------------------------------------------------------------------------*/

void    MB_CommSetParentStateCallback(MBCommPStateCallback callback);


/*---------------------------------------------------------------------------*
  Name:         MB_GetSegmentLength

  Description:  Gets the length of the segment buffer that is required for the specified binary file.

  Arguments:    file:   FSFile structure that points to a binary file
  
  Returns:      Returns a positive value if it can properly get size. Otherwise returns 0.
 *---------------------------------------------------------------------------*/

u32     MB_GetSegmentLength(FSFile *file);


/*---------------------------------------------------------------------------*
  Name:         MB_ReadSegment

  Description:  Reads the required segment data from the specified binary file.

  Arguments:    file:   FSFile structure that points to a binary file
                buf:   Buffer for reading segment data
                len:   Size of buf
  
  Returns:      Returns TRUE if it properly reads segment data. Otherwise returns FALSE.
 *---------------------------------------------------------------------------*/

BOOL    MB_ReadSegment(FSFile *file, void *buf, u32 len);


/*---------------------------------------------------------------------------*
  Name:         MB_RegisterFile

  Description:  Registers the specified file in the download list.

  Arguments:    game_reg:   Pointer to the MBGameRegistry structure where information of the program to be registered is stored. 
                           
                buf:      - Pointer to the memory that holds extracted segment information
                           This segment information is obtained with the MB_ReadSegment function.
  
  Returns:      Returns TRUE if the file was registered properly and FALSE otherwise.
                
 *---------------------------------------------------------------------------*/

BOOL    MB_RegisterFile(const MBGameRegistry *game_reg, const void *buf);


/*---------------------------------------------------------------------------*
  Name:         MB_UnregisterFile

  Description:  Deletes the specified file from the download list.

  Arguments:    game_reg:   Pointer to the MBGameRegistry structure where information of the program to be registered is stored. 

                           
  Returns:      Returns TRUE if the file was deleted properly and FALSE otherwise.
                
 *---------------------------------------------------------------------------*/

void   *MB_UnregisterFile(const MBGameRegistry *game_reg);


/*---------------------------------------------------------------------------*
  Name:         MB_CommResponseRequest

  Description:  Specifies a response to a connect request from a child.

  Arguments:    child_aid:   aid that specifies the child to send a response to
                ack:       - Specifies the acknowledgement response result to a child.
                            Enumerated value for the MBCommResponseRequestType type.

  Returns:      Returns TRUE if the specified child is waiting for a connect response, and if the specified response type is appropriate; otherwise returns FALSE.
                
                
 *---------------------------------------------------------------------------*/

BOOL    MB_CommResponseRequest(u16 child_aid, MBCommResponseRequestType ack);


/*---------------------------------------------------------------------------*
  Name:         MB_CommStartSending

  Description:  Begins sending a boot image to a child that has a specified aid (and that has been entered).

  Arguments:    child_aid:   aid that specifies the child to begin sending a boot image to

  Returns:      Returns TRUE if the specified child has been entered and FALSE otherwise.
                
 *---------------------------------------------------------------------------*/

static inline BOOL MB_CommStartSending(u16 child_aid)
{
    return MB_CommResponseRequest(child_aid, MB_COMM_RESPONSE_REQUEST_DOWNLOAD);
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommStartSendingAll

  Description:  Begins sending a boot image to the children that have entered.

  Arguments:    None.

  Returns:      Returns TRUE if there is a child that has performed entry and FALSE otherwise.
                
 *---------------------------------------------------------------------------*/

static inline BOOL MB_CommStartSendingAll(void)
{
    u8      child, num;

    for (num = 0, child = 1; child <= WM_NUM_MAX_CHILD; child++)
    {
        if (TRUE == MB_CommStartSending(child))
        {
            num++;
        }
    }

    if (num == 0)
        return FALSE;

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommIsBootable

  Description:  Determines whether a boot is possible.

  Arguments:    child_aid:   AID of target child

  Returns:      yes - TRUE  no - FALSE
 *---------------------------------------------------------------------------*/

BOOL    MB_CommIsBootable(u16 child_aid);


/*---------------------------------------------------------------------------*
  Name:         MB_CommBootRequest

  Description:  Sends a boot request to a specified child that has completed download.

  Arguments:    child_aid:   AID of target child

  Returns:      success - TRUE  failed - FALSE
 *---------------------------------------------------------------------------*/

static inline BOOL MB_CommBootRequest(u16 child_aid)
{
    return MB_CommResponseRequest(child_aid, MB_COMM_RESPONSE_REQUEST_BOOT);
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommBootRequestAll

  Description:  Sends a boot request to children that have completed download.

  Arguments:    None.

  Returns:      success - TRUE  failed - FALSE
 *---------------------------------------------------------------------------*/

static inline BOOL MB_CommBootRequestAll(void)
{
    u8      child, num;

    for (num = 0, child = 1; child <= WM_NUM_MAX_CHILD; child++)
    {
        if (TRUE == MB_CommBootRequest(child))
        {
            num++;
        }
    }

    if (num == 0)
        return FALSE;

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         MB_GetGameEntryBitmap

  Description:  Obtains the bitmap of AIDs that have been entered for the GameRegistry being distributed.
                

  Arguments:    game_req:      - Pointer to the target GameRegistry

  Returns:      Bitmap of AIDs entered in the specified GameRegistry
                (Parent AID: 0, Child AID: 1-15)
                If a game is not in distribution, the return value is 0.
                (If a game is being distributed, parent AID:0 is always included in the entry members.)
                  
 *---------------------------------------------------------------------------*/

u16     MB_GetGameEntryBitmap(const MBGameRegistry *game_reg);


/* ---------------------------------------------------------------------

        Multiboot library (MB) API - For booted child

   ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MB_GetMultiBootParam

  Description:  Acquires a pointer to the location where parent information for a multiboot child is maintained.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static inline const MBParam *MB_GetMultiBootParam(void)
{
    return (const MBParam *)HW_WM_BOOT_BUF;
}


/*---------------------------------------------------------------------------*
  Name:         MB_IsMultiBootChild

  Description:  Determines whether a device is a multiboot child.
                If TRUE, MB_GetMultiBootParentBssDesc() is valid.

  Arguments:    None.

  Returns:      TRUE if a multiboot child.
 *---------------------------------------------------------------------------*/

static inline BOOL MB_IsMultiBootChild(void)
{
    return MB_GetMultiBootParam()->boot_type == MB_TYPE_MULTIBOOT;
}


/*---------------------------------------------------------------------------*
  Name:         MB_GetMultiBootParentBssDesc

  Description:  If a multiboot child, returns a pointer to parent information.

  Arguments:    None.

  Returns:      Returns a valid pointer if the device is a multiboot child. Otherwise returns NULL.
 *---------------------------------------------------------------------------*/

static inline const MBParentBssDesc *MB_GetMultiBootParentBssDesc(void)
{
    return MB_IsMultiBootChild()? &MB_GetMultiBootParam()->parent_bss_desc : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MB_GetMultiBootDownloadParameter

  Description:  If a multiboot child, the user-defined extended parameter specified at download is returned.
                

  Arguments:    None.

  Returns:      Returns a valid pointer if the device is a multiboot child. Otherwise returns NULL.
 *---------------------------------------------------------------------------*/

static inline const u8 *MB_GetMultiBootDownloadParameter(void)
{
    return MB_IsMultiBootChild()? (const u8 *)HW_DOWNLOAD_PARAMETER : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MB_ReadMultiBootParentBssDesc

  Description:  Based on information inherited from the multiboot parent, sets information in the WMBssDesc structure that is used in the WM_StartConnect function.
                
                

  Arguments:    p_desc:            pointer to destination WMBssDesc
                                    (must be aligned 32-bytes)
                parent_max_size:   max packet length of parent in MP-protocol.
                                    (must be equal to parent's WMParentParam!)
                child_max_size:    max packet length of child in MP-protocol.
                                    (must be equal to parent's WMParentParam!)
                ks_flag:           if using key-sharing mode, TRUE.
                                    (must be equal to parent's WMParentParam!)
                cs_flag:           if using continuous mode, TRUE.
                                    (must be equal to parent's WMParentParam!)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MB_ReadMultiBootParentBssDesc(WMBssDesc *p_desc,
                                      u16 parent_max_size, u16 child_max_size, BOOL ks_flag,
                                      BOOL cs_flag);

/*---------------------------------------------------------------------------*
  Name:         MB_SetLifeTime

  Description:  Explicitly specifies the lifetime of MB wireless operation.
                Default value: ( 0xFFFF, 40, 0xFFFF, 40 ).

  Arguments:    tableNumber:   CAM table number that sets the lifetime.
                camLifeTime:   Lifetime of CAM table
                frameLifeTime:   Lifetime in frame units
                mpLifeTime:    MP communication lifetime

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MB_SetLifeTime(u16 tableNumber, u16 camLifeTime, u16 frameLifeTime, u16 mpLifeTime);

/*---------------------------------------------------------------------------*
  Name:         MB_SetPowerSaveMode

  Description:  Configures power-save mode.
                Power-save mode is enabled by default.
                This function is an option for stable operation during situations where power consumption is not a concern. This option should not be used by typical game applications unless they are being operated in an environment in which connection to power is guaranteed.
                
                
                

  Arguments:    enable:    To enable, TRUE; to disable, FALSE.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MB_SetPowerSaveMode(BOOL enable);

/*---------------------------------------------------------------------------*/
/* Internal Functions */

/*---------------------------------------------------------------------------*
  Name:         MBi_AdjustSegmentMapForCloneboot

  Description:  Adjusts placement of the .parent section for clone booting.
                This originally had a fixed placement at ARM9.static[00005000-00007000], but this is not necessarily true anymore because extended TWL-SDK features cause the size of crt0.o to be increased.
                
                
                It is therefore not possible for hybrid applications to clone boot at this point in time. Nevertheless, this is tentatively possible (without any guarantees) if you continue to call this function manually and change the arguments to emuchild.exe.
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_AdjustSegmentMapForCloneboot();


/*****************************************************************************/
/* Obsolete Interfaces */

#define MB_StartParentEx(channel)    SDK_ERR("MB_StartParentEx() is discontinued. please use MB_StartParentFromIdle()\n")
#define MB_EndEx()    SDK_ERR("MB_EndEx() is discontinued. please use MB_EndToIdle()\n")



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // NITRO_MB_MB_H_
