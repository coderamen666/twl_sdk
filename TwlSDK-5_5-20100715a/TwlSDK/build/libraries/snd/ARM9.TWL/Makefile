#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - snd - ARM9.TWL
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

SUBDIRS          =
SUBMAKES         =   Makefile.sndex

#----------------------------------------------------------------------------
override TARGET_PLATFORM    =   TWL

# Build ARM & THUMB libraries
TWL_CODEGEN     ?=   ALL

# Codegen for main processor
TWL_PROC         =   ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?=   ALL

SRCDIR           =   ../common/src ../ARM9/src

SRCS             =   snd_interface.c \
                     snd_main.c      \
                     snd_command.c   \
                     snd_alarm.c     \
                     snd_work.c      \
                     snd_util.c      \
                     snd_bank.c      \
                     snd_data.c

TARGET_LIB       =   libsnd$(TWL_LIBSUFFIX).a

include $(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS  =   $(TARGETS)
INSTALL_DIR      =   $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:   $(TARGETS)

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
