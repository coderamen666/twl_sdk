set NNS_MAYA_2010_APP_ROOT=C:\Program Files\Autodesk\Maya2010
set NNS_MAYA_2010_N3BE_FILE=%1
"%NNS_MAYA_2010_APP_ROOT%\bin\mayabatch.exe" -command "NNS_BatchExport \"$NNS_MAYA_2010_N3BE_FILE\""
pause
