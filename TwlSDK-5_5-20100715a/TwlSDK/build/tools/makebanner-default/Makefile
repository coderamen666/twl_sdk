#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - makebanner-default
#  File:     Makefile
#
#  Copyright 2003-2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-09-18#$
#  $Rev: 8573 $
#  $Author: okubata_ryoma $
#---------------------------------------------------------------------------

TARGET_PLATFORM	 = NITRO

include $(TWLSDK_ROOT)/build/buildtools/commondefs

BANNER_ICON		 = default.bmp
BANNER_SPEC		 = default.bsf

TARGETS			 = $(BANNER_SPEC:.bsf=.bnr)
INSTALL_DIR		 = $(TWL_INSTALL_INCDIR)/nitro/specfiles
INSTALL_TARGETS	 = $(TARGETS)

BANNER_ICON_NAME = $(basename $(BANNER_ICON))

LDIRT_CLEAN		 = $(TARGETS) \
			 	   $(BANNER_ICON_NAME).nbfs \
			 	   $(BANNER_ICON_NAME).nbfc \
			 	   $(BANNER_ICON_NAME).nbfp \
			 	   $(TARGETS:.bnr=.srl)

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(BANNER_SPEC) $(BANNER_ICON_NAME).nbfc $(BANNER_ICON_NAME).nbfp
			$(MAKEBANNER) -N $(BANNER_ICON_NAME) $(BANNER_SPEC) $(TARGETS)

#----------------------------------------------------------------------------
#  test
#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/nitro/commondefs.emtype.IS

test:			$(TARGETS)
			$(ADDBANNER) $(TARGETS) $(TARGETS:.bnr=.srl)
ifdef	ISD_NITLOAD
			$(ISD_NITLOAD) $(TARGETS:.bnr=.srl)
endif

#
