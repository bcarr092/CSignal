#ifndef __CSIGNAL_WRAPPER_H__
#define __CSIGNAL_WRAPPER_H__

#include <Python.h>

#include <csignal.h>

/*! \fn     PyObject* python_calculate_FFT  (
              PyObject* in_signal
            )
    \brief  Calculates the FFT of in_signal and returns a list of complex
            values in the return value. See the function definition for
            csignal_calculated_FFT for more details on return codes.

    \return A list of Python Complex values is returned or None if an error
             occurrs.
 */
PyObject*
python_calculate_FFT(
                     PyObject* in_signal
                     );

/*! \fn     PyObject* python_filter_signal  (
              fir_passband_filter*  in_filter,
              PyObject*             in_signal
            )
    \brief  Filters in_signal using the filter defined in in_filter. Returns
            a Python list of filtered samples. See the documentation for
            csignal_filter_signal for more information regarding the behaviour
            of the filter function and the desired parameters.
 */
PyObject*
python_filter_signal(
                     fir_passband_filter*   in_filter,
                     PyObject*              in_signal
                     );

/*! \fn     fir_passband_filter* python_initialize_kaiser_filter (
              float in_first_stopband,
              float in_first_passband,
              float in_second_passband,
              float in_second_stopband,
              float in_passband_attenuation,
              float in_stopband_attenuation,
              int   in_sampling_frequency
            )
    \brief  This is the python wrapper to create a Kaiser filter. Please see
            the documentation in kaiser_filter.h for a complete description of
            the permitted values.
*/
fir_passband_filter*
python_initialize_kaiser_filter(
                                float in_first_stopband,
                                float in_first_passband,
                                float in_second_passband,
                                float in_second_stopband,
                                float in_passband_attenuation,
                                float in_stopband_attenuation,
                                int   in_sampling_frequency
                                );

/*! \fn     fir_passband_filter* python_initialize_kaiser_lowpass_filter (
              float in_passband,
              float in_stopband,
              float in_passband_attenuation,
              float in_stopband_attenuation,
              int   in_sampling_frequency
            )
    \brief  This is the python wrapper to create a Kaiser low pass filter. 
            Please see the documentation in kaiser_filter.h for a complete
            description of the permitted values.
*/
fir_passband_filter*
python_initialize_kaiser_lowpass_filter(
                                        float in_passband,
                                        float in_stopband,
                                        float in_passband_attenuation,
                                        float in_stopband_attenuation,
                                        int   in_sampling_frequency
                                        );

/*! \fn     PyObject* python_get_gold_code (
              gold_code*  in_gold_code,
              size_t      in_number_of_bits
            )
    \brief  Requests in_number_of_bits from the LFSRs defined by
            in_gold_code. For full documentation the parameters for this
            function please see gold_code.h. The only additional check
            done in this wrapper is to ensure in_number_of_bits is
            non-negative.

    \return Returns a python list of unsigned char values or None if an error
            occured.
 */
PyObject*
python_get_gold_code(
                     gold_code*  in_gold_code,
                     size_t      in_number_of_bits
                     );

/*! \fn     gold_code* python_initialize_gold_code  (
              int in_degree,
              unsigned long in_generator_polynomial_1,
              unsigned long in_generator_polynomial_2,
              unsigned long in_initial_state_1,
              unsigned long in_initial_state_2
            )
    \brief  Initializes gold code LFSRs generator polynomials and an intial
            states. Both the generators and initial states need to be
            configured properly for the gold code generator to function
            correctly. Please see the documentation in gold_code.h and
            spreading_code.h
*/
gold_code*
python_initialize_gold_code(
  unsigned int in_degree,
  unsigned long in_generator_polynomial_1,
  unsigned long in_generator_polynomial_2,
  unsigned long in_initial_state_1,
  unsigned long in_initial_state_2
);

/*! \fn     PyObject* python_get_spreading_code  (
              spreading_code* in_spreading_code,
              size_t          in_number_of_bits
            )
    \brief  Requests in_number_of_bits from the LFSR defined by
            in_spreading_code. For full documentation the parameters for this
            function please see spreading_code.h. The only additional check
            done in this wrapper is to ensure in_number_of_bits is
            non-negative.

    \return Returns a python list of unsigned char values or None if an error
            occured.
 */
