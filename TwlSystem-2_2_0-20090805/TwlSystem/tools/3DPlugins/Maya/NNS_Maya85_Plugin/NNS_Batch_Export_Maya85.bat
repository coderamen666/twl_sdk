set NNS_MAYA_85_APP_ROOT=C:\Program Files\Autodesk\Maya8.5
set NNS_MAYA_85_N3BE_FILE=%1
"%NNS_MAYA_85_APP_ROOT%\bin\mayabatch.exe" -command "NNS_BatchExport \"$NNS_MAYA_85_N3BE_FILE\""
pause
