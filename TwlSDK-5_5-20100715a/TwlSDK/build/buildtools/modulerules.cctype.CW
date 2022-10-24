AINCLUDE_DIRS	 = $(LINCLUDES)	   $(EINCLUDES)     $(GINCLUDES)
ALIBRARY_DIRS	 = $(LLIBRARY_DIRS) $(ELIBRARY_DIRS) $(GLIBRARY_DIRS)
ALIBRARIES		 = $(LLIBRARIES)	   $(ELIBRARIES)    $(GLIBRARIES)

# ISDBG_LIBS is in a location that cannot be searched by vpath
ALIBRARIES_DEPEND = $(filter-out $(ISDBG_LIBS),$(ALIBRARIES))

# Convert Unix and Windows format paths
AINCLUDE_DIRS_M	:= $(if $(filter-out 	,$(AINCLUDE_DIRS)),$(call cygpathm_multi,$(AINCLUDE_DIRS)))
ALIBRARY_DIRS_M	:= $(if $(filter-out 	,$(ALIBRARY_DIRS)),$(call cygpathm_multi,$(ALIBRARY_DIRS)))
ALIBRARY_DIRS_U	:= $(if $(filter-out 	,$(ALIBRARY_DIRS)),$(call cygpathu_multi,$(ALIBRARY_DIRS)))

# 
# VPATH does not support the path name included white space like "C:/Program Files"
#	Please, be careful !!!
#
ifdef	MSYSTEM
ALIBVPATH		 = $(subst \:,\ ,$(subst $(SPACE),:,$(ALIBRARY_DIRS)))
else
ALIBVPATH		 = $(foreach DIR,$(ALIBRARY_DIRS_U),$(call decode_specialchar,$(DIR)))
endif

vpath	%.a		$(ALIBVPATH)
vpath	%.o		$(ALIBVPATH)

INCLUDE_DIRS	 = $(call decode_specialchar,$(foreach DIR,$(AINCLUDE_DIRS_M),-I'$(DIR)'))
LIBRARY_DIRS	 = $(call decode_specialchar,$(foreach DIR,$(ALIBRARY_DIRS_M),-L'$(DIR)'))
LIBRARIES		 = $(subst \ -l,\ ,$(addprefix -l,$(ALIBRARIES)))
INCLUDES		 = -gccinc -I. $(INCLUDE_DIRS)

#
# Generate dependency files
#

ifdef	MSYSTEM
DEPEND_RULE		 = $(QUOTE)/ARM_EABI_Support/d;s/\\x0D//g;s/\\([^ ]\\)\\\\\([^ ]\\)/\\1\\/\\2/g;$(QUOTE)
else
DEPEND_RULE		 = $(QUOTE)/ARM_EABI_Support/d;s/\\x0D//g;s!\\([A-Za-z]\\):[\\\\\/]!$(CYGPATH_MOUNTPOINT)\\/\\l\\1\\/!g;s/\\([^ ]\\)\\\\\([^ ]\\)/\\1\\/\\2/g;$(QUOTE)
endif

ifneq ($(MAKEFLAGS_ALWAYS),TRUE)
define MOVE_SRC_DEPEND
	$(SED) -e $(DEPEND_RULE) $(dir $@)/$(*F).d > $(DEPENDDIR)/$(*F).d && $(RM) $(dir $@)/$(*F).d
endef
else
define MOVE_SRC_DEPEND
$(EMPTY)
endef
endif


#
# Precompiled header
#

#----------------------------------------
# if USE_STD_PCHDR == TRUE
ifeq ($(USE_STD_PCHDR),TRUE)

.PHONY:	sdk-mch

