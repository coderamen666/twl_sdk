#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - PRC
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

SUBDIRS				=

#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO

SRCS				=
PATTERN_SOURCE		= src/patterns.c
PATTERN_HEADER		= include/patterns.h
PATTERN_TARGETS		= $(PATTERN_SOURCE) $(PATTERN_HEADER)
PATTERNS			= data/patterns.txt
LNEWDIRS			= ./src ./include
#LDIRT_CLEAN		= $(LNEWDIRS)
TWL_FORCE_BUILD 	= 1

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:			$(PATTERN_TARGETS)

pattern:			$(PATTERN_TARGETS)

$(PATTERN_SOURCE): $(PATTERNS)
	$(PERL) $(ROOT)/tools/bin/pdic2c.pl -c -C $@ -H $(PATTERN_HEADER) 64 $<

$(PATTERN_HEADER): $(PATTERNS)
	$(PERL) $(ROOT)/tools/bin/pdic2c.pl -h -H $@ 64 $<

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
