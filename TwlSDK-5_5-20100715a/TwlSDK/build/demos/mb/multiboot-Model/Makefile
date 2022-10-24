#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB
# File:     Makefile
#
# Copyright 2006-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

# Creates child boot binary
SUBDIRS		 =	child


# Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
# The mb_child.bin sample program in the $TwlSDK/bin/ARM9-TS/Release directory provides the same functionality as a commercial system that is a multiboot child. Load this binary on other systems just like a sample program and run them together.
# 
# 
# 
# 
# 


#----------------------------------------------------------------------------

INCDIR		 =	common/include parent/include
SRCDIR		 =	common/src parent/src
SRCS		 =	main.c      \
				common.c    \
				disp.c      \
				font.c      \
				gmain.c


# WH module was moved to wireless_shared

WH_DIR		 =	$(ROOT)/build/demos/wireless_shared/wh
SRCDIR		+=	$(WH_DIR)
INCDIR		+=	$(WH_DIR)
SRCS		+=	$(WH_DIR)/wh.c

# MBP was moved to wireless_shared

MBP_DIR		 =	$(ROOT)/build/demos/wireless_shared/mbp
SRCDIR		+=	$(MBP_DIR)
INCDIR		+=	$(MBP_DIR)
SRCS		+=	$(MBP_DIR)/mbp.c


TARGET_NEF	 =	main.nef
TARGET_BIN	 =	main.srl

# makerom settings
MAKEROM_ROMROOT		 =	./data
MAKEROM_ROMFILES	 =	*.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			 =	main.TWL.rsf
else
ROM_SPEC			 =	main.rsf
endif

# It is safer to set a slightly large IRQ stack size in the LCF file when running complex processes within the MB callback.
# Because OS_Printf() in particular consumes a large amount of stack, try using the lighter version, OS_TPrintf(), in callbacks whenever possible.
# 
# 

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN


#===== End of Makefile =====
