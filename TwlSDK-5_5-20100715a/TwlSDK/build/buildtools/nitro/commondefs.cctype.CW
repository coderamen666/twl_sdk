#! make -f
#----------------------------------------------------------------------------
#  Project:  TwlSDK
#  File:     commondefs.cctype.CW
#
#  Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-11-26#$
# $Rev: 11201 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

NITRO_STD_PCHDR	?= True
PCHDR_SRC		?= $(ROOT)/include/nitro.h

#----------------------------------------------------------------------------
#CWFOLDER_IRIS	?= D:/Program Files/Freescale/CW for NINTENDO DS V2.0
#CWFOLDER_NITRO	?= $(CWFOLDER_IRIS)
CWFOLDER_NITRO	?= $(CWFOLDER_TWL)
LM_LICENSE_FILE	:= $(subst /,\,$(CWFOLDER_NITRO))\license.dat
CW_ROOT			:= $(subst $(SPACE),\ ,$(subst \,/,$(CWFOLDER_NITRO)))
CW_ROOT_ARM		:= $(subst $(OPEN_PAREN),\$(OPEN_PAREN),$(subst $(CLOSE_PAREN),\$(CLOSE_PAREN),$(subst $(SPACE),\ ,$(subst \,/,$(CWFOLDER_NITRO)))))
CW_BINDIR		:= $(call eupath,$(CW_ROOT))/ARM_Tools/Command_Line_Tools
CW_ARMDIR		:= $(CW_ROOT_ARM)/ARM_EABI_Support

#----------------------------------------------------------------------------
SDK_VERINFO_CW_CC	?= $(NITRO_BUILDTOOLSDIR)/verinfo.cw.cc
SDK_VERINFO_CW_LD	?= $(NITRO_BUILDTOOLSDIR)/verinfo.cw.ld
SDK_VERINFO_CC		:= $(SDK_VERINFO_CW_CC)
SDK_VERINFO_LD		:= $(SDK_VERINFO_CW_LD)

ifneq	($(filter build_and_install sdk-mch,$(MAKECMDGOALS)),)

-include $(SDK_VERINFO_CW_CC) $(SDK_VERINFO_CW_LD)
#--------Support for CW2.0beta2 and later versions
ifndef	SDK_CW_BUILD_VERSION_CC
#--------If SDK_CW_BUILD_VERSION_CC is not defined, use make to create verinfo
MAKE_VERINFO	:= $(shell $(REMAKE) make-verinfo)
-include $(SDK_VERINFO_CW_CC) $(SDK_VERINFO_CW_LD)
endif

SDK_BUILD_VERSION_CC	:= $(SDK_CW_BUILD_VERSION_CC)
SDK_BUILD_NUMBER_CC		:= $(SDK_CW_BUILD_NUMBER_CC)
SDK_BUILD_NUMBER2_CC	:= $(SDK_CW_BUILD_NUMBER2_CC)
SDK_BUILD_VERSION_LD	:= $(SDK_CW_BUILD_VERSION_LD)
SDK_BUILD_NUMBER_LD		:= $(SDK_CW_BUILD_NUMBER_LD)
SDK_BUILD_NUMBER2_LD	:= $(SDK_CW_BUILD_NUMBER2_LD)

