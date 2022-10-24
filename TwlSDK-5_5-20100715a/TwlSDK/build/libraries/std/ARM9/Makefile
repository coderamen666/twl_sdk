#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - std/ARM9
# File:     Makefile
#
# Copyright 2005-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN		?= ALL
NITRO_PROC		= ARM9

SRCDIR			= ../common/src
INCDIR			= ../common/include

SRCS			= std_string.c std_sprintf.c std_stdlib.c \
				  std_unicode.c std_unicode_array.c

TARGET_LIB		= libstd$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
