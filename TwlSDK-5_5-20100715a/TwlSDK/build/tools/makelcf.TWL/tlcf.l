%{
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     tlcf.l

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-03-16#$
  $Rev: 11311 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<string.h>
#include	"makelcf.h"
#include	"tlcf.tab.h"

#define	RETURN(x) { debug_printf( "%s [%s]\n", #x, tlcf_yytext ); return(x); }

BOOL  GetLoopStackLevel( void );

void  tlcf_yyerror( const char* str )
{
    if ( tlcf_yytext[0] >= ' ' )
    {
        fprintf( stderr, "makelcf: line %d: %s \'%s\'\n", tlcf_yylineno, str, tlcf_yytext );
    }
    else
    {
        fprintf( stderr, "makelcf: line %d: %s\n", tlcf_yylineno, str );
    }
}

static char* getSection( const char* default_section )
{
    char*  head = NULL;
    char*  p;
    
    for ( p = tlcf_yytext; *p; p ++ )
    {
	switch ( *p )
	{
	case '=':
	case ':':
	    if ( !head )
	    {
		head = p;
	    }
	    break;
	    
	case '>':
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
    return default_section ? strdup( default_section ) : NULL;
}

%}
st			[ \t]*
ed			[ \t]*\r?\n
sec			(=[.a-zA-Z_0-9]*)?
mod			(:[htre])?
flt			(:[f])?
identifier		[A-Za-z_][A-Za-z_0-9]*
equation		(==|!=|\.EQ\.|\.NE\.|\.GT\.|\.GE\.|\.LT\.|\.LE\.)
value			[^>\n]*

STATIC_CRT0_OBJ	\<STATIC\.CRT0\.OBJECT{sec}\>

STATIC_OBJ		\<STATIC\.OBJECT{sec}{mod}\>
F_S_OBJS		\<FOREACH\.STATIC\.OBJECTS{sec}{flt}\>
E_S_OBJS		\<END\.STATIC\.OBJECTS{sec}{flt}\>
FOR_STATIC_OBJS_1	^{st}{F_S_OBJS}{ed}
FOR_STATIC_OBJS_2	{F_S_OBJS}
END_STATIC_OBJS_1	^{st}{E_S_OBJS}{ed}
END_STATIC_OBJS_2	{E_S_OBJS}

STATIC_LIB		\<STATIC\.LIBRARY{sec}{mod}\>
F_S_LIBS		\<FOREACH\.STATIC\.LIBRARIES{sec}{flt}\>
E_S_LIBS		\<END\.STATIC\.LIBRARIES{sec}{flt}\>
FOR_STATIC_LIBS_1	^{st}{F_S_LIBS}{ed}
FOR_STATIC_LIBS_2	{F_S_LIBS}
END_STATIC_LIBS_1	^{st}{E_S_LIBS}{ed}
END_STATIC_LIBS_2	{E_S_LIBS}

STATIC_SYM		\<STATIC\.SEARCHSYMBOL\>
F_S_SYMS		\<FOREACH\.STATIC\.SEARCHSYMBOLS\>
E_S_SYMS		\<END\.STATIC\.SEARCHSYMBOLS\>
FOR_STATIC_SYMS_1	^{st}{F_S_SYMS}{ed}
FOR_STATIC_SYMS_2	{F_S_SYMS}
END_STATIC_SYMS_1	^{st}{E_S_SYMS}{ed}
END_STATIC_SYMS_2	{E_S_SYMS}

STATIC_FOR		\<STATIC\.FORCE\>
F_S_FORS		\<FOREACH\.STATIC\.FORCES\>
E_S_FORS		\<END\.STATIC\.FORCES\>
FOR_STATIC_FORS_1	^{st}{F_S_FORS}{ed}
FOR_STATIC_FORS_2	{F_S_FORS}
END_STATIC_FORS_1	^{st}{E_S_FORS}{ed}
END_STATIC_FORS_2	{E_S_FORS}

F_ATLS			\<FOREACH\.AUTOLOADS{sec}{flt}\>
E_ATLS			\<END\.AUTOLOADS{sec}{flt}\>
FOR_AUTOLOADS_1		^{st}{F_ATLS}{ed}
FOR_AUTOLOADS_2		{F_ATLS}
END_AUTOLOADS_1		^{st}{E_ATLS}{ed}
END_AUTOLOADS_2		{E_ATLS}

AUTOLOAD_OBJ		\<AUTOLOAD\.OBJECT{sec}{mod}\>
F_A_OBJS		\<FOREACH\.AUTOLOAD\.OBJECTS{sec}{flt}\>
E_A_OBJS		\<END\.AUTOLOAD\.OBJECTS{sec}{flt}\>
FOR_AUTOLOAD_OBJS_1	^{st}{F_A_OBJS}{ed}
FOR_AUTOLOAD_OBJS_2	{F_A_OBJS}
END_AUTOLOAD_OBJS_1	^{st}{E_A_OBJS}{ed}
END_AUTOLOAD_OBJS_2	{E_A_OBJS}

AUTOLOAD_LIB		\<AUTOLOAD\.LIBRARY{sec}{mod}\>
F_A_LIBS		\<FOREACH\.AUTOLOAD\.LIBRARIES{sec}{flt}\>
E_A_LIBS		\<END\.AUTOLOAD\.LIBRARIES{sec}{flt}\>
FOR_AUTOLOAD_LIBS_1	^{st}{F_A_LIBS}{ed}
FOR_AUTOLOAD_LIBS_2	{F_A_LIBS}
END_AUTOLOAD_LIBS_1	^{st}{E_A_LIBS}{ed}
END_AUTOLOAD_LIBS_2	{E_A_LIBS}

AUTOLOAD_SYM		\<AUTOLOAD\.SEARCHSYMBOL\>
F_A_SYMS		\<FOREACH\.AUTOLOAD\.SEARCHSYMBOLS\>
E_A_SYMS		\<END\.AUTOLOAD\.SEARCHSYMBOLS\>
FOR_AUTOLOAD_SYMS_1	^{st}{F_A_SYMS}{ed}
FOR_AUTOLOAD_SYMS_2	{F_A_SYMS}
END_AUTOLOAD_SYMS_1	^{st}{E_A_SYMS}{ed}
END_AUTOLOAD_SYMS_2	{E_A_SYMS}

AUTOLOAD_FOR		\<AUTOLOAD\.FORCE\>
F_A_FORS		\<FOREACH\.AUTOLOAD\.FORCES\>
E_A_FORS		\<END\.AUTOLOAD\.FORCES\>
FOR_AUTOLOAD_FORS_1	^{st}{F_A_FORS}{ed}
FOR_AUTOLOAD_FORS_2	{F_A_FORS}
END_AUTOLOAD_FORS_1	^{st}{E_A_FORS}{ed}
END_AUTOLOAD_FORS_2	{E_A_FORS}

F_OVLS			\<FOREACH\.OVERLAYS{sec}{flt}\>
E_OVLS			\<END\.OVERLAYS{sec}{flt}\>
FOR_OVERLAYS_1		^{st}{F_OVLS}{ed}
FOR_OVERLAYS_2		{F_OVLS}
END_OVERLAYS_1		^{st}{E_OVLS}{ed}
END_OVERLAYS_2		{E_OVLS}

OVERLAY_OBJ		\<OVERLAY\.OBJECT{sec}{mod}\>
F_O_OBJS		\<FOREACH\.OVERLAY\.OBJECTS{sec}{flt}\>
E_O_OBJS		\<END\.OVERLAY\.OBJECTS{sec}{flt}\>
FOR_OVERLAY_OBJS_1	^{st}{F_O_OBJS}{ed}
FOR_OVERLAY_OBJS_2	{F_O_OBJS}
END_OVERLAY_OBJS_1	^{st}{E_O_OBJS}{ed}
END_OVERLAY_OBJS_2	{E_O_OBJS}

OVERLAY_LIB		\<OVERLAY\.LIBRARY{sec}{mod}\>
F_O_LIBS		\<FOREACH\.OVERLAY\.LIBRARIES{sec}{flt}\>
E_O_LIBS		\<END\.OVERLAY\.LIBRARIES{sec}{flt}\>
FOR_OVERLAY_LIBS_1	^{st}{F_O_LIBS}{ed}
FOR_OVERLAY_LIBS_2	{F_O_LIBS}
END_OVERLAY_LIBS_1	^{st}{E_O_LIBS}{ed}
END_OVERLAY_LIBS_2	{E_O_LIBS}

OVERLAY_SYM		\<OVERLAY\.SEARCHSYMBOL\>
F_O_SYMS		\<FOREACH\.OVERLAY\.SEARCHSYMBOLS\>
E_O_SYMS		\<END\.OVERLAY\.SEARCHSYMBOLS\>
FOR_OVERLAY_SYMS_1	^{st}{F_O_SYMS}{ed}
FOR_OVERLAY_SYMS_2	{F_O_SYMS}
END_OVERLAY_SYMS_1	^{st}{E_O_SYMS}{ed}
END_OVERLAY_SYMS_2	{E_O_SYMS}

OVERLAY_FOR		\<OVERLAY\.FORCE\>
F_O_FORS		\<FOREACH\.OVERLAY\.FORCES\>
E_O_FORS		\<END\.OVERLAY\.FORCES\>
FOR_OVERLAY_FORS_1	^{st}{F_O_FORS}{ed}
FOR_OVERLAY_FORS_2	{F_O_FORS}
END_OVERLAY_FORS_1	^{st}{E_O_FORS}{ed}
END_OVERLAY_FORS_2	{E_O_FORS}

F_LTDATLS		\<FOREACH\.LTDAUTOLOADS{sec}{flt}\>
E_LTDATLS		\<END\.LTDAUTOLOADS{sec}{flt}\>
FOR_LTDAUTOLOADS_1		^{st}{F_LTDATLS}{ed}
FOR_LTDAUTOLOADS_2		{F_LTDATLS}
END_LTDAUTOLOADS_1		^{st}{E_LTDATLS}{ed}
END_LTDAUTOLOADS_2		{E_LTDATLS}

LTDAUTOLOAD_OBJ		\<LTDAUTOLOAD\.OBJECT{sec}{mod}\>
F_LA_OBJS		\<FOREACH\.LTDAUTOLOAD\.OBJECTS{sec}{flt}\>
E_LA_OBJS		\<END\.LTDAUTOLOAD\.OBJECTS{sec}{flt}\>
FOR_LTDAUTOLOAD_OBJS_1	^{st}{F_LA_OBJS}{ed}
FOR_LTDAUTOLOAD_OBJS_2	{F_LA_OBJS}
END_LTDAUTOLOAD_OBJS_1	^{st}{E_LA_OBJS}{ed}
END_LTDAUTOLOAD_OBJS_2	{E_LA_OBJS}

LTDAUTOLOAD_LIB		\<LTDAUTOLOAD\.LIBRARY{sec}{mod}\>
F_LA_LIBS		\<FOREACH\.LTDAUTOLOAD\.LIBRARIES{sec}{flt}\>
E_LA_LIBS		\<END\.LTDAUTOLOAD\.LIBRARIES{sec}{flt}\>
FOR_LTDAUTOLOAD_LIBS_1	^{st}{F_LA_LIBS}{ed}
FOR_LTDAUTOLOAD_LIBS_2	{F_LA_LIBS}
END_LTDAUTOLOAD_LIBS_1	^{st}{E_LA_LIBS}{ed}
END_LTDAUTOLOAD_LIBS_2	{E_LA_LIBS}

LTDAUTOLOAD_SYM		\<LTDAUTOLOAD\.SEARCHSYMBOL\>
F_LA_SYMS		\<FOREACH\.LTDAUTOLOAD\.SEARCHSYMBOLS\>
E_LA_SYMS		\<END\.LTDAUTOLOAD\.SEARCHSYMBOLS\>
FOR_LTDAUTOLOAD_SYMS_1	^{st}{F_LA_SYMS}{ed}
FOR_LTDAUTOLOAD_SYMS_2	{F_LA_SYMS}
END_LTDAUTOLOAD_SYMS_1	^{st}{E_LA_SYMS}{ed}
END_LTDAUTOLOAD_SYMS_2	{E_LA_SYMS}

LTDAUTOLOAD_FOR		\<LTDAUTOLOAD\.FORCE\>
F_LA_FORS		\<FOREACH\.LTDAUTOLOAD\.FORCES\>
E_LA_FORS		\<END\.LTDAUTOLOAD\.FORCES\>
FOR_LTDAUTOLOAD_FORS_1	^{st}{F_LA_FORS}{ed}
FOR_LTDAUTOLOAD_FORS_2	{F_LA_FORS}
END_LTDAUTOLOAD_FORS_1	^{st}{E_LA_FORS}{ed}
END_LTDAUTOLOAD_FORS_2	{E_LA_FORS}

F_LTDOVLS		\<FOREACH\.LTDOVERLAYS{sec}{flt}\>
E_LTDOVLS		\<END\.LTDOVERLAYS{sec}{flt}\>
FOR_LTDOVERLAYS_1		^{st}{F_LTDOVLS}{ed}
FOR_LTDOVERLAYS_2		{F_LTDOVLS}
END_LTDOVERLAYS_1		^{st}{E_LTDOVLS}{ed}
END_LTDOVERLAYS_2		{E_LTDOVLS}

LTDOVERLAY_OBJ		\<LTDOVERLAY\.OBJECT{sec}{mod}\>
F_LO_OBJS		\<FOREACH\.LTDOVERLAY\.OBJECTS{sec}{flt}\>
E_LO_OBJS		\<END\.LTDOVERLAY\.OBJECTS{sec}{flt}\>
FOR_LTDOVERLAY_OBJS_1	^{st}{F_LO_OBJS}{ed}
FOR_LTDOVERLAY_OBJS_2	{F_LO_OBJS}
END_LTDOVERLAY_OBJS_1	^{st}{E_LO_OBJS}{ed}
END_LTDOVERLAY_OBJS_2	{E_LO_OBJS}

LTDOVERLAY_LIB		\<LTDOVERLAY\.LIBRARY{sec}{mod}\>
F_LO_LIBS		\<FOREACH\.LTDOVERLAY\.LIBRARIES{sec}{flt}\>
E_LO_LIBS		\<END\.LTDOVERLAY\.LIBRARIES{sec}{flt}\>
FOR_LTDOVERLAY_LIBS_1	^{st}{F_LO_LIBS}{ed}
FOR_LTDOVERLAY_LIBS_2	{F_LO_LIBS}
END_LTDOVERLAY_LIBS_1	^{st}{E_LO_LIBS}{ed}
END_LTDOVERLAY_LIBS_2	{E_LO_LIBS}

LTDOVERLAY_SYM		\<LTDOVERLAY\.SEARCHSYMBOL\>
F_LO_SYMS		\<FOREACH\.LTDOVERLAY\.SEARCHSYMBOLS\>
E_LO_SYMS		\<END\.LTDOVERLAY\.SEARCHSYMBOLS\>
FOR_LTDOVERLAY_SYMS_1	^{st}{F_LO_SYMS}{ed}
FOR_LTDOVERLAY_SYMS_2	{F_LO_SYMS}
END_LTDOVERLAY_SYMS_1	^{st}{E_LO_SYMS}{ed}
END_LTDOVERLAY_SYMS_2	{E_LO_SYMS}

LTDOVERLAY_FOR		\<LTDOVERLAY\.FORCE\>
F_LO_FORS		\<FOREACH\.LTDOVERLAY\.FORCES\>
E_LO_FORS		\<END\.LTDOVERLAY\.FORCES\>
FOR_LTDOVERLAY_FORS_1	^{st}{F_LO_FORS}{ed}
FOR_LTDOVERLAY_FORS_2	{F_LO_FORS}
END_LTDOVERLAY_FORS_1	^{st}{E_LO_FORS}{ed}
END_LTDOVERLAY_FORS_2	{E_LO_FORS}


%x IFCOND1
%x IFCOND2
%x IFCOND3
%option pointer
%option yylineno
%option noyywrap
%%

 //===========================================================================
 //    LCF TOKENS
 //===========================================================================

\<IF\.				{ BEGIN IFCOND1; RETURN(tIF_OPEN);  }
<IFCOND1>{identifier}		{ BEGIN IFCOND2; tlcf_yylval.string = strdup(tlcf_yytext); RETURN(tIF_ID);    }
<IFCOND2>{equation}		{ BEGIN IFCOND3; tlcf_yylval.string = strdup(tlcf_yytext); RETURN(tIF_COMP);  }
<IFCOND3>{value}		{                tlcf_yylval.string = strdup(tlcf_yytext); RETURN(tIF_VALUE); }
<IFCOND3>\>			{ BEGIN INITIAL; RETURN(tIF_CLOSE); }
\<ELSE\>			{ RETURN(tIF_ELSE);  }
\<ENDIF\>			{ RETURN(tIF_ENDIF); }

\<STATIC\.NAME\>		{ RETURN(tSTATIC_NAME);            }
\<STATIC\.ADDRESS\>		{ RETURN(tSTATIC_ADDRESS);         }
\<STATIC\.STACKSIZE\>		{ RETURN(tSTATIC_STACKSIZE);       }
\<STATIC\.IRQSTACKSIZE\>	{ RETURN(tSTATIC_IRQSTACKSIZE);    }

{STATIC_CRT0_OBJ}       { tlcf_yylval.string = getSection("=*"); RETURN(tSTATIC_CRT0_OBJECT); }

{STATIC_OBJ}			{ tlcf_yylval.string = getSection(NULL); RETURN(tSTATIC_OBJECT);          }
{FOR_STATIC_OBJS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_STATIC_OBJECTS); }
{FOR_STATIC_OBJS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_STATIC_OBJECTS); }
{END_STATIC_OBJS_1}		{ RETURN(tEND_STATIC_OBJECTS);     }
{END_STATIC_OBJS_2}		{ RETURN(tEND_STATIC_OBJECTS);     }

