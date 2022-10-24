/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd - src - VramManager
  File:     gfd_FramePlttVramMan.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/gfd/gfd_common.h>
#include <nnsys/gfd/VramManager/gfd_VramMan.h>
#include <nnsys/gfd/VramManager/gfd_FramePlttVramMan.h>

#define NNS_GFD_TEXPLTT_BASE_ADDR	0x00000
#define NNS_GFD_TEXPLTT_MAX_ADDR    0x18000


//------------------------------------------------------------------------------
typedef struct NNSGfdFramPlttVramManager
{
    u32     loAddr;
    u32     hiAddr;
    u32     szTotal;
    
} NNSGfdFrmPlttVramManager;

//------------------------------------------------------------------------------
static NNSGfdFrmPlttVramManager       s_managerState_ = { 0, 0, 0 };
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Gets the alignment boundary size for each format.
// alignment offset in the high order direction
static NNS_GFD_INLINE u32 GetUpperSideAlignmentSize_( u32 addr, BOOL b4Pltt )
{
    if( b4Pltt ) 
    {
        return 0x07 & (0x08 - (addr & 0x07));
    }else{
        return 0x0F & (0x10 - (addr & 0x0F));
    }
}

// alignment offset in the low order direction
static NNS_GFD_INLINE u32 GetLowerSideAlignmentSize_( u32 addr, BOOL b4Pltt )
{
    if( b4Pltt ) 
    {
        return addr & 0x07;
    }else{
        return addr & 0x0F;
    }
}

//------------------------------------------------------------------------------
// Checks if the manager is in a valid state.
static BOOL IsVramManagerValid_( )
{
    if( ( s_managerState_.loAddr <= s_managerState_.hiAddr ) && 
          s_managerState_.szTotal != 0 )
    {
        return TRUE;
    }else{
        return FALSE;
    }
}

// Get size.
//------------------------------------------------------------------------------
// Gets remaining capacity.
static NNS_GFD_INLINE u32 GetCapacity_()
{
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );
    
    return (u32)( s_managerState_.hiAddr - s_managerState_.loAddr );
}



// Get
//------------------------------------------------------------------------------
static NNS_GFD_INLINE BOOL AllocFromLo_( u32 szByte, BOOL b4Pltt, u32* pResAddr )
{
    const u32 addr          = s_managerState_.loAddr;
    const u32 alignment     = GetUpperSideAlignmentSize_( addr, b4Pltt );// Perform alignment best suited to the format.
    const u32 increase      = szByte + alignment;
    
    
    NNS_GFD_ASSERT( szByte != 0 );
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );
    
    if( GetCapacity_() >= increase )
    {   
        const u32 tail          = s_managerState_.loAddr + increase;
        //
        // 4 color palette has a region that cannot be referenced.
        // 
        if( b4Pltt )
        {
            if( tail > NNS_GFD_4PLTT_MAX_ADDR )
            {
                // error
                return FALSE;
            }
        }
         
        *pResAddr               = s_managerState_.loAddr + alignment;
        s_managerState_.loAddr += increase;
        
        return TRUE;
    }else{
        
        return FALSE;
    }
}



//------------------------------------------------------------------------------
static NNS_GFD_INLINE BOOL AllocFromHi_( u32 szByte, BOOL b4Pltt, u32* pResAddr )
{
    NNS_GFD_ASSERT( szByte != 0 );
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );
    
    if( s_managerState_.hiAddr >= szByte )
    {
        const u32 addr          = (u32)(s_managerState_.hiAddr - szByte);
        const u32 alignment     = GetLowerSideAlignmentSize_( addr, b4Pltt );
        const u32 increase      = szByte + alignment;
        
        //OS_Printf("addr = %d\n", addr );
        //OS_Printf("alignment = %d\n", alignment );
        //OS_Printf("increase = %d\n", increase );
        
        if( GetCapacity_() >= increase )
        {    
            const u32 tail          = s_managerState_.hiAddr;
            
            if( b4Pltt )
            {
                if( tail > NNS_GFD_4PLTT_MAX_ADDR )
                {
                    // error
                    return FALSE;
                }
            }
            
            s_managerState_.hiAddr -= increase;
            *pResAddr               = s_managerState_.hiAddr;
            
            return TRUE;
        }
    }
    
    return FALSE;
}

