/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - child
  File:     func_3.cpp

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include	"func.h"


/******************************************************************************/
/* Local implement */

namespace
{

/* Test class for constructors/destructors */
    class   Foo
    {
      private:
        const int line_;
      public:
                Foo(int line):line_(line)
        {
            OS_TPrintf("func_3_cpp::Foo::constructor called. (object is at func_3_cpp:%d)\n",
                       line_);
        }
               ~Foo()
        {
            OS_TPrintf("func_3_cpp::Foo::destructor called. (object is at func_3_cpp:%d)\n", line_);
        }
    };

}                                      // Unnamed namespace


/******************************************************************************/
/* Variables */

static int i = 0;

/*
 * This is constructed when FS_StartOverlay() is run.
 * This is destroyed when FS_EndOverlay() is run.
 */
static Foo foo1(__LINE__);


/******************************************************************************/
/* Functions */

#if	defined(__cplusplus)
extern  "C"
{
#endif


    void    func_3(char *dst)
    {
        /*
         * This is constructed during the first function call.
         * If it has been constructed, it will be destroyed when FS_EndOverlay() is run.
         */
        static Foo foo2(__LINE__);

              ++i;
                (void)OS_SPrintf(dst, "func_3. called %d time%s.\n", i, (i == 1) ? "" : "s", &i);
    }


#if	defined(__cplusplus)
}                                      /* extern "C" */
#endif


/******************************************************************************/
