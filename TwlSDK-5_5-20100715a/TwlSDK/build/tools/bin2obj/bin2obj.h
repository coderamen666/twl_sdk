/*---------------------------------------------------------------------------*
  Project:  TwlSDK - bin2obj
  File:     bin2obj.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef BIN2OBJ_H__
#define BIN2OBJ_H__

#include <stdio.h>
#include <malloc.h>                    // calloc()
#include <stdlib.h>                    // free(), exit()
#include <sys/stat.h>                  // stat()
#include <string.h>                    // strlen/strdup/strcpy/memcpy
#include <getopt.h>                    // getopt_long_only
#include <ctype.h>                     // isalnum(), isdight()
#include <assert.h>                    // assert()
#include "types.h"
#include "object.h"

typedef struct
{
    char   *app_name;
    char   *binary_filename;
    char   *object_filename;
    char   *section_rodata;
    char   *section_rwdata;
    char   *symbol_begin;
    char   *symbol_end;
    u32     align;
    BOOL    writable;
    u8      endian;			// ELFDATA2LSB/ELFDATA2MSB
    u16     machine;			// EM_PPC=20 EM_ARM=40
}
Bin2ObjArgs;

#define DEFAULT_ALIGN           4
#define DEFALUT_WRITABLE        TRUE
#define DEFAULT_SYMBOL_BEGIN    "%b_begin"
#define DEFAULT_SYMBOL_END      "%b_end"

#define DEFAULT_SECTION_RWDATA  ".data"
#define DEFAULT_SECTION_RODATA  ".rodata"

// for Compatiblity with CodeWarrior BinToElf
#define COMPATIBLE_SYMBOL_BEGIN "_binary_%t"
#define COMPATIBLE_SYMBOL_END   "_binary_%t_end"

BOOL    bin2obj(const Bin2ObjArgs * args);

char   *create_symbol_string(const char *filename, const char *symbol_format);
int     replace_word(char **str, int pos, int len, const char *substr);
char   *StrDup(const char *str);
char   *StrCatDup(const char *str1, const char *str2);
char   *StrNDup(const char *str, int len);
void   *Calloc(int size);
void    UnpackFileName(const char *path, char **dir, char **base, char **ext);

void    cook_args(Bin2ObjArgs * t, int argc, char *argv[]);
void    free_args(Bin2ObjArgs * t);

void    object_init(Object * obj, u16, u8);
void    map_section(Object * obj);
void    conv_to_big_endian(const Object * obj, Object * cooked_obj);
u32     roundup(u32, u32);
BOOL    output_object(const Object * obj, const char *filename);
BOOL    add_datasec(Object * obj,
                    const char *section_rodata, const char *section_rwdata,
                    const char *symbol_format_begin, const char *symbol_format_end,
                    const char *filename, BOOL writable, u32 align);
u32     add_section_name(Object * obj, const char *name);
u32     add_section(Object * obj, const char *name, u32 type, u32 flags, u32 size, u32 align);
u32     add_symbol_name(Object * obj, const char *name);
u32     add_symbol(Object * obj, const char *symbol, u32 value, u32 size, u32 section);

#endif //BIN2OBJ_H__
