%{
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     spec.l

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2011-04-15#$
  $Rev: 11409 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<string.h>
#include	"makelcf.h"
#include	"defval.h"
#include	"spec.tab.h"

#define	RETURN(x) { debug_printf( "%s [%s]\n", #x, spec_yytext ); return(x); }

static void  YYText_ResolveDevVal( void );
static char* getSection( void );


%}

ws			[ \t]
nl			\r?\n
identifier		[A-Za-z_][A-Za-z_0-9]*
decnum			-?[1-9][0-9]*
octnum			-?0[0-7]*
hexnum			-?0[xX][0-9A-Fa-f]+
qstring			\"[^\"\n\r]*\"
string_wo_ws		[^ \t\n\r]+

%s START
%s PARAM
%s PARAM_CONTINUE
%option pointer
%option yylineno
%option noyywrap
%%

 //======================================================================
 //    SPEC TOKENS
 //======================================================================

#.*			;					/* Comment */
\/\/.*			;					/* Comment */

<INITIAL>.*	{
	YYText_ResolveDevVal();
	BEGIN START;
}

<PARAM_CONTINUE>.*	{
	YYText_ResolveDevVal();
	BEGIN PARAM;
}

<START>(Static|STATIC|static)	 		{ BEGIN PARAM; RETURN(tSTATIC);       }
<START>(Autoload|AUTOLOAD|autoload) 		{ BEGIN PARAM; RETURN(tAUTOLOAD);     }
<START>(Overlay|OVERLAY|overlay) 		{ BEGIN PARAM; RETURN(tOVERLAY);      }
<START>(Ltd|LTD|ltd)(Autoload|AUTOLOAD|autoload)	{ BEGIN PARAM; RETURN(tLTDAUTOLOAD); }
<START>(Ltd|LTD|ltd)(Overlay|OVERLAY|overlay)	{ BEGIN PARAM; RETURN(tLTDOVERLAY); }
<START>(Property|PROPERTY|property)		{ BEGIN PARAM; RETURN(tPROPERTY);     }
<START>(Properties|PROPERTIES|properties)	{ BEGIN PARAM; RETURN(tPROPERTY);     }

<START>(Group|GROUP|group)			{ BEGIN PARAM; RETURN(tGROUP);        }
<START>(Address|ADDRESS|address)		{ BEGIN PARAM; RETURN(tADDRESS);      }
<START>(After|AFTER|after)			{ BEGIN PARAM; RETURN(tAFTER);        }
<START>(Object|OBJECT|object)[sS]?		{ BEGIN PARAM; RETURN(tOBJECT);       }
<START>(Librar|LIBRAR|librar)(y|Y|ies|IES)?	{ BEGIN PARAM; RETURN(tLIBRARY);      }
<START>(Search|SEARCH|search)_?(Symbol|SYMBOL|symbol)[sS]? { BEGIN PARAM; RETURN(tSEARCHSYMBOL); }
<START>(Force|FORCE|force)			{ BEGIN PARAM; RETURN(tFORCE);        }
<START>(StackSize|STACKSIZE|stacksize)		{ BEGIN PARAM; RETURN(tSTACKSIZE);    }
<START>(Compress|COMPRESS|compress)		{ BEGIN PARAM; RETURN(tCOMPRESS);    }

<START>(OverlayDefs|OVERLAYDEFS|overlaydefs)	{ BEGIN PARAM; RETURN(tOVERLAYDEFS);  }
<START>(OverlayTable|OVERLAYTABLE|overlaytable)	{ BEGIN PARAM; RETURN(tOVERLAYTABLE); }
<START>(Ltd|LTD|ltd)(OverlayDefs|OVERLAYDEFS|overlaydefs)	{ BEGIN PARAM; RETURN(tLTDOVERLAYDEFS); }
<START>(Ltd|LTD|ltd)(OverlayTable|OVERLAYTABLE|overlaytable)	{ BEGIN PARAM; RETURN(tLTDOVERLAYTABLE); }
<START>(Sur?ffix|SUR?FFIX|sur?ffix)		{ BEGIN PARAM; RETURN(tSUFFIX);       }
<START>(Flx|FLX|flx)(Sur?ffix|SUR?FFIX|sur?ffix)	{ BEGIN PARAM; RETURN(tFLXSUFFIX); }
<START>(Ltd|LTD|ltd)(Sur?ffix|SUR?FFIX|sur?ffix)	{ BEGIN PARAM; RETURN(tLTDSUFFIX); }

