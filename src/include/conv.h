/*! \file   conv.h
    \brief  Header file for convolution of signals.
 */
#ifndef __CONV_H__
#define __CONV_H__

#include <cpcommon.h>

#include "csignal_error_codes.h"

csignal_error_code
convolve  (
           USIZE       in_signal_one_length,
           FLOAT64*    in_signal_one,
           USIZE       in_signal_two_length,
           FLOAT64*    in_signal_two,
           USIZE*      out_filtered_signal_length,
           FLOAT64**   out_filtered_signal
           );

#endif  /*  __CONV_H__  */