PyObject*
python_get_spreading_code(
  spreading_code* in_spreading_code,
  size_t          in_number_of_bits
);

/*! \fn     spreading_code* python_initialize_spreading_code  (
              int   in_degree,
              unsigned long  in_generator_polynomial,
              unsigned long  in_initial_state
            )
    \brief  Initializes the LFSR with a generator polynomial and an intial
            state. Both the generator and initial state need to be configured
            properly for the LFSR to function. Please see the documentation
            in spreading_code.h and spreading_code.c
*/
spreading_code*
python_initialize_spreading_code(
  unsigned int in_degree,
  unsigned long in_generator_polynomial,
  unsigned long in_initial_state
);

/*! \fn     PyObject* python_spread_signal (
              gold_code*  io_gold_code,
              size_t      in_chip_duration,
              PyObject*   in_signal
            )
    \brief  Spreads the signal in in_signal by the code generated by
            io_gold_code. See csiganl_spread_signal for all the details
            related to the implementation and valid parameter values.

    \return A Python list of signed short values.
 */
PyObject*
python_spread_signal(
  gold_code*  io_gold_code,
  size_t      in_chip_duration,
  PyObject*   in_signal
);

/*! \fn     CPC_BOOL python_write_FLOAT_wav (
              PyObject* in_file_name,
              size_t    in_number_of_channels,
              int       in_sample_rate,
              size_t    in_number_of_samples,
              PyObject* in_samples
            )
    \brief  The python wrapper for the csignal_write_FLOAT_wav function. For
            a complete description of the parameters please see the
            documentation for csignal_write_FLOAT_wav.

    \return CPC_TRUE if the WAV file was successfully created, CPC_FALSE
            otherwise.
*/
CPC_BOOL
python_write_FLOAT_wav(
                       PyObject*  in_file_name,
                       USIZE      in_number_of_channels,
                       UINT32     in_sample_rate,
                       USIZE      in_number_of_samples,
                       PyObject*  in_samples
                       );
CPC_BOOL
python_write_LPCM_wav(
                       PyObject*  in_file_name,
                       USIZE      in_number_of_channels,
                       UINT32     in_sample_rate,
                       USIZE      in_number_of_samples,
                       PyObject*  in_samples
                       );

/*! \fn     PyObject* python_modulate_symbol  (
               unsigned int     in_symbol,
               unsigned int     in_constellation_size,
               unsigned int     in_sample_rate,
               size_t  in_symbol_duration,
               int     in_baseband_pulse_amplitude,
               unsigned float   in_carrier_frequency
            )
    \brief  Modulates in_symbol into a signal of length in_symbol_duration. For
            a complete description of each parameter and the function please
            see the documentation for csignal_modulate_symbol.

    \return Two arrarys of floats if no error occurs or None. One array for the
            inphase components of the signal (index 0) and the other for the
            quadrature components of the signal (index 1).
 */
PyObject*
python_modulate_symbol(
                       UINT32 in_symbol,
                       UINT32 in_constellation_size
                       );


/*! \fn     bit_packer* python_bit_packer_initialize( void )
    \brief  Initializes and returns a new bit packe struct.
 
    \return A newly created bit_packer struct or Python's None.
 */
bit_packer*
python_bit_packer_initialize( void );

/*! \fn     bit_stream* python_bit_stream_initialize  (
              PyObject*  in_data
            )
    \brief  Instatiates a new bit stream and returns it. The data in in_data
            is copied into the new bit_stream struct.
    
    \param  in_data A PyString containing data to be buffered by the bit stream.
    \return None if an error occurrs or a valid bit_stream object.
 */
bit_stream*
python_bit_stream_initialize  (
                               PyObject*  in_circular,
                               PyObject*  in_data
                               );

/*! \fn     bit_stream* python_bit_stream_initialize_from_bit_packer (
              bit_packer* in_bit_packer
            )
    \brief  Creates a new bit_strem with a ->data pointer that points to the
            ->data pointer of in_bit_packer. Note that this creates a stream
            that points to the data buffer of bit_packer and therefore has its
            dirty_bit set. Please see the notes for the function
            bit_stream_initialize_from_bit_packer.
 
    \param  in_bit_packer The bit_packer whose data pointer and data_length
                          will be copied to the newly created bit_stream.
    \return A newly created bit_stream that points to the data pointer of
            in_bit_packer.
 */
