%{
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     spec.y

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-11#$
  $Rev: 11140 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>
#include	"makelcf.h"

%}

%token	tSTATIC
%token  tAUTOLOAD
%token  tOVERLAY
%token  tLTDAUTOLOAD
%token  tLTDOVERLAY
%token  tPROPERTY
%token	tGROUP
%token	tADDRESS
%token  tAFTER
%token  tOBJECT
%token  tLIBRARY
%token  tSEARCHSYMBOL
%token  tFORCE
%token  tSTACKSIZE
%token  tCOMPRESS
%token	tOVERLAYDEFS
%token  tOVERLAYTABLE
%token  tLTDOVERLAYDEFS
%token  tLTDOVERLAYTABLE
%token  tSUFFIX
%token  tFLXSUFFIX
%token  tLTDSUFFIX
%token	tBEGIN
%token  tEND
%token  tNL

%union
{
	u32	  	integer;
	char*     	string;
	tObjectName	object;
};

%token	<integer>	tNUMBER
%token	<string>	tSTRING_FORCE
%token	<string>	tSECTIONNAME
%token	<string>	tSTRING_STAR
%token	<string>	tSTRING_GROUP
%token	<string>	tSTRING_FUNCTION
%token	<string>	tSTRING_ID
%token	<string>	tSTRING_QUATED
%token	<string>	tSTRING_NOSPACE
%type	<string>	gFILENAME
%type	<object>	gOBJECTNAME
%%

/*=========================================================================*
      ConfigFile
 *=========================================================================*/

specFile		: sections
			;

sections		: /*NULL*/
			| sections section
			;

section			: tNL
			| sectionStatic
			| sectionOverlay
			| sectionAutoload
			| sectionLtdoverlay
			| sectionLtdautoload
			| sectionProperty
			;

/*===========================================================================
 *     STATIC staticname
 *     {
 *        .....
 *     }
 */
sectionStatic		: staticKeyword begin staticParams end
			;

staticKeyword		: tSTATIC tSTRING_ID tNL
			{
				if ( !StaticSetName( $2 ) ) YYABORT;
			}
			;

staticParams		: /*NULL*/
			| staticParams staticParam
			;

staticParam		: tNL
			| staticAddress
			| staticObjects
			| staticLibraries
			| staticSearchSymbols
			| staticForces
			| staticStackSize
			;

staticAddress		: tADDRESS tNUMBER tNL
			{
				if ( !StaticSetAddress( $2 ) ) YYABORT;
			}
			;

staticObjects		: tOBJECT staticObjectList tNL
			{
				if ( !StaticSetObjectSection( "*" ) ) YYABORT;
			}
			| tOBJECT staticObjectList tSECTIONNAME tNL
			{
				if ( !StaticSetObjectSection( $3 ) ) YYABORT;
				debug_printf( "$3=%s\n", $3 );
				free( $3 );
			}
			;

staticObjectList	: /*NULL*/
			| staticObjectList gOBJECTNAME
			{
				if ( !StaticAddObject( $2.string, $2.type ) ) YYABORT;
			}
			;

staticLibraries		: tLIBRARY staticLibraryList tNL
			{
				if ( !StaticSetLibrarySection( "*" ) ) YYABORT;
			}
			| tLIBRARY staticLibraryList tSECTIONNAME tNL
			{
				if ( !StaticSetLibrarySection( $3 ) ) YYABORT;
				debug_printf( "$3=%s\n", $3 );
				free( $3 );
			}
			;

staticLibraryList	: /*NULL*/
			| staticLibraryList gOBJECTNAME
			{
				if ( !StaticAddLibrary( $2.string, $2.type ) ) YYABORT;
			}
			;

staticSearchSymbols	: tSEARCHSYMBOL staticSearchSymbolList tNL
			;

staticSearchSymbolList	: /*NULL*/
			| staticSearchSymbolList tSTRING_ID
			{
				if ( !StaticAddSearchSymbol( $2 ) ) YYABORT;
			}
			;

staticForces	: tFORCE staticForceList tNL
			;

staticForceList	: /*NULL*/
			| staticForceList tSTRING_FORCE
			{
				if ( !StaticAddForce( $2 ) ) YYABORT;
			}
			;

