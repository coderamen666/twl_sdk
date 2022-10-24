-- =================================================================
-- NINTENDO SetNitro Material 3D Studio Max
-- Description 	: Switch Standard Materials to NITRO Standard Materials
-- Date			: 07/15/04
-- Updated		: 03/30/05
-- Author		: Clark Morse and Alain Schneuwly
-- 
-- Copyright 2004, 2005 Nintendo. All rights reserved.
-- =================================================================

global gTempMat
global gCurMat
global gTempVar

fn checkPropExists &curMat &tempMat prop =
(
	if isProperty curMat prop and isProperty tempMat prop then
	(
		return true
	)
	
	else
	(
		warning = StringStream ""
		
		-- display warning
		if isProperty tempMat #name then
		(
			str = tempMat.name as string;
			format "Warning: % property doesn't exist in material: % and was not copied." prop str to: warning
		)
		
		else
		(
			format "Warning: % property doesn't exist in material: <name not available> and was not copied." prop to: warning
		)
		
		msg = warning as string
		print msg
		
		return false
	)
)

-- convert standard materials to NITRO Standard
fn convertStandard &curMat =
(	
	-- find the material index of this material
	idx = findItem meditMaterials curMat	
	
	-- get the show in viewport flag
	showInView = (getmtlmeditflags curMat)[6] or (getmtlmeditflags curMat)[5]
 	
	gTempMat = curMat;
			
	if NITROMATERIAL != undefined then
	(
		curMat = NITROMATERIAL()
	)
	
	else
	(
		messageBox "Cannot complete conversion: NITROStdMat.ms is not loaded.  Make sure it is in the plugins folder and restart 3ds max." title: "NITROStdMat.ms not found"
		return false
	)
	
	-- make sure it converted correctly to NITROSTANDARD
	if not isProperty curMat #standard do
	(
		errorMsg = StringStream ""
		if isProperty curMat #name then
		(
			format "Error: Nitro Standard creation did not work for material: %! Check that NITROStdMat.ms is correct." curMat.name to: errorMsg
		)
		
		else
		(
			format "Error: Nitro Standard creation did not work! Check that NITROStdMat.ms is correct." to: errorMsg
		)
		
		fullMsg = errorMsg as string
		
		messageBox fullMsg title: "Error in NITRO Conversion"
		
		return false
	)
	
	-- get all of the material parameters
	standardParamArray 	= getPropNames gTempMat
		
	-- copy all the params over
	for i = 1 to standardParamArray.count do
	(
		if checkPropExists &curMat.standard &gTempMat standardParamArray[i] do
		(
			value = getProperty gTempMat standardParamArray[i]
			setProperty &curMat.standard standardParamArray[i] value	
			
			curPropStr = standardParamArray[i] as string
						
			-- check if the property is animated.  If it is, copy over the controller
			if execute("gTempMat." + curPropStr + ".isAnimated") do
			(
				gCurMat = &curMat;
				execute("(*gCurMat).standard." + curPropStr + ".controller = gTempMat." + curPropStr + ".controller");
			)
		)
	)
	
	-- for some reason, these are separate, so just copy them
	if checkPropExists &curMat &gTempMat #name do (curMat.name = gTempMat.name)
	if checkPropExists &curMat &gTempMat #effectsChannel do (curMat.effectsChannel = gTempMat.effectsChannel)	
	if checkPropExists &curMat &gTempMat #showInViewport do (curMat.showInViewport = showInView)
			
	if idx != 0 do
	(
		setMeditMaterial idx curMat
	)
			
	return true
)

