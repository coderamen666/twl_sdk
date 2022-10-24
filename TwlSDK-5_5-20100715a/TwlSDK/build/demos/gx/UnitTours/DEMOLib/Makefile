#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
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
# $Date:: 2008-12-24#$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	= TWL NITRO

SUBDIRS			=

#----------------------------------------------------------------------------

SRCS			 =	DEMOKey.c		\
					DEMOInit.c		\
					DEMOUtility.c	\
					DEMOFont.c		\
					DEMOBitmap.c	\
					DEMOWave.c		\
					DEMOHostio.c

TARGET_LIB		 =	libDEMO$(TWL_LIBSUFFIX).a

# Create libraries for both HYBRID and LIMITED.
TWL_ARCHGEN		?=	ALL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifdef	TWLSDK_INSTALL_ROOT
INSTALL_TARGETS	 =	$(TARGETS)
INSTALL_DIR		 =	$(TWL_INSTALL_ROOT)/build/demos/gx/UnitTours/DEMOLib/lib/$(TWL_BUILDTYPE)
endif # TWLSDK_INSTALL_ROOT


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
