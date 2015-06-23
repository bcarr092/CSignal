#include "csignal_wrapper.h"

csignal_error_code
python_convert_list_to_array  (
                               PyObject*  in_py_list,
                               USIZE*     out_array_length,
                               FLOAT64**  out_array
                               );

csignal_error_code
python_convert_array_to_list  (
                               USIZE      in_array_length,
                               FLOAT64*   in_array,
                               PyObject** out_list
                               );

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
          CPC_ERROR( "Entry 0x%x is not a double.", i );

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
  unsigned int in_degree,
  unsigned long in_generator_polynomial_1,
  unsigned long in_generator_polynomial_2,
  unsigned long in_initial_state_1,
  unsigned long in_initial_state_2
)
{
  gold_code* shift_registers = NULL;

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
  unsigned int in_degree,
  unsigned long in_generator_polynomial,
  unsigned long in_initial_state
)
{
  spreading_code* shift_register = NULL;

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
          CPC_ERROR( "Entry 0x%x is not a float.", i );

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
        else
        {
          Py_XDECREF( new_signal );
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
  FLOAT64** samples     = NULL;
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
                    "Item 0x%x, 0x%x is not a float.",
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
      CPC_ERROR( "File name is not a string: 0x%x.", in_file_name );

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

  if( NULL != samples )
  {
    for( USIZE i = 0; i < in_number_of_channels; i++ )
    {
      if( NULL != samples[i] )
      {
        cpc_safe_free( ( void** )&( samples[i] ) );
      }
    }

    cpc_safe_free( ( void** )&samples );
  }

  return( return_value );
}

PyObject*
python_modulate_symbol(
  unsigned int     in_symbol,
  unsigned int     in_constellation_size,
  unsigned int     in_sample_rate,
  size_t           in_symbol_duration,
  int              in_baseband_pulse_amplitude,
  float            in_carrier_frequency
)
{
  PyObject* return_value  = NULL;
  FLOAT64* inphase        = NULL;
  FLOAT64* quadrature     = NULL;

  if  (
       0 >= in_carrier_frequency
       || 0 == in_constellation_size
       || 0 == in_sample_rate
       || 0 >= in_symbol_duration
       )
  {
    CPC_ERROR(
              "Invalid inputs: f_c=%.2f, cs=0x%x, sr=0x%x, sd=0x%x",
              in_carrier_frequency,
              in_constellation_size,
              in_sample_rate,
              in_symbol_duration
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
                  0 != PyList_SetItem (
                        inphase_list,
                        i,
                        Py_BuildValue( "d", inphase[i] )
                                       )
                  || 0 != PyList_SetItem  (
                            quadrature_list,
                            i,
                            Py_BuildValue( "d", quadrature[i] )
                                           )
                  )
                {
                  CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Error adding values.");
                  
                  PyErr_Print( );
                }
              }

              if  (
                   PyList_Check( list )
                   && PyList_Check( inphase_list )
                   && PyList_Check( quadrature_list )
                   )
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

bit_stream*
python_bit_stream_initialize_from_bit_packer (
                                              bit_packer* in_bit_packer
                                              )
{
  bit_stream* stream = NULL;
  
  if( NULL != in_bit_packer )
  {
    csignal_error_code result =
      bit_stream_initialize_from_bit_packer( in_bit_packer, &stream );
    
    if( CPC_ERROR_CODE_NO_ERROR != result )
    {
      CPC_ERROR( "Could not create stream from packer: 0x%x.", result );
      
      stream = NULL;
    }
  }
  else
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Packer is null." );
  }
  
  return( stream );
}

bit_stream*
python_bit_stream_initialize  (
                               PyObject*  in_data
                               )
{
  bit_stream* stream = NULL;
  
  if( NULL != in_data )
  {
    if( PyString_Check( in_data ) )
    {
      Py_ssize_t length = PyString_Size( in_data );
      char* buffer      = PyString_AsString( in_data );
      
      csignal_error_code result =
        bit_stream_initialize  (
                                ( UCHAR* ) buffer,
                                length,
                                &stream
                                );
      
      if( CPC_ERROR_CODE_NO_ERROR != result )
      {
        CPC_ERROR( "Could not initialize stream: 0x%x.", result );
        
        stream = NULL;
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "data is not a string." );
    }
  }
  else
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Data is null." );
  }
  
  return( stream );
}

