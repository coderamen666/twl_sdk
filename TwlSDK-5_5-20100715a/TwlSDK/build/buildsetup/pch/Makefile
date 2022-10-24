#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - buildsetup - pch
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
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

TARGET_PLATFORM	 = TWL NITRO
NITRO_CODEGEN 	?= ALL
TWL_CODEGEN		?= ALL
TWL_ARCHGEN		?= ALL

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------
ifdef	MAKE_PCH_TWL_BUILD_TYPE

export TWL_BUILD_TYPE	 = $(MAKE_PCH_TWL_BUILD_TYPE)
SUBDIRS_P				 = $(GSUBDIRS)
override SKIP_SUBDIR	 =

else

do-build:
	@+$(MAKE) MAKE_PCH_TWL_BUILD_TYPE=DEBUG
	@+$(MAKE) MAKE_PCH_TWL_BUILD_TYPE=RELEASE
	@+$(MAKE) MAKE_PCH_TWL_BUILD_TYPE=FINALROM

endif

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
