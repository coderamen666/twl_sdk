/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - include
  File:     math/fft.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MATH_FFT_H_
#define NITRO_MATH_FFT_H_

#include <nitro/types.h>
#include <nitro/fx/fx.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Type Definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Function Declarations
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         MATH_MakeFFTSinTable

  Description:  Creates sine table used in fast Fourier transform.

  Arguments:    sinTable - Pointer to location storing 2^nShift * 3/4  sine table
                nShift - log2 of the number of data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_MakeFFTSinTable(fx16 *sinTable, u32 nShift);

/*---------------------------------------------------------------------------*
  Name:         MATHi_FFT

  Description:  Internal function that performs fast Fourier transform.

  Arguments:    data - Data to transform, with even-numbered data elements being real and odd-numbered data elements being imaginary.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATHi_FFT(fx32 *data, u32 nShift, const fx16 *sinTable);

/*---------------------------------------------------------------------------*
  Name:         MATHi_IFFT

  Description:  Internal function that reverses the fast Fourier transform.

  Arguments:    data - Data to transform, with even-numbered data elements being real and odd-numbered data elements being imaginary.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATHi_IFFT(fx32 *data, u32 nShift, const fx16 *sinTable);

/*---------------------------------------------------------------------------*
  Name:         MATH_FFT

  Description:  Performs fast Fourier transform.

  Arguments:    data - Data to transform, with even-numbered data elements being real and odd-numbered data elements being imaginary.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_FFT(fx32 *data, u32 nShift, const fx16 *sinTable);

/*---------------------------------------------------------------------------*
  Name:         MATH_IFFT

  Description:  Performs the inverse transformation of fast Fourier transform.

  Arguments:    data - Data to transform, with even-numbered data elements being real and odd-numbered data elements being imaginary.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_IFFT(fx32 *data, u32 nShift, const fx16 *sinTable);

/*---------------------------------------------------------------------------*
  Name:         MATH_FFTReal

  Description:  Performs fast Fourier transform.

  Arguments:    data - Data that includes only real number data.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_FFTReal(fx32 *data, u32 nShift, const fx16 *sinTable, const fx16 *sinTable2);

/*---------------------------------------------------------------------------*
  Name:         MATH_IFFTReal

  Description:  Performs the inverse transformation of fast Fourier transform.

  Arguments:    data - Data that includes only real number data.
                       The transformation result will be returned overwritten.
                nShift - log2 of the number of data
                sinTable - Sine value table based on a circle divided into equal sections; the number of sections is the number of data elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_IFFTReal(fx32 *data, u32 nShift, const fx16 *sinTable, const fx16 *sinTable2);


#ifdef __cplusplus
}/* extern "C" */
#endif

/* NITRO_MATH_FFT_H_ */
#endif
