#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-2
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
# $Date:: 2008-09-15#$
# $Rev: 8479 $
# $Author: kitase_hirotake $
#----------------------------------------------------------------------------

SUBDIRS		 =

#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL

TARGET_BIN		 = main.srl
MAKEROM_ROMROOT  = $(TWLSDK_ROOT)/data
MAKEROM_ROMFILES = shutter_sound/*.wav videorec_sound/*.wav

SRCS			 = main.c snd_data.c

SRCDIR           = src ../simpleShoot-1/src
INCDIR           = include ../simpleShoot-1/include

DATA_DIR		 = $(ROOT)/data/rec_icon
DATA_OBJS        = $(OBJDIR)/rec_icon.o
EXT_OBJS        += $(DATA_OBJS)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

$(OBJDIR)/%.o : $(DATA_DIR)/%.bmp
	$(NTEXCONV) -no -bg -bgb -bgf direct $(call cygpathm,$<) >/dev/null && \
	$(BIN2OBJ) --compatible $(basename $(call cygpathm,$<)).nbfs $@ >/dev/null
	rm $(basename $<).nbfs

#===== End of Makefile =====