fn convertMultiSub &curMat = 
(
	-- find the material index of this material
	multiMatIdx = findItem meditMaterials curMat	
 	
	if isProperty curMat #materialList do
	(
		for i = 1 to curMat.materialList.count do
		( 
			-- first, since this is a sub material, make sure that it is a standard material before we try to convert it
			curMatStd = findString (curMat.materialList[i] as string) ":Standard"
			if curMatStd == undefined then -- CHANGED_FOR_MAX9
			(				
				curMatStd = findString (curMat.materialList[i] as string) ":Normal"
			)
			curMatNit = findString (curMat.materialList[i] as string) ":NITRO_Standard"
			curMatMor = findString (curMat.materialList[i] as string) ":NITRO_Morpher"
			
			if curMatStd == undefined do
			(	
				if curMatNit == undefined and curMatMor == undefined do
				(
					-- this is a type that is not supported for conversion, so display a warning
					warningMsg = StringStream ""
					matNameExists = isProperty curMat #name
					subNameExists = isProperty curMat.materialList[i] #name
					
					if subNameExists then (format "Warning: Sub material % " curMat.materialList[i].name to: warningMsg)
					else (format "Warning: Sub Material <name not available> " to: warningMsg)
					
					if matNameExists then (format "on multi-material: % is not a Max Standard material and cannot be converted." curMat.name to: warningMsg)
					else (format "on multi-material: <name not available> is not a Max Standard material and cannot be converted." to: warningMsg)
					
					fullMsg = warningMsg as string
					print fullMsg
				)
				
				continue
			)
			
			gTempMat = curMat.materialList[i];
					
			-- find the material index of this material
			idx = findItem meditMaterials curMat.materialList[i]
			
			-- get the show in viewport flag
			showInView = (getmtlmeditflags curMat.materialList[i])[6] or (getmtlmeditflags curMat.materialList[i])[5]
			
			if NITROMATERIAL != undefined then
			(
				curMat.materialList[i] = NITROMATERIAL();
			)
			
			else
			(
				messageBox "Cannot complete conversion: NITROStdMat.ms is not loaded.  Make sure it is in the plugins folder and restart 3ds max." title: "NITROStdMat.ms not found"
				return false
			)
			
			-- make sure it converted correctly to NITROSTANDARD
			if not isProperty curMat.materialList[i] #standard do
			(
				errorMsg = StringStream ""
				if isProperty curMat.materialList[i] #name then
				(
					format "Error: Nitro Standard creation did not work for material: %! Check that NITROStdMat.ms is correct." curMat.materialList[i].name to: errorMsg
				)
				
				else
				(
					format "Error: Nitro Standard creation did not work! Check that NITROStdMat.ms is correct." to: errorMsg
				)
				
				fullMsg = errorMsg as string
				
				messageBox fullMsg title: "Error in NITRO Conversion"
				
				return false
			)
			
			-- get all of the material parameters
			standardParamArray 	= getPropNames gTempMat
			
			-- copy all the params over
			for j = 1 to standardParamArray.count do
			(
				if checkPropExists &curMat.materialList[i].standard &gTempMat standardParamArray[i] do
				(	
					value = getProperty gTempMat standardParamArray[j]
					setProperty &curMat.materialList[i].standard standardParamArray[j] value
					
					curPropStr = standardParamArray[j] as string
					
					gCurMat = &curMat.materialList[i]
					
					-- check if the property is animated.  If it is, copy over the controller
					if execute("gTempMat." + curPropStr + ".isAnimated") do
					(
						execute("(*gCurMat).standard." + curPropStr + ".controller = gTempMat." + curPropStr + ".controller");
					)

				)
			)	
			
			-- for some reason, these are separate, so just copy them
			if checkPropExists &curMat.materialList[i] &gTempMat #name do (curMat.materialList[i].name = gTempMat.name)
			if checkPropExists &curMat.materialList[i] &gTempMat #effectsChannel do (curMat.materialList[i].effectsChannel = gTempMat.effectsChannel)	
			if checkPropExists &curMat.materialList[i] &gTempMat #showInViewport do (curMat.materialList[i].showInViewport = showInView)
			
			-- set the material in the material editor if it was in there prior to conversion
			if idx != 0 do
			(
				setMeditMaterial idx curMat.materialList[i]
			)
			
		)
		
		-- set the multi material in the material editor if it was in there prior to conversion
		if multiMatIdx != 0 do
		(
			setMeditMaterial multiMatIdx curMat
		)
		
	)

)

