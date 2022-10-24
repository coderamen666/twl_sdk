#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB - multiboot
# File:     Makefile
#
# Copyright 2004-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS				=	

# Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
# The mb_child.bin sample program in the $TwlSDK/bin/ARM9-TS/Release directory provides the same functionality as a commercial system that is a multiboot child. Load this binary on other systems just like a sample program and run them together.
# 
# 
# 
# 
# 

#----------------------------------------------------------------------------
TARGET_PLATFORM		=	TWL NITRO

SRCS				=	common.c data.c dispfunc.c parent.c main.c
TARGET_NEF			=	main.nef
TARGET_BIN			=	main.srl

#SRCDIR				=	# using default

# makerom settings
MAKEROM_ROMROOT		=	./data
MAKEROM_ROMFILES	=	*.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			=	main.TWL.rsf
LCFILE_SPEC			=	ARM9.TWL-$(TWL_PLATFORM).lsf
else
ROM_SPEC			=	main.rsf
LCFILE_SPEC			=	ARM9-$(NITRO_PLATFORM).lsf
endif

#----------------------------------------------------------------------------

do-build:			$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	NITRO_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#NITRO_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#NITRO_ELFTOBIN


#===== End of Makefile =====