ifeq		($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" ">=" 4.0),1)
CW_MAJOR_VER	?= 3
endif
ifeq		($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" "==" 3.0),1)
CW_MAJOR_VER	?= 2
endif
# "__declspec(force_export)" support
ifeq		($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" ">=" 3.0),1)
SDK_CW_FORCE_EXPORT_SUPPORT	:= yes
endif
endif
CW_MAJOR_VER	?= 1

######## IRIS V0.2 Hotfix 1
######## NITRO V0.3
#CW_AVOID_STRB		:= -avoid_strb all,noerr
#
######## NITRO V0.4 or other
CW_AVOID_STRB		:= -avoid_byte strb -warn_byte none
#CW_AVOID_STRB		:= -avoid_byte strb -warn_byte all
#

#----------------------------------------------------------------------------

MWCC		:= $(CW_BINDIR)/mwccarm.exe
MWAS		:= $(CW_BINDIR)/mwasmarm.exe
MWLD		:= $(CW_BINDIR)/mwldarm.exe
MWAR		:= $(MWLD)

CC			:= "$(call delete_space_escape,$(MWCC))"
AS			:= "$(call delete_space_escape,$(MWAS))"
LD			:= "$(call delete_space_escape,$(MWLD))"
AR			:= "$(call delete_space_escape,$(MWAR))"

ELFTOBIN			:= $(CW_BINDIR)/elftobin.exe
ELFTOBIN_TYPE		 = $(NITRO_PLATFORM)$(CODEGEN_ARCH)/$(NITRO_BUILD_DIR)
ELFTOBIN_ARM9		?= $(NITRO_COMPONENTSDIR)/printserver/$(NITRO_BUILDTYPE_ARM9)/printserver.$(NITRO_ELF_EXT)
ELFTOBIN_ARM7		?= $(NITRO_COMPONENTSDIR)/ferret/$(NITRO_BUILDTYPE_ARM7)/ferret.$(NITRO_ELF_EXT)
ELFTOBIN_ROMHEADER	?= $(NITRO_TOOLSDIR)/elftobin/romHeader.bin

#----------------------------------------------------------------------------
#  Workaround flags
#	SDK_CW_WARNOFF_SAFESTRB	: use code32.h to wipe warning STRB out
#	SDK_CW_WA_OPT4			: workaround for -O4
#	SDK_CW_WA_CONSTPOOLS	: workaround for CW 'ConstantPools.c'line: 346
#	SDK_CW_WA_OPT_BLX		: workaround for blxeq,blxne with opt>=2
#	SDK_CW_WA_LIBCXX		: workaround for 'C++' libname in lcf
#

#--- flags ---
ifeq	($(NITRO_PLATFORM),TEG)
SDK_CW_WARNOFF_SAFESTRB := yes
else
SDK_CW_WARNOFF_SAFESTRB	:=
endif

######## IRIS V0.2 Hotfix 1
######## NITRO V0.3
######## NITRO V0.4.1
#SDK_CW_WA_OPT4			:= yes
#SDK_CW_WA_CONSTPOOLS	:= yes
#SDK_CW_WA_OPT_BLX		:= yes
######## NITRO V0.5
######## NITRO V0.5 + HotFix1
######## NITRO V0.6
#SDK_CW_WA_LIBCXX		:= yes

#-----------------------------------
#
ifdef	SDK_CW_WA_OPT_BLX
ifdef	NITRO_BLXCHECKED
#	Workaround by pragma
MACRO_FLAGS	+= -DSDK_CW_WA_OPT_BLX
else
#	Workaround by option
SDK_CW_AVOID_ARM_ARCH5	:= yes
endif
endif

ifdef	SDK_CW_WARNOFF_SAFESTRB
MACRO_FLAGS	+= -DSDK_CW_WARNOFF_SAFESTRB
endif

ifdef	SDK_CW_WA_OPT4
ifeq	($(CODEGEN_CC),ARM)
MACRO_FLAGS	+= $(if $(filter DEBUG,$(NITRO_BUILD_TYPE)),,-DSDK_CW_WA_OPT4)
endif
endif

ifdef	SDK_CW_WA_CONSTPOOLS
ifeq	($(CODEGEN_CC),THUMB)
MACRO_FLAGS	+= $(if $(filter DEBUG,$(NITRO_BUILD_TYPE)),,-DSDK_CW_WA_CONSTPOOLS)
endif
endif

ifdef	SDK_CW_FORCE_EXPORT_SUPPORT
MACRO_FLAGS   +=  -DSDK_CW_FORCE_EXPORT_SUPPORT
endif

#----------------------------------------------------------------------------
# SDK small build

ifdef	SDK_SMALL_BUILD
MACRO_FLAGS	+=	-DSDK_SMALL_BUILD
endif

#----------------------------------------------------------------------------
#  Workaround flags
#	SDK_TEG_WA_VBLANK	: workaround TEG vblank problems
#

ifeq	($(NITRO_PLATFORM),TEG)
MACRO_FLAGS	+= -DSDK_TEG_WA_VBLANK
endif


#----------------------------------------------------------------------------
######## IRIS V0.2 Hotfix 1
######## NITRO V0.3
######## NITRO V0.4
#CW_LIBNAME_RULE_A   := ARM
#CW_LIBNAME_RULE_T   := Thumb
#CW_LIBNAME_RULE_AI  := ARM_i
#CW_LIBNAME_RULE_RA  := ARM_Runtime
#CW_LIBNAME_RULE_RT  := Thumb_Runtime
#CW_LIBNAME_RULE_RAI := ARM_Runtime_i
######## NITRO V0.5 or other
CW_LIBNAME_RULE_A   := NITRO_A
CW_LIBNAME_RULE_T   := NITRO_T
CW_LIBNAME_RULE_AI  := NITRO_Ai
CW_LIBNAME_RULE_RA  := NITRO_Runtime_A
CW_LIBNAME_RULE_RT  := NITRO_Runtime_T
CW_LIBNAME_RULE_RAI := NITRO_Runtime_Ai
#

CW_INCDIRS	 = $(CW_ARMDIR)/msl/MSL_C/MSL_ARM/Include					\
			   $(CW_ARMDIR)/msl/MSL_C/MSL_Common/Include				\
			   $(CW_ARMDIR)/msl/MSL_C/MSL_Common_Embedded/Math/Include	\
			   $(CW_ARMDIR)/msl/MSL_C++/MSL_ARM/Include					\
			   $(CW_ARMDIR)/msl/MSL_C++/MSL_Common/Include				\
			   $(CW_ARMDIR)/msl/MSL_Extras/MSL_Common/Include			\
			   $(CW_ARMDIR)/Profiler/include							\
			   $(CW_ARMDIR)/Runtime/Runtime_ARM/Runtime_NITRO/Common_Includes
#--------Because the Runtime directory was changed in versions CW2.0beta2 and later
ifneq		($(CW_MAJOR_VER),1)
CW_INCDIRS	+= $(CW_ARMDIR)/msl/MSL_Extras/MSL_ARM/Include
endif

CW_CXX			= $(if $(SDK_CW_WA_LIBCXX),CXX,CPP)
CW_LIBCXX_DIR	= $(if $(SDK_CW_WA_LIBCXX),$(ROOT)/lib/CodeWarrior,$(CW_ARMDIR)/msl/MSL_C++/MSL_ARM/Lib)
#--------Because the Runtime directory was changed in versions CW2.0beta2 and later
ifneq		($(CW_MAJOR_VER),1)
CW_LIBDIRS	?= $(CW_ARMDIR)/msl/MSL_C/MSL_ARM/Lib					\
			   $(CW_LIBCXX_DIR)										\
			   $(CW_ARMDIR)/msl/MSL_Extras/MSL_ARM/Lib 				\
			   $(CW_ARMDIR)/Runtime/Runtime_ARM/Runtime_NITRO/Lib	\
			   $(CW_ARMDIR)/Mathlib/lib
endif
CW_LIBDIRS	?= $(CW_ARMDIR)/msl/MSL_C/MSL_ARM/Lib		\
			   $(CW_LIBCXX_DIR)							\
			   $(CW_ARMDIR)/msl/MSL_Extras/MSL_ARM/Lib 	\
			   $(CW_ARMDIR)/Runtime/Lib					\
			   $(CW_ARMDIR)/Mathlib/lib

ifeq		($(CODEGEN_PROC)-$(NITRO_PLATFORM),ARM9-TEG)
CW_SUFFIX	 = _LE_strb.a
else
CW_SUFFIX	 = _LE.a
endif

ifeq		($(CODEGEN_PROC),ARM9)
CW_LIBFP	?= FP_fastI_v5t_LE.a
# CW_LIBFP	?= FP_fixedI_v5t_LE.a
# CW_LIBFP	?= FP_flush0_v5t_LE.a
# CW_LIBFP	?= FP_fullI_v5t_LE.a
else
CW_LIBFP	?= FP_fastI_v4t_LE.a
# CW_LIBFP	?= FP_fixedI_v4t_LE.a
# CW_LIBFP	?= FP_flush0_v4t_LE.a
# CW_LIBFP	?= FP_fullI_v4t_LE.a
endif

CW_LIBCXX_ARM	= MSL_$(CW_CXX)_$(CW_LIBNAME_RULE_AI)$(CW_SUFFIX)

CW_LIBS_ARM	= MSL_C_$(CW_LIBNAME_RULE_AI)$(CW_SUFFIX)		\
			  MSL_Extras_$(CW_LIBNAME_RULE_AI)$(CW_SUFFIX)	\
			  $(CW_LIBCXX_ARM)								\
			  $(CW_LIBFP)									\
			  $(CW_LIBNAME_RULE_RAI)$(CW_SUFFIX)

# Workaround for Byte Access: Don't use thumb libraries if ARM9-TEG
ifeq		($(CODEGEN_PROC)-$(NITRO_PLATFORM),ARM9-TEG)

CW_LIBCXX_THUMB = $(CW_LIBCXX_ARM)

CW_LIBS_THUMB	= $(CW_LIBS_ARM)

else

CW_LIBCXX_THUMB = MSL_$(CW_CXX)_$(CW_LIBNAME_RULE_T)_LE.a

CW_LIBS_THUMB	= MSL_C_$(CW_LIBNAME_RULE_T)_LE.a		\
				  MSL_Extras_$(CW_LIBNAME_RULE_T)_LE.a	\
				  $(CW_LIBCXX_THUMB)					\
				  $(CW_LIBFP)							\
				  $(CW_LIBNAME_RULE_RT)_LE.a
endif

ifeq	($(CODEGEN_PROC),ARM9)
#STUBS_LIBS	+= libstubscw$(NITRO_LIBSUFFIX).a
else #	($(CODEGEN_PROC),ARM7)
#STUBS_LIBS	+= libstubscw_sp$(NITRO_LIBSUFFIX).a
endif
	$(ECHO) "==== test";

#----------------------------------------------------------------------------
#  Switch by codegen target
#    NITRO_PLATFORM  TEG/TS
#    NITRO_MEMSIZE   4M/8M
#    CODEGEN_PROC    ARM7/ARM9
#    CODEGEN_CC      ARM/THUMB
#    CODEGEN_AS      ARM/THUMB
#

MACRO_FLAGS		+= -DSDK_$(NITRO_PLATFORM)  \
				   -DSDK_$(NITRO_MEMSIZE)   \
				   -DSDK_$(CODEGEN_PROC)

MACRO_FLAGS_CC	+= -DSDK_CODE_$(CODEGEN_CC)
MACRO_FLAGS_AS	+= -DSDK_CODE_$(CODEGEN_AS)


#-------------------------------------- CODEGEN_PROC ARM9/ARM7

CCFLAGS_PROC_ARM9 = $(if $(SDK_CW_AVOID_ARM_ARCH5),arm7tdmi,arm946e)


CCFLAGS_PROC	 = -proc $(if $(filter ARM9,$(CODEGEN_PROC)),$(CCFLAGS_PROC_ARM9),arm7tdmi)
ASFLAGS_PROC	 = -proc $(if $(filter ARM9,$(CODEGEN_PROC)),arm5TE,arm4T)

ifeq		($(NITRO_PLATFORM),TEG)
CCFLAGS_PROC	+= $(CW_AVOID_STRB)
endif

#-------------------------------------- NITRO_CODEGEN(=CODEGEN_CC) ARM/THUMB

CC_LIBS	 		 = $(if $(filter ARM,$(CODEGEN_CC)),$(CW_LIBS_ARM),$(CW_LIBS_THUMB))

#-------------------------------------- CODEGEN_CC  ARM/THUMB

CCFLAGS_ISET	 = $(if $(filter THUMB,$(CODEGEN_CC)),-thumb,-nothumb)

#-------------------------------------- CODEGEN_AS  ARM/THUMB

ASFLAGS_ISET	 = $(if $(filter THUMB,$(CODEGEN_AS)),-16,-32)

CCFLAGS_ARCH	?= $(CCFLAGS_PROC) $(CCFLAGS_ISET) -nopic -nopid -interworking
ASFLAGS_ARCH	?= $(ASFLAGS_PROC) $(ASFLAGS_ISET)
LDFLAGS_ARCH	?= $(CCFLAGS_PROC) $(CCFLAGS_ISET) -nopic -nopid -interworking
ARFLAGS_ARCH	?=

#----------------------------------------------------------------------------
#  Standard includes/libraries for CodeWarrior
#

export	MWCIncludes    := $(subst \;,$(OPEN_PAREN),$(subst $(OPEN_PAREN),;,$(subst \;,$(CLOSE_PAREN),$(subst $(CLOSE_PAREN),;,$(subst \;, ,$(subst $(SPACE),;,$(CW_INCDIRS)))))))
export	MWLibraries    := $(subst \;,$(OPEN_PAREN),$(subst $(OPEN_PAREN),;,$(subst \;,$(CLOSE_PAREN),$(subst $(CLOSE_PAREN),;,$(subst \;, ,$(subst $(SPACE),;,$(CW_LIBDIRS)))))))
export	MWLibraryFiles := $(subst \;,$(OPEN_PAREN),$(subst $(OPEN_PAREN),;,$(subst \;,$(CLOSE_PAREN),$(subst $(CLOSE_PAREN),;,$(subst \;, ,$(subst $(SPACE),;,$(CC_LIBS)))))))

#----------------------------------------------------------------------------
#  declare NITRO target
#

MACRO_FLAGS	+= -DSDK_NITRO

#----------------------------------------------------------------------------
#  Switch by C Compiler CW/SNC
#

MACRO_FLAGS	+= -DSDK_$(NITRO_CCTYPE) \
               -DSDK_$(NITRO_CCTYPE)_MAJOR_VER=$(CW_MAJOR_VER)


#----------------------------------------------------------------------------
#  Switch by DEBUG/RELEASE/FINALROM
#

MACRO_FLAGS		+= -DSDK_$(NITRO_BUILD_TYPE)

CCFLAGS_OPT_O_	 = -O4 -inline on,noauto
CCFLAGS_OPT_O_	+= -opt $(if $(filter ARM9,$(CODEGEN_PROC)),speed,space)

CCFLAGS_OPT		?= $(if $(filter DEBUG,$(NITRO_BUILD_TYPE)),-O0 -inline off,$(CCFLAGS_OPT_O_))
ASFLAGS_OPT		?=
LDFLAGS_OPT		?=
ARFLAGS_OPT		?=

CCFLAGS_OPT		+= $(if $(NITRO_NO_OPT_G),,-g)
LDFLAGS_OPT		+= $(if $(NITRO_NO_OPT_G),,-g) $(if $(filter 3,$(CW_MAJOR_VER)),-segment_veneers)

#--------To avoid problem with size increase in versions CW2.0beta2 and later, -ipa file option is required
ifneq		($(CW_MAJOR_VER),1)
CCFLAGS_OPT		+= -ipa file
endif


#----------------------------------------------------------------------------
#  Switch for code profiling
#
CCFLAGS_OPT	+= $(if $(NITRO_PROFILE),-profile)
CCFLAGS_OPT	+= $(if $(NITRO_PROFILE_TYPE),-DSDK_PROFILE_$(NITRO_PROFILE_TYPE))


#----------------------------------------------------------------------------
#  Switch for anti-thread policy
#

ifdef		NITRO_NO_THREAD
MACRO_FLAGS	+= -DSDK_NO_THREAD
endif


#----------------------------------------------------------------------------
#  Switch for AUTOTEST/UNITTEST
#
ifdef		NITRO_AUTOTEST
MACRO_FLAGS	+= -DSDK_AUTOTEST
endif
ifdef		NITRO_UTEST
MACRO_FLAGS	+= -DSDK_UTEST
endif


#----------------------------------------------------------------------------
#  Switch for using system call when SVC_WaitVBlankIntr
#
MACRO_FLAGS	+= $(if $(NITRO_SVC_WAITVBLANK_COMPATIBLE),-DSDK_SVC_WAITVBLANK_COMPATIBLE)


#----------------------------------------------------------------------------
#  Switch for automatic PrintServer
#
ifdef		NITRO_ENABLE_ARM7_PRINT
MACRO_FLAGS	+= -DSDK_ENABLE_ARM7_PRINT
endif


#----------------------------------------------------------------------------
#  My macro flags
#
MACRO_FLAGS	+= $(LMACRO_FLAGS)


#----------------------------------------------------------------------------
CCFLAGS_WARNING_FULL	 = all,cmdline,illpragmas,emptydecl,possible,unusedarg,unusedvar,unused,extracomma,pedantic,hidevirtual,implicitconv,impl_int2float,impl_float2int,impl_signedunsigned,notinlined,largeargs,structclass,padding,notused,missingreturn,unusedexpr,ptrintconv,anyptrintconv,undefmacro,filecaps,sysfilecaps,tokenpasting

CCFLAGS_WARNING_NIGHTLY	 = all,padding,notused,ptrintconv,undefmacro

CCFLAGS_MSGSTYLE		?= -msgstyle std

ifeq	($(TWLSDK_WARNING_NIGHTLY),TRUE)
CCFLAGS_WARNING			?= -w $(CCFLAGS_WARNING_NIGHTLY)
endif

ifeq	($(NITRO_WARNING_STRICT),TRUE)
CCFLAGS_WARNING			?= -w $(CCFLAGS_WARNING_FULL)
else
CCFLAGS_WARNING			?= -w all
endif
LDFLAGS_WARNING			?= -w on
ARFLAGS_WARNING			?= -w on

CCFLAGS_ENCODING		?= $(if $(filter en,$(NITROSDK_LANG)),-enc ascii,-enc SJIS)

CCFLAGS_CHARTYPE		?= -char signed

CCFLAGS_MISC			 = $(CCFLAGS_MSGSTYLE) $(CCFLAGS_WARNING) $(CCFLAGS_ENCODING) $(CCFLAGS_CHARTYPE)
ASFLAGS_MISC			 = $(CCFLAGS_MSGSTYLE)
LDFLAGS_MISC			 = $(CCFLAGS_MSGSTYLE) $(LDFLAGS_WARNING)
ARFLAGS_MISC			 = $(CCFLAGS_MSGSTYLE) $(ARFLAGS_WARNING)


#----------------------------------------------------------------------------

CCFLAGS_SYS	?= -stdinc -enum int -stdkeywords off -Cpp_exceptions off
ASFLAGS_SYS	?= -nostdinc -DSDK_ASM
LDFLAGS_SYS	?= -stdlib -map closure -main _start
ARFLAGS_SYS	?= -library


CCFLAGS		= -lang c $(CCFLAGS_ARCH) $(CCFLAGS_OPT) $(CCFLAGS_MISC) \
		  $(CCFLAGS_SYS)  $(MACRO_FLAGS) $(MACRO_FLAGS_CC)

CXXFLAGS	= -lang c++ $(CCFLAGS_ARCH) $(CCFLAGS_OPT) $(CCFLAGS_MISC) \
		  $(CCFLAGS_SYS)  $(MACRO_FLAGS) $(MACRO_FLAGS_CC)

ASFLAGS		= $(ASFLAGS_ARCH) $(ASFLAGS_OPT) $(ASFLAGS_MISC) \
		  $(ASFLAGS_SYS)  $(MACRO_FLAGS) $(MACRO_FLAGS_AS)

LDFLAGS		= $(LDFLAGS_ARCH) $(LDFLAGS_OPT) $(LDFLAGS_MISC) \
		  $(LDFLAGS_SYS)

ARFLAGS		= $(ARFLAGS_ARCH) $(ARFLAGS_OPT) $(ARFLAGS_MISC) \
		  $(ARFLAGS_SYS)


#----- End of commondefs.cctype.CW -----
