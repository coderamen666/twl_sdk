%{
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     tlcf.y

  Copyright 2003-2009 Nintendo.  All rights reserved.

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
#include	<string.h>
#include	"makelcf.h"
#include	"defval.h"

// IF-ELSE-ENDIF
static  tCondStack	cond_stack[COND_STACK_MAX];
static  int		cond_stack_level = 0;
static  BOOL		cond_valid = TRUE;
static	tCondBlock	cond_block = COND_BLOCK_NOCOND;

static BOOL GetCompResult(const char* id, const char* comp, const char* value, BOOL* pvalid)
{
	int   result;
	BOOL  valid;
	const char* defval;
	
	defval = SearchDefValCleaned(id);
	result = strcmp(defval, value);
	if      ( strcmp( comp, ".EQ." ) == 0 || strcmp( comp, "==" ) == 0 ) valid = ( result == 0 );
	else if ( strcmp( comp, ".NE." ) == 0 || strcmp( comp, "!=" ) == 0 ) valid = ( result != 0 );
	else if ( strcmp( comp, ".GT." ) == 0 ) valid = ( result >  0 );
	else if ( strcmp( comp, ".GE." ) == 0 ) valid = ( result >= 0 );
	else if ( strcmp( comp, ".LT." ) == 0 ) valid = ( result <  0 );
	else if ( strcmp( comp, ".LE." ) == 0 ) valid = ( result <= 0 );
	else
	{
		tlcf_yyerror( "Illegal IF-condition" );
		return FALSE;
	}
	debug_printf("id(%s)=[%s] comp=[%s] value=[%s] valid=%d\n", id, defval, comp, value, valid);

	*pvalid = valid;
	return TRUE;
}

static BOOL CondIf(const char* id, const char* comp, const char* value)
{
	if (cond_stack_level >= COND_STACK_MAX)
	{
		tlcf_yyerror( "Too deep if-else-endif block" );
		return FALSE;
	}
	cond_stack[cond_stack_level].block = cond_block;
	cond_stack[cond_stack_level].valid = cond_valid;
	cond_stack_level ++;
	cond_block = COND_BLOCK_IF;
	
	// Update validation only if previous level validation is TRUE
	if (cond_stack[cond_stack_level-1].valid)
	{
		if (!GetCompResult(id, comp, value, &cond_valid))
		{
			tlcf_yyerror( "Unresolved IF condition" );
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL CondElse(void)
{
	if (cond_block != COND_BLOCK_IF || cond_stack_level <= 0)
	{
		tlcf_yyerror( "IF-ELSE-ENDIF unmatched" );
		return FALSE;
	}
	cond_block = COND_BLOCK_ELSE;
	
	// Update validation only if previous level validation is TRUE
	if (cond_stack[cond_stack_level-1].valid)
	{
		cond_valid = cond_valid ? FALSE : TRUE;		// Flip validation
	}
	return TRUE;
}

static BOOL CondEndIf(void)
{
	if ((cond_block != COND_BLOCK_IF && cond_block != COND_BLOCK_ELSE) || cond_stack_level <= 0)
	{
		tlcf_yyerror( "IF-ELSE-ENDIF unmatched" );
		return FALSE;
	}
	cond_stack_level --;
	cond_block = cond_stack[cond_stack_level].block;
	cond_valid = cond_stack[cond_stack_level].valid;
	return TRUE;
}


// TOKEN HANDLEING
tTokenBuffer		tokenBuffer[TOKEN_LEN];
int			tokenBufferEnd = 0;
static  tLoopStack	loop_stack[LOOP_STACK_MAX];
static	int		loop_stack_level = 0;

static BOOL  StoreToken( int id, const char *str )
{
	// Validation check for IF-ELSE-ENDIF
	if ( !cond_valid ) return TRUE;

	if ( tokenBufferEnd >= TOKEN_LEN )
	{
		tlcf_yyerror( "Out of token buffer" );
		return FALSE;
	}
	tokenBuffer[tokenBufferEnd].id     = id;
	tokenBuffer[tokenBufferEnd].string = str;
	tokenBufferEnd ++;
	return TRUE;
}

static BOOL  PushLoopStack( int id, const char* sectionName )
{
	// Validation check for IF-ELSE-ENDIF
	if ( !cond_valid ) return TRUE;
	
	if ( loop_stack_level >= LOOP_STACK_MAX )
	{
		tlcf_yyerror( "Out of stack level" );
		return FALSE;
	}
	loop_stack[loop_stack_level].id    = id;
	loop_stack[loop_stack_level].start = tokenBufferEnd;
	loop_stack_level ++;
	return StoreToken( id, sectionName );
}

static BOOL  PopLoopStack( int id )
{
	int start;
	
	// Validation check for IF-ELSE-ENDIF
	if ( !cond_valid ) return TRUE;
	
	loop_stack_level --;
	if ( loop_stack_level < 0 || id != loop_stack[loop_stack_level].id )
	{
		tlcf_yyerror( "Unmatched foreach-end" );
		return FALSE;
	}
	start = loop_stack[loop_stack_level].start;
	tokenBuffer[start].loop_end = tokenBufferEnd-1;
	
	// Don't store 'END' token to TokenBuffer
	return TRUE;
}

BOOL  GetLoopStackLevel( void )
{
	return loop_stack_level;
}

%}

%union
{
	char	*string;
};

%token	<string>	tSTRING

%token	<string>	tFOREACH_STATIC_OBJECTS
%token	<string>	tFOREACH_STATIC_LIBRARIES
%token	<string>	tFOREACH_STATIC_SEARCHSYMBOLS
%token	<string>	tFOREACH_STATIC_FORCES
%token			tEND_STATIC_OBJECTS
%token			tEND_STATIC_LIBRARIES
%token			tEND_STATIC_SEARCHSYMBOLS
%token			tEND_STATIC_FORCES
%token			tSTATIC_NAME
%token			tSTATIC_ADDRESS
%token			tSTATIC_STACKSIZE
%token			tSTATIC_IRQSTACKSIZE
%token	<string>	tSTATIC_CRT0_OBJECT
%token	<string>	tSTATIC_OBJECT
%token	<string>	tSTATIC_LIBRARY
%token	<string>	tSTATIC_SEARCHSYMBOL
%token	<string>	tSTATIC_FORCE

%token	<string>	tFOREACH_AUTOLOADS
%token	<string>	tFOREACH_AUTOLOAD_OBJECTS
%token	<string>	tFOREACH_AUTOLOAD_LIBRARIES
%token	<string>	tFOREACH_AUTOLOAD_SEARCHSYMBOLS
%token	<string>	tFOREACH_AUTOLOAD_FORCES
%token			tEND_AUTOLOADS
%token			tEND_AUTOLOAD_OBJECTS
%token			tEND_AUTOLOAD_LIBRARIES
%token			tEND_AUTOLOAD_SEARCHSYMBOLS
%token			tEND_AUTOLOAD_FORCES
%token			tNUMBER_AUTOLOADS
%token			tAUTOLOAD_ID
%token			tAUTOLOAD_NAME
%token			tAUTOLOAD_ADDRESS
%token	<string>	tAUTOLOAD_OBJECT
%token	<string>	tAUTOLOAD_LIBRARY
%token	<string>	tAUTOLOAD_SEARCHSYMBOL
%token	<string>	tAUTOLOAD_FORCE

%token	<string>	tFOREACH_OVERLAYS
%token	<string>	tFOREACH_OVERLAY_OBJECTS
%token	<string>	tFOREACH_OVERLAY_LIBRARIES
%token	<string>	tFOREACH_OVERLAY_SEARCHSYMBOLS
%token	<string>	tFOREACH_OVERLAY_FORCES
%token			tEND_OVERLAYS
%token			tEND_OVERLAY_OBJECTS
%token			tEND_OVERLAY_LIBRARIES
%token			tEND_OVERLAY_SEARCHSYMBOLS
%token			tEND_OVERLAY_FORCES
%token			tNUMBER_OVERLAYS
%token			tOVERLAY_ID
%token			tOVERLAY_NAME
%token			tOVERLAY_GROUP
%token			tOVERLAY_ADDRESS
%token			tOVERLAY_COMPRESS
%token	<string>	tOVERLAY_OBJECT
%token	<string>	tOVERLAY_LIBRARY
%token	<string>	tOVERLAY_SEARCHSYMBOL
%token	<string>	tOVERLAY_FORCE

%token	<string>	tFOREACH_LTDAUTOLOADS
%token	<string>	tFOREACH_LTDAUTOLOAD_OBJECTS
%token	<string>	tFOREACH_LTDAUTOLOAD_LIBRARIES
%token	<string>	tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS
%token	<string>	tFOREACH_LTDAUTOLOAD_FORCES
%token			tEND_LTDAUTOLOADS
%token			tEND_LTDAUTOLOAD_OBJECTS
%token			tEND_LTDAUTOLOAD_LIBRARIES
%token			tEND_LTDAUTOLOAD_SEARCHSYMBOLS
%token			tEND_LTDAUTOLOAD_FORCES
%token			tNUMBER_LTDAUTOLOADS
%token			tLTDAUTOLOAD_ID
%token			tLTDAUTOLOAD_NAME
%token			tLTDAUTOLOAD_ADDRESS
%token	<string>	tLTDAUTOLOAD_OBJECT
%token	<string>	tLTDAUTOLOAD_LIBRARY
%token	<string>	tLTDAUTOLOAD_SEARCHSYMBOL
%token	<string>	tLTDAUTOLOAD_FORCE

%token	<string>	tFOREACH_LTDOVERLAYS
%token	<string>	tFOREACH_LTDOVERLAY_OBJECTS
%token	<string>	tFOREACH_LTDOVERLAY_LIBRARIES
%token	<string>	tFOREACH_LTDOVERLAY_SEARCHSYMBOLS
%token	<string>	tFOREACH_LTDOVERLAY_FORCES
%token			tEND_LTDOVERLAYS
%token			tEND_LTDOVERLAY_OBJECTS
%token			tEND_LTDOVERLAY_LIBRARIES
%token			tEND_LTDOVERLAY_SEARCHSYMBOLS
%token			tEND_LTDOVERLAY_FORCES
%token			tNUMBER_LTDOVERLAYS
%token			tLTDOVERLAY_ID
%token			tLTDOVERLAY_NAME
%token			tLTDOVERLAY_GROUP
%token			tLTDOVERLAY_ADDRESS
%token			tLTDOVERLAY_COMPRESS
%token	<string>	tLTDOVERLAY_OBJECT
%token	<string>	tLTDOVERLAY_LIBRARY
%token	<string>	tLTDOVERLAY_SEARCHSYMBOL
%token	<string>	tLTDOVERLAY_FORCE

%token			tPROPERTY_OVERLAYDEFS
%token			tPROPERTY_OVERLAYTABLE
%token			tPROPERTY_LTDOVERLAYDEFS
%token			tPROPERTY_LTDOVERLAYTABLE
%token			tPROPERTY_SUFFIX
%token			tPROPERTY_FLXSUFFIX
%token			tPROPERTY_LTDSUFFIX

%token			tIF_OPEN
%token			tIF_CLOSE
%token	<string>	tIF_ID
%token	<string>	tIF_COMP
%token	<string>	tIF_VALUE
%token			tIF_ELSE
%token			tIF_ENDIF
%%

/*=========================================================================*
      ConfigFile
 *=========================================================================*/

tlcfFile	: tokens
		;

tokens		: /*NULL*/
		| tokens token
		;

token		: tSTRING			{ if (!StoreToken( tSTRING,                $1   )) YYABORT; }
		| tSTATIC_CRT0_OBJECT	{ if (!StoreToken( tSTATIC_CRT0_OBJECT,    $1   )) YYABORT; }
		| tSTATIC_NAME			{ if (!StoreToken( tSTATIC_NAME,           NULL )) YYABORT; }
		| tSTATIC_ADDRESS		{ if (!StoreToken( tSTATIC_ADDRESS,        NULL )) YYABORT; }
		| tSTATIC_OBJECT		{ if (!StoreToken( tSTATIC_OBJECT,         $1   )) YYABORT; }
		| tSTATIC_LIBRARY		{ if (!StoreToken( tSTATIC_LIBRARY,        $1   )) YYABORT; }
		| tSTATIC_SEARCHSYMBOL		{ if (!StoreToken( tSTATIC_SEARCHSYMBOL,   $1   )) YYABORT; }
		| tSTATIC_FORCE				{ if (!StoreToken( tSTATIC_FORCE,   $1   )) YYABORT; }
		| tSTATIC_STACKSIZE		{ if (!StoreToken( tSTATIC_STACKSIZE,      NULL )) YYABORT; }
		| tSTATIC_IRQSTACKSIZE		{ if (!StoreToken( tSTATIC_IRQSTACKSIZE,   NULL )) YYABORT; }
		| tFOREACH_STATIC_OBJECTS	{ if (!PushLoopStack( tFOREACH_STATIC_OBJECTS, $1   )) YYABORT; }
		| tEND_STATIC_OBJECTS    	{ if (!PopLoopStack ( tFOREACH_STATIC_OBJECTS       )) YYABORT; }
		| tFOREACH_STATIC_LIBRARIES	{ if (!PushLoopStack( tFOREACH_STATIC_LIBRARIES, $1 )) YYABORT; }
		| tEND_STATIC_LIBRARIES    	{ if (!PopLoopStack ( tFOREACH_STATIC_LIBRARIES     )) YYABORT; }
		| tFOREACH_STATIC_SEARCHSYMBOLS	{ if (!PushLoopStack( tFOREACH_STATIC_SEARCHSYMBOLS, $1 )) YYABORT; }
		| tEND_STATIC_SEARCHSYMBOLS	{ if (!PopLoopStack ( tFOREACH_STATIC_SEARCHSYMBOLS     )) YYABORT; }
		| tFOREACH_STATIC_FORCES		{ if (!PushLoopStack( tFOREACH_STATIC_FORCES, $1 )) YYABORT; }
		| tEND_STATIC_FORCES		{ if (!PopLoopStack ( tFOREACH_STATIC_FORCES     )) YYABORT; }
		| tAUTOLOAD_ID			{ if (!StoreToken( tAUTOLOAD_ID,           NULL )) YYABORT; }
		| tAUTOLOAD_NAME		{ if (!StoreToken( tAUTOLOAD_NAME,         NULL )) YYABORT; }
		| tAUTOLOAD_ADDRESS		{ if (!StoreToken( tAUTOLOAD_ADDRESS,      NULL )) YYABORT; }
		| tAUTOLOAD_OBJECT		{ if (!StoreToken( tAUTOLOAD_OBJECT,       $1   )) YYABORT; }
		| tAUTOLOAD_LIBRARY		{ if (!StoreToken( tAUTOLOAD_LIBRARY,      $1   )) YYABORT; }
		| tAUTOLOAD_SEARCHSYMBOL	{ if (!StoreToken( tAUTOLOAD_SEARCHSYMBOL, $1   )) YYABORT; }
		| tAUTOLOAD_FORCE			{ if (!StoreToken( tAUTOLOAD_FORCE, $1   )) YYABORT; }
		| tNUMBER_AUTOLOADS		{ if (!StoreToken( tNUMBER_AUTOLOADS,      NULL )) YYABORT; }
		| tFOREACH_AUTOLOADS		{ if (!PushLoopStack( tFOREACH_AUTOLOADS,          $1 )) YYABORT; }
		| tEND_AUTOLOADS		{ if (!PopLoopStack ( tFOREACH_AUTOLOADS              )) YYABORT; }
		| tFOREACH_AUTOLOAD_OBJECTS	{ if (!PushLoopStack( tFOREACH_AUTOLOAD_OBJECTS,   $1 )) YYABORT; }
		| tEND_AUTOLOAD_OBJECTS		{ if (!PopLoopStack ( tFOREACH_AUTOLOAD_OBJECTS       )) YYABORT; }
		| tFOREACH_AUTOLOAD_LIBRARIES	{ if (!PushLoopStack( tFOREACH_AUTOLOAD_LIBRARIES, $1 )) YYABORT; }
		| tEND_AUTOLOAD_LIBRARIES	{ if (!PopLoopStack ( tFOREACH_AUTOLOAD_LIBRARIES     )) YYABORT; }
		| tFOREACH_AUTOLOAD_SEARCHSYMBOLS{ if (!PushLoopStack( tFOREACH_AUTOLOAD_SEARCHSYMBOLS, $1 )) YYABORT; }
		| tEND_AUTOLOAD_SEARCHSYMBOLS	 { if (!PopLoopStack ( tFOREACH_AUTOLOAD_SEARCHSYMBOLS     )) YYABORT; }
		| tFOREACH_AUTOLOAD_FORCES		 { if (!PushLoopStack( tFOREACH_AUTOLOAD_FORCES, $1 )) YYABORT; }
		| tEND_AUTOLOAD_FORCES			 { if (!PopLoopStack ( tFOREACH_AUTOLOAD_FORCES     )) YYABORT; }
		| tOVERLAY_ID			{ if (!StoreToken( tOVERLAY_ID,            NULL )) YYABORT; }
		| tOVERLAY_NAME			{ if (!StoreToken( tOVERLAY_NAME,          NULL )) YYABORT; }
		| tOVERLAY_GROUP		{ if (!StoreToken( tOVERLAY_GROUP,         NULL )) YYABORT; }
		| tOVERLAY_ADDRESS		{ if (!StoreToken( tOVERLAY_ADDRESS,       NULL )) YYABORT; }
		| tOVERLAY_COMPRESS		{ if (!StoreToken( tOVERLAY_COMPRESS,      NULL )) YYABORT; }
		| tOVERLAY_OBJECT		{ if (!StoreToken( tOVERLAY_OBJECT,        $1   )) YYABORT; }
		| tOVERLAY_LIBRARY		{ if (!StoreToken( tOVERLAY_LIBRARY,       $1   )) YYABORT; }
		| tOVERLAY_SEARCHSYMBOL		{ if (!StoreToken( tOVERLAY_SEARCHSYMBOL,  $1   )) YYABORT; }
		| tOVERLAY_FORCE			{ if (!StoreToken( tOVERLAY_FORCE,  $1   )) YYABORT; }
		| tNUMBER_OVERLAYS		{ if (!StoreToken( tNUMBER_OVERLAYS,       NULL )) YYABORT; }
		| tFOREACH_OVERLAYS		{ if (!PushLoopStack( tFOREACH_OVERLAYS,          $1 )) YYABORT; }
		| tEND_OVERLAYS			{ if (!PopLoopStack ( tFOREACH_OVERLAYS              )) YYABORT; }
		| tFOREACH_OVERLAY_OBJECTS	{ if (!PushLoopStack( tFOREACH_OVERLAY_OBJECTS,   $1 )) YYABORT; }
		| tEND_OVERLAY_OBJECTS		{ if (!PopLoopStack ( tFOREACH_OVERLAY_OBJECTS       )) YYABORT; }
		| tFOREACH_OVERLAY_LIBRARIES	{ if (!PushLoopStack( tFOREACH_OVERLAY_LIBRARIES, $1 )) YYABORT; }
		| tEND_OVERLAY_LIBRARIES	{ if (!PopLoopStack ( tFOREACH_OVERLAY_LIBRARIES     )) YYABORT; }
		| tFOREACH_OVERLAY_SEARCHSYMBOLS{ if (!PushLoopStack( tFOREACH_OVERLAY_SEARCHSYMBOLS, $1 )) YYABORT; }
		| tEND_OVERLAY_SEARCHSYMBOLS	{ if (!PopLoopStack ( tFOREACH_OVERLAY_SEARCHSYMBOLS     )) YYABORT; }
		| tFOREACH_OVERLAY_FORCES		{ if (!PushLoopStack( tFOREACH_OVERLAY_FORCES, $1 )) YYABORT; }
		| tEND_OVERLAY_FORCES			{ if (!PopLoopStack ( tFOREACH_OVERLAY_FORCES     )) YYABORT; }
		| tLTDAUTOLOAD_ID			{ if (!StoreToken( tLTDAUTOLOAD_ID,           NULL )) YYABORT; }
		| tLTDAUTOLOAD_NAME		{ if (!StoreToken( tLTDAUTOLOAD_NAME,         NULL )) YYABORT; }
		| tLTDAUTOLOAD_ADDRESS		{ if (!StoreToken( tLTDAUTOLOAD_ADDRESS,      NULL )) YYABORT; }
		| tLTDAUTOLOAD_OBJECT		{ if (!StoreToken( tLTDAUTOLOAD_OBJECT,       $1   )) YYABORT; }
		| tLTDAUTOLOAD_LIBRARY		{ if (!StoreToken( tLTDAUTOLOAD_LIBRARY,      $1   )) YYABORT; }
		| tLTDAUTOLOAD_SEARCHSYMBOL	{ if (!StoreToken( tLTDAUTOLOAD_SEARCHSYMBOL, $1   )) YYABORT; }
		| tLTDAUTOLOAD_FORCE		{ if (!StoreToken( tLTDAUTOLOAD_FORCE, $1   )) YYABORT; }
		| tNUMBER_LTDAUTOLOADS		{ if (!StoreToken( tNUMBER_LTDAUTOLOADS,      NULL )) YYABORT; }
		| tFOREACH_LTDAUTOLOADS		{ if (!PushLoopStack( tFOREACH_LTDAUTOLOADS,          $1 )) YYABORT; }
		| tEND_LTDAUTOLOADS		{ if (!PopLoopStack ( tFOREACH_LTDAUTOLOADS              )) YYABORT; }
		| tFOREACH_LTDAUTOLOAD_OBJECTS	{ if (!PushLoopStack( tFOREACH_LTDAUTOLOAD_OBJECTS,   $1 )) YYABORT; }
		| tEND_LTDAUTOLOAD_OBJECTS		{ if (!PopLoopStack ( tFOREACH_LTDAUTOLOAD_OBJECTS       )) YYABORT; }
		| tFOREACH_LTDAUTOLOAD_LIBRARIES	{ if (!PushLoopStack( tFOREACH_LTDAUTOLOAD_LIBRARIES, $1 )) YYABORT; }
		| tEND_LTDAUTOLOAD_LIBRARIES	{ if (!PopLoopStack ( tFOREACH_LTDAUTOLOAD_LIBRARIES     )) YYABORT; }
		| tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS{ if (!PushLoopStack( tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS, $1 )) YYABORT; }
		| tEND_LTDAUTOLOAD_SEARCHSYMBOLS	 { if (!PopLoopStack ( tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS     )) YYABORT; }
		| tFOREACH_LTDAUTOLOAD_FORCES		 { if (!PushLoopStack( tFOREACH_LTDAUTOLOAD_FORCES, $1 )) YYABORT; }
		| tEND_LTDAUTOLOAD_FORCES			 { if (!PopLoopStack ( tFOREACH_LTDAUTOLOAD_FORCES     )) YYABORT; }
		| tLTDOVERLAY_ID			{ if (!StoreToken( tLTDOVERLAY_ID,            NULL )) YYABORT; }
		| tLTDOVERLAY_NAME			{ if (!StoreToken( tLTDOVERLAY_NAME,          NULL )) YYABORT; }
		| tLTDOVERLAY_GROUP		{ if (!StoreToken( tLTDOVERLAY_GROUP,         NULL )) YYABORT; }
		| tLTDOVERLAY_ADDRESS		{ if (!StoreToken( tLTDOVERLAY_ADDRESS,       NULL )) YYABORT; }
		| tLTDOVERLAY_COMPRESS		{ if (!StoreToken( tLTDOVERLAY_COMPRESS,      NULL )) YYABORT; }
		| tLTDOVERLAY_OBJECT		{ if (!StoreToken( tLTDOVERLAY_OBJECT,        $1   )) YYABORT; }
		| tLTDOVERLAY_LIBRARY		{ if (!StoreToken( tLTDOVERLAY_LIBRARY,       $1   )) YYABORT; }
		| tLTDOVERLAY_SEARCHSYMBOL		{ if (!StoreToken( tLTDOVERLAY_SEARCHSYMBOL,  $1   )) YYABORT; }
		| tLTDOVERLAY_FORCE				{ if (!StoreToken( tLTDOVERLAY_FORCE,  $1   )) YYABORT; }
		| tNUMBER_LTDOVERLAYS		{ if (!StoreToken( tNUMBER_LTDOVERLAYS,       NULL )) YYABORT; }
		| tFOREACH_LTDOVERLAYS		{ if (!PushLoopStack( tFOREACH_LTDOVERLAYS,          $1 )) YYABORT; }
		| tEND_LTDOVERLAYS			{ if (!PopLoopStack ( tFOREACH_LTDOVERLAYS              )) YYABORT; }
		| tFOREACH_LTDOVERLAY_OBJECTS	{ if (!PushLoopStack( tFOREACH_LTDOVERLAY_OBJECTS,   $1 )) YYABORT; }
		| tEND_LTDOVERLAY_OBJECTS		{ if (!PopLoopStack ( tFOREACH_LTDOVERLAY_OBJECTS       )) YYABORT; }
		| tFOREACH_LTDOVERLAY_LIBRARIES	{ if (!PushLoopStack( tFOREACH_LTDOVERLAY_LIBRARIES, $1 )) YYABORT; }
		| tEND_LTDOVERLAY_LIBRARIES	{ if (!PopLoopStack ( tFOREACH_LTDOVERLAY_LIBRARIES     )) YYABORT; }
		| tFOREACH_LTDOVERLAY_SEARCHSYMBOLS{ if (!PushLoopStack( tFOREACH_LTDOVERLAY_SEARCHSYMBOLS, $1 )) YYABORT; }
		| tEND_LTDOVERLAY_SEARCHSYMBOLS	{ if (!PopLoopStack ( tFOREACH_LTDOVERLAY_SEARCHSYMBOLS     )) YYABORT; }
		| tFOREACH_LTDOVERLAY_FORCES	{ if (!PushLoopStack( tFOREACH_LTDOVERLAY_FORCES, $1 )) YYABORT; }
		| tEND_LTDOVERLAY_FORCES		{ if (!PopLoopStack ( tFOREACH_LTDOVERLAY_FORCES     )) YYABORT; }
		| tPROPERTY_OVERLAYDEFS		{ if (!StoreToken( tPROPERTY_OVERLAYDEFS,  NULL )) YYABORT; }
		| tPROPERTY_OVERLAYTABLE	{ if (!StoreToken( tPROPERTY_OVERLAYTABLE, NULL )) YYABORT; }
		| tPROPERTY_LTDOVERLAYDEFS	{ if (!StoreToken( tPROPERTY_LTDOVERLAYDEFS,  NULL )) YYABORT; }
		| tPROPERTY_LTDOVERLAYTABLE	{ if (!StoreToken( tPROPERTY_LTDOVERLAYTABLE, NULL )) YYABORT; }
		| tPROPERTY_SUFFIX		{ if (!StoreToken( tPROPERTY_SUFFIX,       NULL )) YYABORT; }
		| tPROPERTY_FLXSUFFIX	{ if (!StoreToken( tPROPERTY_FLXSUFFIX,    NULL )) YYABORT; }
		| tPROPERTY_LTDSUFFIX	{ if (!StoreToken( tPROPERTY_LTDSUFFIX,    NULL )) YYABORT; }
		| cond_if | cond_else | cond_endif
		;

cond_if		: tIF_OPEN tIF_ID tIF_COMP tIF_VALUE tIF_CLOSE { if (!CondIf( $2, $3, $4 )) YYABORT; }
		| tIF_OPEN tIF_ID tIF_COMP           tIF_CLOSE { if (!CondIf( $2, $3, "" )) YYABORT; }
		;

cond_else	: tIF_ELSE  { if (!CondElse()) YYABORT; }
		;

cond_endif	: tIF_ENDIF { if (!CondEndIf()) YYABORT; }
		;

%%
