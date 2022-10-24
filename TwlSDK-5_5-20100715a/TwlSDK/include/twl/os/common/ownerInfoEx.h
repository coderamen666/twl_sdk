/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     ownerInfoEx.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-07-10#$
  $Rev: 11355 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_COMMON_OWNERINFO_EX_H_
#define TWL_OS_COMMON_OWNERINFO_EX_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <twl/types.h>
#include    <twl/spec.h>
#include    <nitro/std.h>
#ifndef SDK_TWL
#include    <nitro/hw/common/mmap_shared.h>
#else
#include    <twl/hw/common/mmap_shared.h>
#endif

#include <nitro/os/common/ownerInfo.h>


/*---------------------------------------------------------------------------*
    Constants
 *---------------------------------------------------------------------------*/
// Region code
typedef enum OSTWLRegionCode
{
    OS_TWL_REGION_JAPAN     = 0,
    OS_TWL_REGION_AMERICA   = 1,
    OS_TWL_REGION_EUROPE    = 2,
    OS_TWL_REGION_AUSTRALIA = 3,
    OS_TWL_REGION_CHINA     = 4,
    OS_TWL_REGION_KOREA     = 5,
    OS_TWL_REGION_MAX
} OSTWLRegion;

#define OS_TWL_HWINFO_SERIALNO_LEN_MAX        15                                // The maximum length for a system's serial number (this can extend as far as 14 bytes because it includes the terminating character)
#define OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN   16                                // A unique ID that can be moved
#define OS_TWL_NICKNAME_LENGTH                OS_OWNERINFO_NICKNAME_MAX         // Nickname Length
#define OS_TWL_COMMENT_LENGTH                 OS_OWNERINFO_COMMENT_MAX          // Comment Length
#define OS_TWL_PCTL_PASSWORD_LENGTH           4                                 // Number of password digits
#define OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX  64                                // The maximum (MAX) number of UTF-16 characters in the answer to the secret question


/*---------------------------------------------------------------------------*
    Structures
 *---------------------------------------------------------------------------*/

// TWL owner information
typedef struct OSOwnerInfoEx
{
    u8      language;                  // Language code
    u8      favoriteColor;             // Favorite color (0 to 15)
    OSBirthday birthday;               // Birthdate
    u16     nickName[OS_OWNERINFO_NICKNAME_MAX + 1];
    // Nickname (maximum 10 characters of Unicode (UTF16), no terminal code)
    u16     nickNameLength;            // Number of characters in nickname (0 to 10)
    u16     comment[OS_OWNERINFO_COMMENT_MAX + 1];
    // Comment (maximum 26 characters of Unicode (UTF16), no terminal code)
    u16     commentLength;             // Number of characters in comment (0 to 26)
    // Data only available on a TWL system
    u8      country;                   // Country and region code
    u8      padding;

}
OSOwnerInfoEx;


// Parental Controls
// Rating organizations
typedef enum OSTWLRatingOgn
{
    OS_TWL_PCTL_OGN_CERO        =   0,  // CERO                         // Japan
    OS_TWL_PCTL_OGN_ESRB        =   1,  // ESRB                         // North America
    OS_TWL_PCTL_OGN_RESERVED2   =   2,  // BBFC [obsolete]
    OS_TWL_PCTL_OGN_USK         =   3,  // USK                          // Germany
    OS_TWL_PCTL_OGN_PEGI_GEN    =   4,  // PEGI general                 // Europe
    OS_TWL_PCTL_OGN_RESERVED5   =   5,  // PEGI Finland [obsolete]
    OS_TWL_PCTL_OGN_PEGI_PRT    =   6,  // PEGI Portugal                // Portugal
    OS_TWL_PCTL_OGN_PEGI_BBFC   =   7,  // PEGI and BBFC Great Britain   // England
    OS_TWL_PCTL_OGN_COB         =   8,  // COB                          // Australia, New Zealand
    OS_TWL_PCTL_OGN_GRB         =   9,  // GRB                          // South Korea

    OS_TWL_PCTL_OGN_MAX         =   16

} OSTWLRatingOgn;


