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
# $Date:: 2008-09-26#$
# $Rev: 8688 $
# $Author: sato_masaki $
#----------------------------------------------------------------------------

override TARGET_PLATFORM = TWL

# Codegen for subprocessor
TWL_PROC		 =  ARM9
TWL_CODEGEN		?= ALL

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

# Directory for firmware transferred with LoadDeviceEx
TWLWIRELESS_FW_DIR = ../../../../../../add-ins/TwlWireless/$(TWLWIRELESS_PACKAGE)/binfile

SRCDIR			 = ./src ../../common.TWL/src
INCDIR			 = ./inc ../../inc ../../ARM9/inc ../../common.TWL/inc ../common/inc

SRCS			 = 	nwm_init.c			\
					nwm_system.c		\
					nwm_end.c			\
					nwm_loaddev.c		\
					nwm_unloaddev.c		\
					nwm_open.c			\
					nwm_close.c			\
					nwm_startscan.c		\
					nwm_connect.c		\
					nwm_disconnect.c	\
					nwm_setwepkey.c		\
					nwm_setpowersave.c	\
					nwm_sendframe.c		\
					nwm_recvframe.c		\
					nwm_wpa.c			\
					nwm_qos.c			\
					nwm_cmd.c			\
					nwm_reset.c			\
					nwm_passphrase.c	\
					nwm_installfirm.c	

ifdef NWM_INCLUDE_FIRM
EXT_OBJS         = $(OBJDIR)/nwm_firm.o
endif

TARGET_LIB		 = libnwm$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)

INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)

GDIRT_INSTALLED	 = $(addprefix $(INSTALL_DIR)/,$(notdir $(INSTALL_TARGETS)))

#----------------------------------------------------------------------------
# New wireless debugging flags and so on

ifdef	NWM_DEBUG
CCFLAGS +=	-DNWM_DEBUG
endif

# Include wireless firmware with the library
ifdef NWM_INCLUDE_FIRM
CCFLAGS += -DNWM_INCLUDE_FIRM
endif

CCFLAGS += -DENABLE_RXACK

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef NWM_INCLUDE_FIRM
$(OBJDIR)/nwm_firm.o:	 $(TWLWIRELESS_FW_DIR)/nwm_firm.bin
			$(BIN2OBJ) $< $@
endif

#===== End of Makefile =====
