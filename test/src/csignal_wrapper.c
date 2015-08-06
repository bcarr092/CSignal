#include "csignal_wrapper.h"

/*! \fn     csignal_error_code python_convert_list_to_array  (
              PyObject*  in_py_list,
              USIZE*     out_array_length,
              FLOAT64**  out_array
            )
    \brief  Converts a Python list to an array of FLOAT64 values. The created
            array is of length out_array_length.
 
    \param  in_py_list  The Python list of doubles to convert to an array of
                        Floats.
    \param  out_array_length  The length of the created array. 0 if an error
                              occurrs. Must be freed by caller.
    \param  out_array A newly malloc'd array populated with the FLOAT values
                      from in_py_list. NULL if an error occurrs. Must be freed
                      by the caller.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If any element of in_py_list is not
                                              a float.
 */
csignal_error_code
python_convert_list_to_array  (
                               PyObject*  in_py_list,
                               USIZE*     out_array_length,
                               FLOAT64**  out_array
                               );

/*! \fn     csignal_error_code python_convert_array_to_list  (
              USIZE      in_array_length,
              FLOAT64*   in_array,
              PyObject** out_list
            )
    \brief  Converst a C array to a python list of doubles.
 
    \param  in_array_length The number of elements in in_array.
    \param  in_array  The array of FLOAT64 values to be added to the python list
    \param  out_list  A newly created Python list. The reference is stolen by
                      the caller, i.e., this function creates a new reference.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If in_array or out_list is null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If any element of in_array failed
                                              to convert to a Python float.
            CPC_ERROR_CODE_API_ERROR  If the Python list could not be created.
 */
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

  FLOAT64*  signal  = NULL;
  FLOAT64* fft      = NULL;
  
  USIZE signal_length = 0;
  USIZE fft_length    = 0;

  if( !PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING(
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
      );
  }
  else
  {
    csignal_error_code result =
      python_convert_list_to_array( in_signal, &signal_length, &signal );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
        csignal_calculate_FFT( signal_length, signal, &fft_length, &fft );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
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
                     fir_passband_filter*   in_filter,
                     PyObject*              in_signal
                     )
{
  PyObject* return_value  = NULL;

  FLOAT64* signal           = NULL;
  FLOAT64* filtered_signal  = NULL;
  
  USIZE signal_length           = 0;
  USIZE filtered_signal_length  = 0;

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
    csignal_error_code result =
    python_convert_list_to_array( in_signal, &signal_length, &signal );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
      csignal_filter_signal(
                            in_filter,
                            signal_length,
                            signal,
                            &filtered_signal_length,
                            &filtered_signal
                            );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        result =
          python_convert_array_to_list  (
                                         filtered_signal_length,
                                         filtered_signal,
                                         &return_value
                                         );
      }
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
python_initialize_kaiser_lowpass_filter(
                                        FLOAT32 in_passband,
                                        FLOAT32 in_stopband,
                                        FLOAT32 in_passband_attenuation,
                                        FLOAT32 in_stopband_attenuation,
                                        UINT32  in_sampling_frequency
                                        )
{
  fir_passband_filter* filter     = NULL;
  csignal_error_code return_value =
    csignal_inititalize_kaiser_lowpass_filter(
                                              in_passband,
                                              in_stopband,
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
  
  return( filter );
}

fir_passband_filter*
python_initialize_kaiser_filter(
                                FLOAT32 in_first_stopband,
                                FLOAT32 in_first_passband,
                                FLOAT32 in_second_passband,
                                FLOAT32 in_second_stopband,
                                FLOAT32 in_passband_attenuation,
                                FLOAT32 in_stopband_attenuation,
                                UINT32  in_sampling_frequency
                                )
{
  fir_passband_filter* filter     = NULL;
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

  return( filter );
}

PyObject*
python_get_gold_code(
                     gold_code* in_gold_code,
                     USIZE      in_number_of_bits
                     )
{
  PyObject* return_value = NULL;

  USIZE code_length = 0;
  UCHAR* code       = NULL;
  
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;

  if( NULL == in_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code was null." );
  }
  else
  {
    result =
      csignal_get_gold_code(
                            in_gold_code,
                            in_number_of_bits,
                            &code_length,
                            &code
                            );

    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      return_value =
        PyString_FromStringAndSize( ( CHAR* ) code, code_length );
      
      if( NULL == return_value || ! PyString_Check( return_value ) )
      {
        result = CPC_ERROR_CODE_API_ERROR;
        
        CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not convert buffer." );
        
        return_value = NULL;
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

  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( return_value );
  }
  else
  {
    Py_XDECREF( return_value );
    
    Py_RETURN_NONE;
  }
}

gold_code*
python_initialize_gold_code(
                            UINT32 in_degree,
                            UINT32 in_generator_polynomial_1,
                            UINT32 in_generator_polynomial_2,
                            UINT32 in_initial_state_1,
                            UINT32 in_initial_state_2
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
                                 UINT32 in_degree,
                                 UINT32 in_generator_polynomial,
                                 UINT32 in_initial_state
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
  PyObject* return_value  = NULL;

  FLOAT64* signal     = NULL;
  
  USIZE signal_length = 0;
  
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
    csignal_error_code result =
      python_convert_list_to_array( in_signal, &signal_length, &signal );

    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
        csignal_spread_signal(
                              io_gold_code,
                              in_chip_duration,
                              signal_length,
                              signal
                              );

      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        result =
          python_convert_array_to_list( signal_length, signal, &return_value );
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

CPC_BOOL
python_write_LPCM_wav(
                       PyObject*  in_file_name,
                       USIZE      in_number_of_channels,
                       UINT32     in_sample_rate,
                       USIZE      in_number_of_samples,
                       PyObject*  in_samples
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
    cpc_error_code result =
    cpc_safe_malloc(
                    ( void** )&samples,
                    sizeof( FLOAT64* )* in_number_of_channels
                    );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      for( USIZE i = 0; i < in_number_of_channels; i++ )
      {
        USIZE samples_length = 0;
        
        samples[ i ]  = NULL;
        
        result =
        python_convert_list_to_array  (
                                       PyList_GetItem( in_samples, i ),
                                       &samples_length,
                                       &( samples[ i ] )
                                       );
        
        if( CPC_ERROR_CODE_NO_ERROR != result )
        {
          CPC_ERROR( "Could not set samples list for channel %d.", i );
          
          return_value = CPC_FALSE;
          
          break;
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc samples array: 0x%x.", result );
      
      return_value = CPC_FALSE;
    }
  }
  
  if( return_value )
  {
    csignal_error_code result =
    csignal_write_LPCM_wav(
                           PyString_AsString( in_file_name ),
                           in_number_of_channels,
                           in_sample_rate,
                           in_number_of_samples,
                           samples
                           );
    
    if( CPC_ERROR_CODE_NO_ERROR != result )
    {
      CPC_ERROR( "Could not write FLOAT WAV file: 0x%x.", result );
      
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

CPC_BOOL
python_write_FLOAT_wav(
                       PyObject*  in_file_name,
                       USIZE      in_number_of_channels,
                       UINT32     in_sample_rate,
                       USIZE      in_number_of_samples,
                       PyObject*  in_samples
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
    cpc_error_code result =
      cpc_safe_malloc(
                      ( void** )&samples,
                      sizeof( FLOAT64* )* in_number_of_channels
                      );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      for( USIZE i = 0; i < in_number_of_channels; i++ )
      {
        USIZE samples_length = 0;
        
        samples[ i ]  = NULL;
        
        result =
          python_convert_list_to_array  (
                                         PyList_GetItem( in_samples, i ),
                                         &samples_length,
                                         &( samples[ i ] )
                                         );
        
        if( CPC_ERROR_CODE_NO_ERROR != result )
        {
          CPC_ERROR( "Could not set samples list for channel %d.", i );
          
          return_value = CPC_FALSE;
          
          break;
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc samples array: 0x%x.", result );
      
      return_value = CPC_FALSE;
    }
  }

  if( return_value )
  {
    csignal_error_code result =
      csignal_write_FLOAT_wav(
                              PyString_AsString( in_file_name ),
                              in_number_of_channels,
                              in_sample_rate,
                              in_number_of_samples,
                              samples
                            );

    if( CPC_ERROR_CODE_NO_ERROR != result )
    {
      CPC_ERROR( "Could not write FLOAT WAV file: 0x%x.", result );

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
                       UINT32 in_symbol,
                       UINT32 in_constellation_size
                       )
{
  FLOAT64 inphase_value    = 0.0;
  FLOAT64 quadrature_value = 0.0;
 
  PyObject* inphase       = NULL;
  PyObject* quadrature    = NULL;
  PyObject* return_value  = NULL;

  if( 0 == in_constellation_size )
  {
    CPC_ERROR(
              "Constellation size (%d) must be strictly positive.",
              in_constellation_size
              );
  }
  else
  {
    csignal_error_code result =
      csignal_modulate_symbol(
                              in_symbol,
                              in_constellation_size,
                              &inphase_value,
                              &quadrature_value
                              );

    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      inphase     = PyFloat_FromDouble( inphase_value );
      quadrature  = PyFloat_FromDouble( quadrature_value );
      
      if  (
           NULL != inphase && PyFloat_Check( inphase )
           && NULL != quadrature && PyFloat_Check( quadrature )
           )
      {
        return_value = PyTuple_New( 2 );
        
        if( NULL != return_value && PyTuple_Check( return_value ) )
        {
          if  (
               0 != PyTuple_SetItem( return_value, 0, inphase )
               || 0 != PyTuple_SetItem( return_value, 1, quadrature )
               )
          {
            CPC_LOG_STRING  (
                             CPC_LOG_LEVEL_ERROR,
                             "Could not add items to tuple."
                             );
            
            return_value = NULL;
          }
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create tuple." );
          
          return_value = NULL;
        }
      }
      else
      {
        CPC_ERROR (
                   "Could not convert inphase (%.02f) and"
                   " quadrature (%.02f).",
                   inphase_value,
                   quadrature_value
                   );
      }
    }
    else
    {
      CPC_ERROR (
                 "Could not modulate symbol (%d): 0x%x.",
                 in_symbol,
                 return_value
                 );
    }
  }

  if( NULL != return_value )
  {
    return( return_value );
  }
  else
  {
    Py_XDECREF( inphase );
    Py_XDECREF( quadrature );
    Py_XDECREF( return_value );
    
    Py_RETURN_NONE;
  }
}

bit_stream*
python_bit_stream_initialize_from_bit_packer (
                                              PyObject*   in_circular,
                                              bit_packer* in_bit_packer
                                              )
{
  bit_stream* stream = NULL;
  
  if( NULL == in_bit_packer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Packer is null." );
  }
  else if( ! PyBool_Check( in_circular ) )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Circular is not a boolean object." );
  }
  else
  {
    CPC_BOOL circular = ( in_circular == Py_True ) ? CPC_TRUE : CPC_FALSE;
    
    csignal_error_code result =
      bit_stream_initialize_from_bit_packer( circular, in_bit_packer, &stream );
    
    if( CPC_ERROR_CODE_NO_ERROR != result )
    {
      CPC_ERROR( "Could not create stream from packer: 0x%x.", result );
      
      stream = NULL;
    }
  }
  
  return( stream );
}

bit_stream*
python_bit_stream_initialize  (
                               PyObject*  in_circular,
                               PyObject*  in_data
                               )
{
  bit_stream* stream = NULL;
  
  if( NULL == in_data )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Data is null." );
  }
  else if( ! PyString_Check( in_data ) )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "data is not a string." );
  }
  else if( ! PyBool_Check( in_circular ) )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Circular is not a boolean." );
  }
  else
  {
    Py_ssize_t length = PyString_Size( in_data );
    char* buffer      = PyString_AsString( in_data );
    CPC_BOOL circular = ( in_circular == Py_True ) ? CPC_TRUE : CPC_FALSE;
    
    csignal_error_code result =
      bit_stream_initialize  (
                              circular,
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
    USIZE num_bits_to_read  = in_number_of_bits;
    
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
python_csignal_multiply_signals (
                                 PyObject* in_signal_one,
                                 PyObject* in_signal_two
                                 )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  
  FLOAT64* signal_one     = NULL;
  FLOAT64* signal_two     = NULL;
  FLOAT64* output_signal  = NULL;
  
  USIZE signal_one_length     = 0;
  USIZE signal_two_length     = 0;
  USIZE output_signal_length  = 0;
  
  PyObject* list = NULL;
  
  if  (
       NULL == in_signal_one
       || Py_None == in_signal_one
       || NULL == in_signal_two
       || Py_None == in_signal_two
       )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x) or signal two (0x%x) are null",
               " or Python None.",
               in_signal_one,
               in_signal_two
               );
  }
  else
  {
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
          csignal_multiply_signal (
                                   signal_one_length,
                                   signal_one, 
                                   signal_two_length,
                                   signal_two,
                                   &output_signal_length,
                                   &output_signal
                                   );
        
        if( CPC_ERROR_CODE_NO_ERROR == result )
        {
          result =
          python_convert_array_to_list  (
                                         output_signal_length,
                                         output_signal,
                                         &list
                                         );
        }
      }
    }
  }
  
  if( NULL != signal_one )
  {
    cpc_safe_free( ( void** ) &signal_one );
  }
  
  if( NULL != signal_two )
  {
    cpc_safe_free( ( void** ) &signal_two );
  }
  
  if( NULL != output_signal )
  {
    cpc_safe_free( ( void** ) &output_signal );
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

PyObject*
python_csignal_calculate_energy (
                                 PyObject* in_signal
                                 )
{
  
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  
  FLOAT64* signal     = NULL;
  USIZE signal_length = 0;
  FLOAT64 energy      = 0.0;
  
  PyObject* py_energy = NULL;
  
  if( NULL == in_signal || Py_None == in_signal )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Signal is null or Python None." );
  }
  else
  {
    result =
    python_convert_list_to_array  (
                                   in_signal,
                                   &signal_length,
                                   &signal
                                   );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result = csignal_calculate_energy( signal_length, signal, &energy );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        py_energy = PyFloat_FromDouble( energy );
      }
    }
  }
  
  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( py_energy );
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
  
  if  (
       NULL == in_signal_one
       || Py_None == in_signal_one
       || NULL == in_signal_two
       || Py_None == in_signal_two
       )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x) or two (0x%x) are null or Python None.",
               in_signal_one,
               in_signal_two
               );
  }
  else
  {
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
        }
      }
    }
  }
  
  if( NULL != signal_one )
  {
    cpc_safe_free( ( void** ) &signal_one );
  }
  
  if( NULL != signal_two )
  {
    cpc_safe_free( ( void** ) &signal_two );
  }
  
  if( NULL != convolved_signal )
  {
    cpc_safe_free( ( void** ) &convolved_signal );
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
  
  if( NULL == out_list || Py_None == *out_list )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List is null or Python None." );
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
          return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
          
          CPC_ERROR( "Could not convert set item 0x%x.", i );
          
          break;
        }
      }
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        if( ! PyList_Check( *out_list ) )
        {
          return_value = CPC_ERROR_CODE_API_ERROR;
          
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
 
  if  (
       NULL == in_py_list
       || Py_None == in_py_list
       || NULL == out_array
       || NULL == out_array_length
       )
  {
    CPC_ERROR (
               "Python list (0x%x), array (0x%x) or length (0x%x)"
               " are null or Python None.",
               in_py_list,
               out_array,
               out_array_length
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    *out_array_length = PyList_Size( in_py_list );
    *out_array        = NULL;
    
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_array,
                       sizeof( FLOAT64 ) * *out_array_length
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
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
          
          return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
          
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

PyObject*
python_csignal_demodulate_binary_PAM (
                                      PyObject* in_signal
                                      )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  FLOAT64* signal           = NULL;
  USIZE signal_length       = 0;
  PyObject* decision_value  = NULL;
  
  if( NULL == in_signal || Py_None == in_signal )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Signal is null or Python None." );
  }
  else
  {
    result =
      python_convert_list_to_array  (
                                     in_signal,
                                     &signal_length,
                                     &signal
                                     );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      INT32 decision = 0;
      
      result =
        csignal_demodulate_binary_PAM( signal_length, signal, &decision );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Decision: %d.", decision );
        
        decision_value = PyInt_FromLong( decision );
        
        if( NULL == decision_value )
        {
          result = CPC_ERROR_CODE_API_ERROR;
          
          CPC_ERROR( "Failure converting int value (%d).", decision );
        }
      }
      else
      {
        CPC_ERROR( "Could not demodulate signal: 0x%x.", result );
      }
    }
  }
  
  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( decision_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

PyObject*
python_csignal_sum_signal (
                           PyObject*  in_signal,
                           FLOAT64    in_scalar
                           )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  FLOAT64* signal           = NULL;
  USIZE signal_length       = 0;
  PyObject* sum_value       = NULL;
  
  if( NULL == in_signal || Py_None == in_signal )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Signal is null or Python None." );
  }
  else
  {
    result =
      python_convert_list_to_array  (
                                     in_signal,
                                     &signal_length,
                                     &signal
                                     );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      FLOAT64 sum = 0.0;
      
      result = csignal_sum_signal( signal_length, signal, in_scalar, &sum );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        sum_value = PyFloat_FromDouble( sum );
        
        if( NULL == sum_value )
        {
          result = CPC_ERROR_CODE_API_ERROR;
          
          CPC_ERROR( "Failure converting float value (%e).", sum );
        }
      }
      else
      {
        CPC_ERROR( "Could not sum signal: 0x%x.", result );
      }
    }
  }
  
  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( sum_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

PyObject*
python_bit_stream_peak  (
                         bit_stream* in_bit_stream
                         )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  PyObject* return_value    = NULL;
  
  if( NULL == in_bit_stream )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit stream is null." );
  }
  else
  {
    UCHAR read_offset   = 0;
    UCHAR write_offset  = 0;
    USIZE buffer_length = 0;
    UCHAR* buffer       = NULL;
    
    result =
      bit_stream_peak (
                       in_bit_stream,
                       &read_offset,
                       &write_offset,
                       &buffer_length,
                       &buffer
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      PyObject* string        =
        PyString_FromStringAndSize( ( CHAR* ) buffer, buffer_length );
      PyObject* read_pointer  = PyInt_FromLong( 1 * read_offset );
      PyObject* write_pointer = PyInt_FromLong( 1 * write_offset );
      
      return_value  = PyTuple_New( 3 );
      
      if  (
           NULL != return_value && PyTuple_Check( return_value )
           && NULL != string && PyString_Check( string )
           && NULL != read_pointer && PyInt_Check( read_pointer )
           && NULL != write_pointer && PyInt_Check( write_pointer )
           )
      {
        if  (
             0 != PyTuple_SetItem( return_value, 0, read_pointer )
             || 0 != PyTuple_SetItem( return_value, 1, write_pointer )
             || 0 != PyTuple_SetItem( return_value, 2, string )
             )
        {
          result = CPC_ERROR_CODE_API_ERROR;
          
          CPC_LOG_STRING  (
                           CPC_LOG_LEVEL_ERROR,
                           "Error adding values to tuple."
                           );
          
          Py_XDECREF( return_value );
          Py_XDECREF( string );
          Py_XDECREF( read_pointer );
          Py_XDECREF( write_pointer );
        }
      }
      else
      {
        result = CPC_ERROR_CODE_API_ERROR;
        
        CPC_LOG_STRING  (
                         CPC_LOG_LEVEL_ERROR,
                         "Error creating Python return values."
                         );
        
        Py_XDECREF( return_value );
        Py_XDECREF( string );
        Py_XDECREF( read_pointer );
        Py_XDECREF( write_pointer );
      }
    }
    else
    {
      CPC_ERROR( "Could not peak at bit stream: 0x%x.", result );
    }
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( return_value );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

PyObject*
python_filter_get_group_delay (
                               fir_passband_filter*  in_filter
                               )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  UINT32 group_delay        = 0;
  PyObject* delay           = NULL;
  
  if( NULL == in_filter )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter is null." );
  }
  else
  {
    result = csignal_filter_get_group_delay( in_filter, &group_delay );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      delay = PyInt_FromLong( group_delay );
      
      if( ! PyInt_Check( delay ) )
      {
        result = CPC_ERROR_CODE_API_ERROR;
        
        CPC_ERROR( "Could not create a Python int from %d.", group_delay );
      }
    }
    else
    {
      CPC_ERROR( "Could not get group delay: 0x%x.", result );
    }
  }
 
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( delay );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

