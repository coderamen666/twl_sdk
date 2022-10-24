#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - el - dllB
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
#----------------------------------------------------------------------------

TARGET_PLATFORM	:= TWL

SRCS			 =	dllB.c
TARGET_LIB		 =	dllB$(TWL_LIBSUFFIX).a

LINCLUDES		 =	../simple-2_dllA/include

LDIRT_CLEAN		 = $(wildcard dllB.*.a) $(wildcard dllB_debug.*.a)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------

#SRCDIR			 =	# using default
#LCFILE			 =	# using default

#----------------------------------------------------------------------------

do-build:		$(TARGETS) strip_dllB$(TWL_LIBSUFFIX).a

strip_dllB$(TWL_LIBSUFFIX).a:		$(LIBDIR)/dllB$(TWL_LIBSUFFIX).a
	cp $(LIBDIR)/dllB$(TWL_LIBSUFFIX).a dllB_debug$(TWL_LIBSUFFIX).a
	$(STRIPDEBUG) -o dllB$(TWL_LIBSUFFIX).a -O dllB_debug$(TWL_LIBSUFFIX)_for_debugger.a dllB_debug$(TWL_LIBSUFFIX).a
	cp dllB_debug$(TWL_LIBSUFFIX)_for_debugger.a dllB_debug$(TWL_LIBSUFFIX).a
	cp dllB$(TWL_LIBSUFFIX).a ../simple-2_static/data/dllB.a
	cp dllB_debug$(TWL_LIBSUFFIX).a ../simple-2_static/data/dllB_debug.a

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
