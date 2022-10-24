/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SCFG
  File:     scfg_proc.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $

 *---------------------------------------------------------------------------*/
#include <nitro/pxi/common/fifo.h>
#include <twl/scfg.h>

typedef struct
{
	BOOL		 lock;
	u64          fuseData;
	u32          readFlag;
	SCFGCallback callback;
	void*        callbackArg;
}
SCFGFuseInfo;

SCFGFuseInfo SCFGi_FuseInfo;

static void SCFGi_SwitchCpuSpeed( SCFGCpuSpeed cpuSpeed );
static void SCFGi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err);
static void SCFGi_SendPxiData(u32 command, u16 ordinal, u16 data);
static void SCFGi_Sync( u64 fuseData, void* arg );

/*---------------------------------------------------------------------------*
  Name:         SCFG_Init

  Description:  initialize scfg

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SCFG_Init(void)
{
    PXI_SetFifoRecvCallback( PXI_FIFO_TAG_SCFG, SCFGi_CommonCallback );

	//---- clear fuse info
	SCFGi_FuseInfo.lock = FALSE;
}

//================================================================================
//  CPU SPEED
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         (SCFGi_SwitchCpuSpeed)

  Description:  subroutine of SCFG_SetCpuSpeed.
                switch cpu speed and wait while proper clocks

  Arguments:    cpuSpeed : SCFG_CPU_SPEED_1X : same as NITRO (67.03MHz)
                           SCFG_CPU_SPEED_2X : doubled against NITRO (134.06MHz)

  Returns:      None
 *---------------------------------------------------------------------------*/
#define SCFGi_SWITCH_CPU_WAIT	10    // 10 cycle for safety (actually 8 cycle is needed)
#define SCFGi_SWITCH_CPU_WAIT2  (SCFGi_SWITCH_CPU_WAIT*2)

#include <twl/itcm_begin.h>
#include <nitro/code32.h>
asm void SCFGi_SwitchCpuSpeed( SCFGCpuSpeed cpuSpeed )
{
	//---- get current cpu speed setting
	ldr		r2, =REG_CLK_ADDR
	ldrh	r1, [r2] 

	//---- clear bit
	bic		r1, r1, #REG_SCFG_CLK_CPUSPD_MASK	// #REG_SCFG_CLK_CPUSPD_MASK = #1
	orr		r1, r0, r1
	strh	r1, [r2]

	cmp		r0, #SCFG_CPU_SPEED_1X
	moveq	r0, #SCFGi_SWITCH_CPU_WAIT
	movne	r0, #SCFGi_SWITCH_CPU_WAIT2

_1:	
	subs	r0, r0, #4
	bcs		_1
	bx		lr
}
#include <nitro/codereset.h>
#include <twl/itcm_end.h>

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetCpuSpeed

  Description:  set ARM9 CPU speed

  Arguments:    cpuSpeed : SCFG_CPU_SPEED_1X : same as NITRO (67.03MHz)
                           SCFG_CPU_SPEED_2X : doubled against NITRO (134.06MHz)
  Returns:      None
 *---------------------------------------------------------------------------*/
void SCFG_SetCpuSpeed( SCFGCpuSpeed cpuSpeed )
{
	OSIntrMode enable;

	SDK_ASSERT( cpuSpeed == SCFG_CPU_SPEED_1X || cpuSpeed == SCFG_CPU_SPEED_2X );

	enable = OS_DisableInterrupts();

	//---- if same with current speed, skip switching
	if ( cpuSpeed != SCFG_GetCpuSpeed() )
	{
		SCFGi_SwitchCpuSpeed( cpuSpeed );
	}

	(void)OS_RestoreInterrupts(enable);
}

//================================================================================
//  PXI system
//================================================================================
#define SCFGi_READ_FUSE_DONE  0xf

