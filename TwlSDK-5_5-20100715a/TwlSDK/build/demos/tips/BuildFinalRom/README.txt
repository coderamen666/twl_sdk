----------------------------------------------------------------------------
 Quick Guide for finalrom
----------------------------------------------------------------------------
1) Get your "libsyscall.a" and "rom header template" from Nintendo.
   And put them into ./etc directory.
2) Edit (.bmp or .bin) and .bsf files in ./etc/banner directory for your banner file.
3) Edit ROM-TS.rsf(TWLSDK_PLATFORM=NITRO) or ROM-TS.TWL.rsf(TWLSDK_PLATFORM=TWL) 
   for your application setting.
4) Do 'make' to get final rom.
----------------------------------------------------------------------------