{STATIC_LIB}			{ tlcf_yylval.string = getSection(NULL); RETURN(tSTATIC_LIBRARY);           }
{FOR_STATIC_LIBS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_STATIC_LIBRARIES); }
{FOR_STATIC_LIBS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_STATIC_LIBRARIES); }
{END_STATIC_LIBS_1}		{ RETURN(tEND_STATIC_LIBRARIES);   }
{END_STATIC_LIBS_2}		{ RETURN(tEND_STATIC_LIBRARIES);   }

{STATIC_SYM}			{ RETURN(tSTATIC_SEARCHSYMBOL);          }
{FOR_STATIC_SYMS_1}		{ RETURN(tFOREACH_STATIC_SEARCHSYMBOLS); }
{FOR_STATIC_SYMS_2}		{ RETURN(tFOREACH_STATIC_SEARCHSYMBOLS); }
{END_STATIC_SYMS_1}		{ RETURN(tEND_STATIC_SEARCHSYMBOLS); }
{END_STATIC_SYMS_2}		{ RETURN(tEND_STATIC_SEARCHSYMBOLS); }

{STATIC_FOR}			{ RETURN(tSTATIC_FORCE);          }
{FOR_STATIC_FORS_1}		{ RETURN(tFOREACH_STATIC_FORCES); }
{FOR_STATIC_FORS_2}		{ RETURN(tFOREACH_STATIC_FORCES); }
{END_STATIC_FORS_1}		{ RETURN(tEND_STATIC_FORCES); }
{END_STATIC_FORS_2}		{ RETURN(tEND_STATIC_FORCES); }

