#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - FS - libraries - ARM9
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#
#----------------------------------------------------------------------------

SUBDIRS             =

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN		?= ALL
NITRO_PROC			 = ARM9

SRCDIR               = ../common/src src

SRCS                 = \
					   fs_archive.c			\
					   fs_file.c			\
					   fs_hook.c			\
					   fs_overlay.c			\
					   fs_archive_fatfs.c	\
					   fs_proc_default.c	\
					   fs_proc_mem.c		\
					   fs_proc_rom.c		\
					   fs_api.c

TARGET_LIB           = libfs$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS      = $(TARGETS)
INSTALL_DIR          = $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
