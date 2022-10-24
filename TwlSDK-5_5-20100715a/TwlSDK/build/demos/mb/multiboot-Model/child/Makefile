#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB
# File:     Makefile
#
# Copyright 2004-2009 Nintendo. All rights reserved.
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

SUBDIRS			 =

#----------------------------------------------------------------------------
# You can distribute an SRL file for a child only if it is a NITRO or TWL-HYBRID build.
# The MB library distributes NITRO applications, so even on a TWL system, the distributed hybrid ROM will boot in NTR mode.
# 
#
# See the MB Library Overview in the Function Reference Manual for more information.
TARGET_PLATFORM	:=	ALL
TWL_ARCHGEN		:=	HYBRID

INCDIR			 =	../common/include ./include
SRCDIR			 =	./src ../common/src 

SRCS			 =	main.c		\
					font.c		\
					disp.c		\
					common.c	\
					gmain.c

SHARED_WH		:= TRUE

ifdef	SHARED_WH
# WH module was moved to wireless_shared

WH_DIR			 =	$(ROOT)/build/demos/wireless_shared/wh
SRCDIR			+=	$(WH_DIR)
INCDIR			+=	$(WH_DIR)
SRCS			+=	$(WH_DIR)/wh.c
MACRO_FLAGS		+= -DSHARED_WH

else
# The original source code has been retained as well

SRCS			+=	wmhigh.c

endif	#SHARED_WH


TARGET_BIN		 =	child.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#-------------------------------#
#   Install target		#
#-------------------------------#
INSTALL_TARGETS	 =	$(TARGETS)
INSTALL_DIR		 = ../data

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
