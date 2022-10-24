#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - spi
# File:     Makefile.CORE
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
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

SUBDIRS		 =
SUBMAKES_P 	+= $(PRIVATE_LIBRARIES_PM_ARM9)

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN	?= ALL
NITRO_PROC		 = ARM9

SRCDIR			 = src
INCDIR			 = include

SRCS			 = tp.c 	\
				   mic.c 	\
				   pm.c

TARGET_LIB		 = libspi$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		 = $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
