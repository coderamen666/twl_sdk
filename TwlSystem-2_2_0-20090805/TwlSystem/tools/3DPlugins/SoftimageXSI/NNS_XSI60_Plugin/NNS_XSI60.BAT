@echo off
call C:\Softimage\XSI_6.02\Application\bin\SETENV.BAT
set NNS_XSI_Plugin_ROOT=C:\NNS_XSI60_Plugin
echo "Starting Softimage(R) XSI v6.02"
echo "with NINTENDO TWL-System Plugin for SOFTIMAGE|XSI"
xsi %1 
echo on
