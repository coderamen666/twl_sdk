/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - demolib - include - g3d_demolib
  File:     camera.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_G3DDEMO_CAMERA_H_
#define NNS_G3DDEMO_CAMERA_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
	G3DDemoLookAt structure

 *---------------------------------------------------------------------------*/
typedef struct
{
	VecFx32		camPos;					// camera position (= view point)
	VecFx32		target;					// camera target (= focus point)
	VecFx32		camUp;					// Camera up direction
	MtxFx43		matrix;

} G3DDemoLookAt;


/*---------------------------------------------------------------------------*
	G3DDemoPersp structure

 *---------------------------------------------------------------------------*/
typedef struct
{
	fx32		fovySin;			// sine value of view angle divided by 2
    fx32		fovyCos;			// cosine value of view angle divided by 2
    fx32		aspect;				// aspect ratio
    fx32		nearClip;			// distance from view point to near clip surface
    fx32		farClip;			// distance from view point to far clip surface

} G3DDemoPersp;


/*---------------------------------------------------------------------------*
	G3DDemoCamera structure

 *---------------------------------------------------------------------------*/
typedef struct
{
	G3DDemoPersp	persp;
	G3DDemoLookAt	lookat;

	fx32			distance;
	s16				angleX;
	s16				angleY;
	s16				angleZ;
	s16				padding;

} G3DDemoCamera;


/*---------------------------------------------------------------------------*
	Function Prototypes

 *---------------------------------------------------------------------------*/

void G3DDemo_InitCamera(G3DDemoCamera* camera, fx32 posy, fx32 distance);
void G3DDemo_CalcCamera(G3DDemoCamera* camera);
void G3DDemo_MoveCamera(G3DDemoCamera* camera);


#ifdef __cplusplus
}/* extern "C" */
#endif

// NNS_G3DDEMO_CAMERA_H_
#endif
