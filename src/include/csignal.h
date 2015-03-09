/*! \file   csignal.h
    \brief  This is the main include file for the csignal library. This is the
            only header file that needs to be included by external libraries/
            binaries.
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

#include "csignal_error_codes.h"

/*! \var    csignal_symbol_tracker
    \brief  Struct definition for symbol tracker. A symbol tracker is used to
            iterate through the symbols in a buffer. Symbols of variable bit-
            length are read from index 0 to data_length.
 */
typedef struct csignal_symbol_tracker_t
{
  /*! \var    data
      \brief  The data buffer that contains the data bits
   */
  UCHAR* data;
  
  /*! \var    data_length
      \brief  The size of the data buffer
   */
  USIZE data_length;
  
  /*! \var    byte_offset
      \brief  A pointer to the buffer that will be read from in the next
              invocation of csignal_read_symbol
   */
  USIZE byte_offset;
  
  /*! \var    bit_offset
      \brief  A pointer to the bit in current_byte that will be read from in
              the next invocation of csignal_read_symbol
   */
  UCHAR bit_offset;
  
} csignal_symbol_tracker;

/*! \fn     cpc_error_code csignal_initialize_symbol_tracker  (
              UCHAR*                    in_data,
              USIZE                     in_data_length,
              csignal_symbol_tracker ** out_symbol_tracker
            )
    \brief  Initializes the symbol_tracker struct with a data buffer and sets
            both bit and byte offsets to 0.
 
    \note   in_data must not be freed until the destroy_symbol_tracker function
            is called. For efficiency purposes, i.e. not mallocing a new buffer
            and copying the contents of in_data into it, in_data is simply
            pointed to and not copied.
 
    \param  in_data The data buffer that contains the bits to read as symbols.
    \param  in_data_length  The length of in_data
    \param  out_symbol_tracker  If this function successfully completes this
                                parameter will point to a newly created
                                csignal_symbol_tracker. It will be NULL
                                otherwise.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If in_data is null
 */
csignal_error_code
csignal_initialize_symbol_tracker  (
                                    UCHAR*                    in_data,
                                    USIZE                     in_data_length,
                                    csignal_symbol_tracker ** out_symbol_tracker
                                    );

/*! \fn     cpc_error_code csignal_destroy_symbol_tracker  (
              csignal_symbol_tracker* io_symbol_tracker
            )
    \brief  Frees io_symbol_tracker
 
    \note   The buffer pointed to by the data member is not freed. It is the
            responsibility of the creator of data to destroy the buffer.
 
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_free for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER if io_symbol_tracker is null
 */
csignal_error_code
csignal_destroy_symbol_tracker  (
                                 csignal_symbol_tracker* io_symbol_tracker
                                 );

/*! \fn     csignal_error_code csignal_get_symbol  (
              csignal_symbol_tracker*  in_symbol_tracker,
              USIZE                    in_number_of_bits,
              UINT32*                  out_symbol
            )
    \brief  Returns a symbol from the symbol tracker of lenght in_number_of_bits
            . If k is the number of bytes than there are 2^k possible symbols
            that can be returned.
 
    \note   The in_number_of_bits should be consistent across all calls to this
            function. That is for every call to get_symbol in_number_of_bits
            should be static. The behaviour is undefined if in_number_of_bits
            varies between calls to get_symbol for the same symbol_tracker.
 
    \param  in_symbol_tracker The data structure to read symbols from. After a
                              a call to this function the byte and bit offsets
                              will be offset if a symbol is returned.
    \param  in_number_of_bits The number of bits to read for each symbol. Note
                              that this parameter should be static between calls
                              to this function for the same tracker.
    \param  out_symbol  The symbol read from the tracker that is returned to the
                        caller. If an error is returned by this function than
                        out_symbol is null.
    \return Returns NO_ERROR upon succesful execution or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If the tracker or symbol are null
            CSIGNAL_ERROR_CODE_NO_DATA  If the tracker is at the end of its data
                                        buffer.
 */
csignal_error_code
csignal_get_symbol  (
                     csignal_symbol_tracker*  in_symbol_tracker,
                     USIZE                    in_number_of_bits,
                     UINT32*                  out_symbol
                     );

/*! \fn     csignal_error_code csignal_modulate_symbol (
              UINT32   in_symbol,
              UINT32   in_constellation_size,
              UINT32   in_sample_rate,
              UINT32   in_symbol_duration,
              INT16    in_baseband_pulse_amplitude,
              FLOAT32  in_carrier_frequency,
              INT16*   out_signal
            )
    \brief  Modulates a symbol into a signal. Let the symbol value be m, the
            constellation size be M, the sample rate be f_s, each time t_i be a
            multiple of 1 / f_s, the carrier frequency be f_c, and the duration
            of each symbol (in terms of points) be T. This function returns the
            following:
 
            signal[ i ] = g( t ) * A_mc * cos( 2 * pi * f_c * t_i )
                            - g( t ) * A_ms * sin( 2 * pi * f_c * t_i )
 
            where,
      
            A_mc = cos( 2 * pi * m' / M )
            A_ms = sin( 2 * pi * m' / M )
 
            and g( t ) is a rectangular pulse with amplitude 
            in_baseband_pulse_amplitude for each of the T samples. The full
            array signal is returned and is of size T.
 
    \note   The symbol (in_symbol), m, is first converted to a Gray Code, m',
            before it is modulated.
 
    \param  in_symbol The symbol to be modulated, m, it must be < than
                      in_constellation_size
    \param  in_constellation_size The maximum number of possible symbols. Must
                                  be > 0
    \param  in_sample_rate  The rate at which the signal is sampled. Must be >
                            in_sample_rate
    \param  in_symbol_duration  The number of samples in the signal. Must be >
                                0
    \param  in_baseband_pulse_amplitude The shape of the pulse being modulated
    \param  in_carrier_frequency  The frequency of the carrier wave. Must be >
                                  0
    \param  out_signal  The array of samples returned to the caller if no error
                        is detected.
    \return Returns NO_ERROR upon succesful exection or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If out_signal is null.
            CPC_ERROR_CODE_INVALID_PARAMETER
              If in_symbol >= in_constellation_size
              If in_carrier_frequency < 0
              If in_sample_rate < 0
 */
csignal_error_code
csignal_modulate_symbol (
                         UINT32   in_symbol,
                         UINT32   in_constellation_size,
                         UINT32   in_sample_rate,
                         USIZE    in_symbol_duration,
                         INT16    in_baseband_pulse_amplitude,
                         FLOAT32  in_carrier_frequency,
                         INT16*   out_signal
                         );

/*! \fn     csignal_error_code csignal_spread_signal (
              gold_code* io_gold_code,
              UINT32     in_chip_duration,
              USIZE      in_signal_size,
              INT16*     io_signal
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
                       INT16*     io_signal
                       );

#endif  /*  __CSIGNAL_H__ */
