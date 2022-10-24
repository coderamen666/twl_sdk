set NNS_MAYA_2009_APP_ROOT=C:\Program Files\Autodesk\Maya2009
set NNS_MAYA_2009_N3BE_FILE=%1
"%NNS_MAYA_2009_APP_ROOT%\bin\mayabatch.exe" -command "NNS_BatchExport \"$NNS_MAYA_2009_N3BE_FILE\""
pause