bit_packer*
python_bit_packer_initialize( void )
{
  bit_packer* packer = NULL;
  
  csignal_error_code result = bit_packer_initialize( &packer );
  
  if( CPC_ERROR_CODE_NO_ERROR != result )
  {
    CPC_ERROR( "Could not initialize bit packer: 0x%x.", result );
    
    packer = NULL;
  }
  
  return( packer );
}

csignal_error_code
python_bit_packer_add_bytes (
                             PyObject*    in_data,
                             bit_packer*  io_bit_packer
                             )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL != io_bit_packer && NULL != in_data )
  {
    if( PyString_Check( in_data ) )
    {
      Py_ssize_t length = PyString_Size( in_data );
      char* buffer      = PyString_AsString( in_data );
      
      return_value =
        bit_packer_add_bytes  (
                               ( UCHAR* ) buffer,
                               length,
                               io_bit_packer
                               );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR( "Could not add bytes: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Data is not a string." );
      
      return_value = CSIGNAL_ERROR_CODE_INVALID_TYPE;
    }
  }
  else
  {
    CPC_ERROR (
               "Bit packer (0x%x) or data (0x%x) are null.",
               io_bit_packer,
               in_data
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  
  return( return_value );
}

PyObject*
python_bit_packer_get_bytes (
                             bit_packer* in_bit_packer
                             )
{
  if( NULL != in_bit_packer )
  {
    UCHAR* buffer     = NULL;
    USIZE buffer_size = 0;
    
    csignal_error_code result =
      bit_packer_get_bytes( in_bit_packer, &buffer, &buffer_size );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is 0x%x.", buffer_size );
      CPC_LOG_BUFFER( CPC_LOG_LEVEL_TRACE, "Buffer", buffer, buffer_size, 8 );
      
      PyObject* string =
        PyString_FromStringAndSize( ( CHAR* ) buffer, buffer_size );
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "String: 0x%x.", string );
      
      if( NULL != buffer )
      {
        cpc_safe_free( ( void** ) &buffer );
      }
      
      if( NULL != string )
      {
        if( PyString_Check( string ) )
        {
          return( string );
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Created string failed check." );
          
          Py_DECREF( string );
          
          Py_RETURN_NONE;
        }
      }
      else
      {
        CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create PyString." );
        
        Py_RETURN_NONE;
      }
    }
    else
    {
      CPC_ERROR( "Could not get packer's bytes: 0x%x.", result );
      
      Py_RETURN_NONE;
    }
  }
  else
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Packer is null." );
    
    Py_RETURN_NONE;
  }
}

PyObject*
python_bit_stream_get_bits  (
                             bit_stream*  io_bit_stream,
                             USIZE        in_number_of_bits
                             )
{
  if( NULL != io_bit_stream )
  {
    UCHAR* buffer           = NULL;
    USIZE buffer_size       = 0;
    USIZE num_bits_to_read = in_number_of_bits;
    
    csignal_error_code result =
      bit_stream_get_bits (
                           io_bit_stream,
                           &num_bits_to_read,
                           &buffer,
                           &buffer_size
                           );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is 0x%x.", buffer_size );
      CPC_LOG_BUFFER( CPC_LOG_LEVEL_TRACE, "Buffer", buffer, buffer_size, 8 );
      
      PyObject* string =
        PyString_FromStringAndSize( ( CHAR* ) buffer, buffer_size );
      PyObject* number_of_bits = PyInt_FromSize_t( num_bits_to_read );
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "String: 0x%x.", string );
      
      if( NULL != buffer )
      {
        cpc_safe_free( ( void** ) &buffer );
      }
      
      if( NULL != string && NULL != number_of_bits )
      {
        if( PyString_Check( string ) && PyInt_Check( number_of_bits ) )
        {
          PyObject* return_value = PyTuple_New( 2 );
          
          if( NULL != return_value && PyTuple_Check( return_value ) )
          {
            if  (
                 0 == PyTuple_SetItem( return_value, 0, number_of_bits )
                 && 0 == PyTuple_SetItem( return_value, 1, string )
                 )
            {
              return( return_value );
            }
            else
            {
              CPC_LOG_STRING  (
                               CPC_LOG_LEVEL_ERROR,
                               "Could not add items to tuple."
                               );
              
              Py_DECREF( string );
              Py_DECREF( number_of_bits );
              
              Py_RETURN_NONE;
            }
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create tuple." );
            
            Py_DECREF( string );
            Py_DECREF( number_of_bits );
            
            Py_RETURN_NONE;
          }
        }
        else
        {
          CPC_ERROR (
                     "PyString (0x%x) or PyInt (0x%x) failed check.",
                     string,
                     number_of_bits
                     );
          
          Py_DECREF( string );
          Py_DECREF( number_of_bits );
          
          Py_RETURN_NONE;
        }
      }
      else
      {
        CPC_ERROR (
                   "Could not create PyString (0x%x) or PyInt (0x%x).",
                   string,
                   number_of_bits
                   );
        
        Py_XDECREF( string );
        Py_XDECREF( number_of_bits );
        
        Py_RETURN_NONE;
      }
    }
    else
    {
      Py_RETURN_NONE;
    }
  }
  else
  {
    Py_RETURN_NONE;
  }
}


