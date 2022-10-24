
"%TWLSYSTEM_ROOT%/tools/bin/sndarc.exe" sound_data.sarc

@if errorlevel 1 goto err

call "%TWLSYSTEM_ROOT%/tools/bin/MakeSoundPlayer.bat" sound_data.sdat

@if errorlevel 1 goto err

"%TWLSDK_ROOT%\tools\bin\bin2obj.exe" --align 32 --readonly --begin sound_data_sdat sound_data.sdat sound_data.o  

@if errorlevel 1 goto err

@goto ok

:ok
   @echo =======================
   @echo  Convert is SUCCESS !!!
   @echo =======================
   @goto end

:err
   @pause

:end

