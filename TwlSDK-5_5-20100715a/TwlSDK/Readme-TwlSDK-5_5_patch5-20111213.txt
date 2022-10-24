■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■
■  Readme-TwlSDK-5_5_patch5-20111213.txt
■
■  Plus Patch for TWL-SDK 5.5
■
■  December 13, 2011
■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■


Introduction

    This patch fixes problems found in TWL-SDK 5.5 after its official release. To install this patch, copy the included files over the files in the directory where the TWL-SDK 5.5 official release (20100715) is installed.


Caution

    If the TWL-SDK libraries have already been built, just applying this patch may cause errors during builds as a result of the cache.
    If errors occur during builds, clear the cache by running "make clobber" in the TWL-SDK root directory.
    If you then run "make" in the same directory (the root directory of TWL-SDK), this problem will be resolved.

    Even when there is no source code change to SRL, TAD, and other executable files, if there is a change in the library, a patch sometimes contains those executable files.


File List
■ Source Files
	/TwlSDK/build/demos.TWL/nandApp/SubBanner/src/main.c
	/TwlSDK/build/libraries/snd/ARM9.TWL/src/sndex.c

■ Header Files
	/TwlSDK/include/nitro/version.h
	/TwlSDK/include/twl/na/ARM9/sub_banner.h
	/TwlSDK/include/twl/snd/ARM9/sndex.h
	/TwlSDK/include/twl/version.h

■ Library Files
	/TwlSDK/lib/ARM9-TS/Debug/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Debug/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Debug/crt0.o
	/TwlSDK/lib/ARM9-TS/Debug/libdsp.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libna.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libna.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libna.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libna.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libsndex.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libsndex.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libsndex.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libsndex.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Release/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Release/crt0.o
	/TwlSDK/lib/ARM9-TS/Release/libdsp.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libna.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libna.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libna.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libna.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.a
	/TwlSDK/lib/ARM9-TS/Release/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libsndex.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libsndex.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libsndex.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libsndex.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Rom/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Rom/crt0.o
	/TwlSDK/lib/ARM9-TS/Rom/libdsp.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libna.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libna.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libna.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libna.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libsndex.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libsndex.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libsndex.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libsndex.TWL.LTD.thumb.a

■ Component Files
	/TwlSDK/components/ichneumon/ARM7-TS/Debug/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS/Release/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS/Rom/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Debug/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Release/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Rom/ichneumon_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Debug/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Release/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Debug/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Release/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Rom/mongoose_sub.nef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Debug/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Release/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Rom/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Debug/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Release/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Rom/ferret.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Debug/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD/Release/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD/Rom/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Debug/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Release/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Rom/ferret.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon.TWL.LTD.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon_defs.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose.TWL.LTD.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose_defs.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon_defs.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon.TWL.LTD.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon_defs.TWL.LTD.sbin

■ Documentation and Other Items
	/TwlSDK/bin/ARM9-TS/Rom/mb_child_TWL.srl
	/TwlSDK/bin/ARM9-TS/Rom/NandFiler.srl
	/TwlSDK/bin/ARM9-TS/Rom/NandFiler.tad
	/TwlSDK/bin/ARM9-TS/Rom/TwlNmenu.srl
	/TwlSDK/bin/ARM9-TS/Rom/TwlNmenu.tad
	/TwlSDK/build/buildtools/commondefs
	/TwlSDK/build/buildtools/commondefs.parens
	/TwlSDK/build/demos.TWL/snd/Makefile
	/TwlSDK/build/demos.TWL/tips/TWLBanner_anim2/banner/NBF/sample.xml
	/TwlSDK/build/tools/makelcf/spec.l
	/TwlSDK/build/tools/makelcf.TWL/spec.l
	/TwlSDK/data/for_china/notes_forChina_chrData.bin
	/TwlSDK/man/en_US/na/subBanner/NASubBanner.html
	/TwlSDK/man/en_US/snd/sndex/SNDEXIirFilterTarget.html
	/TwlSDK/man/en_US/snd/sndex/SNDEX_SetIirFilter.html
	/TwlSDK/man/en_US/snd/sndex/SNDEX_SetIirFilterAsync.html
	/TwlSDK/man/en_US/tools/bannercvtr.html
	/TwlSDK/man/en_US/tools/bannerNitroCharacter.html
	/TwlSDK/tools/bin/bannercvtr.exe
	/TwlSDK/tools/bin/makebanner.TWL.exe
	/TwlSDK/tools/bin/makelcf.exe
	/TwlSDK/tools/bin/makelcf.TWL.exe

End
