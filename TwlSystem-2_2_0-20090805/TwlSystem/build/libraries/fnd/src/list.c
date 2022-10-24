/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     list.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/misc.h>
#include <nnsys/fnd/list.h>

#define OBJ_TO_LINK(list,obj)   ((NNSFndLink*)(((u32)(obj))+(list)->offset))


/*---------------------------------------------------------------------------*
  Name:         NNS_FndInitList

  Description:  Initializes list structure.

  Arguments:    list:   Pointer to the list structure.
                offset: Specifies the offset within the structure for the NNSFndLink-type member variable that exists within the structure to be added to the list.
                        
                        This is convenient when using the offsetof macro defined in stddef.h.
                        

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInitList(NNSFndList* list, u16 offset)
{
    NNS_NULL_ASSERT(list);

    list->headObject = NULL;
    list->tailObject = NULL;
    list->numObjects = 0;
    list->offset     = offset;
}

/*---------------------------------------------------------------------------*
  Name:         SetFirstObject                                      [static]

  Description:  Adds the first object to the list.

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to the object you want to add to the list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SetFirstObject(NNSFndList* list, void* object)
{
    NNSFndLink* link;

    NNS_NULL_ASSERT(list  );
    NNS_NULL_ASSERT(object);

    link = OBJ_TO_LINK(list, object);

    link->nextObject = NULL;
    link->prevObject = NULL;
    list->headObject = object;
    list->tailObject = object;
    list->numObjects++;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAppendListObject

  Description:  Adds an object to the end of the list.

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to the object you want to add to the list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndAppendListObject(NNSFndList* list, void* object)
{
    NNS_NULL_ASSERT(list  );
    NNS_NULL_ASSERT(object);

    if (list->headObject == NULL)
    {
        // When the list is empty
        SetFirstObject(list, object);
    }
    else
    {
        NNSFndLink* link = OBJ_TO_LINK(list, object);

        link->prevObject = list->tailObject;
        link->nextObject = NULL;

        OBJ_TO_LINK(list, list->tailObject)->nextObject = object;
        list->tailObject = object;
        list->numObjects++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndPrependListObject

  Description:  Inserts object at the head of the list.

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to the object you want to add to the list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndPrependListObject(NNSFndList* list, void* object)
{
    NNS_NULL_ASSERT(list  );
    NNS_NULL_ASSERT(object);

    if (list->headObject == NULL)
    {
        // When the list is empty
        SetFirstObject(list, object);
    }
    else
    {
        NNSFndLink* link = OBJ_TO_LINK(list, object);

        link->prevObject = NULL;
        link->nextObject = list->headObject;

        OBJ_TO_LINK(list, list->headObject)->prevObject = object;
        list->headObject = object;
        list->numObjects++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndInsertListObject

  Description:  Inserts object in the specified location.
                The object is inserted in front of the object specified in "target".
                When an insert target is not specified (i.e., when target is NULL), the object is added to the end of the list.
                

  Arguments:    list:   Pointer to the list structure.
                target: Pointer to the object at the position where you want to insert an object.
                object: Pointer to the object you want to add to the list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInsertListObject(NNSFndList* list, void* target, void* object)
{
    NNS_NULL_ASSERT(list  );
    NNS_NULL_ASSERT(object);

    if (target == NULL)
    {
        // When the target is not specified, same as NNS_FndAppendListObject()
        NNS_FndAppendListObject(list, object);
    }
    else if (target == list->headObject)
    {
        // When target is the head of the list, same as NNS_FndPrependListObject()
        NNS_FndPrependListObject(list, object);
    }
    else
    {
        NNSFndLink* link    = OBJ_TO_LINK(list, object);
        void*       prevObj = OBJ_TO_LINK(list, target)->prevObject;
        NNSFndLink* prevLnk = OBJ_TO_LINK(list, prevObj);

        link->prevObject    = prevObj;
        link->nextObject    = target;
        prevLnk->nextObject = object;
        OBJ_TO_LINK(list, target)->prevObject = object;
        list->numObjects++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndRemoveListObject

  Description:  Deletes object from the list.

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to the object you want to delete from the list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndRemoveListObject(NNSFndList* list, void* object)
{
    NNSFndLink* link;

    NNS_NULL_ASSERT(list  );
    NNS_NULL_ASSERT(object);

    link = OBJ_TO_LINK(list, object);

    if (link->prevObject == NULL)
    {
        list->headObject = link->nextObject;
    }
    else
    {
        OBJ_TO_LINK(list, link->prevObject)->nextObject = link->nextObject;
    }
    if (link->nextObject == NULL)
    {
        list->tailObject = link->prevObject;
    }
    else
    {
        OBJ_TO_LINK(list, link->nextObject)->prevObject = link->prevObject;
    }
    link->prevObject = NULL;
    link->nextObject = NULL;
    list->numObjects--;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetNextListObject

  Description:  Returns the object linked next after the object specified by 'object.'
                When the object is specified as NULL, the object attached to the head of the list is returned.
                

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to object in list.

  Returns:      Returns a pointer to the next object after the specified object. 
                If there is no next object, returns NULL.
 *---------------------------------------------------------------------------*/
void*
NNS_FndGetNextListObject(const NNSFndList* list, const void* object)
{
    NNS_NULL_ASSERT(list);

    if (object == NULL)
    {
        return list->headObject;
    }
    return OBJ_TO_LINK(list, object)->nextObject;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetPrevListObject

  Description:  Returns the object linked in front of the object specified by 'object.'
                When the object is specified as NULL, the object attached to the end of the list is returned.
                

  Arguments:    list:   Pointer to the list structure.
                object: Pointer to object in list.

  Returns:      Returns a pointer to the previous object before the specified object.
                If there is no object in front, returns NULL.
 *---------------------------------------------------------------------------*/
void*
NNS_FndGetPrevListObject(const NNSFndList* list, const void* object)
{
    NNS_NULL_ASSERT(list);

    if (object == NULL)
    {
        return list->tailObject;
    }
    return OBJ_TO_LINK(list, object)->prevObject;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetNthListObject

  Description:  Returns a pointer to the object linked nth on the list.
                Because the list is traversed in order from the beginning, objects deeper in the list take more time.
                

  Arguments:    index:  Object index.

  Returns:      Returns pointer to object.
                If there is no object for the specified index, NULL is returned.
 *---------------------------------------------------------------------------*/
void*
NNS_FndGetNthListObject(const NNSFndList* list, u16 index)
{
    int         count  = 0;
    NNSFndLink* object = NULL;

    NNS_NULL_ASSERT(list);

    while ((object = NNS_FndGetNextListObject(list, object)) != NULL)
    {
        if (index == count)
        {
            return object;
        }
        count++;
    }
    return NULL;
}

