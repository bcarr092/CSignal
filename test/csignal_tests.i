%module csignal_tests

%inline %{

#include <csignal.h>

/*! \fn     static PyObject* python_calculate_FFT  (
              PyObject* in_signal
            )
    \brief  Calculates the FFT of in_signal and returns a list of complex
            values in the return value. See the function definition for
            csignal_calculated_FFT for more details on return codes.

    \return A list of Python Complex values is returned or None if an error
            occurrs.
 */
static PyObject*
python_calculate_FFT  (
  PyObject* in_signal
                      )
{
  PyObject* return_value = NULL;

  USIZE size        = 0;
  FLOAT64*  signal  = NULL;

  USIZE fft_length  = 0;
  FLOAT64* fft      = NULL;

  if( ! PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING  (
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
                    );
  }
  else
  {
    size    = PyList_Size( in_signal ); 

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** ) &signal, sizeof( FLOAT64 ) * size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[ i ] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        return_code = csignal_calculate_FFT( size, signal, &fft_length, &fft );

        if( CPC_ERROR_CODE_NO_ERROR == return_code )
        {
          PyObject* fft_list = PyList_New( fft_length / 2 );

          for( USIZE i = 0; i < fft_length; i += 2 )
          {
            PyObject* complex =
              PyComplex_FromDoubles( fft[ i ], fft[ i + 1 ] );

            if( 0 != PyList_SetItem ( fft_list, ( i / 2 ), complex ) )
            {
              PyErr_Print();
            }
          }

          if( PyList_Check( fft_list ) )
          {
            return_value = fft_list;
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );
          }
        }
      }
    }
  }

  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }

  if( NULL != fft )
  {
    cpc_safe_free( ( void** ) &fft );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

/*! \fn     static PyObject* python_filter_signal  (
              fir_passband_filter*  in_filter,
              PyObject*             in_signal
                                                    )
    \brief  Filters in_signal using the filter defined in in_filter. Returns
            a Python list of filtered samples. See the documentation for
            csignal_filter_signal for more information regarding the behaviour
            of the filter function and the desired parameters.
 */
static PyObject*
python_filter_signal  (
  fir_passband_filter*  in_filter,
  PyObject*             in_signal
                      )
{
  PyObject* return_value = NULL;

  USIZE size      = 0; 
  FLOAT64* signal = NULL;

  USIZE filtered_signal_length  = 0;
  FLOAT64* filtered_signal      = NULL;

  if( NULL == in_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter is null." );
  }
  else if( ! PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING  (
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
                    );
  }
  else
  {
    size = PyList_Size( in_signal ); 

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** ) &signal, sizeof( FLOAT64 ) * size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[ i ] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      CPC_LOG_BUFFER_FLOAT64  (
        CPC_LOG_LEVEL_TRACE,
        "Signal",
        signal,
        size,
        8
                      );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        return_code =
          csignal_filter_signal (
            in_filter,
            size,
            signal,
            &filtered_signal_length,
            &filtered_signal
                                );

        if( CPC_ERROR_CODE_NO_ERROR == return_code )
        {
          CPC_LOG_BUFFER_FLOAT64  (
            CPC_LOG_LEVEL_TRACE,
            "Filtered signal",
            filtered_signal,
            ( filtered_signal_length > 200 ? 200 : filtered_signal_length ),
            8
                                  );

          PyObject* filtered_list = PyList_New( filtered_signal_length );

          if( NULL == filtered_list )
          {
            cpc_safe_free( ( void** ) &filtered_signal );
          }
          else
          {
            for( USIZE i = 0; i < filtered_signal_length; i++ )
            {
              if  (
                    0
                    != PyList_SetItem (
                        filtered_list,
                        i,
                        Py_BuildValue( "d", filtered_signal[ i ] )
                                      )
                  )
              {
                PyErr_Print();
              }
            }

            if( PyList_Check( filtered_list ) )
            {
              return_value = filtered_list;
            }
            else
            {
              CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );
            }
          }   
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc signal: 0x%x.", return_value );
    }
  }

  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }

  if( NULL != filtered_signal )
  {
    cpc_safe_free( ( void** ) &filtered_signal );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

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
python_initialize_kaiser_filter (
  float in_first_stopband,
  float in_first_passband,
  float in_second_passband,
  float in_second_stopband,
  float in_passband_attenuation,
  float in_stopband_attenuation,
  int   in_sampling_frequency
                                )
{
  fir_passband_filter* filter = NULL;

  if( 0 >= in_sampling_frequency )
  {
    CPC_ERROR (
      "Sampling frequency (%d Hz) must be positive.",
      in_sampling_frequency
              );
  }
  else
  {
    csignal_error_code return_value =
      csignal_initialize_kaiser_filter  (
        in_first_stopband,
        in_first_passband,
        in_second_passband,
        in_second_stopband,
        in_passband_attenuation,
        in_stopband_attenuation,
        in_sampling_frequency,
        &filter
                                        );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR( "Could not initialize kaiser filter: 0x%x.", return_value );

      filter = NULL;
    }
  }

  return( filter );
}
/*! \fn     static PyObject* python_get_gold_code (
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
static PyObject*
python_get_gold_code  (
                        gold_code*  in_gold_code,
                        size_t      in_number_of_bits
                      )
{
  PyObject* return_value = NULL;

  USIZE size  = 0;
  UCHAR* code = NULL;

  if( NULL == in_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code was null." );
  }
  else
  {
    csignal_error_code return_code =
      csignal_get_gold_code (
        in_gold_code,
        in_number_of_bits,
        &size,
        &code
                            );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      PyObject* code_list = PyList_New( size );

      if( NULL == code_list )
      {
        CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Created a null list." );
      }
      else
      {
        for( USIZE i = 0; i < size; i++ )
        {
          if  (
                0
                != PyList_SetItem (
                    code_list,
                    i,
                    Py_BuildValue( "B", code[ i ] )
                                  )
              )
          {
            PyErr_Print();
          }
        }

        if( PyList_Check( code_list ) )
        {
          return_value = code_list;
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not get code: 0x%x.", return_value );
    }
  }

  if( NULL != code )
  {
    cpc_safe_free( ( void** ) &code );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

/*! \fn     gold_code* python_initialize_gold_code  (
                                int in_degree,
                                long in_generator_polynomial_1,
                                long in_generator_polynomial_2,
                                long in_initial_state_1,
                                long in_initial_state_2
                                                    )
    \brief  Initializes gold code LFSRs generator polynomials and an intial
            states. Both the generators and initial states need to be
            configured properly for the gold code generator to function
            correctly. Please see the documentation in gold_code.h and
            spreading_code.h
 */
