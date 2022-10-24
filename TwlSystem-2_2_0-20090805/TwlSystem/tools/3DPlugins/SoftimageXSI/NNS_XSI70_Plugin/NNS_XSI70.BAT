@echo off
call C:\Softimage\XSI_7.0\Application\bin\SETENV.BAT
set NNS_XSI_Plugin_ROOT=C:\NNS_XSI70_Plugin
echo "Starting Softimage(R) XSI v7.0"
echo "with NINTENDO TWL-System Plugin for SOFTIMAGE|XSI"
xsi %1 
echo on
