/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-24#$
  $Rev: 11187 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "../include/mi_dma.h"

#ifdef SDK_ARM9
#include <nitro/itcm_begin.h>
//================================================================================
//          setting DMA
//================================================================================
void MIi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode)
{
	OSIntrMode enabled;
    vu32 *p;

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		enabled = OS_DisableInterrupts();
	}

    p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );

	if ( mode & MIi_DMA_MODE_SRC32 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b32 = src;
		src = (u32)srcp;
	}
	else if ( mode & MIi_DMA_MODE_SRC16 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b16 = (u16)src;
		src = (u32)srcp;
	}

    *p = (vu32)src;
    *(p + 1) = (vu32)dest;
    *(p + 2) = (vu32)ctrl;

	if ( mode & MIi_DMA_MODE_WAIT )
	{
		//---- ARM9 must wait 2 cycle (load is 1/2 cycle)
		{
			u32     dummy = reg_MI_DMA0SAD;
		}
		{
			u32     dummy = reg_MI_DMA0SAD;
		}

		if ( ! (mode & MIi_DMA_MODE_NOCLEAR) )
		{
			//---- for multiple DMA problem in DMA0
			if (dmaNo == MIi_DUMMY_DMA_NO)
			{
				*p = (vu32)MIi_DUMMY_SRC;
				*(p + 1) = (vu32)MIi_DUMMY_DEST;
				*(p + 2) = (vu32)MIi_DUMMY_CNT;
			}
		}
	}

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		(void)OS_RestoreInterrupts(enabled);
	}

	if ( mode & MIi_DMA_MODE_WAIT )
	{
		//---- ARM9 must wait 2 cycle (load is 1/2 cycle)
		{
			u32     dummy = reg_MI_DMA0SAD;
		}
		{
			u32     dummy = reg_MI_DMA0SAD;
		}
	}
}
#include <nitro/itcm_end.h>
#else // ifdef SDK_ARM9
void MIi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode)
{
	OSIntrMode enabled;
    vu32 *p;

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		enabled = OS_DisableInterrupts();
	}

    p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );

	if ( mode & MIi_DMA_MODE_SRC32 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b32 = src;
		src = (u32)srcp;
	}
	else if ( mode & MIi_DMA_MODE_SRC16 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b16 = (u16)src;
		src = (u32)srcp;
	}

    *p = (vu32)src;
    *(p + 1) = (vu32)dest;
    *(p + 2) = (vu32)ctrl;

	if ( mode & MIi_DMA_MODE_WAIT )
	{
		// ARM7 must wait 2 cycle (load is 3 cycle)
        u32     dummy = reg_MI_DMA0SAD;
	}

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		(void)OS_RestoreInterrupts(enabled);
	}
}
#endif

//================================================================================
//            memory oparation using DMA (sync)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill32

  Description:  fill memory with specified data.
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaFill32(u32 dmaNo, void *dest, u32 data, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC32 );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC32 | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy32

  Description:  copy memory with DMA
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend32

  Description:  send u32 data to fixed address
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaSend32(u32 dmaNo, const void *src, volatile void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, 0);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32

  Description:  receive u32 data from fixed address
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaRecv32(u32 dmaNo, volatile const void *src, void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe32

  Description:  pipe data from fixed address to fixed address.
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaPipe32(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, 0);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE32(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill16

  Description:  fill memory with specified data.
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaFill16(u32 dmaNo, void *dest, u16 data, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC16 );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC16 | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy16

  Description:  copy memory with DMA
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend16

  Description:  send u16 data to fixed address
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaSend16(u32 dmaNo, const void *src, volatile void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, 0);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16

  Description:  send u16 data to fixed address
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaRecv16(u32 dmaNo, volatile const void *src, void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe16

  Description:  pipe data from fixed address to fixed address.
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaPipe16(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, BOOL dmaEnable)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, 0);

    if (size > 0)
    {
		//---- check DMA0 source address
		MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

		MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
		if ( dmaEnable )
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16(size),
							  MIi_DMA_MODE_WAIT );
		}
		else
		{
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE16(size),
							  MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR );
		}
		MIi_Wait_AfterDMA(dmaCntp);
	}
}

