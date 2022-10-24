#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - CP - libraries
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
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			=

SRCS			= cp_context.c

#----------------------------------------------------------------------------
TARGET_PLATFORM		 = TWL NITRO

# build ARM & THUMB libraries
NITRO_CODEGEN	 	?= ALL

# to avoid compiler-bug "blx**"
#NITRO_CONSERVATIVE=TRUE

TARGET_LIB		= libcp$(NITRO_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(NITRO_INSTALL_LIBDIR)

ifdef NITRO_TCM_APPLY
CCFLAGS			+= -DSDK_TCM_APPLY
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
