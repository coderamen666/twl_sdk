/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - init2env
  File:     init.l

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
%{
// Parser
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "init.tab.h"
extern int line_count;
%}

%%
\[                      return L_BRACKET;
\]                      return R_BRACKET;
\<                      return L_INEQUALITY;
\>                      return R_INEQUALITY;
\"                      return D_QUOTATION;

(U8|S8|U16|S16|U32|S32|U64|S64|BOOL) {
    init_yylval.letter = (char *)strdup(yytext);
    return TYPENUM;
}

(STRING) {
    init_yylval.letter = (char *)strdup(yytext);
    return TYPESTR;
}

(BINARY) {
    init_yylval.letter = (char *)strdup(yytext);
    return TYPEBIN;
}

[ \f\t]+                  ;

[\r\n]+ {
    line_count++;
}

[^ \f\n\r\t\[\]\"\<\>]+ {
    init_yylval.letter = (char *)strdup(yytext);
    return LETTER;
}
%%
