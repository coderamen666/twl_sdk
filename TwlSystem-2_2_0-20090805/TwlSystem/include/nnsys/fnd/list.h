/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     list.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_LIST_H_
#define NNS_FND_LIST_H_

#include <stddef.h>
#include <nitro/types.h>
#include <nnsys/misc.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         NNSFndLink

  Description:  Node structure for doubly linked list.
                This structure is stored as a structure member of the list structure to be added to.
 *---------------------------------------------------------------------------*/
typedef struct
{
    void*       prevObject;     // Pointer to the previous linked object.
    void*       nextObject;     // Pointer to the next linked object.

} NNSFndLink;


/*---------------------------------------------------------------------------*
  Name:         NNSFndList

  Description:  Doubly-linked list structure.
 *---------------------------------------------------------------------------*/
typedef struct 
{
    void*       headObject;     // Pointer for the object linked to the top of the list.
    void*       tailObject;     // Pointer for the object linked to the end of the list.
    u16         numObjects;     // Number of objects linked in the list.
    u16         offset;         // Offset for NNSFndLink type structure member.

} NNSFndList;


/*---------------------------------------------------------------------------*
  Name:         NNS_FND_INIT_LIST

  Description:  Macro to initialize list structures.
                Actual initialization is performed by the NNSFndInitList function.

                This macro uses the offsetof macro to find the offset from the specified structure name and NNSFndLink-type member variable name. That offset is then passed to the NNSFndInitList function.
                
                

  Arguments:    list:       Pointer to the link structure.
                structName: Structure name of object you wish to link in the list.
                linkName:   The NNSFndLink-type member variable name used by this object's link.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/

#define NNS_FND_INIT_LIST(list, structName, linkName) \
            NNS_FndInitList(list, offsetof(structName, linkName))


/*---------------------------------------------------------------------------*
    Function Prototypes

 *---------------------------------------------------------------------------*/

void    NNS_FndInitList(
                NNSFndList*             list,
                u16                     offset);

void    NNS_FndAppendListObject(
                NNSFndList*             list,
                void*                   object);

void    NNS_FndPrependListObject(
                NNSFndList*             list,
                void*                   object);

void    NNS_FndInsertListObject(
                NNSFndList*             list,
                void*                   target,
                void*                   object);

void    NNS_FndRemoveListObject(
                NNSFndList*             list,
                void*                   object);

void*   NNS_FndGetNextListObject(
                const NNSFndList*       list,
                const void*             object);

void*   NNS_FndGetPrevListObject(
                const NNSFndList*       list,
                const void*             object);

void*   NNS_FndGetNthListObject(
                const NNSFndList*       list,
                u16                     index);


/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetListSize

  Description:  Gets the number of objects registered to the list specified by the argument.
                

  Arguments:    list: Pointer to the list structure.

  Returns:      Returns the number of objects registered to the specified list.
 *---------------------------------------------------------------------------*/
inline u16
NNS_FndGetListSize( const NNSFndList* list )
{
    NNS_NULL_ASSERT( list );
    
    return list->numObjects;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_LIST_H_ */
#endif