PyObject*
python_convolve (
                 PyObject* in_signal_one,
                 PyObject* in_signal_two
                 )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  
  FLOAT64* signal_one       = NULL;
  FLOAT64* signal_two       = NULL;
  FLOAT64* convolved_signal = NULL;
  
  USIZE signal_one_length       = 0;
  USIZE signal_two_length       = 0;
  USIZE convolved_signal_length = 0;
  
  PyObject* list = NULL;
  
  result =
    python_convert_list_to_array  (
                                   in_signal_one,
                                   &signal_one_length,
                                   &signal_one
                                   );
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    result =
      python_convert_list_to_array  (
                                     in_signal_two,
                                     &signal_two_length,
                                     &signal_two
                                     );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
        convolve  (
                   signal_one_length,
                   signal_one,
                   signal_two_length,
                   signal_two,
                   &convolved_signal_length,
                   &convolved_signal
                   );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        result =
          python_convert_array_to_list  (
                                         convolved_signal_length,
                                         convolved_signal,
                                         &list
                                         );
        
        if( CPC_ERROR_CODE_NO_ERROR != NULL )
        {
          cpc_safe_free( ( void** ) &signal_one );
          cpc_safe_free( ( void** ) &signal_two );
          cpc_safe_free( ( void** ) &convolved_signal );
        }
      }
      else
      {
        cpc_safe_free( ( void** ) &signal_one );
        cpc_safe_free( ( void** ) &signal_two );
      }
    }
    else
    {
      cpc_safe_free( ( void** ) &signal_one );
    }
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( list );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

csignal_error_code
python_convert_array_to_list  (
                               USIZE      in_array_length,
                               FLOAT64*   in_array,
                               PyObject** out_list
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_list )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List is null." );
  }
  else
  {
    *out_list = PyList_New( in_array_length );
    
    if( NULL != *out_list )
    {
      for( USIZE i = 0; i < in_array_length; i++ )
      {
        if(
           0
           != PyList_SetItem(
                             *out_list,
                             i,
                             Py_BuildValue( "d", in_array[ i ] )
                             )
           )
        {
          return_value = CPC_ERROR_CODE_API_ERROR;
          
          CPC_ERROR( "Could not convert set item 0x%x.", i );
          
          break;
        }
      }
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        if( ! PyList_Check( *out_list ) )
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List check failed." );
          
          Py_DECREF( *out_list );
        }
      }
      else
      {
        Py_DECREF( *out_list );
      }
    }
    else
    {
      return_value = CPC_ERROR_CODE_API_ERROR;
      
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create list." );
    }
  }
  
  return( return_value );
}

csignal_error_code
python_convert_list_to_array  (
                               PyObject*  in_py_list,
                               USIZE*     out_array_length,
                               FLOAT64**  out_array
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_py_list || NULL == out_array || NULL == out_array_length )
  {
    CPC_ERROR (
               "Python list (0x%x), array (0x%x) or length (0x%x) are null.",
               in_py_list,
               out_array,
               out_array_length
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    *out_array_length = PyList_Size( in_py_list );
    
    csignal_error_code return_code =
      cpc_safe_malloc (
                       ( void** ) out_array,
                       sizeof( FLOAT64 ) * *out_array_length
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_code )
    {
      for( USIZE i = 0; i < *out_array_length; i++ )
      {
        if( PyFloat_Check( PyList_GetItem( in_py_list, i ) ) )
        {
          ( *out_array )[ i ] =
            PyFloat_AsDouble( PyList_GetItem( in_py_list, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not a float.", i );
          
          return_code = CPC_ERROR_CODE_INVALID_PARAMETER;
          
          *out_array_length = 0;
          
          cpc_safe_free( ( void** ) out_array );
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc signal: 0x%x.", return_value );
      
      *out_array_length = 0;
      *out_array        = NULL;
    }
  }
  
  return( return_value );
}
