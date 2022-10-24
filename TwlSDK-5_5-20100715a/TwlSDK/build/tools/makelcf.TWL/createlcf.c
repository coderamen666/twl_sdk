/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     createlcf.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-11#$
  $Rev: 11140 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<strings.h>
#include	<io.h>                 // setmode()
#include	<fcntl.h>              // setmode()
#include	<stdarg.h>             // va_start(),va_end()
#ifdef	SDK_TWL
#include	<twl/hw/common/mmap_shared.h>
#include	<twl/hw/ARM9/mmap_global.h>
#include	<twl/hw/ARM9/mmap_main.h>
#else
#include	<nitro_win32.h>
#endif
#include	"makelcf.h"
#include	"defval.h"
#include	"tlcf.tab.h"

static void PutString(const char *str);
static void PutAddress(u32 address);
static BOOL PutObject(const tObject * object, const char *secName);
static void PutStaticCrt0Object(const char *secName);
static void PutStaticObject(const char *secName);
static void PutOverlayName(void);
static void PutOverlayGroup(void);
static void PutOverlayObject(const char *secName);
static void PutOverlayAddress(void);
static void PutOverlayCompress(void);
static BOOL isObjectOK(const tObject * obj, const char *section, char filter_type);
static BOOL isSectionWildcard(const char *section);
static BOOL isSectionRegular(const char *section);
static int ForeachObjects(tObject ** ptrObject, tObject * start, int n);
static int ForeachStaticObjects(int n);
static int ForeachOverlayObjects(int n);
static int ForeachOverlays(int n);
static int PutToken(int n);
static void PutTokenBuffer(int start, int end);
static void PutTokenBufferAll(void);

FILE   *fp_out;
tObject *StaticObject = NULL;
tObject *StaticLibrary = NULL;
tObject *StaticSearchSymbol = NULL;
tObject *StaticForce = NULL;
tOverlay *Autoload = NULL;
tObject *AutoloadObject = NULL;
tObject *AutoloadLibrary = NULL;
tObject *AutoloadSearchSymbol = NULL;
tObject *AutoloadForce = NULL;
tOverlay *Overlay = NULL;
tObject *OverlayObject = NULL;
tObject *OverlayLibrary = NULL;
tObject *OverlaySearchSymbol = NULL;
tObject *OverlayForce = NULL;
tOverlay *Ltdautoload = NULL;
tObject *LtdautoloadObject = NULL;
tObject *LtdautoloadLibrary = NULL;
tObject *LtdautoloadSearchSymbol = NULL;
tObject *LtdautoloadForce = NULL;
tOverlay *Ltdoverlay = NULL;
tObject *LtdoverlayObject = NULL;
tObject *LtdoverlayLibrary = NULL;
tObject *LtdoverlaySearchSymbol = NULL;
tObject *LtdoverlayForce = NULL;

void lcf_error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
}

/*============================================================================
 *  Put Tokens
 */
static void PutString(const char *str)
{
    if (str)
        fprintf(fp_out, "%s", str);
}

static void PutAddress(u32 address)
{
    fprintf(fp_out, "0x%08lx", address);
}

static void PutNumber(u32 num)
{
    fprintf(fp_out, "%ld", num);
}


static int GetCharPos(const char *str, char target)
{
    int     n = 0;

    if (str)
    {
        while ('\0' != str[n] && target != str[n])
        {
            n++;
        }
    }
    return n;
}

static char *GetObjectParam(const char *str, char start, char end)
{
    int     pos_start = GetCharPos(str, start);
    int     pos_end = GetCharPos(str, end);
    int     len = pos_end - pos_start;
    char   *buffer;

    if (len > 0)
    {
        buffer = Alloc(len);
        strncpy(buffer, str + pos_start + 1, len - 1);
        buffer[len - 1] = '\0';
        return buffer;
    }
    return NULL;
}

static char *GetObjectSection(const char *str)
{
    return GetObjectParam(str, '=', ':');       // return NULL if no section
}

static char GetObjectModifier(const char *str)
{
    char   *mod = GetObjectParam(str, ':', '\0');
    char    ret;

    ret = mod ? mod[0] : '\0';
    Free(&mod);

    return ret;

}

static tMemType GetMemTypeParam(const char *str)
{
    char   *mod = GetObjectParam(str, '=', ':');
    int     i;

    static struct
    {
        tMemType type;
        char   *name;
    }
    table[] =
    {
        {
        MEMTYPE_MAIN, "MAIN"}
        ,
        {
        MEMTYPE_MAINEX, "MAINEX"}
        ,
        {
        MEMTYPE_ITCM, "ITCM"}
        ,
        {
        MEMTYPE_DTCM, "DTCM"}
        ,
        {
        MEMTYPE_ITCM_BSS, "ITCM.BSS"}
        ,
        {
        MEMTYPE_DTCM_BSS, "DTCM.BSS"}
        ,
        {
        MEMTYPE_SHARED, "SHARED"}
        ,
        {
        MEMTYPE_WRAM, "WRAM"}
        ,
        {
        MEMTYPE_WRAM_BSS, "WRAM.BSS"}
        ,
        {
        MEMTYPE_VRAM, "VRAM"}
        ,
        {
        MEMTYPE_NONE, "*"}
    ,};

    for (i = 0; table[i].type != MEMTYPE_NONE; i++)
    {
        if (isSame(mod, table[i].name))
        {
            break;
        }
    }
    Free(&mod);

    return table[i].type;
}

