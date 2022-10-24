/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - gfd - VramManager
  File:     gfd_FramePlttVramMan.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFD_FRAMEPLTTVRAMMAN_H_
#define NNS_GFD_FRAMEPLTTVRAMMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include <nnsys/gfd/VramManager/gfd_PlttVramMan_Types.h>


typedef struct NNSGfdFrmPlttVramState
{
    u32         address[2];
    
} NNSGfdFrmPlttVramState;

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdDumpFrmPlttVramManagerEx

  Description:  The user-defined debug dump callback function used by the NNS_GfdDumpFrmPlttVramManagerEx function.
                
                                            
 *---------------------------------------------------------------------------*/
typedef void (*NNSGfdFrmPlttVramDebugDumpCallBack)( u32 loAddr, u32 hiAddr, u32 szFree, u32 szTotal );



void            NNS_GfdDumpFrmPlttVramManager();
void            NNS_GfdDumpFrmPlttVramManagerEx( NNSGfdFrmPlttVramDebugDumpCallBack pFunc );

void			NNS_GfdInitFrmPlttVramManager( u32 szByte, BOOL useAsDefault );

NNSGfdPlttKey	NNS_GfdAllocFrmPlttVram( u32 szByte, BOOL b4Pltt, u32 bAllocFromLo );
int				NNS_GfdFreeFrmPlttVram( NNSGfdPlttKey plttKey );

void			NNS_GfdGetFrmPlttVramState( NNSGfdFrmPlttVramState* pState );
void			NNS_GfdSetFrmPlttVramState( const NNSGfdFrmPlttVramState* pState );
void			NNS_GfdResetFrmPlttVramState();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_GFD_FRAMEPLTTVRAMMAN_H_
