#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - WM - libraries
# File:     Makefile
#
# Copyright 2003-2008 Nintendo. All rights reserved.
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
#SUBDIRS		 = wm_lib

override TARGET_PLATFORM = TWL

# Codegen for subprocessor
TWL_PROC		 =  ARM9
TWL_CODEGEN		?= ALL

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR			 = ../../ARM9/src ../../common.TWL/src ../common/src
INCDIR			 = ../../inc ../../ARM9/inc ../../common.TWL/inc ../nwm/inc ../common/inc

SRCS			 = 	wm_system.c		\
					wm_sync.c		\
					wm_standard.c	\
					wm_mp.c			\
					wm_dcf.c		\
					wm_ds.c			\
					wm_ks.c			\
					wm_etc.c		\
					wm_common.c

TARGET_LIB		 = libwm$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)

INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)

GDIRT_INSTALLED	 = $(addprefix $(INSTALL_DIR)/,$(notdir $(INSTALL_TARGETS)))

#----------------------------------------------------------------------------
# New wireless debugging flags and so on

ifdef	WM_W_DEBUG
CCFLAGS +=	-DWM_W_DEBUG
endif

ifdef	SEND_WRAM
CCFLAGS += -DSEND_WRAM
endif

#ifdef	RECEIVE_WRAM
#CCFLAGS += -DRECEIVE_WRAM
#endif

#CCFLAGS += -DENABLE_RXACK

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