\<AUTOLOAD\.ID\>		{ RETURN(tAUTOLOAD_ID);            }
\<AUTOLOAD\.NAME\>		{ RETURN(tAUTOLOAD_NAME);          }
\<AUTOLOAD\.ADDRESS\>		{ RETURN(tAUTOLOAD_ADDRESS);       }
\<NUMBER\.AUTOLOADS\>		{ RETURN(tNUMBER_AUTOLOADS);       }

{FOR_AUTOLOADS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOADS); }
{FOR_AUTOLOADS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOADS); }
{END_AUTOLOADS_1}		{ RETURN(tEND_AUTOLOADS);          }
{END_AUTOLOADS_2}		{ RETURN(tEND_AUTOLOADS);          }

{AUTOLOAD_OBJ}			{ tlcf_yylval.string = getSection(NULL); RETURN(tAUTOLOAD_OBJECT);          }
{FOR_AUTOLOAD_OBJS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOAD_OBJECTS); }
{FOR_AUTOLOAD_OBJS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOAD_OBJECTS); }
{END_AUTOLOAD_OBJS_1}		{ RETURN(tEND_AUTOLOAD_OBJECTS);   }
{END_AUTOLOAD_OBJS_2}		{ RETURN(tEND_AUTOLOAD_OBJECTS);   }

