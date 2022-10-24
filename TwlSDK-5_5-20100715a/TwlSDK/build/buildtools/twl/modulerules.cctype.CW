#! make -f
#----------------------------------------------------------------------------
#  Project:  TwlSDK
#  File:     modulerules.cctype.CW
#
#  Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-10-20#$
# $Rev: 11102 $
# $Author: mizutani_nakaba $
#----------------------------------------------------------------------------

.PRECIOUS:	$(OBJDIR)/%.o $(BINDIR)/%.$(TWL_ELF_EXT) $(LCFILE) $(PCHDR_H) $(PCHDR_PCH)

# .c
$(OBJS_CC):%.o:
ifdef SILENT
	echo $(TWLSDK_MSG_COMPILE) $(notdir $<)
endif
	$(CC) $(CCFLAGS) $(INCLUDES) $(PCHDR_CCFLAGS) -c $(call empath,$<) -MD -o $(dir $(call empath,$@))
	@$(MOVE_SRC_DEPEND)

# .cpp
$(OBJS_CXX):%.o:
ifdef SILENT
	echo $(TWLSDK_MSG_COMPILE) $(notdir $<)
endif
	$(CC) $(CXXFLAGS) $(INCLUDES) $(PCHDR_CXXFLAGS) -c $(call empath,$<) -MD -o $(dir $(call empath,$@))
	@$(MOVE_SRC_DEPEND)

# .s
$(OBJS_AS):%.o:
ifdef SILENT
	echo $(TWLSDK_MSG_ASSEMBLE) $(notdir $<)
endif
	$(AS) $(ASFLAGS) $(INCLUDES) -c $(call empath,$<) -MD -o $(dir $(call empath,$@))
	@$(MOVE_SRC_DEPEND)

# .lcf .par
ifdef	LCFILE_SPEC

ADDRESS_STATIC		?= $(DEFAULT_ADDRESS_STATIC)
ADDRESS_ITCM		?= $(DEFAULT_ADDRESS_ITCM)
ADDRESS_DTCM		?= $(DEFAULT_ADDRESS_DTCM)

# Exporting
GLIBRARIES_LINK		 = $(filter-out $(LIBRARIES_UNLINK) $(GLIBRARIES_EX),$(ELIBRARIES) $(GLIBRARIES))
GLIBRARIES_EX_LINK	 =	$(filter-out $(LIBRARIES_UNLINK),$(GLIBRARIES_EX))
LLIBRARIES_LINK		 = $(filter-out $(LIBRARIES_UNLINK) $(LLIBRARIES_EX),$(LLIBRARIES))
LLIBRARIES_EX_LINK	 =	$(filter-out $(LIBRARIES_UNLINK),$(LLIBRARIES_EX))

# Convert Unix and Windows format paths
OBJS_STATIC_M		:= $(if $(filter-out 	,$(OBJS_STATIC)), $(call cygpathm_multi,$(OBJS_STATIC)))
OBJS_AUTOLOAD_M		:= $(if $(filter-out 	,$(OBJS_AUTOLOAD)),$(call cygpathm_multi,$(OBJS_AUTOLOAD)))
OBJS_OVERLAY_M		:= $(if $(filter-out 	,$(OBJS_OVERLAY)),$(call cygpathm_multi,$(OBJS_OVERLAY)))
OBJS_LTDAUTOLOAD_M	:= $(if $(filter-out 	,$(OBJS_LTDAUTOLOAD)),$(call cygpathm_multi,$(OBJS_LTDAUTOLOAD)))
OBJS_LTDOVERLAY_M	:= $(if $(filter-out 	,$(OBJS_LTDOVERLAY)),$(call cygpathm_multi,$(OBJS_LTDOVERLAY)))
OBJDIR_M			:= $(call empath,$(OBJDIR))
LIBDIR_M			:= $(call empath,$(LIBDIR))
BINDIR_M			:= $(call empath,$(BINDIR))
LCFILE_TEMPLATE_M	:= $(call empath,$(LCFILE_TEMPLATE))
LDRES_TEMPLATE_M	:= $(call empath,$(LDRES_TEMPLATE))
LDRES_FILE_M		:= $(call empath,$(LDRES_FILE))
LCFILE_M			:= $(call empath,$(LCFILE))
OBJS_M				 = $(if $(filter-out 	,$(OBJS)),$(call cygpathm_multi,$(OBJS)))

# CODEGEN and LIBSUFFIX are set to TWL_LIBSUFFIX because both variables are currently used to determine whether to link the ARM or Thumb library in the LSF file
# 
MAKELCF_DEFS	= -DTWLSDK_ROOT='$(ROOT_M)'									\
		  -DTARGET_NAME='$(TARGET_NAME)'									\
		  -DOBJS_STATIC='$(foreach OBJ,$(OBJS_STATIC_M),$(OBJ))'			\
		  -DOBJS_AUTOLOAD='$(foreach OBJ,$(OBJS_AUTOLOAD_M),$(OBJ))'		\
		  -DOBJS_OVERLAY='$(foreach OBJ,$(OBJS_OVERLAY_M),$(OBJ))'			\
		  -DOBJS_LTDAUTOLOAD='$(foreach OBJ,$(OBJS_LTDAUTOLOAD_M),$(OBJ))'	\
		  -DOBJS_LTDOVERLAY='$(foreach OBJ,$(OBJS_LTDOVERLAY_M),$(OBJ))'	\
		  -DGLIBS='$(GLIBRARIES_LINK)'										\
		  -DGLIBS_EX='$(GLIBRARIES_EX_LINK)'								\
		  -DLLIBS='$(LLIBRARIES_LINK)'										\
		  -DLLIBS_EX='$(LLIBRARIES_EX_LINK)'								\
		  -DCC_LIBS='$(CC_LIBS)'											\
		  -DCW_LIBS='$(CC_LIBS)'											\
		  -DOBJDIR='$(OBJDIR_M)'											\
		  -DLIBDIR='$(LIBDIR_M)'											\
		  -DBINDIR='$(BINDIR_M)'											\
		  -DCRT0_O='$(CRT0_O)'												\
		  -DPROC='$(CODEGEN_PROC)'											\
		  -DPLATFORM='$(TWL_PLATFORM)'										\
		  -DCODEGEN='$(TWL_LIBSUFFIX)'										\
		  -DARCHGEN='$(ARCHGEN_TYPE)'										\
		  -DLIBSUFFIX='$(TWL_LIBSUFFIX)'									\
		  -DBUILD='$(TWL_BUILD_DIR)'										\
		  -DADDRESS_STATIC='$(ADDRESS_STATIC)'								\
		  -DADDRESS_ITCM='$(ADDRESS_ITCM)'									\
		  -DADDRESS_DTCM='$(ADDRESS_DTCM)'