fn convertMorpher &curMat =
(
	-- find the material index of this material
	idx = findItem meditMaterials curMat	
 	
	gTempMat = curMat;
	
	-- get the index of the morpher channel that is being displayed in the viewport (if any)
	showInViewIdx = -1
	
	if ( curMat.base != undefined ) do
	(
		-- check the base material for being displayed
		if ( (getmtlmeditflags curMat.base)[6] or (getmtlmeditflags curMat.base)[5] ) then
		(
			showInViewIdx = 0 -- use 0 to define the base material index
		)
		
		-- check if any of the other channels are displaying in the viewport
		else
		(
			for i = 1 to 100 do
			(
				gTempVar = &curMat
				channelName = "(*gTempVar).mat_" + (i as string) 
				--print channelName
				
				if execute(channelName) == undefined do ( continue )
				
				if execute("(getmtlmeditflags " + channelName + ")[6] or (getmtlmeditflags " + channelName + ")[5]") do
				(
					showInViewIdx = i
					break
				)
			)
		)
	)
	--showInView = (getmtlmeditflags curMat)[6] or (getmtlmeditflags curMat)[5]
	
	-- get the show in viewport flag
	if checkPropExists &curMat &gTempMat #showInViewport do (showInView = gTempMat.showInViewport)
	
	if NITROITPMATERIAL != undefined then
	(
		curMat = NITROITPMATERIAL()
	)
	
	else
	(
		messageBox "Cannot complete conversion: NITROMorphMat.ms is not loaded.  Make sure it is in the plugins folder and restart 3ds max." title: "NITROMorphMat.ms not found"
		return false
	)
	
	-- make sure it converted correctly to NITROITPMATERIAL
	if not isProperty curMat #morpher do
	(
		errorMsg = StringStream ""
		if isProperty curMat #name then
		(
			format "Error: Nitro Morpher creation did not work for material: %! Check that NITROMorphMat.ms is correct." curMat.name to: errorMsg
		)
		
		else
		(
			format "Error: Nitro Morpher creation did not work! Check that NITROMorphMat.ms is correct." to: errorMsg
		)
		
		fullMsg = errorMsg as string
		
		messageBox fullMsg title: "Error in NITRO Conversion"
		
		return false
	)
	
	-- get all of the material parameters
	morpherParamArray = getPropNames gTempMat
	
	-- copy all the params over
	for i = 1 to morpherParamArray.count do
	(
		if checkPropExists &curMat.morpher &gTempMat morpherParamArray[i] do
		(
			value = getProperty gTempMat morpherParamArray[i]
			setProperty &curMat.morpher morpherParamArray[i] value	
			
			curPropStr = morpherParamArray[i] as string
						
			-- check if the property is animated.  If it is, copy over the controller
			if execute("gTempMat." + curPropStr + ".isAnimated") do
			(
				gCurMat = &curMat;
				execute("(*gCurMat).morpher." + curPropStr + ".controller = gTempMat." + curPropStr + ".controller");
			)
		)
	)
	
	-- for some reason, these are separate, so just copy them
	if checkPropExists &curMat &gTempMat #name do (curMat.name = gTempMat.name)
	if checkPropExists &curMat &gTempMat #effectsChannel do (curMat.effectsChannel = gTempMat.effectsChannel)
	--if checkPropExists &curMat &gTempMat #showInViewport do (curMat.showInViewport = showInView)
	
	-- convert the 100 morpher channels as well
	for i = 1 to 100 do
	(
		gCurMat = &curMat
		
		-- search for all Standard materials to convert
		execute("gTempVar = findString ((*gCurMat).morpher.mat_" + (i as string) + " as string) \":Standard\"")
		if gTempVar == undefined then -- CHANGED_FOR_MAX9
		(				
			execute("gTempVar = findString ((*gCurMat).morpher.mat_" + (i as string) + " as string) \":Normal\"")
		)
		
		if gTempVar != undefined then
		(
			if execute("convertStandard &(*gCurMat).morpher.mat_" + (i as string)) == false do (return false)
		)
	)
	
	-- convert the base morpher channel
	curMatStd = findString (curMat.morpher.base as string) ":Standard"
	if curMatStd == undefined then -- CHANGED_FOR_MAX9
	(				
		curMatStd = findString (curMat.morpher.base as string) ":Normal"
	)
	
	if curMatStd != undefined then
	(
		if convertStandard &curMat.morpher.base == false do (return false)
	)
	
	-- set the channel that was visible in the viewport before conversion to be visible now (if any)
	if showInViewIdx != -1 do
	(
		-- set the base material to be visible in the viewport
		if showInViewIdx == 0 then
		(
			--if checkPropExists &curMat.morpher.base &gTempMat #showInViewport do (curMat.morpher.base.showInViewport = true)
			if checkPropExists &curMat.morpher.base &gTempMat #showInViewport do (showTextureMap curMat curMat.morpher.base on)
		)
		
		-- set the previous channel that was visible to be visible in the viewport
		else
		(
			gTempVar = &curMat
			channelName = "(*gTempVar).morpher.mat_" + (showInViewIdx as string) 
			
			if execute(channelName) != undefined do 
			(			
				if execute("checkPropExists &" + channelName + " &gTempMat #showInViewport") do
				(
					--execute(channelName + ".showInViewport = true");
					execute("showTextureMap (*gTempVar) " + channelName + " on");
					print "converted"
				)
			)
		)
	)

	if idx != 0 do
	(
		setMeditMaterial idx curMat
	)
	
	return true
	
)