//
// Decides to which memory region the section specified by name, address belongs
//
static tMemType GetSectionMemType(const char *name, u32 address)
{
    BOOL    isARM9 = (Static.address < 0x02300000);     // Actual decision value
    tMemType memtype = MEMTYPE_NONE;

    if (isARM9)
    {
        if (isSame(name, "ITCM"))
            memtype = MEMTYPE_ITCM;
        else if (isSame(name, "DTCM"))
            memtype = MEMTYPE_DTCM;
        else if (isSame(name, "WRAM"))
            memtype = MEMTYPE_WRAM;
        else if (isSame(name, "ITCM.BSS"))
            memtype = MEMTYPE_ITCM_BSS;
        else if (isSame(name, "DTCM.BSS"))
            memtype = MEMTYPE_DTCM_BSS;
        else if (isSame(name, "WRAM.BSS"))
            memtype = MEMTYPE_WRAM_BSS;
        else if (HW_ITCM_IMAGE <= address && address < HW_ITCM_END)
            memtype = MEMTYPE_ITCM;
        else if (HW_MAIN_MEM_SUB <= address && address < HW_MAIN_MEM_SUB_END)
            memtype = MEMTYPE_DTCM;
        else if (HW_MAIN_MEM_MAIN <= address && address < HW_MAIN_MEM_MAIN_END)
            memtype = MEMTYPE_MAIN;
        else if (HW_MAIN_MEM_MAIN_END <= address && address < HW_MAIN_MEM_DEBUGGER)
            memtype = MEMTYPE_MAINEX;
        else if (HW_MAIN_MEM_SHARED <= address && address < HW_MAIN_MEM_SHARED_END)
            memtype = MEMTYPE_SHARED;
        else if (HW_WRAM <= address && address < HW_WRAM_END)
            memtype = MEMTYPE_WRAM;
        else if (HW_BG_PLTT <= address && address < HW_CTRDG_ROM)
            memtype = MEMTYPE_VRAM;
    }
    else
    {
        if (isSame(name, "WRAM"))
            memtype = MEMTYPE_WRAM;
        else if (HW_MAIN_MEM_MAIN <= address && address < HW_MAIN_MEM_SUB_END)
            memtype = MEMTYPE_MAIN;
        else if (HW_MAIN_MEM_SHARED <= address && address < HW_MAIN_MEM_SHARED_END)
            memtype = MEMTYPE_SHARED;
        else if (HW_WRAM <= address && address < HW_WRAM_END)
            memtype = MEMTYPE_WRAM;
        else if (HW_BG_PLTT <= address && address < HW_CTRDG_ROM)
            memtype = MEMTYPE_VRAM;
    }
    return memtype;
}

//
// Return overlay that corresponds to the section specified by name
//
static tOverlay *GetOverlayByName(const char *name)
{
    tOverlay *t;

    for (t = OverlayList.head; t; t = t->next)
    {
        if (isSame(t->name, name))
            return t;
    }
    for (t = AutoloadList.head; t; t = t->next)
    {
        if (isSame(t->name, name))
            return t;
    }
    for (t = LtdoverlayList.head; t; t = t->next)
    {
        if (isSame(t->name, name))
            return t;
    }
    for (t = LtdautoloadList.head; t; t = t->next)
    {
        if (isSame(t->name, name))
            return t;
    }
    return NULL;
}

//
// Decides to which memory region the section specified by name belongs
//
static tMemType RecgSectionMemTypeByName(const char *name)
{
    tOverlay *overlay;
    tMemType memtype, mt;
    tAfter *after;

    if (isSame(name, Static.name))
    {
        Static.memtype = GetSectionMemType(Static.name, Static.address);
        return Static.memtype;
    }

    overlay = GetOverlayByName(name);
    if (!overlay)
    {
        lcf_error("Unknown section %s, Ignored.", name);
        return MEMTYPE_NONE;
    }

    if (overlay->memtype == MEMTYPE_NONE)
    {
        memtype = GetSectionMemType(overlay->name, overlay->address);

        if (memtype == MEMTYPE_NONE)
        {
            overlay->memtype = MEMTYPE_DUMMY;

            for (after = overlay->afters.head; after; after = after->next)
            {
                mt = RecgSectionMemTypeByName(after->name);

                if (mt == MEMTYPE_DUMMY)
                {
                    // Looping
                    lcf_error("Recursive AFTER reference in section %s, Ignored.", name);
                    overlay->memtype = MEMTYPE_NONE;
                    return MEMTYPE_NONE;
                }
                else if (memtype == MEMTYPE_NONE)
                {
                    memtype = mt;
                }
                else if (memtype != mt)
                {
                    // Different memtype sections are collected in AFTER
                    lcf_error("Bad address mapping in section %s, Ignored.", name);
                    overlay->memtype = MEMTYPE_NONE;
                    return MEMTYPE_NONE;
                }
            }
        }
        overlay->memtype = memtype;
    }
    return overlay->memtype;
}