gold_code*
python_initialize_gold_code (
                              int in_degree,
                              long in_generator_polynomial_1,
                              long in_generator_polynomial_2,
                              long in_initial_state_1,
                              long in_initial_state_2
                            )
{
  gold_code* shift_registers = NULL;

  if  (
        0 > in_generator_polynomial_1
        || 0 > in_generator_polynomial_2
        || 0 > in_initial_state_1
        || 0 > in_initial_state_2
        || 0 > in_degree
      )
  {
    CPC_ERROR (
      "Generator polynomial 1 (0x%x), state 1 (0x%x), degree (0x%x)"
      " generator polynomial 2 (0x%x), or state 2 (0x%x) are negative.",
      in_generator_polynomial_1,
      in_initial_state_1,
      in_degree,
      in_generator_polynomial_2,
      in_initial_state_2
              );
  }
  else
  {
    csignal_error_code return_value =
      csignal_initialize_gold_code  (
        in_degree,
        in_generator_polynomial_1,
        in_generator_polynomial_2,
        in_initial_state_1,
        in_initial_state_2,
        &shift_registers
                                    );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR (
        "Could not create gold code struct: 0x%x.",
        return_value
                );

      shift_registers = NULL;
    }
  }

  return( shift_registers );
}

/*! \fn     static PyObject* python_get_spreading_code  (
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
static PyObject*
python_get_spreading_code (
                            spreading_code* in_spreading_code,
                            size_t          in_number_of_bits
                          )
{
  PyObject* return_value = NULL;

  USIZE size  = 0;
  UCHAR* code = NULL;

  if( NULL == in_spreading_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Spreading code is null." );
  }
  else
  {
    csignal_error_code return_code =
      csignal_get_spreading_code  (
      in_spreading_code,
      in_number_of_bits,
      &size,
      &code
                                  );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      PyObject* code_list = PyList_New( size );

      if( NULL == code_list )
      {
        CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Created a null list." );
      }
      else
      {
        for( USIZE i = 0; i < size; i++ )
        {
          if  (
                0
                != PyList_SetItem (
                    code_list,
                    i,
                    Py_BuildValue( "B", code[ i ] )
                                  )
              )
          {
            PyErr_Print();
          }
        }

        if( PyList_Check( code_list ) )
        {
          return_value = code_list;
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not get code: 0x%x.", return_value );
    }
  }
  
  if( NULL != code )
  {
    cpc_safe_free( ( void** ) &code );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

/*! \fn     spreading_code* python_initialize_spreading_code  (
                                int   in_degree,
                                long  in_generator_polynomial,
                                long  in_initial_state
                                                              )
    \brief  Initializes the LFSR with a generator polynomial and an intial
            state. Both the generator and initial state need to be configured
            properly for the LFSR to function. Please see the documentation
            in spreading_code.h and spreading_code.c
 */
