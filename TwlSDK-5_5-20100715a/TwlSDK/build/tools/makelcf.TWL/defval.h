/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom
  File:     defval.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef	 DEFVAL_H_
#define	 DEFVAL_H_

#define	DEFVAL_DEFAULT_BUFFER_SIZE	(1024)

BOOL    AddDefValFromFile(char *filename);
void    AddDefVal(char *opt);
const char *SearchDefVal(const char *name);
const char *SearchDefValCleaned(const char *name);
char   *ResolveDefVal(char *str);
char   *ResolveStringModifier(const char *in_value, char modifier);

#endif //DEFVAL_H_