bit_stream*
python_bit_stream_initialize_from_bit_packer (
                                              PyObject*   in_circular,
                                              bit_packer* in_bit_packer
                                              );

/*! \fn     csignal_error_code csignal_error_code (
              PyObject*     in_data,
              bit_packer*   io_bit_packer
            )
            )
    \brief  Adds the data in in_data (PyString) to the bit packer.
 
    \param  in_data The data buffer that contains the bits to store. This is a
                    pointer to a buffer of unsigned chars. This must be a
                    PyString.
    \param  io_bit_packer  The bit packer struct to add the bits to.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc and bit_packer_add_bits for other possible
            errors):
 
            CPC_ERROR_CODE_NULL_POINTER If in_data or io_bit_packer is null
            CSIGNAL_ERROR_CODE_INVALID_TYPE If in_data is not a PyString
 */
csignal_error_code
python_bit_packer_add_bytes (
                             PyObject*    in_data,
                             bit_packer*  io_bit_packer
                             );

/*! \fn     PyObject* python_bit_packer_get_bytes (
              bit_packer* in_bit_packer
            )
    \brief  Copies the bytes in the bit packer to a PyString and returns the
            PyString.
 
    \param  in_bit_packer The bit packer whose data buffer is to be copied to
                          a newly created PyString.
    \return A newly created PyString (not null terminated) that contains the
            same data stored by the bit packer at the time of the call. None is
            returned if an error occurs.
 */
PyObject*
python_bit_packer_get_bytes (
                             bit_packer* in_bit_packer
                             );

/*! \fn     PyObject* python_bit_stream_get_bits  (
              bit_stream*  io_bit_stream,
              USIZE        in_number
            )
    \brief  Extracts up to in_number of bits from io_bit_stream and returns
            a PyString containing the bytes.
 
    \note The PyString returned is not NULL-terminated.
 
    \param  io_bit_stream The bit stream to extract bits from. The internal
                          pointers in the bit stream are modified.
    \param  in_number The number of bits to get from io_bit_stream.
    \return A PyString containing a buffer of bytes containing the requested
            bits. Note that the bits are stored in the MSBs of each byte.
 */
PyObject*
python_bit_stream_get_bits  (
                             bit_stream*  io_bit_stream,
                             USIZE        in_number
                             );

/*! \fn     PyObject* python_convolve (
              PyObject* in_signal_one,
              PyObject* in_signal_two
            )
    \brief  Convolves both input signals and returns a new Python list. The list
            is equal in length to the sum of lengths of the input params. This
            function returns a new reference.
 
    \param  in_signal_one The first of the two signals to be convolved.
    \param  in_signal_two The second of the two signals to be convolved.
    \return None on error. A new Python on list (new reference on success).
 */
PyObject*
python_convolve (
                 PyObject* in_signal_one,
                 PyObject* in_signal_two
                 );

/*! \fn     PyObject* python_csignal_multiply_signals (
              PyObject* in_signal_one,
              PyObject* in_signal_two
            )
    \brief  Multiplies signals one and two and returns a Python list containing
            the result.
 
    \param  in_signal_one The first signal to multiply.
    \param  in_signal_two The second signal to multiply.
    \return A Python list containing a component-wise multiplication of signals
            one and two.
 */
PyObject*
python_csignal_multiply_signals (
                                 PyObject* in_signal_one,
                                 PyObject* in_signal_two
                                 );

/*! \fn     PyObject* python_csignal_calculate_energy (
              PyObject* in_signal_one
            )
    \brief  Calcualtes the energy in the signal. Energy is calculated as taking
            the sum of the square of the components of in_signal.
 
    \param  in_signal The signal whose energy is to be calculated.
    \return A Python float containing the energy in signal.
 */
PyObject*
python_csignal_calculate_energy (
                                 PyObject* in_signal
                                 );

/*! \fn     PyObject* python_csignal_calculate_thresholds (
              PyObject*            in_spreading_code,
              fir_passband_filter* in_wideband_filter,
              fir_passband_filter* in_narrowband_filter,
              PyObject*            in_signal,
              UINT32               in_decimation
            )
    \brief  See the description for cisignal_calcualte_thresholds in detect.h.
 
    \param  in_spreading_code The input signal containing the spreading code.
    \param  in_wideband_filter  The wideband filter to apply to the whole signal.
    \param  in_narrowband_filter  The filter to apply after despreading.
    \param  in_signal The signal to despread and calculate the energy of.
    \return A Python List of floats containing the despread energy at the 
            specified offset. Note that the offset of the returned list is
            i / in_decimation.
 */
