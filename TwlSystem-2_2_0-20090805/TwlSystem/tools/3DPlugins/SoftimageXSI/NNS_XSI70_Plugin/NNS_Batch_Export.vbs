'Scene cnv.vbs 2004/10/21

Option Explicit

sub command_main( arg0 )

	dim fso, fp, str, myDll
	
	set myDll = CreateObject("NNS_Export.Interface")
	set fso		= CreateObject("Scripting.FileSystemObject")
	if fso.FileExists( arg0 ) <> true then
		
		logmessage "Can't open file. " & arg0, siErrorMsg
		exit sub
	
	end if
	
	myDll.standalone application, arg0

end sub

