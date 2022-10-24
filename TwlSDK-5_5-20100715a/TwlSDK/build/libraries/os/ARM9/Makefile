#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - os
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
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

SUBDIRS			 =
SUBMAKES_P 		 = Makefile.CALLTRACE Makefile.FUNCTIONCOST

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN	?= ALL
NITRO_PROC		 = ARM9

SRCDIR			 = ../common/src src
INCDIR			 = ../common/include

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
				   os_vramExclusive.c		\
				   os_china.c				\
				   os_argument.c			\
				   os_entropy.c				\
				   os_terminate_proc.c		\
				   os_event.c				\
				   os_resource.c

TARGET_LIB		 = libos$(NITRO_LIBSUFFIX).a

DATA_DIR		 = $(ROOT)/data/for_china
DATA_OBJS		 = $(patsubst $(DATA_DIR)/%.bin,$(OBJDIR)/%.o,$(wildcard $(DATA_DIR)/*.bin))
EXT_OBJS	+= $(DATA_OBJS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		 = $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

OBJS			+= $(DATA_OBJS)

# Convert screen data directly to object
$(OBJDIR)/%.o: $(DATA_DIR)/%.bin
			$(BIN2OBJ) --compatible $(call empath,$<) $@ >/dev/null

#===== End of Makefile =====
