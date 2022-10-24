#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - ctrdg/ARM9
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2007-11-07#$
# $Rev: 2155 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

TWL_CODEGEN		?= ALL
TWL_PROC		 = ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR			 = ../common/src ../ARM9/src
INCDIR			 = ../common/include ../ARM9/include

SRCS		 =	ctrdg.c							\
				ctrdg_proc.c					\
				ctrdg_vib.c						\
				ctrdg_backup.c					\
				ctrdg_flash_common.c			\
				ctrdg_flash_AT29LV512.c			\
				ctrdg_flash_LE26FV10N1TS-10.c	\
				ctrdg_flash_LE39FW512.c			\
				ctrdg_flash_MX29L010.c			\
				ctrdg_flash_MX29L512.c			\
				ctrdg_sram.c					\
				ctrdg_task.c

TARGET_LIB	 = libctrdg$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
