/*! \file   detect.h
    \brief  Functions used to detect the presence of input (i.e., received)
            signals.
 */
#ifndef __DETECT_H__
#define __DETECT_H__

#include <cpcommon.h>

#include "csignal.h"
#include "fir_filter.h"
#include "gold_code.h"

#include "csignal_error_codes.h"

/*! \def    csignal_error_code csignal_calculate_thresholds  (
              UINT32               in_spreading_code_length,
              FLOAT64*             in_spreading_code,
              fir_passband_filter* in_wideband_filter,
              fir_passband_filter* in_narrowband_filter,
              USIZE                in_signal_length,
              FLOAT64*             in_signal,
              UINT32               in_decimation,
              USIZE*               out_thresholds_length,
              FLOAT64**            out_thresholds
            )
    \brief  This function executes the following:
 
            input -> filter (wideband) -> despread (with spreading_code ) ->
            filter( narrowband ) -> calculate energy -> store energy,
 
            where input is in_signal starting at offsets i + decimation (i is
            from the range [0,in_signal_length-in_spreading_code_lenght], and
            store energy stores the energy of the aforementioned process at
            index i / decimation of thresholds.
 
    \note   out_thresholds must be freed by the caller.
 
    \param  in_spreading_code_length  The number of elements in 
                                      in_spreading_code.
    \param  in_spreading_code The input signal containing the spreading code.
    \param  in_wideband_filter  The wideband filter to apply to the whole signal.
    \param  in_narrowband_filter  The filter to apply after despreading.
    \param  in_siganl_length  The number of elements in in_signal.
    \param  in_signal The signal to despread and calculate the energy of.
    \param  out_thresholds_length The number of elements in out_thresholds, or
                                  0 if an error occurrs.
    \param  out_thresholds  The energy in in_signal after filtering and
                            despreading.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
 */
csignal_error_code
csignal_calculate_thresholds  (
                               UINT32               in_spreading_code_length,
                               FLOAT64*             in_spreading_code,
                               fir_passband_filter* in_wideband_filter,
                               fir_passband_filter* in_narrowband_filter,
                               USIZE                in_signal_length,
                               FLOAT64*             in_signal,
                               UINT32               in_decimation,
                               USIZE*               out_thresholds_length,
                               FLOAT64**            out_thresholds
                               );

#endif  /*  __DETECT_H__  */