PCHDR_C_DEFS	 = $(sort $(filter -DSDK_%,$(CCFLAGS)))
PCHDR_C_NODEF	 = $(filter-out -D%,$(CCFLAGS))
PCHDR_C_NAME	 = $(subst $(SPACE),,$(subst -DSDK_,.,$(subst =,-,$(PCHDR_C_DEFS))))
PCHDR_C_NAME    := $(subst TWLHYB,HYB,$(subst TWLLTD,LTD,$(PCHDR_C_NAME)))
PCHDR_C_NAME    := $(subst ARM9,A9,$(subst ARM7,A7,$(PCHDR_C_NAME)))
PCHDR_C_NAME    := $(subst RELEASE,REL,$(subst DEBUG,DBG,$(subst FINALROM,ROM,$(PCHDR_C_NAME))))
PCHDR_C_NAME    := $(subst CW_FORCE_EXPORT_SUPPORT,CWFES,$(subst CW_MAJOR_VER,CWVER,$(PCHDR_C_NAME)))
PCHDR_C_NAME    := $(subst CODE_ARM,ARM,$(subst CODE_THUMB,THUMB,$(PCHDR_C_NAME)))
PCHDR_C_NAME    := $(subst PROFILE_CALLTRACE,PRCT,$(subst PROFILE_FUNCTIONCOST,PRFC,$(PCHDR_C_NAME)))
PCHDR_C_NAME    := $(subst LINK_ISTD,ISTD,$(subst NITRO,NTR,$(PCHDR_C_NAME)))
PCHDR_C_BASE	 = $(PCHDR_CACHE)/$(notdir $(PCHDR_SRC))$(PCHDR_C_NAME).C
PCHDR_C_MCH		 = $(PCHDR_C_BASE).mch
PCHDR_C_DEPEND	 = $(PCHDR_C_BASE).depend
PCHDR_CCFLAGS	 = -include $(call empath,$(PCHDR_C_MCH))
CCFLAGS_NOLDEF	 = $(PCHDR_C_DEFS) $(PCHDR_C_NODEF) 

PCHDR_CXX_DEFS	 = $(sort $(filter -DSDK_%,$(CXXFLAGS)))
PCHDR_CXX_NODEF	 = $(filter-out -D%,$(CXXFLAGS))
PCHDR_CXX_NAME	 = $(subst $(SPACE),,$(subst -DSDK_,.,$(subst =,-,$(PCHDR_CXX_DEFS))))
PCHDR_CXX_NAME  := $(subst TWLHYB,HYB,$(subst TWLLTD,LTD,$(PCHDR_CXX_NAME)))
PCHDR_CXX_NAME  := $(subst ARM9,A9,$(subst ARM7,A7,$(PCHDR_CXX_NAME)))
PCHDR_CXX_NAME  := $(subst RELEASE,REL,$(subst DEBUG,DBG,$(subst FINALROM,ROM,$(PCHDR_CXX_NAME))))
PCHDR_CXX_NAME  := $(subst CW_FORCE_EXPORT_SUPPORT,CWFES,$(subst CW_MAJOR_VER,CWVER,$(PCHDR_CXX_NAME)))
PCHDR_CXX_NAME  := $(subst CODE_ARM,ARM,$(subst CODE_THUMB,THUMB,$(PCHDR_CXX_NAME)))
PCHDR_CXX_NAME  := $(subst PROFILE_CALLTRACE,PRCT,$(subst PROFILE_FUNCTIONCOST,PRFC,$(PCHDR_CXX_NAME)))
PCHDR_CXX_NAME  := $(subst LINK_ISTD,ISTD,$(subst NITRO,NTR,$(PCHDR_CXX_NAME)))
PCHDR_CXX_BASE	 = $(PCHDR_CACHE)/$(notdir $(PCHDR_SRC))$(PCHDR_CXX_NAME).CXX
PCHDR_CXX_MCH	 = $(PCHDR_CXX_BASE).mch
PCHDR_CXX_DEPEND = $(PCHDR_CXX_BASE).depend
PCHDR_CXXFLAGS	 = -include $(call empath,$(PCHDR_CXX_MCH))
CXXFLAGS_NOLDEF	 = $(PCHDR_CXX_DEFS) $(PCHDR_CXX_NODEF) 


#----------------------------------------
# if MAKECMDGOALS contains build_and_install or sdk-mch
ifneq	($(filter build_and_install sdk-mch,$(MAKECMDGOALS)),)

SYSTEM_GINCLUDES_M	 = $(if $(filter-out 	,$(SYSTEM_GINCLUDES)),$(call cygpathm_multi,$(SYSTEM_GINCLUDES)))
SYSTEM_INCLUDE_DIRS	 = $(call decode_specialchar,$(foreach DIR,$(SYSTEM_GINCLUDES_M),-I'$(DIR)'))
SYSTEM_INCLUDES		 = -gccinc -I. $(SYSTEM_INCLUDE_DIRS)


## $(call BUILD_PCHDR,$(MCH))
ifdef SILENT
define BUILD_PCHDR_ECHO
	echo $(TWLSDK_MSG_MAKE_MCH) $(1)
endef
else
define BUILD_PCHDR_ECHO
	true
endef
endif

.DELETE_ON_ERROR: $(PCHDR_CACHE)/%_lock

