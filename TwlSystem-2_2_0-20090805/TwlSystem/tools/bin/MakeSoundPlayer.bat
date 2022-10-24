@echo off

if "%1" == "" goto help

if not defined TWL_PLATFORM set TWL_PLATFORM=TS

if defined TWL_DEBUG ( set TWL_BUILD_DIR=Debug 
) else ( 
if defined TWL_FINALROM ( set TWL_BUILD_DIR=Rom 
) else ( 
set TWL_BUILD_DIR=Release ) )

"%TWLSDK_ROOT%/tools/bin/makerom.TWL.exe" -F -DSDAT_FILE=%1 -DTWLSDK_ROOT=%TWLSDK_ROOT% -DTWLSYSTEM_ROOT=%TWLSYSTEM_ROOT% -DTARGET_PLATFORM="TWL-HYB"  -DMAKEROM_WRAM_MAPPING=MAP2_TS_HYB -DMAKEROM_CODEC_MODE=TWL -DCOMPSUFFIX9=_LZ -DCOMPSUFFIX7= %TWLSYSTEM_ROOT%/tools/SoundPlayer/SoundPlayer.rsf %~dp1SoundPlayer.srl


if errorlevel 1 exit 1

goto end

:help
  @echo MakeSoundPlayer
  @echo Copyright 2004-2008 Nintendo. All rights reserved.
  @echo.
  @echo Usage:
  @echo   MakeSoundPlayer.bat sdat-file
  @goto end

:end