MAKELCF_FORMAT	?= -V$(CW_MAJOR_VER)

MAKELCF_OPTS	 = $(MAKELCF_FLAGS) $(MAKELCF_DEFS) $(MAKELCF_FORMAT)

$(LCFILE_AUTOGEN): $(LCFILE_SPEC) $(LCFILE_TEMPLATE) $(MAKEFILE) $(TWL_BUILDTOOLSDIR)/commondefs
ifdef SILENT
	echo $(TWLSDK_MSG_MAKE_LCF) $(notdir $@)
endif
	$(MAKELCF) $(MAKELCF_OPTS)  $(call empath,$<) $(LCFILE_TEMPLATE_M) $(call empath,$@)

$(LDRES_AUTOGEN):  $(LCFILE_SPEC) $(LDRES_TEMPLATE)  $(MAKEFILE) $(TWL_BUILDTOOLSDIR)/commondefs
ifdef SILENT
	echo $(TWLSDK_MSG_MAKE_LCF) $(notdir $@)
endif
	@$(MAKELCF) $(MAKELCF_OPTS)  $(call empath,$<) $(LDRES_TEMPLATE_M)  $(call empath,$@)

endif

#----------------------------------------------------------------------------
# Compress/digest

ifeq    ($(CODEGEN_PROC),ARM9)
# Always add digests into binary files if ARM9
#ifdef	TWL_DIGEST
COMPSTATIC_CMDS    +=   -a
#endif
ifdef   TWL_COMPRESS
    COMPSTATIC_LTDSTATIC_FLAG   ?=  -S0
    COMPSTATIC_SPEC_FILE        ?=  $(BINDIR)/compspec.list
    COMPSTATIC_CMDS             +=  -9 -c -C $(COMPSTATIC_LTDSTATIC_FLAG)
endif
endif

ifdef	COMPSTATIC_CMDS
COMPSUFFIX				?= $(DEFAULT_COMPSUFFIX)
ifneq	($(COMPSUFFIX),)
COMPSTATIC_FLAGS	+= -e$(COMPSUFFIX)
endif
ifneq	($(COMPSTATIC_SPEC_FILE),)
COMPSTATIC_FLAGS	+= -l$(COMPSTATIC_SPEC_FILE)
endif
endif


# libcxx workaround
ifdef	SDK_CW_WA_LIBCXX
CW_LIBCXX	= $(CW_LIBCXX_DIR)/$(CW_LIBCXX_ARM)
$(CW_LIBCXX):	$(ROOT)/build/buildsetup/libcxx/Makefile
	+$(MAKE) -f $<
endif


# .$(TWL_ELF_EXT)
ifndef	USE_MY_RULE_NEF
ifneq	($(TWL_MAKEROM),)
$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT): $(OBJS) $(LCFILE) $(MAKEFILE) $(LDEPENDS_NEF) $(EDEPENDS_NEF) $(ALIBRARIES_DEPEND) $(LDRES_FILE) $(CW_LIBCXX)
ifdef SILENT
	echo $(TWLSDK_MSG_LINK) $(notdir $@)
endif
	$(LD) $(LDFLAGS) $(LIBRARY_DIRS) @$(LDRES_FILE_M) $(LCFILE_M) -o $(call empath,$@)
ifdef	COMPSTATIC_CMDS
	$(COMPSTATIC) $(COMPSTATIC_CMDS) $(COMPSTATIC_FLAGS) -f $(BINDIR)/component.files $(STDOUT)
endif
else
$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT): $(OBJS) $(LCFILE) $(MAKEFILE) $(LDEPENDS_NEF) $(EDEPENDS_NEF) $(ALIBRARIES_DEPEND) $(CW_LIBCXX)
ifdef SILENT
	echo $(TWLSDK_MSG_LINK) $(notdir $@)
endif
	$(LD) $(LDFLAGS) $(LIBRARY_DIRS) $(foreach OBJ,$(OBJS_M),$(OBJ)) $(LIBRARIES) $(LCFILE_M) -o $(call empath,$@)
endif
endif

# .a
ifndef	USE_MY_RULE_A
$(LIBDIR)/$(TARGET_LIB): $(OBJS)
ifdef SILENT
	echo $(TWLSDK_MSG_ARCHIVE) $(notdir $@)
endif
	$(AR) $(ARFLAGS) $(foreach OBJ,$(OBJS_M),$(OBJ)) -o $(call empath,$@)
endif

#----- End of modulerules.cctype.CW -----
