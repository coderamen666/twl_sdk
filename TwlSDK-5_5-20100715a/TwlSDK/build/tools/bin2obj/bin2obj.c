/*---------------------------------------------------------------------------*
  Project:  TwlSDK - bin2obj
  File:     bin2obj.c

  Copyright 2005-2008 Nintendo. All rights reserved.

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

/*---------------------------------------------------------------------------*
  Name:         Main

  Description:  bin2obj main.
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    Bin2ObjArgs args;
    BOOL    result;

    cook_args(&args, argc, argv);
    result = bin2obj(&args);
    free_args(&args);

    return result ? 0 : 1;
}

/*---------------------------------------------------------------------------*
  Name:         bin2obj

  Description:  Outputs object information.

  Arguments:    t: bin2obj argument after processing
  
  Returns:      TRUE for success; FALSE for failure.
 *---------------------------------------------------------------------------*/
BOOL bin2obj(const Bin2ObjArgs * t)
{
    Object  object;

    //
    // Create object data
    //
    object_init(&object, t->machine, t->endian);

    if (!add_datasec(&object,
                     t->section_rodata, t->section_rwdata,
                     t->symbol_begin, t->symbol_end, t->binary_filename, t->writable, t->align))
    {
        return FALSE;
    }

    map_section(&object);

    //
    // Output object
    //
    if (!output_object(&object, t->object_filename))
    {
        return FALSE;
    }

    return TRUE;
}
