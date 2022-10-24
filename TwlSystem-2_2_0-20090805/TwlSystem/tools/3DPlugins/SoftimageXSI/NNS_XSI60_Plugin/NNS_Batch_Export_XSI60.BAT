@echo off
call C:\Softimage\XSI_6.02\Application\bin\SETENV.BAT
set NNS_XSI_Plugin_ROOT=C:\NNS_XSI60_Plugin
set NNS_Batch_Export=%NNS_XSI_Plugin_ROOT%\NNS_Batch_Export.vbs
echo "Starting Softimage(R) XSI v6.02"
echo "with NINTENDO TWL-System Plugin for SOFTIMAGE|XSI"

xsi -script %NNS_Batch_Export% -main command_main -args -arg0 %1

echo on
