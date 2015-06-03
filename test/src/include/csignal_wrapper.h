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

PyObject*
python_test (
             gold_code*  io_gold_code,
             size_t      in_chip_duration,
             PyObject* in_signal
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
              int     in_symbol,
              int     in_constellation_size,
              int     in_sample_rate,
              size_t  in_symbol_duration,
              int     in_baseband_pulse_amplitude,
              float   in_carrier_frequency
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
  int     in_symbol,
  int     in_constellation_size,
  int     in_sample_rate,
  size_t  in_symbol_duration,
  int     in_baseband_pulse_amplitude,
  float   in_carrier_frequency
);

/*! \fn     PyObject* python_get_symbol (
              csignal_symbol_tracker* in_symbol_tracker,
              size_t                  in_number_of_bits
            )
    \brief  Wrapper around csignal_get_symbol that checks for errors returned
            by the function and either returns 'None' or a symbol if an error
            is found or not, respectively. See a more complete description of
            the parameters and errors by reading the csignal_get_symbol
            documentation.

    \param  in_symbol_tracker The symbol tracker to read data symbols from
    \param  in_number_of_bits The number of bits in each symbol
    \return None if an error is found, or a symbol
 */
PyObject*
python_get_symbol(
  csignal_symbol_tracker* in_symbol_tracker,
  size_t                  in_number_of_bits
);

/*! \fn     csignal_symbol_tracker* python_initialize_symbol_tracker (
              PyObject* in_data
            )
    \brief  Creates a new symbol tracker object that points to the data in
            in_data. Symbols of variable bit-length will be read from in_data.

    \param  in_data The data buffer to read symbols from.
    \return A newly created csignal_symbol_tracker if one could be created.
            NULL otherwise.
*/
csignal_symbol_tracker*
python_initialize_symbol_tracker(
PyObject* in_data
);

#endif  /*  __CSIGNAL_WRAPPER_H__ */