{AUTOLOAD_LIB}			{ tlcf_yylval.string = getSection(NULL); RETURN(tAUTOLOAD_LIBRARY);           }
{FOR_AUTOLOAD_LIBS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOAD_LIBRARIES); }
{FOR_AUTOLOAD_LIBS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_AUTOLOAD_LIBRARIES); }
{END_AUTOLOAD_LIBS_1}		{ RETURN(tEND_AUTOLOAD_LIBRARIES); }
{END_AUTOLOAD_LIBS_2}		{ RETURN(tEND_AUTOLOAD_LIBRARIES); }

{AUTOLOAD_SYM}			{ RETURN(tAUTOLOAD_SEARCHSYMBOL);          }
{FOR_AUTOLOAD_SYMS_1}		{ RETURN(tFOREACH_AUTOLOAD_SEARCHSYMBOLS); }
{FOR_AUTOLOAD_SYMS_2}		{ RETURN(tFOREACH_AUTOLOAD_SEARCHSYMBOLS); }
{END_AUTOLOAD_SYMS_1}		{ RETURN(tEND_AUTOLOAD_SEARCHSYMBOLS); }
{END_AUTOLOAD_SYMS_2}		{ RETURN(tEND_AUTOLOAD_SEARCHSYMBOLS); }

{AUTOLOAD_FOR}			{ RETURN(tAUTOLOAD_FORCE);          }
{FOR_AUTOLOAD_FORS_1}		{ RETURN(tFOREACH_AUTOLOAD_FORCES); }
{FOR_AUTOLOAD_FORS_2}		{ RETURN(tFOREACH_AUTOLOAD_FORCES); }
{END_AUTOLOAD_FORS_1}		{ RETURN(tEND_AUTOLOAD_FORCES); }
{END_AUTOLOAD_FORS_2}		{ RETURN(tEND_AUTOLOAD_FORCES); }

