#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - os - argument-2
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-18#$
# $Rev: 10784 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	=	TWL NITRO

SRCS			=	main.c
TARGET_BIN		=	main.srl

#----------------------------------------------------------------------------
ifdef	TARGET_FINALROM
SKIP_DEMO = TRUE
else
ifdef	TWL_FINALROM
SKIP_DEMO = TRUE
else
ifdef	NITRO_FINALROM
SKIP_DEMO = TRUE
endif
endif
endif

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

BURYARG_TWL    = $(TOOLSDIR)/bin/buryarg.TWL.exe
BURYARG_NITRO  = $(TOOLSDIR)/bin/buryarg.exe

ARGUMENT_STR   = "-x (C) NINTENDO" 2008

#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------

do-build:		$(TARGETS)
ifndef	SKIP_DEMO
	@echo "-- replace argument data" $(TWL_STDOUT)
ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
#	$(BURYARG_TWL) -q $(TARGETS)           $(ARGUMENT_STR)
	$(BURYARG_TWL) -q $(TARGETS:.srl=.tlf) $(ARGUMENT_STR)
	$(MAKEROM) $(MAKEROM_FLAGS) $(MAKEROM_DEFS) $(ROM_SPEC_M) $(call empath,$(TARGETS))
else
	$(BURYARG_NITRO) -q $(TARGETS)           $(ARGUMENT_STR)
	$(BURYARG_NITRO) -q $(TARGETS:.srl=.nlf) $(ARGUMENT_STR)
endif
	@echo -- done. try to execute $(TARGET_BIN). $(TWL_STDOUT)
endif

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
