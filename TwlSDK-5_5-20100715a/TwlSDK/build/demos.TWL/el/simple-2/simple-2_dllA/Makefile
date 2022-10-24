#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - el - dllA
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

SRCS			 =	dllA.c
TARGET_LIB		 =	dllA$(TWL_LIBSUFFIX).a

LINCLUDES		 =	../simple-2_dllB/include

LDIRT_CLEAN		 = $(wildcard dllA.*.a) $(wildcard dllA_debug.*.a)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------

#SRCDIR			 =	# using default
#LCFILE			 =	# using default

#----------------------------------------------------------------------------

do-build:		$(TARGETS) strip_dllA$(TWL_LIBSUFFIX).a

strip_dllA$(TWL_LIBSUFFIX).a:		$(LIBDIR)/dllA$(TWL_LIBSUFFIX).a
	cp $(LIBDIR)/dllA$(TWL_LIBSUFFIX).a dllA_debug$(TWL_LIBSUFFIX).a
	$(STRIPDEBUG) -o dllA$(TWL_LIBSUFFIX).a -O dllA_debug$(TWL_LIBSUFFIX)_for_debugger.a dllA_debug$(TWL_LIBSUFFIX).a
	cp dllA_debug$(TWL_LIBSUFFIX)_for_debugger.a dllA_debug$(TWL_LIBSUFFIX).a
	cp dllA$(TWL_LIBSUFFIX).a ../simple-2_static/data/dllA.a
	cp dllA_debug$(TWL_LIBSUFFIX).a ../simple-2_static/data/dllA_debug.a

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