\<OVERLAY\.ID\>			{ RETURN(tOVERLAY_ID);             }
\<OVERLAY\.NAME\>		{ RETURN(tOVERLAY_NAME);           }
\<OVERLAY\.GROUP\>		{ RETURN(tOVERLAY_GROUP);          }
\<OVERLAY\.ADDRESS\>		{ RETURN(tOVERLAY_ADDRESS);        }
\<OVERLAY\.COMPRESS\>		{ RETURN(tOVERLAY_COMPRESS);        }
\<NUMBER\.OVERLAYS\>		{ RETURN(tNUMBER_OVERLAYS);        }
\<NUMBER_OVERLAYS\>		{ RETURN(tNUMBER_OVERLAYS);        }

{FOR_OVERLAYS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAYS); }
{FOR_OVERLAYS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAYS); }
{END_OVERLAYS_1}		{ RETURN(tEND_OVERLAYS);           }
{END_OVERLAYS_2}		{ RETURN(tEND_OVERLAYS);           }

{OVERLAY_OBJ}			{ tlcf_yylval.string = getSection(NULL); RETURN(tOVERLAY_OBJECT);          }
{FOR_OVERLAY_OBJS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAY_OBJECTS); }
{FOR_OVERLAY_OBJS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAY_OBJECTS); }
{END_OVERLAY_OBJS_1}		{ RETURN(tEND_OVERLAY_OBJECTS);    }
{END_OVERLAY_OBJS_2}		{ RETURN(tEND_OVERLAY_OBJECTS);    }

