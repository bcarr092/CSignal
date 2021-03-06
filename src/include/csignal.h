/*! \file   csignal.h
    \brief  This is the main include file for the csignal library. This is the
            only header file that needs to be included by external libraries/
            binaries.
 
    \note   In this library all signals are stored as FLOAT64 values (double)
            as are all filter values as well as functions performed on signals
            (e.g. FFT). It is the responsibility of the caller to convert the
            signals into the format that they require.
 
    \author Brent Carrara
 */
#ifndef __CSIGNAL_H__
#define __CSIGNAL_H__

#include <cpcommon.h>

#include "wav.h"
#include "spreading_code.h"
#include "gold_code.h"
#include "fir_filter.h"
#include "kaiser_filter.h"
#include "fft.h"
#include "bit_packer.h"
#include "bit_stream.h"
#include "conv.h"
#include "detect.h"

#include "csignal_error_codes.h"

/*! \fn     void csignal_initialize( void )
    \brief  Initializes the csignal library.
 
 */
void
csignal_initialize( void );

/*! \fn     void csignal_terminate( void )
    \brief  Terminates the csignal library.
 
 */
void
csignal_terminate( void );

/*! \fn     csignal_error_code csignal_spread_signal (
              gold_code* io_gold_code,
              UINT32     in_chip_duration,
              USIZE      in_signal_size,
              FLOAT64*   io_signal
            )
    \brief  This function will spread the signal in io_signal according to the
            spreading code generated by io_gold_code using chips of duration
            in_chip_duration samples.
 
    \param  io_gold_code  The spreading sequence to use
    \param  in_chip_duration  The number of smaples in each chip. Must evenly
                              divide in_signal_size.
    \param  in_signal_size  The number fo samples in io_signal. Must be evenly
                            divisible by in_chip_duration.
    \param  io_signal The signal to spread
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc and csignal_get_gold_code for other possible
            errors):
 
            CPC_ERROR_CODE_NULL_POINTER If io_gold_code or io_signal are null
            CPC_ERROR_CODE_INVALID_PARAMETER If in_chip_duration does not evenly
                                             divide in_signal_size or
                                             in_chip_duration is zero.
 */
csignal_error_code
csignal_spread_signal (
                       gold_code* io_gold_code,
                       UINT32     in_chip_duration,
                       USIZE      in_signal_size,
                       FLOAT64*   io_signal
                       );

/*! \fn     csignal_error_code csignal_multiply_signal  (
              USIZE       in_signal_one_length,
              FLOAT64*    in_signal_one,
              USIZE       in_signal_two_length,
              FLOAT64*    in_signal_two,
              USIZE*      out_signal_length,
              FLOAT64**   out_signal
            )
    \brief  Multipy two signals and put the result in a newly created array
            out_signal. out_signal is malloc'd in this function and returned
            to be freed by the caller.
 
    \note   If out_signal_length is non-zero and out_signal is non-Null this
            function will create a new buffer. The caller must free out_signal
            if it is not null if not set beforehand.
 
    \param  in_signal_one_length  The number of elements in signal_one.
    \param  in_signal_one The first of two signals to be multiplied.
    \param  in_signal_two_length  The number of elements in signal two.
    \param  in_signal_two The second of two signal to be multiplied.
    \param  out_signal_length The number of elements in out_signal. Should be
                              equal to in_signal_one_length, or zero.
    \param  out_signal A newly created array containing the values of signal_one
                        and signal_two multiplied, or NULL if an error occurrs.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER If the length of signal one and two
                                              are not equal.
 */
csignal_error_code
csignal_multiply_signal  (
                          USIZE       in_signal_one_length,
                          FLOAT64*    in_signal_one,
                          USIZE       in_signal_two_length,
                          FLOAT64*    in_signal_two,
                          USIZE*      out_signal_length,
                          FLOAT64**   out_signal
                          );

/*! \fn     csignal_error_code csignal_calculate_energy  (
              USIZE    in_signal_length,
              FLOAT64* in_signal,
              FLOAT64* out_energy
            )
    \brief  Calculates the signal's energy, E = sum( x[ i ]^2 ).
 
    \param  in_signal_length  The number of elements in in_signal.
    \param  in_signal The signal whose energy is to be calculated.
    \param  out_energy  The energy in in_signal is equal to the sum of elements
                        squared.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
 */
csignal_error_code
csignal_calculate_energy  (
                           USIZE    in_signal_length,
                           FLOAT64* in_signal,
                           FLOAT64* out_energy
                           );

