/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - cplusplus-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-19#$
  $Rev: 1027 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "new.h"

//================================================================
// Foo
//
class Foo
{
  private:
    u32  a;
    u32  b;
    int* array;

  public:
    Foo();
    ~Foo();

    void printA(void)
    {
        OS_Printf("a = %d\n", a);
    }
    void printB(void);

    void printArray(void)
    {
        for( int n=0; n<16; n++ ){ OS_Printf("%d ", array[n]); }
        OS_Printf("\n"); 
    }
};

/*---------------------------------------------------------------------------*
  Name:         Foo::Foo

  Description:  Constructor for class Foo.  
                Simply initialize private members.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
Foo::Foo()
{
    a = 1234;
    b = 5678;

    array = new int[16];
    for( int n=0; n<16; n++ ){ array[n] = n; }
}

/*---------------------------------------------------------------------------*
  Name:         Foo::~Foo

  Description:  Destructor for class Foo.  
                Performs an OSReport so we know it got called properly.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
Foo::~Foo()
{
    OS_Printf("Foo destructor\n");
}

/*---------------------------------------------------------------------------*
  Name:         Foo::printB

  Description:  function to print out b member

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void Foo::printB()
{
    OS_Printf("b = %d\n", b);
}



//================================================================================
//---- static class instanse
static Foo  staticFoo;

//--------------------------------------------------------------------------------
// NitroMain
//
void NitroMain()
{
    // needless to call OS_Init() because called in NitroStartUp()
    //OS_Init();

    Foo* foo;
    foo = new Foo;

    foo->printA();
    foo->printB();
    foo->printArray();

    staticFoo.printA();
    staticFoo.printB();
    staticFoo.printArray();

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}
