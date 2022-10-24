-- =================================================================
-- NINTENDO NITRO-System 3D Studio Max
-- Description 	: Set Billboard
-- Date			: 03/02/04
-- Author		: Alain Schneuwly, Clark Morse
-- 
-- Copyright 2004 Nintendo. All rights reserved.
-- =================================================================

plugin material NITROITPMATERIAL
name:"NITRO Morpher"
classID:#(0x75825cee, 0x1a3508ad)

extends:MorpherMaterial replaceUI:false version:160
(
	fn check_specular light_0 light_1 light_2 light_3 specularTblOn =
	(
		if light_0.checked or light_1.checked or light_2.checked or light_3.checked then
			specularTblOn.enabled = true
		else
			specularTblOn.enabled = false
	)

	fn set_tex_effect_mtx_enable 
		val
		tex_effect_mtx_0_0
		tex_effect_mtx_0_1
		tex_effect_mtx_1_0
		tex_effect_mtx_1_1
		tex_effect_mtx_2_0
		tex_effect_mtx_2_1
		tex_effect_mtx_3_0
		tex_effect_mtx_3_1 =
	(
		tex_effect_mtx_0_0.enabled = val
		tex_effect_mtx_0_1.enabled = val
		tex_effect_mtx_1_0.enabled = val
		tex_effect_mtx_1_1.enabled = val
		tex_effect_mtx_2_0.enabled = val
		tex_effect_mtx_2_1.enabled = val
		tex_effect_mtx_3_0.enabled = val
		tex_effect_mtx_3_1.enabled = val
	)

	parameters main rollout:nitro
	(
		-- lights
		light_0				type:#integer	ui:light_0 default:1
		light_1				type:#integer	ui:light_1 default:0
		light_2				type:#integer	ui:light_2 default:0
		light_3				type:#integer	ui:light_3 default:0
											
		specularTblOn		type:#integer	ui:specularTblOn default:0
		fogOn				type:#integer	ui:fogOn default:0
		wireFrame			type:#integer	ui:wireFrame default:0
		displayFace			type:#integer	ui:displayFace default:1
		polyMode			type:#integer	ui:polyMode default:1
		polyID				type:#integer	ui:polyID default:0
		texGen				type:#integer	ui:texGen default:1
		texGenST			type:#integer	ui:texGenST default:2
		comPress			type:#integer	ui:comPress default:1
		iflLoop				type:#integer	ui:iflLoop default:0
	)

	parameters main_1_5_0 rollout:nitro
	(
		depthTestDecal		type:#integer	ui:depthTestDecal default:0
		transUpdateDepth	type:#integer	ui:transUpdateDepth default:0
		render1Pixel		type:#integer	ui:render1Pixel default:0
		farClipping			type:#integer	ui:farClipping default:0
		priorityRadio		type:#integer	ui:priorityRadio default: 1
		priorityValue		type:#integer	ui:priorityValue default: 1
		priorityLabel		type:#integer	ui:priorityLabel default: 0	
	)

	parameters main_1_6_0 rollout:nitro
	(
		tex_effect_mtx_0_0	type:#float		ui:tex_effect_mtx_0_0 default: 1.0000
		tex_effect_mtx_0_1	type:#float		ui:tex_effect_mtx_0_1 default: 0.0000
		tex_effect_mtx_1_0	type:#float		ui:tex_effect_mtx_1_0 default: 0.0000
		tex_effect_mtx_1_1	type:#float		ui:tex_effect_mtx_1_1 default: 1.0000
		tex_effect_mtx_2_0	type:#float		ui:tex_effect_mtx_2_0 default: 0.0000
		tex_effect_mtx_2_1	type:#float		ui:tex_effect_mtx_2_1 default: 0.0000
		tex_effect_mtx_3_0	type:#float		ui:tex_effect_mtx_3_0 default: 0.0000
		tex_effect_mtx_3_1	type:#float		ui:tex_effect_mtx_3_1 default: 0.0000
	)

	rollout nitro "NITRO Set Material Attribute"
	(
		group "Priority"
		(
			radiobuttons priorityRadio labels:#("Don't Care", "Value") align: #left columns: 1
			spinner priorityValue "" range:[1,255,0] scale:1 fieldwidth:40 align: #left across: 2
			label priorityLabel "1 (High Priority) - 255 (Low Priority)" align:#left offset:[-75,0]
		)
		group "Lighting"
		(
			checkbox light_0 "Light0" across: 4
			checkbox light_1 "Light1"
			checkbox light_2 "Light2"
			checkbox light_3 "Light3"
		)

		checkbox specularTblOn "Use Specular Reflection Shininess Table"

		on light_0 changed state do check_specular light_0 light_1 light_2 light_3 specularTblOn
		on light_1 changed state do check_specular light_0 light_1 light_2 light_3 specularTblOn
		on light_2 changed state do check_specular light_0 light_1 light_2 light_3 specularTblOn
		on light_3 changed state do check_specular light_0 light_1 light_2 light_3 specularTblOn

		checkbox fogOn "Fog"
		checkbox wireFrame "Wire Frame"

		-- 1.5.0 flags
		checkbox depthTestDecal		"Depth Test for Decal Polygon"
		checkbox transUpdateDepth	"Translucent Polygons Update Depth Buffer"
		checkbox render1Pixel		"Render 1-pixel Polygon"
		checkbox farClipping		"Far Clipping"

		group "Display Face"
		(
			radiobuttons displayFace labels:#("Front Face", "Back Face", "Both Face")
		)

		group "Polygon Mode"
		(
			radiobuttons polyMode labels:#("Modulate", "Decal", "Toon / Highlight", "Shadow")
		)

		spinner polyID "Polygon ID:" fieldwidth:35 range:[0,63,0] scale:1 align:#center

		group "Tex Gen Mode"
		(
			radiobuttons texGen labels:#("None", "TexCoord", "Normal", "Vertex")
		)	

		group "Tex Gen ST Source"
		(
			radiobuttons texGenST labels:#("Polygon", "Material") enabled:false
		)

		-- 1.6.0 additions
		group "Texture Effect Matrix"
		(
			spinner tex_effect_mtx_0_0 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625 across: 2
			spinner tex_effect_mtx_0_1 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625

			spinner tex_effect_mtx_1_0 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625 across: 2
			spinner tex_effect_mtx_1_1 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625

			spinner tex_effect_mtx_2_0 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625 across: 2
			spinner tex_effect_mtx_2_1 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625

			spinner tex_effect_mtx_3_0 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625 across: 2
			spinner tex_effect_mtx_3_1 "" align: #left range: [-500000.0000, 500000.0000, 0] scale: 0.0625
		)

		fn check_tex_effect_mtx texGen = 
		(
			if ( texGen.state == 3 or texGen.state == 4 ) then
			(
				set_tex_effect_mtx_enable true tex_effect_mtx_0_0 tex_effect_mtx_0_1 tex_effect_mtx_1_0 tex_effect_mtx_1_1 tex_effect_mtx_2_0 tex_effect_mtx_2_1 tex_effect_mtx_3_0 tex_effect_mtx_3_1
			)
			else
			(
				set_tex_effect_mtx_enable false tex_effect_mtx_0_0 tex_effect_mtx_0_1 tex_effect_mtx_1_0 tex_effect_mtx_1_1 tex_effect_mtx_2_0 tex_effect_mtx_2_1 tex_effect_mtx_3_0 tex_effect_mtx_3_1
			)
		)

		-- 1.6.0 events
		on tex_effect_mtx_0_0 entered do
		(
			tex_effect_mtx_0_0.value = floor((tex_effect_mtx_0_0.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_0_1 entered do
		(
			tex_effect_mtx_0_1.value = floor((tex_effect_mtx_0_1.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_1_0 entered do
		(
			tex_effect_mtx_1_0.value = floor((tex_effect_mtx_1_0.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_1_1 entered do
		(
			tex_effect_mtx_1_1.value = floor((tex_effect_mtx_1_1.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_2_0 entered do
		(
			tex_effect_mtx_2_0.value = floor((tex_effect_mtx_2_0.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_2_1 entered do
		(
			tex_effect_mtx_2_1.value = floor((tex_effect_mtx_2_1.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_3_0 entered do
		(
			tex_effect_mtx_3_0.value = floor((tex_effect_mtx_3_0.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on tex_effect_mtx_3_1 entered do
		(
			tex_effect_mtx_3_1.value = floor((tex_effect_mtx_3_1.value / (1.0 / 4096)) + 0.5) * (1.0 / 4096)
		)

		on texGen changed state do
		(
			if texGen.state == 1 then texGenST.enabled = false
			if texGen.state == 2 then texGenST.enabled = false
			if texGen.state == 3 then texGenST.enabled = true
			if texGen.state == 4 then texGenST.enabled = true

			check_tex_effect_mtx texGen
		)

		-- 1.5.0 events
		on priorityRadio changed state do
		(
			if priorityRadio.state == 1 then priorityValue.enabled = false
			else priorityValue.enabled = true 
		)

		group "Compress"
		(
			radiobuttons comPress labels:#("Compress if possible", "Don't compress")
		)

		checkbox iflLoop "Loop IFL Texture Animation" align:#center

		on nitro open do
		(
			if texGen.state == 1 then texGenST.enabled = false
			if texGen.state == 2 then texGenST.enabled = false
			if texGen.state == 3 then texGenST.enabled = true
			if texGen.state == 4 then texGenST.enabled = true

			if priorityRadio.state == 1 then priorityValue.enabled = false
			else priorityValue.enabled = true

			check_tex_effect_mtx texGen

			check_specular light_0 light_1 light_2 light_3 specularTblOn
		)
	)
)