staticStackSize		: tSTACKSIZE tNUMBER tNL
			{
				if ( !StaticSetStackSize( $2 ) ) YYABORT;
			}
			| tSTACKSIZE tNUMBER tNUMBER
			{
				if ( !StaticSetStackSize( $2 ) ) YYABORT;
				if ( !StaticSetStackSizeIrq( $3 ) ) YYABORT;
			}
			;

/*===========================================================================
 *     OVERLAY overlayname
 *     {
 *        .....
 *     }
 */
sectionOverlay		: overlayKeyword begin overlayParams end
			;

overlayKeyword		: tOVERLAY tSTRING_ID tNL
			{
				if ( !AddOverlay( $2 ) ) YYABORT;
			}
			;

overlayParams		: /*NULL*/
			| overlayParams overlayParam
			;

overlayParam		: tNL
			| overlayGroup
			| overlayAddress
			| overlayAfters
			| overlayObjects
			| overlaySearchSymbols
			| overlayForces
			| overlayLibraries
			| overlayCompress
			;

overlayGroup		: tGROUP tSTRING_ID tNL
			{
				if ( !OverlaySetGroup( $2 ) ) YYABORT;
			}
			;

overlayAddress		: tADDRESS tNUMBER tNL
			{
				if ( !OverlaySetAddress( $2 ) ) YYABORT;
			}
			;

overlayAfters		: tAFTER overlayAfterList tNL
			;

overlayAfterList	: /*NULL*/
			| overlayAfterList tSTRING_ID
			{
				if ( !OverlayAddAfter( $2 ) ) YYABORT;
			}
			;

overlayObjects		: tOBJECT overlayObjectList tNL
			{
				if ( !OverlaySetObjectSection( "*" ) ) YYABORT;
			}
			| tOBJECT overlayObjectList tSECTIONNAME tNL
			{
				if ( !OverlaySetObjectSection( $3 ) ) YYABORT;
				free( $3 );
			}
			;

overlayObjectList	: /*NULL*/
			| overlayObjectList gOBJECTNAME
			{
				if ( !OverlayAddObject( $2.string, $2.type ) ) YYABORT;
			}
			;

overlayLibraries	: tLIBRARY overlayLibraryList tNL
			{
				if ( !OverlaySetLibrarySection( "*" ) ) YYABORT;
			}
			| tLIBRARY overlayLibraryList tSECTIONNAME tNL
			{
				if ( !OverlaySetLibrarySection( $3 ) ) YYABORT;
				free( $3 );
			}
			;

overlayLibraryList	: /*NULL*/
			| overlayLibraryList gOBJECTNAME
			{
				if ( !OverlayAddLibrary( $2.string, $2.type ) ) YYABORT;
			}
			;

overlaySearchSymbols	: tSEARCHSYMBOL overlaySearchSymbolList tNL
			;

overlaySearchSymbolList	: /*NULL*/
			| overlaySearchSymbolList tSTRING_ID
			{
				if ( !OverlayAddSearchSymbol( $2 ) ) YYABORT;
			}
			;

overlayForces	: tFORCE overlayForceList tNL
			;

overlayForceList	: /*NULL*/
			| overlayForceList tSTRING_FORCE
			{
				if ( !OverlayAddForce( $2 ) ) YYABORT;
			}
			;

overlayCompress	: tCOMPRESS tSTRING_ID tNL
			{
				if ( !OverlaySetCompress( $2 ) ) YYABORT;
			}
			;

/*===========================================================================
 *     AUTOLOAD autoloadname
 *     {
 *        .....
 *     }
 */
sectionAutoload		: autoloadKeyword begin autoloadParams end
			;

autoloadKeyword		: tAUTOLOAD tSTRING_ID tNL
			{
				if ( !AddAutoload( $2 ) ) YYABORT;
			}
			;

autoloadParams		: /*NULL*/
			| autoloadParams autoloadParam
			;

/* mostly same as overlayParam except no "group" */
autoloadParam		: tNL
			| overlayAddress
			| overlayAfters
			| overlayObjects
			| overlaySearchSymbols
			| overlayForces
			| overlayLibraries
			;