//====== COMMONS ======

/*---------------------------------------------------------------------------*
  Name:         isObjectOK

  Description:  Determines whether the designated object matches the conditions designated by the section or the filter_type.
                

  Arguments:    obj         : object
                              (What is actually determined is 
                              obj->sectionName, obj->objextType )
                section     : Designated section name
                filter_type : Qualifying options

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL isObjectOK(const tObject * obj, const char *section, char filter_type)
{
    //
    //  Checks pertaining to section naming
    //          A "*" can be used as the section wildcard.
    //          When the section is "*", all obj will pass
    //          When obj->section is "*", the section designation is a general section
    //          Pass for general sections (.text, .data, .bss, .init, and the like)
    //
    if (isSectionWildcard(section) ||
        (isSectionWildcard(obj->sectionName) && isSectionRegular(section)) ||
        isSame(obj->sectionName, section))
    {
        switch (filter_type)
        {
        case 'f':                     // only regular file
            if (obj->objectType == OBJTYPE_FILE)
                return TRUE;
            break;

        case '\0':                    // OK if NULL
            return TRUE;

        default:
            lcf_error("Unknown type setting Ignored.");
            break;
        }
    }
    return FALSE;
}

static BOOL isSectionWildcard(const char *section)
{
    return !section || isSame(section, "*");
}

static BOOL isSectionRegular(const char *section)
{
    return (isSame(section, ".text") ||
            isSame(section, ".rodata") ||
            isSame(section, ".sdata") ||
            isSame(section, ".data") ||
            isSame(section, ".sbss") ||
            isSame(section, ".bss") ||
            isSame(section, ".init") ||
            isSame(section, ".exception") || isSame(section, ".ctor") || isSame(section, ".sinit"));
}

// Foreach Objects
static int ForeachObjects(tObject ** ptrObject, tObject * start, int n)
{
    tTokenBuffer *token;
    char   *section;
    char    filter;

    token = &tokenBuffer[n];
    section = GetObjectSection(token->string);
    filter = GetObjectModifier(token->string);

    debug_printf("section =[%s]\n", section ? section : "");
    debug_printf("filter  =[%c]\n", filter ? filter : ' ');

    while (start)
    {
        if (isObjectOK(start, section, filter))
        {
            *ptrObject = start;
            PutTokenBuffer(n + 1, token->loop_end);
        }

        start = start->next;
    }

    Free(&section);

    *ptrObject = NULL;
    return token->loop_end;
}

// Foreach Objects Force
static int ForeachObjectsForce(tObject ** ptrObject, tObject * start, int n)
{
    tTokenBuffer *token;
    char   *section;
    char    filter;

    token = &tokenBuffer[n];
    section = GetObjectSection(token->string);
    filter = GetObjectModifier(token->string);

    debug_printf("section =[%s]\n", section ? section : "");
    debug_printf("filter  =[%c]\n", filter ? filter : ' ');

    while (start)
    {
        *ptrObject = start;
        PutTokenBuffer(n + 1, token->loop_end);

        start = start->next;
    }

    Free(&section);

    *ptrObject = NULL;
    return token->loop_end;
}

// Object
static BOOL PutObject(const tObject * object, const char *sectionName)
{
    if (object)
    {
        char    modifier = GetObjectModifier(sectionName);

        PutString(ResolveStringModifier(object->objectName, modifier));

        if (isNeedSection(object))
        {
            char   *section = GetObjectSection(sectionName);

            if (section)
            {
                PutString(" (");
                PutString(section);
                PutString(")");
                Free(&section);
            }
        }
        return TRUE;
    }
    return FALSE;
}

// Address
static void PutSectionAddress(tOverlay * overlay)
{
    if (overlay->address)
    {
        PutAddress(overlay->address);
    }
    else
    {
        tAfter *t = overlay->afters.head;
        if (t)
        {
            fprintf(fp_out, "AFTER(%s", t->name);
            while (NULL != (t = t->next))
            {
                fprintf(fp_out, ",%s", t->name);
            }
            fprintf(fp_out, ")");
        }
    }
}


//======== STATIC ========//

// Crt0 Object
static void PutStaticCrt0Object(const char *sectionName)
{
    const char *objectName = SearchDefVal("CRT0_O");

    if(objectName)
    {
        char*   section = GetObjectSection(sectionName);

        PutString(objectName);

        if (section)
        {
            PutString(" (");
            PutString(section);
            PutString(")");
            Free(&section);
        }
    }
    else
    {
        lcf_error("No <STATIC.CRT0.OBJECT> Ignored.");
    }
}

// Object
static void PutStaticObject(const char *sectionName)
{
    if (!PutObject(StaticObject, sectionName))
    {
        lcf_error("No <FOREACH.STATIC.OBJECTS> Ignored.");
    }
}

// Foreach Objects
static int ForeachStaticObjects(int n)
{
    return ForeachObjects(&StaticObject, Static.objects.head, n);
}

// Library
static void PutStaticLibrary(const char *sectionName)
{
    if (!PutObject(StaticLibrary, sectionName))
    {
        lcf_error("No <FOREACH.STATIC.LIBRARIES> Ignored.");
    }
}

// Foreach Libraries
static int ForeachStaticLibraries(int n)
{
    return ForeachObjects(&StaticLibrary, Static.libraries.head, n);
}

// SearchSymbol
static void PutStaticSearchSymbol(const char *dummy)
{
    if (!PutObject(StaticSearchSymbol, NULL))
    {
        lcf_error("No <FOREACH.STATIC.SEARCHSYMBOLS> Ignored.");
    }
}

// Foreach SearchSymbols
static int ForeachStaticSearchSymbols(int n)
{
    return ForeachObjects(&StaticSearchSymbol, Static.searchSymbols.head, n);
}

// Force
static void PutStaticForce(const char *dummy)
{
    if (!PutObject(StaticForce, NULL))
    {
        lcf_error("No <FOREACH.STATIC.FORCES> Ignored.");
    }
}

// Foreach Forces
static int ForeachStaticForces(int n)
{
    return ForeachObjectsForce(&StaticForce, Static.forces.head, n);
}


//======== AUTOLOAD ========//

// Id
static void PutAutoloadId(void)
{
    if (Autoload)
        PutNumber(Autoload->id);
    else
        lcf_error("No <FOREACH.AUTOLOADS> Ignored.");
}

// Name
static void PutAutoloadName(void)
{
    if (Autoload)
        PutString(Autoload->name);
    else
        lcf_error("No <FOREACH.AUTOLOADS> Ignored.");
}

// Address
static void PutAutoloadAddress(void)
{
    if (Autoload)
        PutSectionAddress(Autoload);
    else
        lcf_error("No <FOREACH.AUTOLOADS> Ignored.");
}

// Object
static void PutAutoloadObject(const char *sectionName)
{
    if (!PutObject(AutoloadObject, sectionName))
    {
        lcf_error("No <FOREACH.AUTOLOAD.OBJECTS> Ignored.");
    }
}

// Foreach Objects
static int ForeachAutoloadObjects(int n)
{
    return ForeachObjects(&AutoloadObject, Autoload->objects.head, n);
}

// Library
static void PutAutoloadLibrary(const char *sectionName)
{
    if (!PutObject(AutoloadLibrary, sectionName))
    {
        lcf_error("No <FOREACH.AUTOLOAD.LIBRARIES> Ignored.");
    }
}

// Foreach Libraries
static int ForeachAutoloadLibraries(int n)
{
    return ForeachObjects(&AutoloadLibrary, Autoload->libraries.head, n);
}

// SearchSymbol
static void PutAutoloadSearchSymbol(const char *dummy)
{
    if (!PutObject(AutoloadSearchSymbol, NULL))
    {
        lcf_error("No <FOREACH.AUTOLOAD.SEARCHSYMBOLS> Ignored.");
    }
}

// Foreach SearchSymbols
static int ForeachAutoloadSearchSymbols(int n)
{
    return ForeachObjects(&AutoloadSearchSymbol, Autoload->searchSymbols.head, n);
}

// Force
static void PutAutoloadForce(const char *dummy)
{
    if (!PutObject(AutoloadForce, NULL))
    {
        lcf_error("No <FOREACH.AUTOLOAD.FORCES> Ignored.");
    }
}

// Foreach Forces
static int ForeachAutoloadForces(int n)
{
    return ForeachObjectsForce(&AutoloadForce, Autoload->forces.head, n);
}

// Foreach Autoloads
static int ForeachAutoloads(int n)
{
    tOverlay *start = AutoloadList.head;

    while (start)
    {
        Autoload = start;
        PutTokenBuffer(n + 1, tokenBuffer[n].loop_end);
        start = start->next;
    }
    Autoload = NULL;
    return tokenBuffer[n].loop_end;
}


//======== OVERLAY ========//

// Id
static void PutOverlayId(void)
{
    if (Overlay)
        PutNumber(Overlay->id);
    else
        lcf_error("No <FOREACH.OVERLAYS> Ignored.");
}

// Name
static void PutOverlayName(void)
{
    if (Overlay)
        PutString(Overlay->name);
    else
        lcf_error("No <FOREACH.OVERLAYS> Ignored.");
}

// Group
static void PutOverlayGroup(void)
{
    if (Overlay)
    {
        // If a group has not been set, use the name of the overlay as a group
        PutString(Overlay->group ? Overlay->group : Overlay->name);
    }
    else
    {
        lcf_error("No <FOREACH.OVERLAYS> Ignored.");
    }
}

// Address
static void PutOverlayAddress(void)
{
    if (Overlay)
        PutSectionAddress(Overlay);
    else
        lcf_error("No <FOREACH.OVERLAYS> Ignored.");
}

// Compression spec
static void PutOverlayCompress(void)
{
    if (Overlay)
    {
        fprintf(fp_out, "%c", Overlay->compressSpec);
    }
    else
    {
        lcf_error("No <FOREACH.OVERLAYS> Ignored.");
    }
}

// Object
static void PutOverlayObject(const char *sectionName)
{
    if (!PutObject(OverlayObject, sectionName))
    {
        lcf_error("No <FOREACH.OVERLAY.OBJECTS> Ignored.");
    }
}

// Foreach Objects
static int ForeachOverlayObjects(int n)
{
    return ForeachObjects(&OverlayObject, Overlay->objects.head, n);
}

// Library
static void PutOverlayLibrary(const char *sectionName)
{
    if (!PutObject(OverlayLibrary, sectionName))
    {
        lcf_error("No <FOREACH.OVERLAY.LIBRARIES> Ignored.");
    }
}

// Foreach Libraries
static int ForeachOverlayLibraries(int n)
{
    return ForeachObjects(&OverlayLibrary, Overlay->libraries.head, n);
}

// SearchSymbol
static void PutOverlaySearchSymbol(const char *dummy)
{
    if (!PutObject(OverlaySearchSymbol, NULL))
    {
        lcf_error("No <FOREACH.OVERLAY.SEARCHSYMBOLS> Ignored.");
    }
}

// Foreach SearchSymbols
static int ForeachOverlaySearchSymbols(int n)
{
    return ForeachObjects(&OverlaySearchSymbol, Overlay->searchSymbols.head, n);
}

// Force
static void PutOverlayForce(const char *dummy)
{
    if (!PutObject(OverlayForce, NULL))
    {
        lcf_error("No <FOREACH.OVERLAY.FORCES> Ignored.");
    }
}

// Foreach Forces
static int ForeachOverlayForces(int n)
{
    return ForeachObjectsForce(&OverlayForce, Overlay->forces.head, n);
}

// Foreach Overlays
static int ForeachOverlays(int n)
{
    tOverlay *start = OverlayList.head;
    tTokenBuffer *token = &tokenBuffer[n];
    tMemType memtype = GetMemTypeParam(token->string);

    while (start)
    {
        if (memtype == MEMTYPE_NONE || memtype == RecgSectionMemTypeByName(start->name))
        {
            Overlay = start;
            PutTokenBuffer(n + 1, tokenBuffer[n].loop_end);
        }
        start = start->next;
    }
    Overlay = NULL;
    return tokenBuffer[n].loop_end;
}

//======== LTDAUTOLOAD ========//

// Id
static void PutLtdautoloadId(void)
{
    if (Ltdautoload)
        PutNumber(Ltdautoload->id);
    else
        lcf_error("No <FOREACH.LTDAUTOLOADS> Ignored.");
}

// Name
static void PutLtdautoloadName(void)
{
    if (Ltdautoload)
        PutString(Ltdautoload->name);
    else
        lcf_error("No <FOREACH.LTDAUTOLOADS> Ignored.");
}

// Address
static void PutLtdautoloadAddress(void)
{
    if (Ltdautoload)
        PutSectionAddress(Ltdautoload);
    else
        lcf_error("No <FOREACH.LTDAUTOLOADS> Ignored.");
}

// Object
static void PutLtdautoloadObject(const char *sectionName)
{
    if (!PutObject(LtdautoloadObject, sectionName))
    {
        lcf_error("No <FOREACH.LTDAUTOLOAD.OBJECTS> Ignored.");
    }
}

// Foreach Objects
static int ForeachLtdautoloadObjects(int n)
{
    return ForeachObjects(&LtdautoloadObject, Ltdautoload->objects.head, n);
}

// Library
static void PutLtdautoloadLibrary(const char *sectionName)
{
    if (!PutObject(LtdautoloadLibrary, sectionName))
    {
        lcf_error("No <FOREACH.LTDAUTOLOAD.LIBRARIES> Ignored.");
    }
}

// Foreach Libraries
static int ForeachLtdautoloadLibraries(int n)
{
    return ForeachObjects(&LtdautoloadLibrary, Ltdautoload->libraries.head, n);
}

// SearchSymbol
static void PutLtdautoloadSearchSymbol(const char *dummy)
{
    if (!PutObject(LtdautoloadSearchSymbol, NULL))
    {
        lcf_error("No <FOREACH.LTDAUTOLOAD.SEARCHSYMBOLS> Ignored.");
    }
}

// Foreach SearchSymbols
static int ForeachLtdautoloadSearchSymbols(int n)
{
    return ForeachObjects(&LtdautoloadSearchSymbol, Ltdautoload->searchSymbols.head, n);
}

// Force
static void PutLtdautoloadForce(const char *dummy)
{
   if (!PutObject(LtdautoloadForce, NULL))
    {
        lcf_error("No <FOREACH.LTDAUTOLOAD.FORCES> Ignored.");
    }
}

// Foreach Forces
static int ForeachLtdautoloadForces(int n)
{
    return ForeachObjectsForce(&LtdautoloadForce, Ltdautoload->forces.head, n);
}

// Foreach Ltdautoloads
static int ForeachLtdautoloads(int n)
{
    tOverlay *start = LtdautoloadList.head;

    while (start)
    {
        Ltdautoload = start;
        PutTokenBuffer(n + 1, tokenBuffer[n].loop_end);
        start = start->next;
    }
    Ltdautoload = NULL;
    return tokenBuffer[n].loop_end;
}

//======== LTDOVERLAY ========//

// Id
static void PutLtdoverlayId(void)
{
    if (Ltdoverlay)
        PutNumber(Ltdoverlay->id);
    else
        lcf_error("No <FOREACH.LTDOVERLAYS> Ignored.");
}

// Name
static void PutLtdoverlayName(void)
{
    if (Ltdoverlay)
        PutString(Ltdoverlay->name);
    else
        lcf_error("No <FOREACH.LTDOVERLAYS> Ignored.");
}

// Group
static void PutLtdoverlayGroup(void)
{
    if (Ltdoverlay)
    {
        // If a group has not been set, use the name of the overlay as a group
        PutString(Ltdoverlay->group ? Ltdoverlay->group : Ltdoverlay->name);
    }
    else
    {
        lcf_error("No <FOREACH.LTDOVERLAYS> Ignored.");
    }
}

// Address
static void PutLtdoverlayAddress(void)
{
    if (Ltdoverlay)
        PutSectionAddress(Ltdoverlay);
    else
        lcf_error("No <FOREACH.LTDOVERLAYS> Ignored.");
}

// Compression spec
static void PutLtdoverlayCompress(void)
{
    if (Ltdoverlay)
    {
        fprintf(fp_out, "%c", Ltdoverlay->compressSpec);
    }
    else
    {
        lcf_error("No <FOREACH.LTDOVERLAYS> Ignored.");
    }
}

// Object
static void PutLtdoverlayObject(const char *sectionName)
{
    if (!PutObject(LtdoverlayObject, sectionName))
    {
        lcf_error("No <FOREACH.LTDOVERLAY.OBJECTS> Ignored.");
    }
}

// Foreach Objects
static int ForeachLtdoverlayObjects(int n)
{
    return ForeachObjects(&LtdoverlayObject, Ltdoverlay->objects.head, n);
}

// Library
static void PutLtdoverlayLibrary(const char *sectionName)
{
    if (!PutObject(LtdoverlayLibrary, sectionName))
    {
        lcf_error("No <FOREACH.LTDOVERLAY.LIBRARIES> Ignored.");
    }
}

// Foreach Libraries
static int ForeachLtdoverlayLibraries(int n)
{
    return ForeachObjects(&LtdoverlayLibrary, Ltdoverlay->libraries.head, n);
}

// SearchSymbol
static void PutLtdoverlaySearchSymbol(const char *dummy)
{
    if (!PutObject(LtdoverlaySearchSymbol, NULL))
    {
        lcf_error("No <FOREACH.LTDOVERLAY.SEARCHSYMBOLS> Ignored.");
    }
}

// Foreach SearchSymbols
static int ForeachLtdoverlaySearchSymbols(int n)
{
    return ForeachObjects(&LtdoverlaySearchSymbol, Ltdoverlay->searchSymbols.head, n);
}

// Force
static void PutLtdoverlayForce(const char *dummy)
{
    if (!PutObject(LtdoverlayForce, NULL))
    {
        lcf_error("No <FOREACH.LTDOVERLAY.FORCES> Ignored.");
    }
}

// Foreach Forces
static int ForeachLtdoverlayForces(int n)
{
    return ForeachObjectsForce(&LtdoverlayForce, Ltdoverlay->forces.head, n);
}

// Foreach Ltdoverlays
static int ForeachLtdoverlays(int n)
{
    tOverlay *start = LtdoverlayList.head;
    tTokenBuffer *token = &tokenBuffer[n];
    tMemType memtype = GetMemTypeParam(token->string);

    while (start)
    {
        if (memtype == MEMTYPE_NONE || memtype == RecgSectionMemTypeByName(start->name))
        {
            Ltdoverlay = start;
            PutTokenBuffer(n + 1, tokenBuffer[n].loop_end);
        }
        start = start->next;
    }
    Ltdoverlay = NULL;
    return tokenBuffer[n].loop_end;
}

/*============================================================================
 *  Put Token List
 */
