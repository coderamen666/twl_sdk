#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - GX - demos - UnitTours/2D_BmpBg_Vram
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	= TWL NITRO

SUBDIRS			=

#----------------------------------------------------------------------------

SRCS			=	main.c \
					image.c

PICTURES_DIR	=	../2D_BmpBg_MainRam/pictures
PICTURES		=	picture_256_192_a.ppm \
					picture_256_192_b.ppm \
					picture_256_192_c.ppm \
					picture_256_192_d.ppm

PCITURES_C_DIR	=	pictures
PICTURES_C		=	$(addprefix $(PCITURES_C_DIR)/,$(PICTURES:.ppm=.autogen.c))

TARGET_BIN		=	main.srl
LINCLUDES		=	$(PCITURES_C_DIR)

LDIRT_CLEAN		=	$(PICTURES_C)

#SRCDIR			=	# using default
#LCFILE			=	# using default

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

PPMCONV			=	$(TOOLSDIR)/bin/ppmconv.exe

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

$(PCITURES_C_DIR)/%.autogen.c:	$(PICTURES_DIR)/%.ppm
	install -d $(@D)
	$(PPMCONV) $< > $@

$(OBJDIR)/image.o: image.c $(PICTURES_C)

#===== End of Makefile =====
