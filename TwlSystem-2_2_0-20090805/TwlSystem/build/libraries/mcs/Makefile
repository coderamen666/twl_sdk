#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - libraries - mcs
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
# $Revision: 1412 $
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------

SRCS	= \
	base.c          \
	ringBuffer.c    \
	fileIOmain.c    \
	print.c         \
	istd_stub.c

TARGET_LIB	= libnnsmcs$(NITRO_LIBSUFFIX).a


include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR		=	$(TWLSYSTEM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
