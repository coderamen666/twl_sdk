/*---------------------------------------------------------------------------*
  Header File
 *---------------------------------------------------------------------------*/

#ifndef PATTERN_DICT_H_
#define PATTERN_DICT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/prc/types.h>


#define PDIC_WHOLE_POINT_NUM             840
#define PDIC_WHOLE_STROKE_NUM            148
#define PDIC_PATTERN_NUM                 97
#define PDIC_CODE_NUM                    50
#define PDIC_NORMALIZE_SIZE              64


extern PRCPrototypeList PrototypeList;
extern char *PatternName[PDIC_CODE_NUM];


#ifdef __cplusplus
} /* extern "C" */
#endif

/* PATTERN_DICT_H_ */
#endif

/*---------------------------------------------------------------------------*
  End of File
 *---------------------------------------------------------------------------*/
