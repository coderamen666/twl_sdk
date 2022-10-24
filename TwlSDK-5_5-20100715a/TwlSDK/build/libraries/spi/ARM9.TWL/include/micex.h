/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mic
  File:     micex.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_LIBRARIES_SPI_ARM9_MICEX_H_
#define TWL_LIBRARIES_SPI_ARM9_MICEX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/spi/ARM9/mic.h>

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
/* Lock definition for exclusive processing of asynchronous functions */
typedef enum MICLock
{
    MIC_LOCK_OFF = 0,                  // Unlock status
    MIC_LOCK_ON,                       // Lock status
    MIC_LOCK_MAX
}
MICLock;

/* Work structure */
typedef struct MICWork
{
    MICLock lock;                      // Exclusive lock
    MICCallback callback;              // For saving an asynchronous callback function
    void   *callbackArg;               // For saving arguments to the callback function
    MICResult commonResult;            // For saving asynchronous function processing results
    MICCallback full;                  // For saving the sampling completion callback
    void   *fullArg;                   // For saving arguments to the completion callback function
    void   *dst_buf;                   // For saving a storage area for individual sampling results

}
MICWork;

/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/
MICWork*    MICi_GetSysWork(void);

MICResult   MICEXi_StartLimitedSampling(const MICAutoParam* param);
MICResult   MICEXi_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg);
MICResult   MICEXi_StopLimitedSampling(void);
MICResult   MICEXi_StopLimitedSamplingAsync(MICCallback callback, void* arg);
MICResult   MICEXi_AdjustLimitedSampling(u32 rate);
MICResult   MICEXi_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg);


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_LIBRARIES_SPI_ARM9_MICEX_H_ */