PyObject*
python_csignal_calculate_thresholds (
                                     PyObject*            in_spreading_code,
                                     PyObject*            in_signal,
                                     UINT32               in_decimation
                                     );

/*! \fn     PyObject* python_csignal_demodulate_binary_PAM (
             PyObject* in_signal
            )
    \brief  Demodultes in_signal into the values [ -1, 1 ] using a correlator
            demodulator. See csignal_demodulate_binary_PAM for a complete
            description of the behaviour of the function.
 
    \param  in_signal The signal to be demodulated.
    \return None if an error occurs or a PyInt containing one of [ -1, 1 ].
 */
PyObject*
python_csignal_demodulate_binary_PAM (
                                      PyObject* in_signal
                                      );

/*! \fn     PyObject* python_csignal_sum_signal (
              PyObject*  in_signal,
              FLOAT64    in_scalar
            )
    \brief  Sums in_signal, scaling each element by in_scalar.
 
    \param  in_signal The signal to be summed and scaled.
    \param  in_scalar The multiplicative scalar factor to apply to all elements
                      of in_signal.
    \return None if an error occurrs, a PyFloat containing the scaled, summed
            values of in_signal otherwise.
 */
PyObject*
python_csignal_sum_signal (
                           PyObject*  in_signal,
                           FLOAT64    in_scalar
                           );

/*! \fn     PyObject* python_bit_stream_peak  (
              bit_stream* in_bit_stream
            )
    \brief  Provides visibility into the unread bits in in_bit_stream without
            modifying the read/write pointers of the stream. See
            bit_stream_peak for a complete description of this function.
 
    \param  in_bit_stream The bit stream whose buffer is to be accessed.
    \return None if an error occurrs, or a three-element tuple containing the
            read bit offset (offset into returned buffer[ 0 ]), write bit offset
            (offset into returned buffer[ len( buffer ] - 1 ], and a buffer.
 */
PyObject*
python_bit_stream_peak  (
                         bit_stream* in_bit_stream
                         );

/*! \fn     PyObject* python_generate_carrier_signal  (
              UINT32   in_sample_rate,
              FLOAT32  in_carrier_frequency
            )
    \brief  Generates samples for a sinusoid with osciallatign frequency
            in_carrier_frequency. See csignal_generate_carrier_signal for more
            details.
 
    \param  in_sample_rate  The rate to sample points of the sinusoid with
                            frequency in_carrier_frequency.
    \param  in_carrier_frequency  The oscillating frequency of the sinusoid
                                  generated.
    \return A Python list containing double samples of a sinusoid oscillating at
            frequency in_carrier_frequency sampled at in_sample_rate.
 */
PyObject*
python_generate_carrier_signal  (
                                 UINT32   in_sample_rate,
                                 FLOAT32  in_carrier_frequency
                                 );

PyObject*
python_filter_get_group_delay (
                               fir_passband_filter*  in_filter
                               );

PyObject*
python_csignal_modulate_BFSK_symbol  (
                                      UINT32     in_symbol,
                                      UINT32     in_samples_per_symbol,
                                      UINT32     in_sample_rate,
                                      FLOAT32    in_carrier_frequency
                                      );

PyObject*
python_detect_calculate_energy (
                                PyObject*            in_signal,
                                PyObject*            in_spread_signal,
                                fir_passband_filter* in_narrowband_filter,
                                fir_passband_filter* in_lowpass_filter
                                );

PyObject*
python_detect_find_highest_energy_offset (
                                  PyObject*            in_signal,
                                  PyObject*            in_spread_signal,
                                  USIZE                in_number_of_tests,
                                  USIZE                in_step_size,
                                  fir_passband_filter* in_narrowband_filter,
                                  fir_passband_filter* in_lowpass_filter,
                                  FLOAT64              in_exhaustive_difference,
                                  UINT32               in_exhaustive_decimation,
                                  FLOAT64              in_threshold
                                          );

#endif  /*  __CSIGNAL_WRAPPER_H__ */