//================================================================================
//            memory oparation using DMA (async)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill32Async

  Description:  fill memory with specified data.
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaFill32Async(u32 dmaNo, void *dest, u32 data, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32_IF(size), MIi_DMA_MODE_SRC32 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR32_IF(size), MIi_DMA_MODE_SRC32 | MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32(size), MIi_DMA_MODE_SRC32 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR32(size), MIi_DMA_MODE_SRC32 | MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy32Async

  Description:  copy memory with DMA
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY32_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY32(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend32Async

  Description:  send u32 data to fixed address
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaSend32Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND32_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND32(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32Async

  Description:  send u32 data to fixed address
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaRecv32Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV32_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV32(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe32Async

  Description:  pipe data from fixed address to fixed address.
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaPipe32Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, 0);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE32_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE32(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill16Async

  Description:  fill memory with specified data.
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaFill16Async(u32 dmaNo, void *dest, u16 data, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16_IF(size), MIi_DMA_MODE_SRC16 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR16_IF(size), MIi_DMA_MODE_SRC16 | MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16(size), MIi_DMA_MODE_SRC16 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR16(size), MIi_DMA_MODE_SRC16 | MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy16Async

  Description:  copy memory with DMA
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY16_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY16(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend16Async

  Description:  send u16 data to fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaSend16Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND16_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND16(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16Async

  Description:  send u16 data to fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaRecv16Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV16_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV16(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe16Async

  Description:  receive u16 data from fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_DmaPipe16Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, 0);
    MIi_WARNING_ADDRINTCM(dest, 0);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16_IF(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE16_IF(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
        else
        {
			if ( dmaEnable )
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16(size), 0 );
			}
			else
			{
				MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE16(size), MIi_DMA_MODE_NOCLEAR );
			}
        }
    }
}

//================================================================================
//       DMA WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_IsDmaBusy

  Description:  check whether DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL MI_IsDmaBusy(u32 dmaNo)
{
    vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_CNT_WOFFSET );

    MIi_ASSERT_DMANO(dmaNo);
    return (BOOL)((*(vu32 *)dmaCntp & REG_MI_DMA0CNT_E_MASK) >> REG_MI_DMA0CNT_E_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_WaitDma

  Description:  wait while DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_WaitDma(u32 dmaNo)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_CNT_WOFFSET );

    MIi_ASSERT_DMANO(dmaNo);
    while (*dmaCntp & REG_MI_DMA0CNT_E_MASK)
    {
    }

    //---- for multiple DMA problem in DMA0
    if (dmaNo == MIi_DUMMY_DMA_NO)
    {
        vu32   *p = MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );
        *p = (vu32)MIi_DUMMY_SRC;
        *(p + 1) = (vu32)MIi_DUMMY_DEST;
        *(p + 2) = (vu32)MIi_DUMMY_CNT;
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MI_StopDma

  Description:  stop DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_StopDma(u32 dmaNo)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu32   *dmaCntp = (vu32*)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

    MIi_ASSERT_DMANO(dmaNo);

    *dmaCntp &= ~(MI_DMA_TIMING_MASK | MI_DMA_CONTINUOUS_ON);
    *dmaCntp &= ~MI_DMA_ENABLE;

    //---- ARM9 must wait 2 cycle (load is 1/2 cycle)
    {
        u32     dummy = dmaCntp[0];
    }
    {
        u32     dummy = dmaCntp[0];
    }

    //---- for multiple DMA problem in DMA0
    if (dmaNo == MIi_DUMMY_DMA_NO)
    {
        vu32   *p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);
        *p = (vu32)MIi_DUMMY_SRC;
        *(p + 1) = (vu32)MIi_DUMMY_DEST;
        *(p + 2) = (vu32)MIi_DUMMY_CNT;
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MI_StopAllDma

  Description:  stop all DMA

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_StopAllDma(void)
{
	MI_StopDma(0);
	MI_StopDma(1);
	MI_StopDma(2);
	MI_StopDma(3);
}

//================================================================================
//        restart DMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaRestart

  Description:  restart DMA
                Just set the enable bit.

  Arguments:    ndmaNo : DMA channel	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) |= MI_DMA_ENABLE;
 No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRestart(u32 dmaNo)
{
    vu32   *dmaCntp = (vu32*)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

	MIi_ASSERT_DMANO(dmaNo);
    *dmaCntp |= MI_DMA_ENABLE;
}

//================================================================================
//           for avoid multiple auto start DMA
//================================================================================
#ifdef SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         MIi_CheckAnotherAutoDMA

  Description:  check whether other auto DMA is running.
                (however, HBlankDMA and VBlankDMA are permit to
                 run at the same time.)

  Arguments:    dmaNo   : DMA channel No.
                dmaType : DMA type
                src     : source address
                size    : DMA size
                dir     : source direction (MI_DMA_SRC_INC/DEC/FIX)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_CheckAnotherAutoDMA(u32 dmaNo, u32 dmaType)
{
    int     n;
    u32     dmaCnt;
    u32     timing;

    for (n = 0; n < MI_DMA_MAX_NUM; n++)
    {
        if (n == dmaNo)
        {
            continue;
        }

        dmaCnt = *(REGType32v *)(REG_DMA0CNT_ADDR + n * 12);

        //---- not used
        if ((dmaCnt & MI_DMA_ENABLE) == 0)
        {
            continue;
        }

        timing = dmaCnt & MI_DMA_TIMING_MASK;

        //---- if two DMAs are same, or HDMA and VDMA, it is allowed
        if (timing == dmaType
            || (timing == MI_DMA_TIMING_V_BLANK && dmaType == MI_DMA_TIMING_H_BLANK)
            || (timing == MI_DMA_TIMING_H_BLANK && dmaType == MI_DMA_TIMING_V_BLANK))
        {
            continue;
        }

        //---- check other auto DMA running
        if (timing == MI_DMA_TIMING_DISP
            || timing == MI_DMA_TIMING_DISP_MMEM
            || timing == MI_DMA_TIMING_CARD
            || timing == MI_DMA_TIMING_CARTRIDGE
            || timing == MI_DMA_TIMING_GXFIFO
            || timing == MI_DMA_TIMING_V_BLANK || timing == MI_DMA_TIMING_H_BLANK)
        {
            OS_TPanic("cannot start auto DMA at the same time.");
        }
    }
}

#endif // ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         MIi_CheckDma0SourceAddress

  Description:  IN case of using DMA0, check source address.
                Source address which is in I/O register or cartridge bus
                is not available.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                size    : DMA size
                dir     : source direction (MI_DMA_SRC_INC/DEC/FIX)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_CheckDma0SourceAddress(u32 dmaNo, u32 src, u32 size, u32 dir)
{
#ifdef SDK_TWL
    //---- If DMAC is fixed, not check here. (only on TWL)
    if ( OS_IsRunOnTwl() && SCFG_IsDmacFixed() )
    {
        return;
    }
#endif

    //---- only in case of DMA0
    if (dmaNo == 0)
    {
        u32     addStart;
        u32     addEnd;

        //---- start address of souce
        addStart = src & 0xff000000;

        //---- end address of source
        switch (dir)
        {
        case MI_DMA_SRC_INC:
            addEnd = src + size;
            break;
        case MI_DMA_SRC_DEC:
            addEnd = src - size;
            break;
        default:
            addEnd = src;
            break;
        }
        addEnd &= 0xff000000;

        //---- check start and end address of source
        if (addStart == 0x04000000 || addStart >= 0x08000000 ||
            addEnd == 0x04000000 || addEnd >= 0x08000000)
        {
            OS_TPanic("illegal DMA0 source address.");
        }
    }
}

//================================================================================
//           check if area is in TCM
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_CheckAddressInTCM

  Description:  Check if specified area (from address to address+size)
                is in ITCM/DTCM.

  Arguments:    addr  : start address
                size  : size

  Returns:      None
 *---------------------------------------------------------------------------*/
#if defined( SDK_ARM9 ) && defined( SDK_DEBUG )
void MIi_CheckAddressInTCM(u32 addr, u32 size)
{
    u32     itcm = HW_ITCM;            // ITCM fixed to HW_ITCM (=0x01ff8000)
    u32     dtcm = OS_GetDTCMAddress();

    SDK_WARNING(itcm >= addr + size
                || addr >= itcm + HW_ITCM_SIZE, "intend to do DMA in ITCM area (%x)", addr);
    SDK_WARNING(dtcm >= addr + size
                || addr >= dtcm + HW_DTCM_SIZE, "intend to do DMA in DTCM area (%x)", addr);
}
#endif

//================================================================================
//		 setting each register directly (internal)
//================================================================================
void MIi_SetDmaSrc16( u32 dmaNo, void *src )
{
	vu32 *p;

    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_WARNING_ADDRINTCM(src, 0); // size is unknown

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );
	*p = (vu32)src;
}
void MIi_SetDmaSrc32( u32 dmaNo, void *src )
{
	vu32 *p;

    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, 0); // size is unknown

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );
	*p = (vu32)src;
}
void MIi_SetDmaDest16( u32 dmaNo, void *dest )
{
	vu32 *p;

    MIi_ASSERT_SRC_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(dest, 0); // size is unknown

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_DAD_WOFFSET );
	*p = (vu32)dest;
}
void MIi_SetDmaDest32( u32 dmaNo, void *dest )
{
	vu32 *p;

    MIi_ASSERT_SRC_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(dest, 0); // size is unknown

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_DAD_WOFFSET );
	*p = (vu32)dest;
}

void MIi_SetDmaSize16( u32 dmaNo, u32 size )
{
	vu32 *p;

    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_CNT_WOFFSET );
	*p = (*p & ~REG_MI_DMA0CNT_WORDCNT_MASK) | (size/2);
}

void MIi_SetDmaSize32( u32 dmaNo, u32 size )
{
	vu32 *p;

    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);

	p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_CNT_WOFFSET );
	*p = (*p & ~REG_MI_DMA0CNT_WORDCNT_MASK) | (size/4);
}
