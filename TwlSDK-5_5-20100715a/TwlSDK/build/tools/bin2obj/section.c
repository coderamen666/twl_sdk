/*---------------------------------------------------------------------------*
  Project:  TwlSDK - bin2obj
  File:     section.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "bin2obj.h"

static s32 read_datasec(Object * obj, const char *filename);
static u32 section_add_string(Section * s, const char *string);

/*---------------------------------------------------------------------------*
  Name:         add_datasec
  
  Description:  Adds data section

  Arguments:    obj                   Pointer to object
                section_rodata        Name of data section dedicated to reading
                section_rwdata        Name of readable/writeable data section
                symbol_format_begin   Head of symbol name (before %f conversion)
                symbol_format_end   End of symbol name (before %f conversion)
                filename              Input binary file name
                writable              TRUE .data  FALSE .rodata
                align                 Alignment
  Returns:      TRUE for success; FALSE for failure.
 *---------------------------------------------------------------------------*/
BOOL add_datasec(Object * obj,
                 const char *section_rodata, const char *section_rwdata,
                 const char *symbol_format_begin, const char *symbol_format_end,
                 const char *filename, BOOL writable, u32 align)
{
    s32     n;
    DataSection *d;
    char   *symbol_begin;              // Head of symbol name (after %f conversion)
    char   *symbol_end;                // End of symbol name (after %f conversion)

    //
    //  Data section read
    //
    if (0 > (n = read_datasec(obj, filename)))
    {
        return FALSE;
    }

    //
    //  Registration of data section
    //
    d = &obj->data[n];
    if (writable)                      // .rodata or .data processing
    {
        d->index =
            add_section(obj, section_rwdata, SHT_PROGBITS, SHF_WRITE | SHF_ALLOC, d->section.size,
                        align);
    }
    else
    {
        d->index =
            add_section(obj, section_rodata, SHT_PROGBITS, SHF_ALLOC, d->section.size, align);
    }

    //
    //  Symbol registration
    //
    symbol_begin = create_symbol_string(filename, symbol_format_begin);
    symbol_end = create_symbol_string(filename, symbol_format_end);
    (void)add_symbol(obj, symbol_begin, 0, d->section.size, d->index);
    (void)add_symbol(obj, symbol_end, d->section.size, 0, d->index);
    free(symbol_begin);
    free(symbol_end);

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         read_datasec
  
  Description:  Reads the data section.

  Arguments:    obj                   Pointer to object
                filename              Input binary file name
  
  Returns:      TRUE for success; FALSE for failure.
 *---------------------------------------------------------------------------*/
static s32 read_datasec(Object * obj, const char *filename)
{
    struct stat st;
    FILE   *fp;
    u32     n = obj->num_data;
    Section *s = &obj->data[n].section;

    //
    //  Allocate a data section area from the heap with a size enough for the input file and load the input file "filename" there.
    //   
    //
    if (stat(filename, &st) || !S_ISREG(st.st_mode) || st.st_size < 0
        || NULL == (fp = fopen(filename, "rb")))
    {
        fprintf(stderr, "Error: Cannot open file %s.\n", filename);
        return -1;
    }

    s->size = st.st_size;

    if (NULL == (s->ptr = malloc(s->size)))
    {
        fprintf(stderr, "Error: No memory.\n");
        fclose(fp);
        return -1;
    }

    if (s->size != fread(s->ptr, sizeof(u8), s->size, fp))
    {
        fprintf(stderr, "Error: Cannot read file %s.\n", filename);
        free(s->ptr);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    obj->num_data = n + 1;
    return n;
}

/*---------------------------------------------------------------------------*
  Name:         add_section_name

  Description:  Adds the section name.

  Arguments:    obj    Object
                name     Section name
  
  Returns:      Start position in the section name table of the added section name
 *---------------------------------------------------------------------------*/
u32 add_section_name(Object * obj, const char *name)
{
    // Register internally in the section
    u32     pos = section_add_string(&obj->section_name, name);

    // Update section information (size)
    if (obj->header.e_shstrndx > 0)
    {
        obj->section[obj->header.e_shstrndx].sh_size = obj->section_name.size;
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         add_section
  
  Description:  Add the section.

  Arguments:    obj                   Pointer to object
                name    Symbol name (If NULL, make it NULL SECTION)
                type    SHT_*
                flags   SHF_*
                size    Section size
                align   Section alignment
  
  Returns:      Registered index
 *---------------------------------------------------------------------------*/
u32 add_section(Object * obj, const char *name, u32 type, u32 flags, u32 size, u32 align)
{
    ELF32_SectionHeader *s;
    u32     n;

    n = obj->header.e_shnum;
    s = &obj->section[n];

    if (name)
    {
        s->sh_name = add_section_name(obj, name);
        s->sh_type = type;
        s->sh_flags = flags;
        s->sh_addr = 0;
        s->sh_offset = 0;
        s->sh_size = size;
        s->sh_link = 0;
        s->sh_info = 0;
        s->sh_addralign = align;
        s->sh_entsize = 0;
    }
    else
    {
        (void)add_section_name(obj, "");
        memset(s, 0, sizeof(ELF32_SectionHeader));
    }
    obj->header.e_shnum = n + 1;

    return n;
}

/*---------------------------------------------------------------------------*
  Name:         add_symbol_name

  Description:  Adds the symbol name.

  Arguments:    obj    Object
                name     Symbol name
  
  Returns:      Start position in the section name table of the added symbol name
 *---------------------------------------------------------------------------*/
u32 add_symbol_name(Object * obj, const char *name)
{
    u32     pos;

    // Register internally in the section
    // When the name is NULL, this is handled as "".
    pos = section_add_string(&obj->symbol_name, name ? name : "");

    // Update section information (size)
    if (obj->symbol_name_index > 0)
    {
        obj->section[obj->symbol_name_index].sh_size = obj->symbol_name.size;
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         add_symbol

  Description:  Adds symbol.

  Arguments:    obj		Pointer to the object
                symbol		Symbol name (If NULL, make it NULL SECTION)
                value		Symbol value
                size		Symbol size
                section		Related section
  
  Returns:      Registered index
 *---------------------------------------------------------------------------*/
u32 add_symbol(Object * obj, const char *symbol, u32 value, u32 size, u32 section)
{
    ELF32_SectionHeader *symtab = &obj->section[obj->symbol_index];
    u32     n = symtab->sh_info + 1;
    ELF32_Symbol *l = &obj->symbol[n];

    if (obj->symbol_index > 0)
    {
        if (symbol)
        {
            // Symbols in the C language have a '_' appended to the front of the normal function name
            l->st_name = add_symbol_name(obj, symbol);
            l->st_value = value;
            l->st_size = size;
            l->st_info = ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT);
            l->st_other = 0;
            l->st_shndx = section;
        }
        else
        {
            (void)add_symbol_name(obj, NULL);
            memset(l, 0, sizeof(ELF32_Symbol));
        }

        // Update symbol table
        symtab->sh_info = n;
        symtab->sh_size = (n + 1) * symtab->sh_entsize;
    }
    else
    {
        fprintf(stderr, "Warning: no symbol section. [%s] is skipped.\n", symbol);
    }
    return n;
}

/*---------------------------------------------------------------------------*
  Name:         section_add_string

  Description:  Adds string to section.

  Arguments:    section: Section
                string: Characters
  
  Returns:      Start position of the added string's table (i.e. the size before the function was called)
 *---------------------------------------------------------------------------*/
static u32 section_add_string(Section * s, const char *string)
{
    int     new_size = s->size + strlen(string) + 1;
    u8     *new_ptr;
    int     size;

    if (NULL == (new_ptr = malloc(new_size)))
    {
        fprintf(stderr, "Error: No memory.\n");
        exit(1);
    }
    memcpy(new_ptr, s->ptr, s->size);
    strcpy(new_ptr + s->size, string);
    free(s->ptr);

    size = s->size;
    s->ptr = new_ptr;
    s->size = new_size;

    return size;
}