## $(call BUILD_PCHDR,$(BASE),$(CCFLAGS))
# $(BASE) must be a full path
define BUILD_PCHDR
	$(call BUILD_PCHDR_ECHO,$(notdir $(1).mch)) &&	\
	$(CP) $(PCHDR_SRC) $(1).h &&			\
	($(CD) $(dir $(1).mch) &&				\
	 $(RM) $(1).mch	&&						\
	 $(RM) $(1).depend &&					\
	 $(CC) $(subst -include ,-DSDK_PCHDR_DUMMY_,$(2)) $(SYSTEM_INCLUDES) -MD -precompile $(notdir $(1).m) $(call empath,$(1).h)) && \
	 $(MV) $(1).m $(1).mch &&				\
	(($(ECHO) $(1).mch: $(PCHDR_SRC) \\; $(SED) -e '1d' -e $(DEPEND_RULE) $(1).d) > $(1).dep \
	  && $(MV) $(1).dep $(1).depend			\
	  && $(RM) $(1).d $(1).h)
endef

## $(call J_SAFE_BUILD_PCHDR,$(BASE),$(CCFLAGS),CCFLAGS)
define J_SAFE_BUILD_PCHDR
	trap 'rm -rf $(1)_lock 2> /dev/null ; exit 1' 1 2 3 15; \
	until mkdir $(1)_lock 2> /dev/null; do					\
		sleep 1;											\
	done;													\
	$(REMAKE) sdk-mch $(3)="$(subst ",\",$(2))" $(4)=TRUE;	\
	rmdir $(1)_lock;										\
	trap 1 2 3 15
endef


# precompiled header for C

#----------------------------------------
# if OBJS_CC != ""
ifneq	($(strip $(OBJS_CC)),)

-include $(PCHDR_C_DEPEND)

$(PCHDR_C_DEPEND): $(PCHDR_C_MCH)
sdk-mch: $(PCHDR_C_MCH)

$(PCHDR_C_MCH):
ifneq	($(filter sdk-mch,$(MAKECMDGOALS)),)
ifeq	($(PCHDR_C_FLAG),TRUE)
	$(call BUILD_PCHDR,$(PCHDR_C_BASE),$(CCFLAGS_NOLDEF))
endif
else
	+$(if $(findstring j,$(MAKEFLAGS)),														\
		$(call J_SAFE_BUILD_PCHDR,$(PCHDR_C_BASE),$(CCFLAGS_NOLDEF),CCFLAGS,PCHDR_C_FLAG),	\
		$(call BUILD_PCHDR,$(PCHDR_C_BASE),$(CCFLAGS_NOLDEF)) )
endif

endif
# endif OBJS_CC != ""
#----------------------------------------


# precompiled header for C++

#----------------------------------------
# if OBJS_CXX != ""
ifneq	($(strip $(OBJS_CXX)),)

-include $(PCHDR_CXX_DEPEND)

$(PCHDR_CXX_DEPEND): $(PCHDR_CXX_MCH)
sdk-mch: $(PCHDR_CXX_MCH)

$(PCHDR_CXX_MCH):
ifneq	($(filter sdk-mch,$(MAKECMDGOALS)),)
ifeq	($(PCHDR_CXX_FLAG),TRUE)
	$(call BUILD_PCHDR,$(PCHDR_CXX_BASE),$(CXXFLAGS_NOLDEF))
endif
else
	+$(if $(findstring j,$(MAKEFLAGS)),																\
		$(call J_SAFE_BUILD_PCHDR,$(PCHDR_CXX_BASE),$(CXXFLAGS_NOLDEF),CXXFLAGS,PCHDR_CXX_FLAG),	\
		$(call BUILD_PCHDR,$(PCHDR_CXX_BASE),$(CXXFLAGS_NOLDEF)) )
endif

endif
# endif OBJS_CXX != ""
#----------------------------------------

endif
# endif MAKECMDGOALS contain build_and_install or sdk-mch
#----------------------------------------

endif
# endif USE_STD_PCHDR == TRUE
#----------------------------------------