{OVERLAY_LIB}			{ tlcf_yylval.string = getSection(NULL); RETURN(tOVERLAY_LIBRARY);           }
{FOR_OVERLAY_LIBS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAY_LIBRARIES); }
{FOR_OVERLAY_LIBS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_OVERLAY_LIBRARIES); }
{END_OVERLAY_LIBS_1}		{ RETURN(tEND_OVERLAY_LIBRARIES);  }
{END_OVERLAY_LIBS_2}		{ RETURN(tEND_OVERLAY_LIBRARIES);  }

{OVERLAY_SYM}			{ RETURN(tOVERLAY_SEARCHSYMBOL);          }
{FOR_OVERLAY_SYMS_1}		{ RETURN(tFOREACH_OVERLAY_SEARCHSYMBOLS); }
{FOR_OVERLAY_SYMS_2}		{ RETURN(tFOREACH_OVERLAY_SEARCHSYMBOLS); }
{END_OVERLAY_SYMS_1}		{ RETURN(tEND_OVERLAY_SEARCHSYMBOLS);  }
{END_OVERLAY_SYMS_2}		{ RETURN(tEND_OVERLAY_SEARCHSYMBOLS);  }

{OVERLAY_FOR}			{ RETURN(tOVERLAY_FORCE);          }
{FOR_OVERLAY_FORS_1}		{ RETURN(tFOREACH_OVERLAY_FORCES); }
{FOR_OVERLAY_FORS_2}		{ RETURN(tFOREACH_OVERLAY_FORCES); }
{END_OVERLAY_FORS_1}		{ RETURN(tEND_OVERLAY_FORCES);  }
{END_OVERLAY_FORS_2}		{ RETURN(tEND_OVERLAY_FORCES);  }

