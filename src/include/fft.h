/*! \file   fft.h
    \brief  This file contains the headers for to calculate the FFT of a real-
            valued signal. The algorightm to calculate the FFT is from the
            following reference:

            Numerical recipes in C: the art of scientific computing by Press, et
            al. (Chapter 6)
            http://www2.units.it/ipl/students_area/imm2/files/Numerical_Recipes.pdf
 */
#ifndef __FFT_H__
#define __FFT_H__

#include <cpcommon.h>

#include "csignal_error_codes.h"

/*! \fn     csignal_error_code csignal_calculate_FFT (
              UINT32     in_signal_length,
              INT16*     in_signal,
              UINT32*    out_fft_length,
              FLOAT32**  out_fft
            )
    \brief  Calculates the FFT of the input signal, in_signal.
 
    \param  in_signal_length  The number of elements in in_signal.
    \param  in_signal The signal whose FFT is to be calculated.
    \param  out_fft_length  The number of elements returned in out_fft. This
                            will always be 2 times the power of 2 larger than or
                            equal to in_signal_length. It is 2x larger because
                            a real and imaginary component are returned in
                            adjacent indices in out_fft.
    \param  out_fft  The FFT of in_signal. The positive frequency components are
                    returned first starting with DC f=0Hz all the way up to the
                    sampling rate that in_signal was generated at divided by 2.
                    Then the negative freqeuncy components follow.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for more error codes):
 
            CPC_ERROR_CODE_NULL_POINTER If in_signal, out_signal_lenght or
                                        out_signal are null.
 */
csignal_error_code
csignal_calculate_FFT (
                       UINT32     in_signal_length,
                       INT16*     in_signal,
                       UINT32*    out_fft_length,
                       FLOAT32**  out_fft
                       );

#endif  /*  __FFT_H__ */
