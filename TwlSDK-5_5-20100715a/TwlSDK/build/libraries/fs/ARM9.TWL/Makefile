#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - FS - libraries - ARM9
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#
#----------------------------------------------------------------------------

SUBDIRS          =

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

TWL_CODEGEN     ?= ALL
TWL_PROC		 = ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR           = ../common/src src

SRCS             = \
				   fs_archive.c			\
				   fs_file.c			\
				   fs_hook.c			\
				   fs_overlay.c			\
				   fs_archive_fatfs.c	\
				   fs_proc_default.c	\
				   fs_proc_mem.c		\
				   fs_proc_rom.c		\
				   fs_api.c

TARGET_LIB       = libfs$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

SRCS			+= $(PRIVATE_LIBRARIES_FS_ARM9_TWL_SRCS)

INSTALL_TARGETS  = $(TARGETS)
INSTALL_DIR      = $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