\<LTDAUTOLOAD\.ID\>			{ RETURN(tLTDAUTOLOAD_ID);            }
\<LTDAUTOLOAD\.NAME\>		{ RETURN(tLTDAUTOLOAD_NAME);          }
\<LTDAUTOLOAD\.ADDRESS\>	{ RETURN(tLTDAUTOLOAD_ADDRESS);       }
\<NUMBER\.LTDAUTOLOADS\>	{ RETURN(tNUMBER_LTDAUTOLOADS);       }

{FOR_LTDAUTOLOADS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOADS); }
{FOR_LTDAUTOLOADS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOADS); }
{END_LTDAUTOLOADS_1}		{ RETURN(tEND_LTDAUTOLOADS);          }
{END_LTDAUTOLOADS_2}		{ RETURN(tEND_LTDAUTOLOADS);          }

{LTDAUTOLOAD_OBJ}			{ tlcf_yylval.string = getSection(NULL); RETURN(tLTDAUTOLOAD_OBJECT);          }
{FOR_LTDAUTOLOAD_OBJS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOAD_OBJECTS); }
{FOR_LTDAUTOLOAD_OBJS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOAD_OBJECTS); }
{END_LTDAUTOLOAD_OBJS_1}		{ RETURN(tEND_LTDAUTOLOAD_OBJECTS);   }
{END_LTDAUTOLOAD_OBJS_2}		{ RETURN(tEND_LTDAUTOLOAD_OBJECTS);   }

{LTDAUTOLOAD_LIB}			{ tlcf_yylval.string = getSection(NULL); RETURN(tLTDAUTOLOAD_LIBRARY);           }
{FOR_LTDAUTOLOAD_LIBS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOAD_LIBRARIES); }
{FOR_LTDAUTOLOAD_LIBS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDAUTOLOAD_LIBRARIES); }
{END_LTDAUTOLOAD_LIBS_1}		{ RETURN(tEND_LTDAUTOLOAD_LIBRARIES); }
{END_LTDAUTOLOAD_LIBS_2}		{ RETURN(tEND_LTDAUTOLOAD_LIBRARIES); }

{LTDAUTOLOAD_SYM}			{ RETURN(tLTDAUTOLOAD_SEARCHSYMBOL);          }
{FOR_LTDAUTOLOAD_SYMS_1}		{ RETURN(tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS); }
{FOR_LTDAUTOLOAD_SYMS_2}		{ RETURN(tFOREACH_LTDAUTOLOAD_SEARCHSYMBOLS); }
{END_LTDAUTOLOAD_SYMS_1}		{ RETURN(tEND_LTDAUTOLOAD_SEARCHSYMBOLS); }
{END_LTDAUTOLOAD_SYMS_2}		{ RETURN(tEND_LTDAUTOLOAD_SEARCHSYMBOLS); }

{LTDAUTOLOAD_FOR}			{ RETURN(tLTDAUTOLOAD_FORCE);          }
{FOR_LTDAUTOLOAD_FORS_1}		{ RETURN(tFOREACH_LTDAUTOLOAD_FORCES); }
{FOR_LTDAUTOLOAD_FORS_2}		{ RETURN(tFOREACH_LTDAUTOLOAD_FORCES); }
{END_LTDAUTOLOAD_FORS_1}		{ RETURN(tEND_LTDAUTOLOAD_FORCES); }
{END_LTDAUTOLOAD_FORS_2}		{ RETURN(tEND_LTDAUTOLOAD_FORCES); }

\<LTDOVERLAY\.ID\>			{ RETURN(tLTDOVERLAY_ID);             }
\<LTDOVERLAY\.NAME\>		{ RETURN(tLTDOVERLAY_NAME);           }
\<LTDOVERLAY\.GROUP\>		{ RETURN(tLTDOVERLAY_GROUP);          }
\<LTDOVERLAY\.ADDRESS\>		{ RETURN(tLTDOVERLAY_ADDRESS);        }
\<LTDOVERLAY\.COMPRESS\>	{ RETURN(tLTDOVERLAY_COMPRESS);       }
\<NUMBER\.LTDOVERLAYS\>		{ RETURN(tNUMBER_LTDOVERLAYS);        }
\<NUMBER_LTDOVERLAYS\>		{ RETURN(tNUMBER_LTDOVERLAYS);        }

{FOR_LTDOVERLAYS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAYS); }
{FOR_LTDOVERLAYS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAYS); }
{END_LTDOVERLAYS_1}		{ RETURN(tEND_LTDOVERLAYS);           }
{END_LTDOVERLAYS_2}		{ RETURN(tEND_LTDOVERLAYS);           }

