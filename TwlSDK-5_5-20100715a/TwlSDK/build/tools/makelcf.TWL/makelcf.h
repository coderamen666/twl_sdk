/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     makelcf.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-11#$
  $Rev: 11140 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef	MAKELCF_H_
#define	MAKELCF_H_

#include "misc.h"

/*============================================================================
 *  CONTAINER
 */
typedef struct tAfterList
{
    struct tAfter *head;
    struct tAfter *tail;
}
tAfterList;

typedef struct tAfter
{
    struct tAfter *next;
    const char *name;
}
tAfter;

typedef struct tObjectList
{
    struct tObject *head;
    struct tObject *tail;
}
tObjectList;

typedef enum
{
    OBJTYPE_NONE,
    OBJTYPE_FILE,                      // Regular file
    OBJTYPE_STAR,                      // "*"
    OBJTYPE_GROUP,                     // GROUP(xxxx)
    OBJTYPE_OBJECT                     // OBJECT(yyy,zzz)
}
tObjectType;

#define isNeedSection(obj)      ((obj)->objectType != OBJTYPE_OBJECT)

typedef struct tObject
{
    struct tObject *next;
    const char *objectName;
    const char *sectionName;
    tObjectType objectType;
}
tObject;

typedef struct tOverlayList
{
    struct tOverlay *head;
    struct tOverlay *tail;
    u32     num;
}
tOverlayList;

typedef enum
{
    MEMTYPE_NONE = 0,
    MEMTYPE_MAIN,
    MEMTYPE_MAINEX,
    MEMTYPE_ITCM,
    MEMTYPE_DTCM,
    MEMTYPE_ITCM_BSS,
    MEMTYPE_DTCM_BSS,
    MEMTYPE_SHARED,
    MEMTYPE_WRAM,
    MEMTYPE_WRAM_BSS,
    MEMTYPE_VRAM,
    MEMTYPE_DUMMY,
}
tMemType;

typedef struct tOverlay
{
    struct tOverlay *next;
    u32     id;
    const char *name;
    const char *group;
    u32     address;
    char    compressSpec;
    struct tAfterList afters;
    struct tObjectList objects;
    struct tObjectList libraries;
    struct tObjectList searchSymbols;
    struct tObjectList forces;
    tMemType memtype;

}
tOverlay;

typedef struct tStatic
{
    const char *name;
    u32     address;
    struct tObjectList objects;
    struct tObjectList libraries;
    struct tObjectList searchSymbols;
    struct tObjectList forces;
    u32     stacksize;
    u32     stacksize_irq;
    tMemType memtype;

}
tStatic;

typedef struct
{
    const char *overlaydefs;
    const char *overlaytable;
    const char *ltdoverlaydefs;
    const char *ltdoverlaytable;
    const char *suffix;
    const char *flxsuffix;
    const char *ltdsuffix;

}
tProperty;

BOOL    AddOverlay(const char *overlayName);
BOOL    AddAutoload(const char *overlayName);
BOOL    AddLtdoverlay(const char *overlayName);
BOOL    AddLtdautoload(const char *overlayName);
BOOL    OverlaySetId(u32 id);
BOOL    OverlaySetGroup(const char *overlayGroup);
BOOL    OverlaySetAddress(u32 address);
BOOL    OverlayAddAfter(const char *overlayName);
BOOL    OverlayAddObject(const char *objectName, tObjectType objectType);
BOOL    OverlaySetObjectSection(const char *sectionName);
BOOL    OverlayAddLibrary(const char *objectName, tObjectType objectType);
BOOL    OverlaySetLibrarySection(const char *sectionName);
BOOL    OverlayAddSearchSymbol(const char *searchName);
BOOL    OverlayAddForce(const char *forcename);
BOOL    OverlaySetCompress(const char *compressSpec);

BOOL    StaticSetName(const char *fileName);
BOOL    StaticSetAddress(u32 address);
BOOL    StaticAddObject(const char *objectName, tObjectType objectType);
BOOL    StaticSetObjectSection(const char *sectionName);
BOOL    StaticAddLibrary(const char *objectName, tObjectType objectType);
BOOL    StaticSetLibrarySection(const char *sectionName);
BOOL    StaticAddSearchSymbol(const char *searchName);
BOOL    StaticAddForce(const char *forceName);
BOOL    StaticSetStackSize(u32 stacksize);
BOOL    StaticSetStackSizeIrq(u32 stacksize);
BOOL    PropertySetOverlayDefs(const char *filename);
BOOL    PropertySetOverlayTable(const char *filename);
BOOL    PropertySetLtdoverlayDefs(const char *filename);
BOOL    PropertySetLtdoverlayTable(const char *filename);
BOOL    PropertySetSuffix(const char *suffix);
BOOL    PropertySetFlxsuffix(const char *flxsuffix);
BOOL    PropertySetLtdsuffix(const char *ltdsuffix);

BOOL    CheckSpec(void);
void    DumpSpec(void);
BOOL    isSame(const char *s1, const char *s2);

int     ParseSpecFile(const char *filename);

extern tStatic Static;
extern tProperty Property;
extern tOverlayList OverlayList;
extern tOverlayList AutoloadList;
extern tOverlayList LtdoverlayList;
extern tOverlayList LtdautoloadList;

#define	DEFAULT_OVERLAYDEFS	"%_defs"
#define	DEFAULT_OVERLAYTABLE	"%_table"
#define	DEFAULT_LTDOVERLAYDEFS	"%_defs"
#define	DEFAULT_LTDOVERLAYTABLE	"%_table"
#define	DEFAULT_SUFFIX		".sbin"
#define	DEFAULT_FLXSUFFIX	".TWL.FLX.sbin"
#define	DEFAULT_LTDSUFFIX	".TWL.LTD.sbin"
/* NITRO-SDK4.0RC: increased 0x400->0x800 */
#define	DEFAULT_IRQSTACKSIZE	0x800

int     spec_yyparse(void);
int     spec_yylex(void);
void    spec_yyerror(const char *str);


/*============================================================================
 *  TOKEN BUFFER
 */
typedef struct tTokenBuffer
{
    int     id;
    const char *string;
    int     loop_end;
}
tTokenBuffer;

typedef struct tLoopStack
{
    int     id;
    int     start;
}
tLoopStack;

#define	TOKEN_LEN		65536
#define	LOOP_STACK_MAX		32

int     ParseTlcfFile(const char *filename);
int     CreateLcfFile(const char *filename);

extern tTokenBuffer tokenBuffer[TOKEN_LEN];
extern int tokenBufferEnd;

int     tlcf_yyparse(void);
int     tlcf_yylex(void);
void    tlcf_yyerror(const char *str);
void    lcf_error(const char *fmt, ...);

extern BOOL DebugMode;
void    debug_printf(const char *str, ...);

typedef struct
{
    char   *string;
    tObjectType type;
}
tObjectName;

// for IF-ELSE-ENDIF
#define	COND_STACK_MAX		32

typedef enum
{
    COND_BLOCK_NOCOND,                 // non conditional block
    COND_BLOCK_IF,                     // if(){} block
    COND_BLOCK_ELSE                    // else{} block
}
tCondBlock;

typedef struct
{
    BOOL    valid;                     // condition is TRUE or FALSE
    tCondBlock block;                  // regular/if{}/else{}
}
tCondStack;

#endif //MAKELCF_H_
