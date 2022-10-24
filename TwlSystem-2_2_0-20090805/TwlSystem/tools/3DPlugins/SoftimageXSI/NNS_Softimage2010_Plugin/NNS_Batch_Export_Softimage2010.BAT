@echo off
call C:\Softimage\Softimage_2010_SP1\Application\bin\SETENV.BAT
set NNS_XSI_Plugin_ROOT=C:\TwlSystem\tools\3DPlugins\SoftimageXSI\NNS_Softimage2010_Plugin
set NNS_Batch_Export=%NNS_XSI_Plugin_ROOT%\NNS_Batch_Export.vbs
echo "Starting Softimage(R) 2010"
echo "with NINTENDO TWL-System Plugin for Softimage"

xsi -script %NNS_Batch_Export% -main command_main -args -arg0 %1

echo on
