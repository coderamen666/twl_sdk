#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - mi/ARM9
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-06-20#$
# $Rev: 6675 $
# $Author: nishimoto_takashi $
#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN		?= ALL
NITRO_PROC		= ARM9

SRCDIR			= ../common/src src
INCDIR			= ../common/include

SRCS			= mi_wram.c \
			  mi_dma.c \
			  mi_dma_hblank.c \
			  mi_dma_vblank.c \
			  mi_dma_mainmem.c \
			  mi_dma_gxcommand.c \
			  mi_memory.c \
			  mi_swap.c \
			  mi_uncompress.c \
			  mi_byteAccess.c \
			  mi_stream.c \
			  mi_card.c \
			  mi_dma_card.c \
			  mi_uncomp_stream.c \
			  mi_compress.c \
			  mi_exMemory.c \
			  mi_init.c \
			  mi_cache.c \
			  mi_secureuncompress.c

TARGET_LIB		= libmi$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
