@echo off
call C:\Softimage\Softimage_7.5\Application\bin\SETENV.BAT
set NNS_XSI_Plugin_ROOT=C:\TwlSystem\tools\3DPlugins\SoftimageXSI\NNS_XSI70_Plugin
echo "Starting Softimage(R) 7.5"
echo "with NINTENDO TWL-System Plugin for Softimage"
xsi %1 
echo on