/*! \fn     csignal_error_code csignal_sum_signal(
              USIZE    in_signal_length,
              FLOAT64* in_signal,
              FLOAT64  in_scalar,
              FLOAT64* out_sum
            )
    \brief  Performs a scalar multiplication of all elements in in_signal by
            the factor in_scalar and sums the scaled components of in_signal.
 
    \note   This function uses a correlator demodulator to determine if the
            signal corresponds to the symbols [ -1, 1 ]. The sufficient
            statistics are calculated as r_i = sum( in_signal * i ), where i is
            either -1 or 1. The largest r_i is chosen as the correct decision
            and i is returned.
 
    \param  in_signal_length  The number of elements in in_signal.
    \param  in_signal The signal to be scaled and summed.
    \param  in_scalar The multiplicative scalar factor to apply to all elements
                      of in_signal.
    \param  out_sum The scaled sum of in_signal.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
 */
csignal_error_code
csignal_sum_signal(
                   USIZE    in_signal_length,
                   FLOAT64* in_signal,
                   FLOAT64  in_scalar,
                   FLOAT64* out_sum
                   );

/*! \fn     csignal_error_code csignal_modulate_symbol (
              UINT32   in_symbol,
              UINT32   in_constellation_size,
              FLOAT64* out_symbol_inphase,
              FLOAT64* out_symbol_quadrature
            )
    \brief  Modulates a data symbol into inphase and quadrature components.
            Let the data symbol value be m, the constellation size be M. This
            function returns the following:
 
            signal_inphase  = cos( 2 * pi * m' / M )
            signal_outphase = sin( 2 * pi * m' / M )
 
    \note   The symbol (in_symbol), m, is first converted to a Gray Code, m',
            before it is modulated.
 
    \param  in_symbol The symbol to be modulated, m, it must be < than
                      in_constellation_size
    \param  in_constellation_size The maximum number of possible symbols. Must
                                  be > 0
    \param  out_symbol_inphase  The inphase value returned to the
                                caller if no error is detected.
    \param  out_symbol_quadrature The quadrature value returned to
                                  the caller if no error is detected.
    \return Returns NO_ERROR upon succesful exection or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If any parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER
              If in_symbol >= in_constellation_size
 */
csignal_error_code
csignal_modulate_symbol (
                         UINT32   in_symbol,
                         UINT32   in_constellation_size,
                         FLOAT64* out_symbol_inphase,
                         FLOAT64* out_symbol_quadrature
                         );

/*! \fn     csignal_error_code csignal_modulate_BFSK_symbol  (
              UINT32     in_symbol,
              UINT32     in_samples_per_symbol,
              UINT32     in_sample_rate,
              FLOAT32    in_carrier_frequency,
              USIZE*     out_signal_length,
              FLOAT64**  out_signal_inphase,
              FLOAT64**  out_signal_quadrature
            )
    \brief  Modualtes a data symbol into inphase and quadrature components.
            Let the data symbol value be m, the constellation size be M, the
            sample rate be Fs, the carrier be Fc, then this fuction returns:
 
            inphase    = cos( 2 * pi * m * Fd ) * cos( 2 * pi * Fc )
            quadrature = sin( 2 * pi * m * Fd ) * sin( 2 * pi * Fc ),
 
            where Fd is the delta frequency used to separate the frequencies
            for each symbol into orthogonal waveforms and is calculated as
            follows:
 
            Fd = ( Fs / in_samples_per_symbol ).
 
    \note The inphase and quadrature signals are generated as if they were
          sampled at a rate of Fs.
 
    \param  in_symbol The symbol to be modulated (must be 0 or 1)
    \param  in_samples_per_symbol The number of samples used to represent a
                                  symbol.
    \param  in_sample_rate  The sample rate the waveform is to be generated at.
    \param  in_carrier_frequency  The carrier frequency of the signal.
    \param  out_signal_length The number of elements in inphase and quadrature.
    \param  out_signal_inphase  Samples from the inphase signal described above.
    \param  out_signal_quadrature Samples from the quadrature signal described
                                  above.
    \return Returns NO_ERROR upon succesful exection or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If any parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If in_symbol > 1 and if
                                              in_samples_per_symbol or
                                              in_sample_rate are 0.
 */
csignal_error_code
csignal_modulate_BFSK_symbol  (
                               UINT32     in_symbol,
                               UINT32     in_samples_per_symbol,
                               UINT32     in_sample_rate,
                               FLOAT32    in_carrier_frequency,
                               UINT32     in_separation_intervals,
                               UINT32     in_symbol_expansion_factor,
                               USIZE*     out_signal_length,
                               FLOAT64**  out_signal_inphase,
                               FLOAT64**  out_signal_quadrature
                               );

csignal_error_code
csignal_BFSK_determine_frequencies  (
                                     UINT32   in_samples_per_symbol,
                                     UINT32   in_sample_rate,
                                     FLOAT32  in_carrier_frequency,
                                     UINT32   in_separation_intervals,
                                     FLOAT64* out_symbol_0_frequency,
                                     FLOAT64* out_symbol_1_frequency,
                                     FLOAT64* out_delta_frequency,
                                     FLOAT64* out_bandwidth
                                     );

#endif  /*  __CSIGNAL_H__ */