// Parental Controls information
typedef struct OSTWLParentalControl
{
    struct {
        u32             isSetParentalControl : 1;   // Have Parental Controls already been set?
        u32             pictoChat :1;               // Is starting PictoChat restricted?
        u32             dsDownload :1;              // Is starting DS Download Play restricted?
        u32             browser :1;                 // Is starting a full browser restricted?
        u32             prepaidPoint :1;            // Is the use of points restricted?
        u32             photoExchange : 1;          // Is exchanging photos restricted?
        u32             ugc : 1;                    // Is user-created content restricted?
        u32             rsv :25;
    } flags;
    u8                  rsv1[ 3 ];
    u8                  ogn;                        // Rating organizations
    u8                  ratingAge;                  // Rating value (age)
    u8                  secretQuestionID;           // Secret question ID
    u8                  secretAnswerLength;         // Number of characters in the answer to the secret question
    u8                  rsv[ 2 ];
    char                password[ OS_TWL_PCTL_PASSWORD_LENGTH + 1 ];   // Password (with terminating character)
    u16                 secretAnswer[ OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF-16 answer (with terminating character) to the secret question
} OSTWLParentalControl;   //  148 bytes

// Format of ratings information in the application
#define OS_TWL_PCTL_OGNINFO_ENABLE_MASK     0x80
#define OS_TWL_PCTL_OGNINFO_ALWAYS_MASK     0x40
#define OS_TWL_PCTL_OGNINFO_AGE_MASK        0x1f


/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetOwnerInfoEx

  Description:  Gets the owner information. TWL version.
                Information that also exists on NITRO systems is taken from the same location as the NITRO version.
                The language code ('language') may differ between NITRO and TWL systems and will therefore be taken from the TWL information.
                

  Arguments:    info:        Pointer to the buffer getting the owner information.
                            Data gets written to this buffer.
                            (*) TWL information only available in TWL mode.
                               (It is always 0 when the program is not running on a TWL system)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetOwnerInfoEx(OSOwnerInfoEx *info);

/*---------------------------------------------------------------------------*
  Name:         OS_IsAvailableWireless

  Description:  Gets information on whether the wireless module's RF unit is enabled.

  Arguments:    None.

  Returns:      TRUE when enabled and FALSE when disabled.
                This is always TRUE when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
BOOL    OS_IsAvailableWireless(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsAgreeEULA

  Description:  Gets a value indicating whether the EULA has been accepted on this system.

  Arguments:    None.

  Returns:      BOOL - TRUE if the EULA has been already accepted and FALSE otherwise.
                This is always FALSE when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
BOOL    OS_IsAgreeEULA(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetAgreeEULAVersion

  Description:  Gets the version of the EULA that was accepted on this system.

  Arguments:    None.

  Returns:      agreedEulaVersion: The accepted EULA version (0-255).
 *---------------------------------------------------------------------------*/
u8    OS_GetAgreedEULAVersion( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetROMHeaderEULAVersion

  Description:  Gets the EULA version registered in the application's ROM header.

  Arguments:    None.

  Returns:      cardEulaVersion: The EULA version (0-255) registered in the application's ROM header.
 *---------------------------------------------------------------------------*/
u8    OS_GetROMHeaderEULAVersion( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetParentalControlPtr

  Description:  Gets a pointer to Parental Controls information.

  Arguments:    None.

  Returns:      Returns a pointer.
                Returns NULL when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
const OSTWLParentalControl* OS_GetParentalControlInfoPtr(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsParentalControledApp

  Description:  Determines whether Parental Controls should restrict application startup.
                

  Arguments:    appRatingInfo: Offset into the application's ROM header
                                    Specifies a pointer to the 16-byte ratings information placed at 0x2f0.
                                    

  Returns:      Returns TRUE when this is restricted by Parental Controls and the player should be prompted to enter a password.
                Returns FALSE if the application may be started without restriction.
                
 *---------------------------------------------------------------------------*/
BOOL    OS_IsParentalControledApp(u8* appRatingInfo);

/*---------------------------------------------------------------------------*
  Name:         OS_GetMovableUniqueID

  Description:  Gets the unique ID (included in the hardware information) that can be moved between systems.

  Arguments:    pUniqueID: Storage location (stores OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN bytes).
                         (This is filled in with 0's when the program is not running on a TWL system)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetMovableUniqueID(u8 *pUniqueID);

/*---------------------------------------------------------------------------*
  Name:         OS_GetUniqueIDPtr

  Description:  Gets a pointer to a unique ID (included in the hardware information) that can be moved between systems.

  Arguments:    None.

  Returns:      Returns a pointer.
                Returns NULL when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
const u8* OS_GetMovableUniqueIDPtr(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsForceDisableWireless

  Description:  Gets information on whether wireless features are to be forcibly disabled (this is included in secure hardware information).
                

  Arguments:    None.

  Returns:      TRUE when it is forcibly disabled and FALSE when it is not disabled.
                This is always FALSE when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
BOOL    OS_IsForceDisableWireless(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegion

  Description:  Gets region information.

  Arguments:    None.

  Returns:      Returns the region number.
                Always returns 0 when the program is not running on a TWL system.
 *---------------------------------------------------------------------------*/
OSTWLRegion  OS_GetRegion(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegionCodeA3

  Description:  Gets the string that corresponds to a region code.

  Arguments:    region: Region code

  Returns:      Returns a pointer to the string that corresponds to the region code.
 *---------------------------------------------------------------------------*/
const char* OS_GetRegionCodeA3(OSTWLRegion region);

/*---------------------------------------------------------------------------*
  Name:         OS_GetISOCountryCodeA2

  Description:  Converts a TWL-specific country or region code into ISO 3166-1 alpha-2.

  Arguments:    twlCountry: Country or region code

  Returns:      Returns a pointer to the string that corresponds to the country or region code.
 *---------------------------------------------------------------------------*/
const char* OS_GetISOCountryCodeA2(u8 twlCountry);

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPictoChatBoot

  Description:  Determines whether a Parental Controls setting has been applied to restrict PictoChat from starting.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPictoChatBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.pictoChat;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictBrowserBoot

  Description:  Determines whether a Parental Controls setting has been applied to restrict the Nintendo DSi Browser from starting.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictBrowserBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.browser;
}


/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPrepaidPoint

  Description:  Determines whether a Parental Controls setting has been applied to restrict behavior related to Nintendo DSi Points specifications.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPrepaidPoint( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.prepaidPoint;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPhotoExchange

  Description:  Determines whether a Parental Controls setting has been applied to restrict the exchange of photo data.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPhotoExchange( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.photoExchange;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictUGC

  Description:  Determines whether a Parental Controls setting has been applied to restrict sending and receiving some user-created content.
                

  Arguments:    None.

  Returns:      TRUE if restrictions have been applied and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictUGC( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.ugc;
}

/*---------------------------------------------------------------------------*
  Name:         OS_CheckParentalControlPassword

  Description:  Determines whether the given string matches the password that removes Parental Controls.
                

  Arguments:    password: The string to be checked

  Returns:      TRUE if they match and FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_CheckParentalControlPassword( const char *password )
{
    return password && (STD_CompareLString(password, OS_GetParentalControlInfoPtr()->password, OS_TWL_PCTL_PASSWORD_LENGTH) == 0);
}

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* TWL_OS_COMMON_OWNERINFO_EX_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