{LTDOVERLAY_OBJ}			{ tlcf_yylval.string = getSection(NULL); RETURN(tLTDOVERLAY_OBJECT);          }
{FOR_LTDOVERLAY_OBJS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAY_OBJECTS); }
{FOR_LTDOVERLAY_OBJS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAY_OBJECTS); }
{END_LTDOVERLAY_OBJS_1}		{ RETURN(tEND_LTDOVERLAY_OBJECTS);    }
{END_LTDOVERLAY_OBJS_2}		{ RETURN(tEND_LTDOVERLAY_OBJECTS);    }

{LTDOVERLAY_LIB}			{ tlcf_yylval.string = getSection(NULL); RETURN(tLTDOVERLAY_LIBRARY);           }
{FOR_LTDOVERLAY_LIBS_1}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAY_LIBRARIES); }
{FOR_LTDOVERLAY_LIBS_2}		{ tlcf_yylval.string = getSection("=*"); RETURN(tFOREACH_LTDOVERLAY_LIBRARIES); }
{END_LTDOVERLAY_LIBS_1}		{ RETURN(tEND_LTDOVERLAY_LIBRARIES);  }
{END_LTDOVERLAY_LIBS_2}		{ RETURN(tEND_LTDOVERLAY_LIBRARIES);  }

{LTDOVERLAY_SYM}			{ RETURN(tLTDOVERLAY_SEARCHSYMBOL);          }
{FOR_LTDOVERLAY_SYMS_1}		{ RETURN(tFOREACH_LTDOVERLAY_SEARCHSYMBOLS); }
{FOR_LTDOVERLAY_SYMS_2}		{ RETURN(tFOREACH_LTDOVERLAY_SEARCHSYMBOLS); }
{END_LTDOVERLAY_SYMS_1}		{ RETURN(tEND_LTDOVERLAY_SEARCHSYMBOLS);  }
{END_LTDOVERLAY_SYMS_2}		{ RETURN(tEND_LTDOVERLAY_SEARCHSYMBOLS);  }

{LTDOVERLAY_FOR}			{ RETURN(tLTDOVERLAY_FORCE);          }
{FOR_LTDOVERLAY_FORS_1}		{ RETURN(tFOREACH_LTDOVERLAY_FORCES); }
{FOR_LTDOVERLAY_FORS_2}		{ RETURN(tFOREACH_LTDOVERLAY_FORCES); }
{END_LTDOVERLAY_FORS_1}		{ RETURN(tEND_LTDOVERLAY_FORCES);  }
{END_LTDOVERLAY_FORS_2}		{ RETURN(tEND_LTDOVERLAY_FORCES);  }

\<PROPERTY\.OVERLAYDEFS\>	{ RETURN(tPROPERTY_OVERLAYDEFS);   }
\<PROPERTY\.OVERLAYTABLE\>	{ RETURN(tPROPERTY_OVERLAYTABLE);  }
\<PROPERTY\.LTDOVERLAYDEFS\>	{ RETURN(tPROPERTY_LTDOVERLAYDEFS); }
\<PROPERTY\.LTDOVERLAYTABLE\>	{ RETURN(tPROPERTY_LTDOVERLAYTABLE); }
\<PROPERTY\.SUR?FFIX\>		{ RETURN(tPROPERTY_SUFFIX);        }
\<PROPERTY\.FLXSUR?FFIX\>	{ RETURN(tPROPERTY_FLXSUFFIX); }
\<PROPERTY\.LTDSUR?FFIX\>	{ RETURN(tPROPERTY_LTDSUFFIX); }

[^<\n]*\n?		|
\<			{
	tlcf_yylval.string = strdup(tlcf_yytext);
	RETURN(tSTRING);
}

.   			{ RETURN(tlcf_yytext[0]); }

%%

/*============================================================================
 *  PARSE TLCF FILE
 */
int  ParseTlcfFile( const char* filename )
{
    FILE *fp;
    int   result;
    
    if ( NULL == ( fp = fopen( filename, "r" ) ) )
    {
	fprintf( stderr, "Cannot open %s\n", filename );
	return 2;
    }
    
    tlcf_yyin  = fp;
    result = tlcf_yyparse();
    fclose( fp );

    if ( GetLoopStackLevel( ) > 0 )
    {
	tlcf_yyerror( "Unmatched foreach-end pair" );
	return 1;
    }

    return result ? 1 : 0;
}