//------------------------------------------------------------------------------
// standard implementation of NNSGfdFrmPlttVramDebugDumpCallBack
static void FrmPlttVramDebugDumpCallBack_( u32 loAddr, u32 hiAddr, u32 szFree, u32 szTotal )
{
#ifdef SDK_FINALROM
    #pragma unused(loAddr)
    #pragma unused(hiAddr)
    #pragma unused(szFree)
    #pragma unused(szTotal)
#endif // SDK_FINALROM
    
    OS_Printf("===== NNS_Gfd FrmPlttVramManager Dump ====\n");
    OS_Printf("head-Addr   : tail-addr   : free-size \n");
    
    
    OS_Printf("0x%08x  : 0x%08x  : 0x%08x  \n", loAddr, hiAddr, szFree );
    
    OS_Printf("    %08d / %08d bytes (%6.2f%%) used \n", szTotal - szFree, szTotal, 
        100.0f * ( szTotal - szFree ) / szTotal ); 
    OS_Printf("==========================================\n");
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdDumpFrmPlttVramManager

  Description:  Sends the manager's content to debugging output.
                
                 
  Arguments:   None.

                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdDumpFrmPlttVramManager()
{
    NNS_GfdDumpFrmPlttVramManagerEx( FrmPlttVramDebugDumpCallBack_ );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdDumpFrmPlttVramManagerEx

  Description:  Sends the manager's content to debugging output.
                
                 
  Arguments:   pFunc            debug dump callback function

                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdDumpFrmPlttVramManagerEx( NNSGfdFrmPlttVramDebugDumpCallBack pFunc )
{
    NNS_GFD_NULL_ASSERT( pFunc );
    
    (*pFunc)( s_managerState_.loAddr, 
              s_managerState_.hiAddr,
              s_managerState_.hiAddr - s_managerState_.loAddr,
              s_managerState_.szTotal );
}


/*---------------------------------------------------------------------------*
  Name:         NNS_GfdInitFrmPlttVramManager

  Description:  Initializes manager.
                
                 
  Arguments:   szByte       number of bytes of palette RAM managed by the frame palette VRAM manager 
               useAsDefault whether or not to use the frame palette VRAM manager by default 

                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdInitFrmPlttVramManager( u32 szByte, BOOL useAsDefault )
{
    NNS_GFD_MINMAX_ASSERT( szByte, 0, NNS_GFD_TEXPLTT_MAX_ADDR );
     
    s_managerState_.szTotal = szByte;
    
    NNS_GfdResetFrmPlttVramState();
    
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );

	if (useAsDefault)
	{
		NNS_GfdDefaultFuncAllocPlttVram = NNS_GfdAllocFrmPlttVram;
		NNS_GfdDefaultFuncFreePlttVram  = NNS_GfdFreeFrmPlttVram;
	}
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdAllocFrmPlttVram

  Description:  Allocates memory.
                
                 
  Arguments:   szByte:  Size
               bPltt4           :  4 color texture?
               allocFrom        :  Allocate from the top of the region (lower position)?
                            
  Returns:     texture palette key
  
 *---------------------------------------------------------------------------*/
NNSGfdPlttKey NNS_GfdAllocFrmPlttVram
( 
    u32         szByte, 
    BOOL        bPltt4, 
    u32         allocFrom
)
{
    u32     addr    = 0x0;
    BOOL    result  = FALSE;
    
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );
    
    {
        //
        // If a size is allocated that is too small to be displayed with a palette key, this rounds the size up.
        //
        szByte = NNSi_GfdGetPlttKeyRoundupSize( szByte );
        //
        // If a size is allocated that is too large for the palette key to display, this returns an error key.
        //
        if( szByte >= NNS_GFD_PLTTSIZE_MAX )
        {
            NNS_GFD_WARNING("Allocation size is too big. : NNS_GfdAllocLnkPlttVram()");
            return NNS_GFD_ALLOC_ERROR_PLTTKEY;
        }
        
        NNS_GFD_MINMAX_ASSERT( szByte, NNS_GFD_PLTTSIZE_MIN, NNS_GFD_PLTTSIZE_MAX );
    }
    
    if( allocFrom == NNS_GFD_ALLOC_FROM_LOW )
    {
        result = AllocFromLo_( szByte, bPltt4, &addr );
    }else{
        result = AllocFromHi_( szByte, bPltt4, &addr );
    }
    
    
    if( result )
    {   
        //
        // generate TexPlttKey
        //
        return NNS_GfdMakePlttKey( addr, szByte );
    }
        
NG_CASE:
    // error
    NNS_GFD_WARNING("failure in Vram Allocation. : NNS_GfdPlttFrmTexAlloc()");
    return NNS_GFD_ALLOC_ERROR_PLTTKEY;
    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdFreeFrmPlttVram

  Description:  Deallocates memory.
                Actually does nothing.
                 
  Arguments:   plttKey          :   texture palette key
                            
  Returns:     success or failure value (if successful, 0)
  
 *---------------------------------------------------------------------------*/
int NNS_GfdFreeFrmPlttVram( NNSGfdPlttKey plttKey )
{
#pragma unused(plttKey)
    // What region is this memory allocated from?
    
    // when Lo Head( 4x4 ) 
    //        => indexTable also deallocated at same time.
    
    // Lo Tail( Nrm )
    // Hi Tail( Nrm )
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdGetPlttFrmManagerState

  Description:  Gets current manager state.
                
                 
  Arguments:   pState          :   manager state information
                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdGetFrmPlttVramState( NNSGfdFrmPlttVramState* pState )
{
    NNS_GFD_NULL_ASSERT( pState );
    NNS_GFD_ASSERT( IsVramManagerValid_( ) );
    
    pState->address[0] = s_managerState_.loAddr;
    pState->address[1] = s_managerState_.hiAddr;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_GfdSetFrmPlttVramState

  Description:  Sets specified state information in manager state.
                 
  Arguments:   pState          :   manager state information
                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdSetFrmPlttVramState( const NNSGfdFrmPlttVramState* pState )
{
    NNS_GFD_NULL_ASSERT( pState );
    
    s_managerState_.loAddr = pState->address[0];
    s_managerState_.hiAddr = pState->address[1];
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdResetPlttFrmManagerState

  Description:  Resets manager's current state.
                 
  Arguments:   None.
                            
  Returns:     None.
  
 *---------------------------------------------------------------------------*/
void NNS_GfdResetFrmPlttVramState()
{ 
    s_managerState_.loAddr = NNS_GFD_TEXPLTT_BASE_ADDR;
    s_managerState_.hiAddr = s_managerState_.szTotal;
}

