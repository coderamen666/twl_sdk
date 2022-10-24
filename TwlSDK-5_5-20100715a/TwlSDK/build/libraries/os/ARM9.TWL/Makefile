#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-10-19#$
# $Rev: 11099 $
# $Author: mizutani_nakaba $
#----------------------------------------------------------------------------

SUBDIRS			 =
SUBMAKES_P 		 = Makefile.CALLTRACE Makefile.FUNCTIONCOST

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

TWL_CODEGEN		?= ALL
TWL_PROC		 =ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR			 = src ../common/src ../ARM9/src
INCDIR			 = include ../common/include

SRCS			 = os_irqHandler.c			\
				   os_irqTable.c			\
				   os_interrupt.c			\
				   os_pxi.c					\
				   os_spinLock.c			\
				   os_printf.c				\
				   os_thread.c				\
				   os_context.c				\
				   os_emulator.c			\
				   os_message.c				\
				   os_mutex.c				\
				   os_cache.c				\
				   os_init.c				\
				   os_arena.c				\
				   os_alloc.c				\
				   os_tcm.c					\
				   os_protectionUnit.c		\
				   os_protectionRegion.c	\
				   os_exception.c			\
				   os_timer.c				\
				   os_tick.c				\
				   os_alarm.c				\
				   os_valarm.c				\
				   os_system.c				\
				   os_systemWork.c			\
				   os_profile.c				\
				   os_reset.c				\
				   os_ownerInfo.c			\
				   os_ownerInfoEx.c			\
				   os_vramExclusive.c		\
				   os_china.c				\
				   os_argument.c			\
				   os_entropy.c				\
				   os_terminate_proc.c		\
				   os_event.c				\
				   os_resource.c			\
				   os_application_jump.c	\
                   os_msJump.c				\
				   os_stub.c				\

ifeq ($(TWL_ARCHGEN),LIMITED)
SRCS +=	    	   os_attention.c
endif

TARGET_LIB		 = libos$(TWL_LIBSUFFIX).a

PC_DIR			 = ./pc_obj
PC_OBJ			 = $(PC_DIR)/os_countryCode.o

DATA_DIR		 = $(ROOT)/data/for_china
DATA_OBJS		 = $(patsubst $(DATA_DIR)/%.bin,$(OBJDIR)/%.o,$(wildcard $(DATA_DIR)/*.bin))

EXT_OBJS		+= $(DATA_OBJS) $(PC_OBJ)

ifeq ($(TWL_ARCHGEN),LIMITED)
LTD_DATA_DIR	 = $(ROOT)/data/attention_limited
LTD_DATA_OBJS	 = $(patsubst $(LTD_DATA_DIR)/%.bin,$(OBJDIR)/%.o,$(wildcard $(LTD_DATA_DIR)/*.bin))
EXT_OBJS		+= $(LTD_DATA_OBJS)
endif

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs


INSTALL_TARGETS	 = $(TARGETS)

INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)

SUBMAKES_P		 += $(PRIVATE_LIBRARIES_OS_ARM9_TWL)

#----------------------------------------------------------------------------
do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

# Convert screen data directly to object
$(OBJDIR)/%.o: $(DATA_DIR)/%.bin
			$(BIN2OBJ) --compatible $(call empath,$<) $@ >/dev/null

ifeq ($(TWL_ARCHGEN),LIMITED)
$(OBJDIR)/%.o: $(LTD_DATA_DIR)/%.bin
			$(BIN2OBJ) --compatible $(call empath,$<) $@ >/dev/null
endif

#===== End of Makefile =====
