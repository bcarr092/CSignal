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
  fir_passband_filter*  in_filter,
  PyObject*             in_signal
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

/*! \fn     int python_write_FLOAT_wav (
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
int
python_write_FLOAT_wav(
  PyObject* in_file_name,
  size_t    in_number_of_channels,
  int       in_sample_rate,
  size_t    in_number_of_samples,
  PyObject* in_samples
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
                       unsigned int     in_symbol,
                       unsigned int     in_constellation_size,
                       unsigned int     in_sample_rate,
                       size_t           in_symbol_duration,
                       int              in_baseband_pulse_amplitude,
                       float            in_carrier_frequency
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

PyObject*
python_convolve (
                 PyObject* in_signal_one,
                 PyObject* in_signal_two
                 );

#endif  /*  __CSIGNAL_WRAPPER_H__ */
