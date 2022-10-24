#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - dsp - g711
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

SUBDIRS             =

#----------------------------------------------------------------------------
override TARGET_PLATFORM	:= TWL

TWL_CODEGEN        ?= ALL
TWL_PROC            = ARM9
TWL_ARCHGEN        ?= ALL

SRCDIR              = ../ARM9.TWL/src src
INCDIR              = ../common/include

SRCS                = dsp_g711.c

# Standard DSP component group that should be statically linked
DSPCOMPONENTS_NAME  = audio

DSPCOMPONENTS_DIR   = $(ROOT)/dspcomponents
DSPCOMPONENTS_SRCS  = $(foreach name,$(DSPCOMPONENTS_NAME),$(DSPCOMPONENTS_DIR)/$(name)/$(name))
DSPCOMPONENTS_OBJS  = $(patsubst %,$(OBJDIR)/dsp_firm_%.o,$(DSPCOMPONENTS_NAME))

TARGET_LIB          = libdsp_g711$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS     = $(TARGETS)
INSTALL_DIR         = $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(DSPCOMPONENTS_OBJS) $(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

OBJS_M             += $(DSPCOMPONENTS_OBJS)

$(OBJDIR)/dsp_firm_audio.o : $(DSPCOMPONENTS_DIR)/audio/audio.a
	$(BIN2OBJ) --compatible -r -b DSPiFirmware_audio $(call empath,$<) $@ >/dev/null


#===== End of Makefile =====