spreading_code*
python_initialize_spreading_code  (
                                int in_degree,
                                long in_generator_polynomial,
                                long in_initial_state
                                  )
{
  spreading_code* shift_register = NULL;

  if( 0 > in_generator_polynomial || 0 > in_initial_state || 0 > in_degree )
  {
    CPC_ERROR (
      "Generator polynomial (0x%x), state 0x%x), or degree (0x%x)  are negative.",
      in_generator_polynomial,
      in_initial_state,
      in_degree
              );
  }
  else
  {
    csignal_error_code return_value =
      csignal_initialize_spreading_code (
        in_degree,
        in_generator_polynomial,
        in_initial_state,
        &shift_register
                                        );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR (
        "Could not create spreading code struct: 0x%x.",
        return_value
                );

      shift_register = NULL;
    }
  }

  return( shift_register );
}

/*! \fn     static PyObject* python_spread_signal (
                        gold_code*  io_gold_code,
                        size_t      in_chip_duration,
                        PyObject*   in_signal
                                                  )
    \brief  Spreads the signal in in_signal by the code generated by
            io_gold_code. See csiganl_spread_signal for all the details
            related to the implementation and valid parameter values.

    \return A Python list of signed short values.
 */
static PyObject*
python_spread_signal  (
                        gold_code*  io_gold_code,
                        size_t      in_chip_duration,
                        PyObject*   in_signal
                      )
{
  PyObject* return_value = NULL;

  USIZE size      = 0; 
  FLOAT64* signal = NULL;

  if( NULL == io_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code is null." );
  }
  else if( 0 >= in_chip_duration )
  {
    CPC_ERROR (
      "Chip duration (0x%x) must be strictly be positive.",
      in_chip_duration
              ); 
  }
  else if( ! PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING  (
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
                    );
  }
  else
  {
    size    = PyList_Size( in_signal ); 
    signal  = NULL;

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** ) &signal, sizeof( FLOAT64 ) * size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[ i ] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      CPC_LOG_BUFFER_FLOAT64  (
        CPC_LOG_LEVEL_TRACE,
        "Signal",
        signal,
        size,
        8
                              );
    }
    else
    {
      CPC_ERROR( "Could not malloc signal: 0x%x.", return_value );
    }

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      return_code =
        csignal_spread_signal (
          io_gold_code,
          in_chip_duration,
          size,
          signal
                              );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        PyObject* new_signal = PyList_New( size );

        CPC_LOG_BUFFER_FLOAT64  (
          CPC_LOG_LEVEL_TRACE,
          "Spread signal",
          signal,
          size,
          8
                                );

        for( USIZE i = 0; i < size; i++ )
        {
          if  (
                0 !=  PyList_SetItem  (
                        new_signal,
                        i,
                        Py_BuildValue( "d", signal[ i ] )
                                      )
              )
          {
            PyErr_Print();

            return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
          }
        }

        if( CPC_ERROR_CODE_NO_ERROR == return_code )
        {
          return_value = new_signal;
        }
      }
      else
      {
        CPC_ERROR( "Could not spread signal: 0x%x.", return_value );
      }
    }
  }

  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

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
python_write_FLOAT_wav (
                        PyObject* in_file_name,
                        size_t    in_number_of_channels,
                        int       in_sample_rate,
                        size_t    in_number_of_samples,
                        PyObject* in_samples
                      )
{
  FLOAT64** samples     = NULL;
  CPC_BOOL return_value = CPC_TRUE;

  if  (
        0 >= in_number_of_channels
        || 0 >= in_sample_rate
        || 0 >= in_number_of_samples
      )
  {
    CPC_ERROR (
      "Number of channels: 0x%x, sample rate: 0x%x,"
      " or number of samples: 0x%x is not positive",
      in_number_of_channels,
      in_sample_rate,
      in_number_of_samples
              );

    return_value = CPC_FALSE;
  }
  else
  {
    if( PyString_Check( in_file_name ) )
    {
      if( PyList_Check( in_samples ) )
      {
        if( PyList_Size( in_samples ) == in_number_of_channels )
        {
          for( USIZE i = 0; i < in_number_of_channels && return_value; i++ )
          {
            if  (
              PyList_Size( PyList_GetItem( in_samples, i ) )
                != in_number_of_samples
                )
            {
              CPC_ERROR (
                "Sample list 0x%x is not of correct size (0x%x), should be 0x%x.",
                i,
                PyList_Size( PyList_GetItem( in_samples, i ) ),
                in_number_of_samples
                        ); 
              
              return_value = CPC_FALSE;
            }
            else
            {
              for( USIZE j = 0; j < in_number_of_samples; j++ )
              {
                PyObject* number =
                  PyList_GetItem( PyList_GetItem( in_samples, i ), j );
  
                if( ! PyFloat_Check( number ) )
                {
                  CPC_ERROR (
                    "Item 0x%x, 0x%x is not an integer.",
                    i,
                    j
                            );
                  
                  return_value = CPC_FALSE;
  
                  break;
                }
              }
            }
          }
        }
        else
        {
          CPC_ERROR (
            "Samples list not of correct size (0x%x), should be 0x%x.",
            PyList_Size( in_samples ),
            in_number_of_channels
                    ); 
          
          return_value = CPC_FALSE;
        }
      }
      else
      {
        CPC_ERROR( "Samples are not a list: 0x%x.", in_samples );
          
        return_value = CPC_FALSE;
      }
    }
    else
    {
      CPC_ERROR( "File name is not a tring: 0x%x.", in_file_name );
  
      return_value = CPC_FALSE;
    }
  }
  
  if( return_value )
  {
    cpc_error_code error =
      cpc_safe_malloc (
        ( void** ) &samples,
        sizeof( FLOAT64* ) * in_number_of_channels
                      );

    if( CPC_ERROR_CODE_NO_ERROR == error )
    {
      for( USIZE i = 0; i < in_number_of_channels; i++ )
      {
        error =
          cpc_safe_malloc (
            ( void** ) &( samples[ i ] ),
            sizeof( FLOAT64 ) * in_number_of_samples
                          );

        if( CPC_ERROR_CODE_NO_ERROR == error )
        {
          for( USIZE j = 0; j < in_number_of_samples; j++ )
          {
            PyObject* number =
              PyList_GetItem( PyList_GetItem( in_samples, i ), j );

            samples[ i ][ j ] = PyFloat_AsDouble( number );
          }
        }
        else
        {
          CPC_ERROR( "Could not malloc sample array: 0x%x.", error );

          return_value = CPC_FALSE;
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc samples array: 0x%x.", error );

      return_value = CPC_FALSE;
    }
  }

  if( return_value )
  {
    csignal_error_code error =
      csignal_write_FLOAT_wav  (
        PyString_AsString( in_file_name ),
        in_number_of_channels,
        in_sample_rate,
        in_number_of_samples,
        samples
                              );

    if( error )
    {
      CPC_ERROR( "Could not write FLOAT WAV file: 0x%x.", error );

      return_value = CPC_FALSE;
    }
  }

  return( return_value );
}

/*! \fn     static PyObject* python_modulate_symbol  (
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
static PyObject*
python_modulate_symbol  (
                          int     in_symbol,
                          int     in_constellation_size,
                          int     in_sample_rate,
                          size_t  in_symbol_duration,
                          int     in_baseband_pulse_amplitude,
                          float   in_carrier_frequency
                        )
{
  PyObject* return_value = NULL;

  FLOAT64* inphase    = NULL;
  FLOAT64* quadrature = NULL;

  if  (
        0 > in_symbol
        || 0 >= in_constellation_size
        || 0 >= in_sample_rate
        || 0 >= in_carrier_frequency
      )
  {
    CPC_ERROR (
                "Invalid input: m=0x%x, M=0x%x, sr=0x%x"
                ", |g|=0x%x, f_c=%.2f",
                in_symbol,
                in_constellation_size,
                in_sample_rate,
                in_baseband_pulse_amplitude,
                in_carrier_frequency
              );
  }
  else
  {
    csignal_error_code return_code =
      cpc_safe_malloc (
        ( void** ) &inphase,
        sizeof( FLOAT64 ) * in_symbol_duration
                      );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      return_code =
        cpc_safe_malloc (
          ( void** ) &quadrature,
          sizeof( FLOAT64 ) * in_symbol_duration
                        );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        return_code =
          csignal_modulate_symbol (
                                    in_symbol,
                                    in_constellation_size,
                                    in_sample_rate,
                                    in_symbol_duration,
                                    in_baseband_pulse_amplitude,
                                    in_carrier_frequency,
                                    inphase,
                                    quadrature
                                  );

        if( CPC_ERROR_CODE_NO_ERROR == return_code )
        {
          PyObject* list = PyList_New( 2 );

          if( NULL != list )
          {
            PyObject* inphase_list    = PyList_New( in_symbol_duration );
            PyObject* quadrature_list = PyList_New( in_symbol_duration );
  
            if( NULL != inphase_list && NULL != quadrature_list )
            {
              PyList_SetItem( list, 0, inphase_list );
              PyList_SetItem( list, 1, quadrature_list );

              for( USIZE i = 0; i < in_symbol_duration; i++ )
              {
                if  (
                  0 != PyList_SetItem (
                        inphase_list,
                        i,
                        Py_BuildValue( "d", inphase[ i ] )
                                      )
                  || 0 != PyList_SetItem  (
                            quadrature_list,
                            i,
                            Py_BuildValue( "d", quadrature[ i ] )
                                          )
                    )
                {
                  PyErr_Print();
                }
              }
    
              if( PyList_Check( list ) )
              {
                return_value = list;
              }
              else
              {
                CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );
              }
            }
            else
            {
              CPC_ERROR (
                "Could not create inphase (0x%x) or quadrature list (0x%x).",
                inphase_list,
                quadrature_list
                        );
            }
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create list." );
          }
        }
        else
        {
          CPC_ERROR( "Could not modulate symbol: 0x%x.", return_value );
        }
      }
      else
      {
        CPC_LOG_STRING  (
          CPC_LOG_LEVEL_ERROR,
          "Could not malloc quadrature array."
                        );
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not malloc inphase array." );
    }
  }

  if( NULL != inphase )
  {
    cpc_safe_free( ( void** ) &inphase );
  }

  if( NULL != quadrature )
  {
    cpc_safe_free( ( void** ) &quadrature );
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

/*! \fn     static PyObject* python_get_symbol (
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
static PyObject*
python_get_symbol (
                    csignal_symbol_tracker* in_symbol_tracker,
                    size_t                  in_number_of_bits
                  )
{
  PyObject* return_value = NULL;

  UINT32 symbol = 0;

  if( 0 >= in_number_of_bits )
  {
    CPC_ERROR( "Invalid input for number of bits: %d.", in_number_of_bits );
  }
  else
  {
    USIZE number_of_bits = in_number_of_bits;

    csignal_error_code return_code = csignal_get_symbol  (
      in_symbol_tracker,
      number_of_bits,
      &symbol
                                                          );

    if( CPC_ERROR_CODE_NO_ERROR != return_code )
    {
      CPC_ERROR( "Could not get symbol: 0x%x.", return_code );
    }
    else
    {
      return_value = Py_BuildValue( "I", symbol );
    }
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

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
python_initialize_symbol_tracker (
                                  PyObject* in_data
                                )
{
  csignal_symbol_tracker* symbol_tracker = NULL;

  Py_ssize_t length = PyString_Size( in_data );
  char* buffer      = PyString_AsString( in_data );

  csignal_error_code return_value =
    csignal_initialize_symbol_tracker (
    ( UCHAR* ) buffer,
    length,
    &symbol_tracker
                                      );

  if( CPC_ERROR_CODE_NO_ERROR != return_value )
  {
    CPC_ERROR( "Could not initialize symbol tracker: 0x%x.", return_value );

    symbol_tracker = NULL;
  }

  return( symbol_tracker );
}

%}

%apply char       { CHAR  }
%apply char       { INT8  }
%apply short      { INT16 }
%apply int        { INT32 }
%apply long long  { INT64 }
%apply float      { FLOAT32 }
%apply double     { FLOAT64 }

%apply unsigned char      { UCHAR   }
%apply unsigned char      { UINT8   }
%apply unsigned short     { UINT16  }
%apply unsigned int       { UINT32  }
%apply unsigned long long { UINT64  }

%apply size_t { SIZE  }

%apply char *       { CHAR *  }
%apply char *       { INT8 *  }
%apply short *      { INT16 * }
%apply int *        { INT32 * }
%apply long long *  { INT64 * }
%apply float *      { FLOAT32 * }
%apply double *     { FLOAT64 * }

%apply unsigned char *      { UCHAR *   }
%apply unsigned char *      { UINT8 *   }
%apply unsigned short *     { UINT16 *  }
%apply unsigned int *       { UINT32 *  }
%apply unsigned long long * { UINT64 *  }

%apply unsigned char { CPC_BOOL }

%apply size_t * { SIZE *  }

%include <csignal.h>
%include <wav.h>
%include <spreading_code.h>
%include <gold_code.h>
%include <csignal_error_codes.h>
%include <fir_filter.h>
%include <kaiser_filter.h>

// These have to be included because we don't recursively parse headers
%include <types.h>
%include <cpcommon_error_codes.h>
%include <log_definitions.h>
%include <log_functions.h>

%include <cpointer.i>
%pointer_functions( double, doubleP )
