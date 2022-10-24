#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - libraries - snd
# File:     Makefile
#
# Copyright 2004-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Revision: 1155 $
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------

SRCS	= \
	main.c \
	resource_mgr.c \
	waveout.c \
	player.c \
	stream.c \
	capture.c \
	sndarc.c \
	heap.c \
	sndarc_loader.c \
	sndarc_player.c \
	sndarc_stream.c \
	output_effect.c \
	seqdata.c	\
	fader.c

TARGET_LIB	=	libnnssnd$(NITRO_LIBSUFFIX).a

include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR		=	$(TWLSYSTEM_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
