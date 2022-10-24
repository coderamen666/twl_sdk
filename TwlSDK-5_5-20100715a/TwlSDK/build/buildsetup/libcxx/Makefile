#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - buildsetup - libcxx
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

TARGET_PLATFORM		= NITRO
NITRO_FORCE_BUILD	= True

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGET_SRC_LIBDIR	= $(CW_ARMDIR)/msl/MSL_C++/MSL_ARM/Lib
TARGET_SRC_LIBS		= MSL_C++_NITRO_A_LE.a			\
					  MSL_C++_NITRO_A_LE_strb.a		\
					  MSL_C++_NITRO_Ai_LE.a			\
					  MSL_C++_NITRO_Ai_LE_strb.a	\
					  MSL_C++_NITRO_T_LE.a			\
					  MSL_C++_NITRO_T_LE_strb.a

TMPDIR				= ./lib

# install
INSTALL_TARGETS		= $(addprefix $(TMPDIR)/,$(subst C++,CXX,$(TARGET_SRC_LIBS)))
INSTALL_DIR			= $(ROOT)/lib/CodeWarrior
LDIRT_CLEAN			= $(ROOT)/lib/CodeWarrior

#----------------------------------------------------------------------------

do-build:
	@$(MKDIRP) $(TMPDIR)
	@$(foreach FILE,$(TARGET_SRC_LIBS),\
		$(CP) $(TARGET_SRC_LIBDIR)/$(FILE) \
			$(TMPDIR)/$(subst _C++_,_CXX_,$(FILE)) $(AND)) true

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----- End of Makefile -----
