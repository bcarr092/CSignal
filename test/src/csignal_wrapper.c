#include "csignal_wrapper.h"

PyObject*
python_calculate_FFT(
  PyObject* in_signal
)
{
  PyObject* return_value = NULL;

  USIZE size = 0;
  FLOAT64*  signal = NULL;

  USIZE fft_length = 0;
  FLOAT64* fft = NULL;

  if( !PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING(
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
      );
  }
  else
  {
    size = PyList_Size( in_signal );

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** )&signal, sizeof( FLOAT64 )* size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[i] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
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
              PyComplex_FromDoubles( fft[i], fft[i + 1] );

            if( 0 != PyList_SetItem( fft_list, ( i / 2 ), complex ) )
            {
              PyErr_Print( );
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
    cpc_safe_free( ( void** )&signal );
  }

  if( NULL != fft )
  {
    cpc_safe_free( ( void** )&fft );
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

PyObject*
python_filter_signal(
  fir_passband_filter*  in_filter,
  PyObject*             in_signal
)
{
  PyObject* return_value = NULL;

  USIZE size = 0;
  FLOAT64* signal = NULL;

  USIZE filtered_signal_length = 0;
  FLOAT64* filtered_signal = NULL;

  if( NULL == in_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter is null." );
  }
  else if( !PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING(
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
      );
  }
  else
  {
    size = PyList_Size( in_signal );

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** )&signal, sizeof( FLOAT64 )* size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[i] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      CPC_LOG_BUFFER_FLOAT64(
        CPC_LOG_LEVEL_TRACE,
        "Signal",
        signal,
        size,
        8
        );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        return_code =
          csignal_filter_signal(
          in_filter,
          size,
          signal,
          &filtered_signal_length,
          &filtered_signal
          );

        if( CPC_ERROR_CODE_NO_ERROR == return_code )
        {
          CPC_LOG_BUFFER_FLOAT64(
            CPC_LOG_LEVEL_TRACE,
            "Filtered signal",
            filtered_signal,
            ( filtered_signal_length > 200 ? 200 : filtered_signal_length ),
            8
            );

          PyObject* filtered_list = PyList_New( filtered_signal_length );

          if( NULL == filtered_list )
          {
            cpc_safe_free( ( void** )&filtered_signal );
          }
          else
          {
            for( USIZE i = 0; i < filtered_signal_length; i++ )
            {
              if(
                0
                != PyList_SetItem(
                filtered_list,
                i,
                Py_BuildValue( "d", filtered_signal[i] )
                )
                )
              {
                PyErr_Print( );
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
    cpc_safe_free( ( void** )&signal );
  }

  if( NULL != filtered_signal )
  {
    cpc_safe_free( ( void** )&filtered_signal );
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

fir_passband_filter*
python_initialize_kaiser_filter(
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
    CPC_ERROR(
      "Sampling frequency (%d Hz) must be positive.",
      in_sampling_frequency
      );
  }
  else
  {
    csignal_error_code return_value =
      csignal_initialize_kaiser_filter(
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

PyObject*
python_get_gold_code(
  gold_code*  in_gold_code,
  size_t      in_number_of_bits
)
{
  PyObject* return_value = NULL;

  USIZE size = 0;
  UCHAR* code = NULL;

  if( NULL == in_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code was null." );
  }
  else
  {
    csignal_error_code return_code =
      csignal_get_gold_code(
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
          if(
            0
            != PyList_SetItem(
            code_list,
            i,
            Py_BuildValue( "B", code[i] )
            )
            )
          {
            PyErr_Print( );
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
    cpc_safe_free( ( void** )&code );
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

gold_code*
python_initialize_gold_code(
  int in_degree,
  unsigned long in_generator_polynomial_1,
  unsigned long in_generator_polynomial_2,
  unsigned long in_initial_state_1,
  unsigned long in_initial_state_2
)
{
  gold_code* shift_registers = NULL;

  if(
    0 > in_generator_polynomial_1
    || 0 > in_generator_polynomial_2
    || 0 > in_initial_state_1
    || 0 > in_initial_state_2
    || 0 > in_degree
    )
  {
    CPC_ERROR(
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
      csignal_initialize_gold_code(
      in_degree,
      in_generator_polynomial_1,
      in_generator_polynomial_2,
      in_initial_state_1,
      in_initial_state_2,
      &shift_registers
      );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR(
        "Could not create gold code struct: 0x%x.",
        return_value
        );

      shift_registers = NULL;
    }
  }

  return( shift_registers );
}

PyObject*
python_get_spreading_code(
  spreading_code* in_spreading_code,
  size_t          in_number_of_bits
)
{
  PyObject* return_value = NULL;

  USIZE size = 0;
  UCHAR* code = NULL;

  if( NULL == in_spreading_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Spreading code is null." );
  }
  else
  {
    csignal_error_code return_code =
      csignal_get_spreading_code(
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
          if(
            0
            != PyList_SetItem(
            code_list,
            i,
            Py_BuildValue( "B", code[i] )
            )
            )
          {
            PyErr_Print( );
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
    cpc_safe_free( ( void** )&code );
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

spreading_code*
python_initialize_spreading_code(
  int in_degree,
  unsigned long in_generator_polynomial,
  unsigned long in_initial_state
)
{
  spreading_code* shift_register = NULL;

  if( 0 > in_generator_polynomial || 0 > in_initial_state || 0 > in_degree )
  {
    CPC_ERROR(
      "Generator polynomial (0x%x), state 0x%x), or degree (0x%x)  are negative.",
      in_generator_polynomial,
      in_initial_state,
      in_degree
      );
  }
  else
  {
    csignal_error_code return_value =
      csignal_initialize_spreading_code(
      in_degree,
      in_generator_polynomial,
      in_initial_state,
      &shift_register
      );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR(
        "Could not create spreading code struct: 0x%x.",
        return_value
        );

      shift_register = NULL;
    }
  }

  return( shift_register );
}

PyObject*
python_spread_signal(
  gold_code*  io_gold_code,
  size_t      in_chip_duration,
  PyObject*   in_signal
)
{
  PyObject* return_value = NULL;

  USIZE size = 0;
  FLOAT64* signal = NULL;

  if( NULL == io_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code is null." );
  }
  else if( 0 >= in_chip_duration )
  {
    CPC_ERROR(
      "Chip duration (0x%x) must be strictly be positive.",
      in_chip_duration
      );
  }
  else if( !PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING(
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
      );
  }
  else
  {
    size = PyList_Size( in_signal );
    signal = NULL;

    csignal_error_code return_code =
      cpc_safe_malloc( ( void** )&signal, sizeof( FLOAT64 )* size );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < size; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[i] = PyFloat_AsDouble( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      CPC_LOG_BUFFER_FLOAT64(
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
        csignal_spread_signal(
        io_gold_code,
        in_chip_duration,
        size,
        signal
        );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        PyObject* new_signal = PyList_New( size );

        CPC_LOG_BUFFER_FLOAT64(
          CPC_LOG_LEVEL_TRACE,
          "Spread signal",
          signal,
          size,
          8
          );

        for( USIZE i = 0; i < size; i++ )
        {
          if(
            0 != PyList_SetItem(
            new_signal,
            i,
            Py_BuildValue( "d", signal[i] )
            )
            )
          {
            PyErr_Print( );

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
    cpc_safe_free( ( void** )&signal );
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

int
python_write_FLOAT_wav(
  PyObject* in_file_name,
  size_t    in_number_of_channels,
  int       in_sample_rate,
  size_t    in_number_of_samples,
  PyObject* in_samples
)
{
  FLOAT64** samples = NULL;
  CPC_BOOL return_value = CPC_TRUE;

  if(
    0 >= in_number_of_channels
    || 0 >= in_sample_rate
    || 0 >= in_number_of_samples
    )
  {
    CPC_ERROR(
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
            if(
              PyList_Size( PyList_GetItem( in_samples, i ) )
              != in_number_of_samples
              )
            {
              CPC_ERROR(
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

                if( !PyFloat_Check( number ) )
                {
                  CPC_ERROR(
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
          CPC_ERROR(
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
      cpc_safe_malloc(
      ( void** )&samples,
      sizeof( FLOAT64* )* in_number_of_channels
      );

    if( CPC_ERROR_CODE_NO_ERROR == error )
    {
      for( USIZE i = 0; i < in_number_of_channels; i++ )
      {
        error =
          cpc_safe_malloc(
          ( void** )&( samples[i] ),
          sizeof( FLOAT64 )* in_number_of_samples
          );

        if( CPC_ERROR_CODE_NO_ERROR == error )
        {
          for( USIZE j = 0; j < in_number_of_samples; j++ )
          {
            PyObject* number =
              PyList_GetItem( PyList_GetItem( in_samples, i ), j );

            samples[i][j] = PyFloat_AsDouble( number );
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
      csignal_write_FLOAT_wav(
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

PyObject*
python_modulate_symbol(
  int     in_symbol,
  int     in_constellation_size,
  int     in_sample_rate,
  size_t  in_symbol_duration,
  int     in_baseband_pulse_amplitude,
  float   in_carrier_frequency
)
{
  PyObject* return_value = NULL;

  FLOAT64* inphase = NULL;
  FLOAT64* quadrature = NULL;

  if(
    0 > in_symbol
    || 0 >= in_constellation_size
    || 0 >= in_sample_rate
    || 0 >= in_carrier_frequency
    )
  {
    CPC_ERROR(
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
      cpc_safe_malloc(
      ( void** )&inphase,
      sizeof( FLOAT64 )* in_symbol_duration
      );

    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      return_code =
        cpc_safe_malloc(
        ( void** )&quadrature,
        sizeof( FLOAT64 )* in_symbol_duration
        );

      if( CPC_ERROR_CODE_NO_ERROR == return_code )
      {
        return_code =
          csignal_modulate_symbol(
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
            PyObject* inphase_list = PyList_New( in_symbol_duration );
            PyObject* quadrature_list = PyList_New( in_symbol_duration );

            if( NULL != inphase_list && NULL != quadrature_list )
            {
              PyList_SetItem( list, 0, inphase_list );
              PyList_SetItem( list, 1, quadrature_list );

              for( USIZE i = 0; i < in_symbol_duration; i++ )
              {
                if(
                  0 != PyList_SetItem(
                  inphase_list,
                  i,
                  Py_BuildValue( "d", inphase[i] )
                  )
                  || 0 != PyList_SetItem(
                  quadrature_list,
                  i,
                  Py_BuildValue( "d", quadrature[i] )
                  )
                  )
                {
                  PyErr_Print( );
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
              CPC_ERROR(
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
        CPC_LOG_STRING(
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
    cpc_safe_free( ( void** )&inphase );
  }

  if( NULL != quadrature )
  {
    cpc_safe_free( ( void** )&quadrature );
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

PyObject*
python_get_symbol(
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

    csignal_error_code return_code = csignal_get_symbol(
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

csignal_symbol_tracker*
python_initialize_symbol_tracker(
  PyObject* in_data
)
{
  csignal_symbol_tracker* symbol_tracker = NULL;

  Py_ssize_t length = PyString_Size( in_data );
  char* buffer = PyString_AsString( in_data );

  csignal_error_code return_value =
    csignal_initialize_symbol_tracker(
    ( UCHAR* )buffer,
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