#----------------------------------------------------------------------------
# Get SDK_CW_BUILD_VERSION SDK_CW_BUILD_NUMBER
#
# IRIS	V0.2 Hotfix 1
#	SDK_CW_BUILD_VERSION_CC=2.0
#	SDK_CW_BUILD_VERSION_LD=2.0
#	SDK_CW_BUILD_NUMBER_CC=0038
#	SDK_CW_BUILD_NUMBER_LD=0050
#
# NITRO V0.3
#	SDK_CW_BUILD_NUMBER_CC=0038
#	SDK_CW_BUILD_NUMBER_LD=0056
#
# NITRO V0.4.1
#	SDK_CW_BUILD_NUMBER_CC=0043
#	SDK_CW_BUILD_NUMBER_LD=0057
#
# NITRO V0.5
#	SDK_CW_BUILD_NUMBER_CC=0046
#	SDK_CW_BUILD_NUMBER_LD=0058
#
# NITRO V0.5 + HotFix1
#	SDK_CW_BUILD_NUMBER_CC=0049
#	SDK_CW_BUILD_NUMBER_LD=0061
#
# NITRO V0.6
#	SDK_CW_BUILD_NUMBER_CC=0050
#	SDK_CW_BUILD_NUMBER_LD=0062
#
# NITRO V0.6.1
#	SDK_CW_BUILD_NUMBER_CC=0050
#	SDK_CW_BUILD_NUMBER_LD=0063
#
# NITRO V0.7
#	SDK_CW_BUILD_NUMBER_CC=0050
#	SDK_CW_BUILD_NUMBER_LD=0063
#
# NITRO V1.0
#	SDK_CW_BUILD_NUMBER_CC=0056
#	SDK_CW_BUILD_NUMBER_LD=0067
#
# NITRO V1.0.2
#	SDK_CW_BUILD_NUMBER_CC=0068
#	SDK_CW_BUILD_NUMBER_LD=0068
#
# NITRO V1.2
#	SDK_CW_BUILD_NUMBER_CC=0073
#	SDK_CW_BUILD_NUMBER_LD=0072
#
# NITRO V1.2SP1
#	SDK_CW_BUILD_NUMBER_CC=0074
#	SDK_CW_BUILD_NUMBER_LD=0072
#
# NITRO V1.2SP2+patch
#	SDK_CW_BUILD_NUMBER_CC=0080
#	SDK_CW_BUILD_NUMBER_LD=0074
#
# NITRO V1.2SP3
#	SDK_CW_BUILD_NUMBER_CC=0084
#	SDK_CW_BUILD_NUMBER_LD=0077
#
# NITRO V1.2SP4
#	SDK_CW_BUILD_NUMBER_CC=0087
#	SDK_CW_BUILD_NUMBER_LD=7702
#
# NITRO V2.0beta
#	SDK_CW_BUILD_VERSION_CC=3.0
#	SDK_CW_BUILD_VERSION_LD=2.0
#	SDK_CW_BUILD_NUMBER_CC=0107
#	SDK_CW_BUILD_NUMBER_LD=0077
#
# NITRO V2.0
#	SDK_CW_BUILD_NUMBER_CC=0114
#	SDK_CW_BUILD_NUMBER_LD=0082
#


ifneq	($(findstring $(MAKECMDGOALS),build_and_install make-verinfo clean-verinfo),)

.PHONY: make-verinfo
make-verinfo: $(SDK_VERINFO_CW_CC) $(SDK_VERINFO_CW_LD)

$(SDK_VERINFO_CW_CC): $(MWCC)
ifdef SILENT
	echo $(TWLSDK_MSG_VERSION) $@
endif
	@$(CC) -version | \
	 $(SED) -e $(QUOTE)/^Version /!d;s/\(build [0-9]\{2,4\}\) /\100 /;s/^Version \([0-9.]*\).*build \([0-9]*\).*(build \([0-9]*\)).*/SDK_CW_BUILD_VERSION_CC=\1\nSDK_CW_BUILD_NUMBER_CC=\3\nSDK_CW_BUILD_NUMBER2_CC=\2/i;$(QUOTE) >$(SDK_VERINFO_CW_CC)
	@$(TOUCH) -r $(CC) $(SDK_VERINFO_CW_CC)

$(SDK_VERINFO_CW_LD): $(MWLD)
ifdef SILENT
	echo $(TWLSDK_MSG_VERSION) $@
endif
	@$(LD) -version | \
	 $(SED) -e $(QUOTE)/^Version /!d;s/\(build [0-9]\{2,4\}\) /\100 /;s/^Version \([0-9.]*\).*build \([0-9]*\).*(build \([0-9]*\)).*/SDK_CW_BUILD_VERSION_LD=\1\nSDK_CW_BUILD_NUMBER_LD=\3\nSDK_CW_BUILD_NUMBER2_LD=\2/i;$(QUOTE) >$(SDK_VERINFO_CW_LD)
	@$(TOUCH) -r $(LD) $(SDK_VERINFO_CW_LD)

endif

.PHONY: clean-verinfo
clean-verinfo:
	@$(RM) $(SDK_VERINFO_CW_CC) $(SDK_VERINFO_CW_LD)