/*===========================================================================
 *     LTDOVERLAY ltdoverlayname
 *     {
 *        .....
 *     }
 */
sectionLtdoverlay		: ltdoverlayKeyword begin ltdoverlayParams end
			;

ltdoverlayKeyword		: tLTDOVERLAY tSTRING_ID tNL
			{
				if ( !AddLtdoverlay( $2 ) ) YYABORT;
			}
			;

ltdoverlayParams		: /*NULL*/
			| ltdoverlayParams ltdoverlayParam
			;

ltdoverlayParam		: tNL
			| overlayGroup
			| overlayAddress
			| overlayAfters
			| overlayObjects
			| overlaySearchSymbols
			| overlayForces
			| overlayLibraries
			| overlayCompress
			;

/*===========================================================================
 *     LTDAUTOLOAD ltdautoloadname
 *     {
 *        .....
 *     }
 */
sectionLtdautoload		: ltdautoloadKeyword begin ltdautoloadParams end
			;

ltdautoloadKeyword		: tLTDAUTOLOAD tSTRING_ID tNL
			{
				if ( !AddLtdautoload( $2 ) ) YYABORT;
			}
			;

ltdautoloadParams		: /*NULL*/
			| ltdautoloadParams ltdautoloadParam
			;

/* mostly same as overlayParam except no "group" */
ltdautoloadParam		: tNL
			| overlayAddress
			| overlayAfters
			| overlayObjects
			| overlaySearchSymbols
			| overlayForces
			| overlayLibraries
			;

/*===========================================================================
 *     PROPERTY
 *     {
 *        .....
 *     }
 */
sectionProperty		: propertyKeyword begin propertyParams end
			;

propertyKeyword		: tPROPERTY tNL
			;

propertyParams		: /*NULL*/
			| propertyParams propertyParam
			;

propertyParam		: tNL
			| propertyName
			| propertyOverlay
			| propertyLtdname
			| propertyLtdoverlay
			| propertySuffix
			| propertyFlxsuffix
			| propertyLtdsuffix
			;

propertyName		: tOVERLAYDEFS gFILENAME tNL
			{
				if ( !PropertySetOverlayDefs( $2 ) ) YYABORT;
			}
			;

propertyOverlay		: tOVERLAYTABLE gFILENAME tNL
			{
				if ( !PropertySetOverlayTable( $2 ) ) YYABORT;
			}
			;

propertyLtdname		: tLTDOVERLAYDEFS gFILENAME tNL
			{
				if ( !PropertySetLtdoverlayDefs( $2 ) ) YYABORT;
			}
			;

propertyLtdoverlay		: tLTDOVERLAYTABLE gFILENAME tNL
			{
				if ( !PropertySetLtdoverlayTable( $2 ) ) YYABORT;
			}
			;

propertySuffix		: tSUFFIX gFILENAME tNL
			{
				if ( !PropertySetSuffix( $2 ) ) YYABORT;
			}
			;

propertyFlxsuffix	: tFLXSUFFIX gFILENAME tNL
			{
				if ( !PropertySetFlxsuffix( $2 ) ) YYABORT;
			}
			;

propertyLtdsuffix	: tLTDSUFFIX gFILENAME tNL
			{
				if ( !PropertySetLtdsuffix( $2 ) ) YYABORT;
			}
			;

/*===========================================================================
 *	MISC
 */

begin			: tBEGIN tNL
			;

end			: tEND tNL
			;

gFILENAME		: tSTRING_ID
			| tSTRING_QUATED
			| tSTRING_NOSPACE
			;

gOBJECTNAME		: gFILENAME
			{
				$$.string = $1;
				$$.type   = OBJTYPE_FILE;
			}
			| tSTRING_STAR
			{
				$$.string = $1;
				$$.type   = OBJTYPE_STAR;
			}
			| tSTRING_GROUP
			{
				$$.string = $1;
				$$.type   = OBJTYPE_GROUP;
			}
			| tSTRING_FUNCTION
			{
				$$.string = $1;
				$$.type   = OBJTYPE_OBJECT;
			}
			;
%%