/*---------------------------------------------------------------------------*
  Name:         ( SCFGi_CommonCallback )

  Description:  SCFG callback for PXI

  Arguments:    tag     :  5bit
                pxiData : 26bit
                err     :  1bit

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SCFGi_CommonCallback( PXIFifoTag tag, u32 pxiData, BOOL err )
{
#pragma unused (tag, err)
	u16 command = (u16)( (pxiData & SCFG_PXI_COMMAND_MASK) >> SCFG_PXI_COMMAND_SHIFT );
	u16 ordinal = (u16)( (pxiData & SCFG_PXI_ORDINAL_MASK) >> SCFG_PXI_ORDINAL_SHIFT );
	u16 data    = (u16)( (pxiData & SCFG_PXI_DATA_MASK)    >> SCFG_PXI_DATA_SHIFT );

	switch( command )
	{
		case SCFGi_PXI_COMMAND_READ:
			//OS_TPrintf("ARM9: (%d) %04x\n", ordinal, data);
			SCFGi_FuseInfo.fuseData |= (((u64)data) << (ordinal << 4));
			SCFGi_FuseInfo.readFlag |= (1 << ordinal);

			//---- check if reading fuse data is complete
			if ( SCFGi_FuseInfo.readFlag == SCFGi_READ_FUSE_DONE )
			{
				if (SCFGi_FuseInfo.callback)
				{
					(SCFGi_FuseInfo.callback)(SCFGi_FuseInfo.fuseData, SCFGi_FuseInfo.callbackArg);
					SCFGi_FuseInfo.callback = NULL;
				}
				SCFGi_FuseInfo.lock = FALSE;
			}
			break;
		case SCFGi_PXI_COMMAND_READ_OP:
			{
				u64		tempData;
			
				*((u16*)&tempData) = data;
				if (SCFGi_FuseInfo.callback)
				{
					(SCFGi_FuseInfo.callback)(tempData, SCFGi_FuseInfo.callbackArg);
					SCFGi_FuseInfo.callback = NULL;
				}
				SCFGi_FuseInfo.lock = FALSE;
			}
			break;
		default:
			//---- unknown pxi command
			OS_TPanic("illegal SCFG Pxi.");
	}
}

/*---------------------------------------------------------------------------*
  Name:         ( SCFGi_SendPxiData )

  Description:  send data via PXI

  Arguments:    data : data to send

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SCFGi_SendPxiData(u32 command, u16 ordinal, u16 data)
{
	u32 pxiData =
		(u32)(((command << SCFG_PXI_COMMAND_SHIFT) & SCFG_PXI_COMMAND_MASK) |
			  ((ordinal << SCFG_PXI_ORDINAL_SHIFT) & SCFG_PXI_ORDINAL_MASK) |
			  ((   data << SCFG_PXI_DATA_SHIFT)    & SCFG_PXI_DATA_MASK ) );

    while (PXI_SendWordByFifo(PXI_FIFO_TAG_SCFG, pxiData, FALSE) != PXI_FIFO_SUCCESS)
    {
        // do nothing
    }
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadFuseDataAsync

  Description:  send a read command to ARM7

  Arguments:    callback : callback
                arg      : callback argument 

  Returns:      TRUE  : success to send a command
                FALSE : failed (locked.)
 *---------------------------------------------------------------------------*/
BOOL SCFG_ReadFuseDataAsync( SCFGCallback callback, void* arg )
{
	OSIntrMode enable = OS_DisableInterrupts();

	if ( SCFGi_FuseInfo.lock )
	{
		(void)OS_RestoreInterrupts(enable);
		return FALSE;
	}

	SCFGi_FuseInfo.lock        = TRUE;
	SCFGi_FuseInfo.callback    = callback;
	SCFGi_FuseInfo.callbackArg = arg;
	SCFGi_FuseInfo.readFlag    = 0;
	SCFGi_FuseInfo.fuseData    = 0;

	//---- send a fuse read command to ARM7
	SCFGi_SendPxiData( SCFGi_PXI_COMMAND_READ, 0, 0 );

	(void)OS_RestoreInterrupts(enable);
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadFuseData

  Description:  read fuse data from ARM7.
                wait till finish to read

  Arguments:    None

  Returns:      0  : failed
                >0 : fuse data
 *---------------------------------------------------------------------------*/
typedef struct
{
	BOOL flag;
	u64  data;
} SCFGiDataForSync;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
u64 SCFG_ReadFuseData( void )
{
	volatile SCFGiDataForSync info;

	info.flag = FALSE;
	if ( SCFG_ReadFuseDataAsync( SCFGi_Sync, (void*)&info ) )
	{
		while( info.flag == FALSE )
		{
		}
		return info.data;
	}
	else
	{
		return 0;
	}
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadBondingOptionAsync

  Description:  send a bonding option read command to ARM7

  Arguments:    callback : callback
                arg      : callback argument 

  Returns:      TRUE  : success to send a command
                FALSE : failed (locked.)
 *---------------------------------------------------------------------------*/
BOOL SCFG_ReadBondingOptionAsync( SCFGCallback callback, void* arg )
{
	OSIntrMode enable = OS_DisableInterrupts();

	if ( SCFGi_FuseInfo.lock )
	{
		(void)OS_RestoreInterrupts(enable);
		return FALSE;
	}

	SCFGi_FuseInfo.lock	= TRUE;
	SCFGi_FuseInfo.callback = callback;
	SCFGi_FuseInfo.callbackArg = arg;

	//---- send a bonding option read command to ARM7
	SCFGi_SendPxiData( SCFGi_PXI_COMMAND_READ_OP, 0, 0 );

	(void)OS_RestoreInterrupts(enable);
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadBondingOption

  Description:  read bonding option information from ARM7.
                wait till finish to read

  Arguments:    None

  Returns:      0xffff  : failed
                0 ~ 3   : bonding option information
 *---------------------------------------------------------------------------*/
u16 SCFG_ReadBondingOption( void )
{
	volatile SCFGiDataForSync info;

	info.flag = FALSE;
	if ( SCFG_ReadBondingOptionAsync( SCFGi_Sync, (void*)&info ) )
	{
		while ( info.flag == FALSE )
		{
		}
		return *(u16*)(&info.data);
	}
	else
	{
		return 0xffff;
	}
}

/*---------------------------------------------------------------------------*
  Name:         ( SCFGi_Sync )

  Description:  callback for SCFG_ReadFuseData to set fusedata 

  Arguments:    fuseData : fuse data
                arg      : pointer to data info struct

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SCFGi_Sync( u64 fuseData, void* arg )
{
	((SCFGiDataForSync*)arg)->flag = TRUE;
	((SCFGiDataForSync*)arg)->data = fuseData;
}