-- for converting an object's material to NITRO Standard
fn convertObjMat &obj =
(	
	if isProperty obj #material do
	(
		if obj.material != undefined do
		( 
			-- print ("[" + (obj.material as string) + "]")
			curMatStd = findString (obj.material as string) ":Standard"
			if curMatStd == undefined then -- CHANGED_FOR_MAX9
			(				
				curMatStd = findString (obj.material as string) ":Normal"
			)
			curMatSub = findString (obj.material as string) "#Multi/Sub-Object"
			if curMatSub == undefined then -- CHANGED_FOR_MAX9
			(				
				curMatSub = findString (obj.material as string) "#マルチ/サブ オブジェクト"
			)
			curMatNit = findString (obj.material as string) ":NITRO_Standard"
			curMatMor = findString (obj.material as string) ":NITRO_Morpher"
			curMatItp = findString (obj.material as string) ":Morpher"
			
			-- standard material
			if curMatStd != undefined then
			(				
				if convertStandard &obj.material == false do (return false)		
			)
			
			-- multi-sub material
			else if curMatSub != undefined then
			(
				if convertMultiSub &obj.material == false do (return false)
			)
			
			else if curMatItp != undefined then
			(
				if convertMorpher &obj.material == false do (return false)
			)
			
			-- other material
			else if (curMatNit == undefined and curMatMor == undefined) do
			(
				str = stringStream ""
				format "Cannot convert material to NITRO Standard (not a supported type): %" obj.material.name to: str
				print str 
			)
		)	
	)
	
	return true
)

-- converts all standard materials in the scene to NITRO Standard
fn convertAll progBar =
(
	objList = $*
	
	for i = 1 to objList.count do
	(	
		if convertObjMat &objList[i] == false do (return false)
		
		if progBar != undefined do
		(
			progBar.value = (100 / objList.count) * i
		)
	)
	
	return true
)

-- main utility script plugin
Utility SET_NITRO "Convert to NITRO Material"
(
	label lbl_textH "Convert all scene Standard" 
	label lbl_textI "Materials to NITRO MAT"
	label lbl_textK""
	button nitroall "Set All NITRO MAT"
	label lbl_textF""
	label lbl_textG "Object Progress"
	progressbar progBar
	
	on nitroall pressed do
	(
		print "[Starting conversion to Nitro Standard materials.]"
		progBar.value = 0
		convertAll progBar
		progBar.value = 0
		print "[Done]"
	)
)