static int PutToken(int n)
{
    switch (tokenBuffer[n].id)
    {
    case tSTRING:
        PutString(tokenBuffer[n].string);
        break;

    case tSTATIC_NAME:
        PutString(Static.name);
        break;

    case tSTATIC_ADDRESS:
        PutAddress(Static.address);
        break;

    case tSTATIC_CRT0_OBJECT:
        PutStaticCrt0Object(tokenBuffer[n].string);
        break;

    case tSTATIC_OBJECT:
        PutStaticObject(tokenBuffer[n].string);
        break;

    case tSTATIC_LIBRARY:
        PutStaticLibrary(tokenBuffer[n].string);
        break;

    case tSTATIC_SEARCHSYMBOL:
        PutStaticSearchSymbol(tokenBuffer[n].string);
        break;

    case tSTATIC_FORCE:
        PutStaticForce(tokenBuffer[n].string);
        break;

    case tSTATIC_STACKSIZE:
        PutNumber(Static.stacksize);
        break;

    case tSTATIC_IRQSTACKSIZE:
        PutNumber(Static.stacksize_irq);
        break;

    case tFOREACH_STATIC_OBJECTS:
        n = ForeachStaticObjects(n);
        break;

    case tFOREACH_STATIC_LIBRARIES:
        n = ForeachStaticLibraries(n);
        break;

    case tFOREACH_STATIC_SEARCHSYMBOLS:
        n = ForeachStaticSearchSymbols(n);
        break;

    case tFOREACH_STATIC_FORCES:
        n = ForeachStaticForces(n);
        break;

    case tAUTOLOAD_ID:
        PutAutoloadId();
        break;

    case tAUTOLOAD_NAME:
        PutAutoloadName();
        break;

    case tAUTOLOAD_ADDRESS:
        PutAutoloadAddress();
        break;

    case tAUTOLOAD_OBJECT:
        PutAutoloadObject(tokenBuffer[n].string);
        break;

    case tAUTOLOAD_LIBRARY:
        PutAutoloadLibrary(tokenBuffer[n].string);
        break;

    case tAUTOLOAD_SEARCHSYMBOL:
        PutAutoloadSearchSymbol(tokenBuffer[n].string);
        break;

    case tAUTOLOAD_FORCE:
        PutAutoloadForce(tokenBuffer[n].string);
        break;

    case tNUMBER_AUTOLOADS:
        PutNumber(AutoloadList.num);
        break;

    case tFOREACH_AUTOLOADS:
        n = ForeachAutoloads(n);
        break;

    case tFOREACH_AUTOLOAD_OBJECTS:
        n = ForeachAutoloadObjects(n);
        break;

    case tFOREACH_AUTOLOAD_LIBRARIES:
        n = ForeachAutoloadLibraries(n);
        break;

    case tFOREACH_AUTOLOAD_SEARCHSYMBOLS:
        n = ForeachAutoloadSearchSymbols(n);
        break;

    case tFOREACH_AUTOLOAD_FORCES:
        n = ForeachAutoloadForces(n);
        break;

    case tOVERLAY_ID:
        PutOverlayId();
        break;

    case tOVERLAY_NAME:
        PutOverlayName();
        break;

    case tOVERLAY_GROUP:
        PutOverlayGroup();
        break;

    case tOVERLAY_ADDRESS:
        PutOverlayAddress();
        break;

    case tOVERLAY_COMPRESS:
        PutOverlayCompress();
        break;

    case tOVERLAY_OBJECT:
        PutOverlayObject(tokenBuffer[n].string);
        break;

    case tOVERLAY_LIBRARY:
        PutOverlayLibrary(tokenBuffer[n].string);
        break;

    case tOVERLAY_SEARCHSYMBOL:
        PutOverlaySearchSymbol(tokenBuffer[n].string);
        break;

    case tOVERLAY_FORCE:
        PutOverlayForce(tokenBuffer[n].string);
        break;

    case tNUMBER_OVERLAYS:
        PutNumber(OverlayList.num);
        break;

    case tFOREACH_OVERLAYS:
        n = ForeachOverlays(n);
        break;

    case tFOREACH_OVERLAY_OBJECTS:
        n = ForeachOverlayObjects(n);
        break;

    case tFOREACH_OVERLAY_LIBRARIES:
        n = ForeachOverlayLibraries(n);
        break;

    case tFOREACH_OVERLAY_SEARCHSYMBOLS:
        n = ForeachOverlaySearchSymbols(n);
        break;

    case tFOREACH_OVERLAY_FORCES:
        n = ForeachOverlayForces(n);
        break;

    case tLTDAUTOLOAD_ID:
        PutLtdautoloadId();
        break;

    case tLTDAUTOLOAD_NAME:
        PutLtdautoloadName();
        break;

    case tLTDAUTOLOAD_ADDRESS:
        PutLtdautoloadAddress();
        break;

    case tLTDAUTOLOAD_OBJECT:
        PutLtdautoloadObject(tokenBuffer[n].string);
        break;

    case tLTDAUTOLOAD_LIBRARY:
        PutLtdautoloadLibrary(tokenBuffer[n].string);
        break;

    case tLTDAUTOLOAD_SEARCHSYMBOL:
        PutLtdautoloadSearchSymbol(tokenBuffer[n].string);
        break;

    case tLTDAUTOLOAD_FORCE:
        PutLtdautoloadForce(tokenBuffer[n].string);
        break;

    case tNUMBER_LTDAUTOLOADS:
        PutNumber(LtdautoloadList.num);
        break;

    case tFOREACH_LTDAUTOLOADS:
        n = ForeachLtdautoloads(n);
        break;

    case tFOREACH_LTDAUTOLOAD_OBJECTS:
        n = ForeachLtdautoloadObjects(n);
        break;

    case tFOREACH_LTDAUTOLOAD_LIBRARIES:
        n = ForeachLtdautoloadLibraries(n);
        break;

    case tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS:
        n = ForeachLtdautoloadSearchSymbols(n);
        break;

    case tFOREACH_LTDAUTOLOAD_FORCES:
        n = ForeachLtdautoloadForces(n);
        break;

    case tLTDOVERLAY_ID:
        PutLtdoverlayId();
        break;

    case tLTDOVERLAY_NAME:
        PutLtdoverlayName();
        break;

    case tLTDOVERLAY_GROUP:
        PutLtdoverlayGroup();
        break;

    case tLTDOVERLAY_ADDRESS:
        PutLtdoverlayAddress();
        break;

    case tLTDOVERLAY_COMPRESS:
        PutLtdoverlayCompress();
        break;

    case tLTDOVERLAY_OBJECT:
        PutLtdoverlayObject(tokenBuffer[n].string);
        break;

    case tLTDOVERLAY_LIBRARY:
        PutLtdoverlayLibrary(tokenBuffer[n].string);
        break;

    case tLTDOVERLAY_SEARCHSYMBOL:
        PutLtdoverlaySearchSymbol(tokenBuffer[n].string);
        break;

    case tLTDOVERLAY_FORCE:
        PutLtdoverlayForce(tokenBuffer[n].string);
        break;

    case tNUMBER_LTDOVERLAYS:
        PutNumber(LtdoverlayList.num);
        break;

    case tFOREACH_LTDOVERLAYS:
        n = ForeachLtdoverlays(n);
        break;

    case tFOREACH_LTDOVERLAY_OBJECTS:
        n = ForeachLtdoverlayObjects(n);
        break;

    case tFOREACH_LTDOVERLAY_LIBRARIES:
        n = ForeachLtdoverlayLibraries(n);
        break;

    case tFOREACH_LTDOVERLAY_SEARCHSYMBOLS:
        n = ForeachLtdoverlaySearchSymbols(n);
        break;

    case tFOREACH_LTDOVERLAY_FORCES:
        n = ForeachLtdoverlayForces(n);
        break;

    case tPROPERTY_OVERLAYDEFS:
        PutString(Property.overlaydefs);
        break;

    case tPROPERTY_OVERLAYTABLE:
        PutString(Property.overlaytable);
        break;

    case tPROPERTY_LTDOVERLAYDEFS:
        PutString(Property.ltdoverlaydefs);
        break;

    case tPROPERTY_LTDOVERLAYTABLE:
        PutString(Property.ltdoverlaytable);
        break;

    case tPROPERTY_SUFFIX:
        PutString(Property.suffix);
        break;

    case tPROPERTY_FLXSUFFIX:
        PutString(Property.flxsuffix);
        break;

    case tPROPERTY_LTDSUFFIX:
        PutString(Property.ltdsuffix);
        break;

    default:
        lcf_error("Unknown token\n");
    }
    return n;
}

static void PutTokenBuffer(int start, int end)
{
    int     i;

    for (i = start; i <= end; i++)
    {
        i = PutToken(i);
    }
}

static void PutTokenBufferAll(void)
{
    PutTokenBuffer(0, tokenBufferEnd - 1);
}


/*============================================================================
 *  Create LCF file
 */
int CreateLcfFile(const char *filename)
{
    if (filename)
    {
        if (NULL == (fp_out = fopen(filename, "wb")))
        {
            fprintf(stderr, "makelcf: Cannot write %s\n", filename);
            return 2;
        }
        PutTokenBufferAll();
        fclose(fp_out);
    }
    else
    {
        setmode(1, O_BINARY);
        fp_out = stdout;
        PutTokenBufferAll();
    }

    return 0;
}