PyObject*
python_csignal_modulate_BFSK_symbol  (
                                      UINT32     in_symbol,
                                      UINT32     in_samples_per_symbol,
                                      UINT32     in_sample_rate,
                                      FLOAT32    in_carrier_frequency
                                      )
{
  USIZE signal_length = 0;
  
  FLOAT64* signal_inphase     = NULL;
  FLOAT64* signal_quadrature  = NULL;
  
  PyObject* inphase       = NULL;
  PyObject* quadrature    = NULL;
  PyObject* return_value  = NULL;
  
  csignal_error_code result =
    csignal_modulate_BFSK_symbol  (
                                   in_symbol,
                                   in_samples_per_symbol,
                                   in_sample_rate,
                                   in_carrier_frequency,
                                   &signal_length,
                                   &signal_inphase,
                                   &signal_quadrature
                                   );
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    result =
      python_convert_array_to_list( signal_length, signal_inphase, &inphase );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
        python_convert_array_to_list  (
                                       signal_length,
                                       signal_quadrature,
                                       &quadrature
                                       );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        return_value = PyTuple_New( 2 );
        
        if( NULL != return_value && PyTuple_Check( return_value ) )
        {
          if  (
               0 != PyTuple_SetItem( return_value, 0, inphase )
               || 0 != PyTuple_SetItem( return_value, 1, quadrature )
               )
          {
            CPC_LOG_STRING  (
                             CPC_LOG_LEVEL_ERROR,
                             "Could not add items to tuple."
                             );
            
            return_value = NULL;
          }
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create tuple." );
          
          return_value = NULL;
        }
      }
      else
      {
        CPC_ERROR( "Could not convert quadrature array to list: 0x%x.", result );
      }
    }
    else
    {
      CPC_ERROR( "Could not convert inphase array to list: 0x%x.", result );
    }
  }
  else
  {
    CPC_ERROR( "Could not generate carrier: 0x%x.", result );
  }
  
  if( NULL != signal_inphase )
  {
    cpc_safe_free( ( void** ) &signal_inphase );
  }
  
  if( NULL != signal_quadrature )
  {
    cpc_safe_free( ( void** ) &signal_quadrature );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( return_value );
  }
  else
  {
    Py_XDECREF( inphase );
    Py_XDECREF( quadrature );
    Py_XDECREF( return_value );
    
    Py_RETURN_NONE;
  }
}

