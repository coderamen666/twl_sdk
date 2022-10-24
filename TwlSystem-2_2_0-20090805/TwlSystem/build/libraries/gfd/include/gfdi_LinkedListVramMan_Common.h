/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd
  File:     gfdi_LinkedListVramMan_Common.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFDI_LINKEDLISTVRAMMAN_COMMON_H_
#define NNS_GFDI_LINKEDLISTVRAMMAN_COMMON_H_


#include <nnsys/gfd.h>

//------------------------------------------------------------------------------
typedef struct NNSiGfdLnkVramBlock NNSiGfdLnkVramBlock;

/*---------------------------------------------------------------------------*
  Name:         NNSiGfdLnkVramBlock

  Description:  Memory region management block
  
 *---------------------------------------------------------------------------*/
struct NNSiGfdLnkVramBlock
{
    u32                         addr;       // Region start address
    u32                         szByte;     // Region size (zero may not be used)
    
    NNSiGfdLnkVramBlock*        pBlkPrev;   // Previous region (No address-positional relation)
    NNSiGfdLnkVramBlock*        pBlkNext;   // Next region (No address-positional relation)
    
};

/*---------------------------------------------------------------------------*
  Name:         NNSiGfdLnkMemRegion

  Description:  Memory interval
                For items satisfying end > start
  
 *---------------------------------------------------------------------------*/
typedef struct NNSiGfdLnkMemRegion
{
    u32       start;
    u32       end;
    
}NNSiGfdLnkMemRegion;

/*---------------------------------------------------------------------------*
  Name:         NNSiGfdLnkVramMan

  Description:  Manager
                Unlike ordinary heaps, there is no management information list for used regions.
                This is because there is no association between the management information region (in main memory) and the management region address (in VRAM) and because lookups for used region management information based on TextureKey (which has address and size information) are difficult.
                
                
                When freeing, returns to the free area of the area used from the address and size.
                
                
 *---------------------------------------------------------------------------*/
typedef struct NNSiGfdLnkVramMan
{
    NNSiGfdLnkVramBlock*         pFreeList;         // Unused region block list
      
}NNSiGfdLnkVramMan;




//------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------
void NNSi_GfdDumpLnkVramManFreeListInfo
( 
    const NNSiGfdLnkVramBlock*      pFreeListHead,
    u32                             szReserved 
);

void NNSi_GfdDumpLnkVramManFreeListInfoEx( 
    const NNSiGfdLnkVramBlock*      pFreeListHead, 
    NNSGfdLnkDumpCallBack           pFunc, 
    void*                           pUserData );
    

void 
NNSi_GfdInitLnkVramMan( NNSiGfdLnkVramMan* pMgr );


NNSiGfdLnkVramBlock* 
NNSi_GfdInitLnkVramBlockPool
( 
    NNSiGfdLnkVramBlock*    pArrayHead, 
    u32                     lengthOfArray 
); 

BOOL
NNSi_GfdAddNewFreeBlock
(
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32                     baseAddr,
    u32                     szByte
);


BOOL
NNSi_GfdAllocLnkVram
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32*                    pRetAddr,
    u32                     szByte
); 

BOOL
NNSi_GfdAllocLnkVramAligned
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32*                    pRetAddr,
    u32                     szByte,
    u32                     alignment
);

void NNSi_GfdMergeAllFreeBlocks( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList
);

BOOL NNSi_GfdFreeLnkVram
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32                     addr,
    u32                     szByte
);





#endif // NNS_GFDI_LINKEDLISTVRAMMAN_COMMON_H_