<PARAM>(ITCM|itcm|Itcm)		{
	spec_yylval.string = strdup("ITCM");
	RETURN(tSTRING_ID);
}

<PARAM>(DTCM|dtcm|Dtcm)		{
	spec_yylval.string = strdup("DTCM");
	RETURN(tSTRING_ID);
}

<PARAM>(WRAM|wram|Wram)		{
	spec_yylval.string = strdup("WRAM");
	RETURN(tSTRING_ID);
}

<PARAM>(SMART|smart|Smart)		{
	spec_yylval.string = strdup("SMART");
	RETURN(tSTRING_ID);
}

<PARAM>(EXCLUSION|exclusion|Exclusion)		{
	spec_yylval.string = strdup("EXCLUSION");
	RETURN(tSTRING_ID);
}

<START>\{		{ BEGIN PARAM; RETURN(tBEGIN);        }
<START>\}		{ BEGIN PARAM; RETURN(tEND);          }

<PARAM>{ws}*\\{ws}*{nl}	{ BEGIN PARAM_CONTINUE; }     	/* Continue to next line */

<PARAM>\({identifier}*\,{ws}[A-Za-z_][A-Za-z_0-9\.]*\.[oa]\)	{
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_FORCE);
}

<PARAM>\({ws}*\.[a-zA-Z_0-9]*{ws}*\)	{
	spec_yylval.string = getSection();
	RETURN(tSECTIONNAME);
}

<PARAM>\({ws}*\.[a-zA-Z_0-9]*\.[a-zA-Z_0-9]*{ws}*\)	{
	spec_yylval.string = getSection();
	RETURN(tSECTIONNAME);
}

<PARAM>\*		{
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_STAR);
}

<PARAM>GROUP\([^\)]*\)	{
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_GROUP);
}

<PARAM>OBJECT\([^\)]*\)	{
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_FUNCTION);
}

{decnum}		|
{octnum}		|
{hexnum}		{			/* Integer Value */
	spec_yylval.integer = strtol((char*)spec_yytext, (char**)NULL, 0);
	RETURN(tNUMBER);
}

{identifier}		{		       	/* String for identifier */
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_ID);
}

{qstring}		{		       	/* String quated by "" */
	spec_yytext[spec_yyleng-1] = '\0';
	spec_yylval.string = strdup(spec_yytext+1);
	RETURN(tSTRING_QUATED);
}

{string_wo_ws}		{			/* String without white space */
	spec_yylval.string = strdup(spec_yytext);
	RETURN(tSTRING_NOSPACE);
}

<PARAM><<EOF>>		{ BEGIN INITIAL; RETURN(tNL); }	/* End of File */

{nl}			{ BEGIN INITIAL; RETURN(tNL); }	/* Newline     */
{ws}+			;	       			/* White space */
.			{ RETURN(spec_yytext[0]);     }	/* Default     */

%%

/*============================================================================
 *  Utilities
 */
void  spec_yyerror( const char* str )
{
    int  line = spec_yylineno;
    if ( spec_yytext[0] == '\n' ) line --;
    fprintf( stderr, "makelcf: line %d: %s\n", line, str );
}

static char* getSection( void )
{
    char*  p;
    char*  head = NULL;

    for ( p = spec_yytext; *p; p ++ )
    {
	switch ( *p )
	{
	case '(':
	    while ( *p == ' ' ) p ++;
	    head = p + 1;
	    break;
	    
	case ')':
	case ' ':
	    if ( head )
            {
	        *p = '\0';
		return strdup( head );
            }
	    break;
	    
	default:
	    break;
	}
    }
    return strdup( "*" );
}

static void  YYText_ResolveDevVal( void )
{
    int   i;
    char* s = ResolveDefVal( spec_yytext );
    
    for ( i = strlen(s)-1; i >= 0; i -- )
    {
    	unput( s[i] );
    }
    free(s);
    return;
}

/*============================================================================
 *  PARSE SPEC FILE
 */
int  ParseSpecFile( const char* filename )
{
    FILE *fp;
    int   result;
    
    if ( NULL == ( fp = fopen( filename, "r" ) ) )
    {
	fprintf( stderr, "makelcf: cannot open %s\n", filename );
	return 2;
    }
    
    spec_yyin = fp;
    result = spec_yyparse();
    fclose( fp );
    if ( result ) return 1;

    return CheckSpec() ? 0 : 1;
}
