#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - el - simple-2
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
# $Date:: 2009-09-24#$
# $Rev: 11063 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM	:= TWL

TARGET_BIN		 = main.srl
TARGET_NEF		 = main.tef

SRCS			 = main.c staticsymlist.c
DATA_DIR		 = ./data

LINCLUDES		 =	../simple-2_dllA/include	\
					../simple-2_dllB/include

LDIRT_CLEAN		 = *.a staticsymlist.c $(wildcard $(DATA_DIR)/*.a)

ROM_SPEC		 =	main.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

GINCLUDES		+=	\
				$(ROOT)/include/twl		\
				$(ROOT)/include/twl/el	\
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------

#SRCDIR			 =	# using default
#LCFILE			 =	# using default

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

staticsymlist.c:		$(OBJDIR)/main.o
	$(MAKELST) -o staticsymlist.c --static $(OBJDIR)/main.o $(call empath,$(TWL_LIBDIR)/libos$(TWL_LIBSUFFIX).a) --dll $(DATA_DIR)/dllA.a $(DATA_DIR)/dllB.a

#===== End of Makefile =====
