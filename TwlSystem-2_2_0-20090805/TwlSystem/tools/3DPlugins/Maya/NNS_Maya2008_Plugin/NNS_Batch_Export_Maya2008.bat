set NNS_MAYA_2008_APP_ROOT=C:\Program Files\Autodesk\Maya2008
set NNS_MAYA_2008_N3BE_FILE=%1
"%NNS_MAYA_2008_APP_ROOT%\bin\mayabatch.exe" -command "NNS_BatchExport \"$NNS_MAYA_2008_N3BE_FILE\""
pause
