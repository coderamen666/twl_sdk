#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tips
# File:     Makefile
#
# Copyright 2003-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-11-10#$
# $Rev: 11137 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM	 = NITRO TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

SUBDIRS_P 	 = SwapCrossOver MultiLink MyBuildMessages NTRBanner DisplayLogo

# if (FINALROM or clean or clobber), go into BuildFinalRom
ifneq	(,$(filter FINALROM,$(TWL_BUILD_TYPE))$(findstring $(MAKECMDGOALS),clean clobber))
SUBDIRS_P	+= BuildFinalRom
endif

SUBDIRS_P 	+= $(PRIVATE_DEMOS_TIPS)

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