PyObject*
python_detect_calculate_energy (
                                PyObject*            in_signal,
                                PyObject*            in_spread_signal,
                                fir_passband_filter* in_narrowband_filter,
                                fir_passband_filter* in_lowpass_filter
                                )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  
  FLOAT64* signal         = NULL;
  FLOAT64* spread_signal  = NULL;
  
  FLOAT64 energy  = 0.0;
  
  USIZE signal_length         = 0;
  USIZE spread_signal_length  = 0;
  
  PyObject* energy_output = NULL;
  
  if  (
       NULL == in_signal
       || Py_None == in_signal
       || NULL == in_spread_signal
       || Py_None == in_spread_signal
       || NULL == in_narrowband_filter
       || NULL == in_lowpass_filter
       )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), narrowband (0x%x),"
               " or lowpass (0x%x) are null or Python None.",
               in_signal,
               in_spread_signal,
               in_narrowband_filter,
               in_lowpass_filter
               );
  }
  else
  {
    result =
    python_convert_list_to_array  (
                                   in_signal,
                                   &signal_length,
                                   &signal
                                   );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
      python_convert_list_to_array  (
                                     in_spread_signal,
                                     &spread_signal_length,
                                     &spread_signal
                                     );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        result =
        detect_calculate_energy (
                                 signal_length,
                                 signal,
                                 spread_signal_length,
                                 spread_signal,
                                 in_narrowband_filter,
                                 in_lowpass_filter,
                                 &energy
                                 );
        
        if( CPC_ERROR_CODE_NO_ERROR == result )
        {
          energy_output = PyFloat_FromDouble( energy );
          
          if( NULL == energy_output || ! PyFloat_Check( energy_output ) )
          {
            result = CPC_ERROR_CODE_API_ERROR;
            
            CPC_ERROR (
                       "Could not convert Float (%.04f) to Python object.",
                       energy
                       );
          }
        }
      }
    }
  }
  
  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }
  
  if( NULL != spread_signal )
  {
    cpc_safe_free( ( void** ) &spread_signal );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( energy_output );
  }
  else
  {
    Py_RETURN_NONE;
  }
}

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
                                          )
{
  csignal_error_code result = CPC_ERROR_CODE_NO_ERROR;
  
  FLOAT64* signal         = NULL;
  FLOAT64* spread_signal  = NULL;
  
  USIZE offset = 0;
  
  USIZE signal_length         = 0;
  USIZE spread_signal_length  = 0;
  
  PyObject* offset_output = NULL;
  
  if  (
       NULL == in_signal
       || Py_None == in_signal
       || NULL == in_spread_signal
       || Py_None == in_spread_signal
       || NULL == in_narrowband_filter
       || NULL == in_lowpass_filter
       )
  {
    result = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), narrowband (0x%x),"
               " or lowpass (0x%x) are null or Python None.",
               in_signal,
               in_spread_signal,
               in_narrowband_filter,
               in_lowpass_filter
               );
  }
  else
  {
    result =
    python_convert_list_to_array  (
                                   in_signal,
                                   &signal_length,
                                   &signal
                                   );
    
    if( CPC_ERROR_CODE_NO_ERROR == result )
    {
      result =
      python_convert_list_to_array  (
                                     in_spread_signal,
                                     &spread_signal_length,
                                     &spread_signal
                                     );
      
      if( CPC_ERROR_CODE_NO_ERROR == result )
      {
        result =
        detect_find_highest_energy_offset (
                                           signal_length,
                                           signal,
                                           spread_signal_length,
                                           spread_signal,
                                           in_number_of_tests,
                                           in_step_size,
                                           in_narrowband_filter,
                                           in_lowpass_filter,
                                           in_threshold,
                                           in_exhaustive_difference,
                                           in_exhaustive_decimation,
                                           &offset
                                           );
        
        if( CPC_ERROR_CODE_NO_ERROR == result )
        {
          offset_output = PyInt_FromSize_t( offset );
          
          if( NULL == offset_output || ! PyInt_Check( offset_output ) )
          {
            result = CPC_ERROR_CODE_API_ERROR;
            
            CPC_ERROR (
                       "Could not convert offset (0x%x) to Python object.",
                       offset
                       );
          }
        }
      }
    }
  }
  
  if( NULL != signal )
  {
    cpc_safe_free( ( void** ) &signal );
  }
  
  if( NULL != spread_signal )
  {
    cpc_safe_free( ( void** ) &spread_signal );
  }
  
  if( CPC_ERROR_CODE_NO_ERROR == result )
  {
    return( offset_output );
  }
  else
  {
    Py_RETURN_NONE;
  }
}
