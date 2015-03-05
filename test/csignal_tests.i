%module csignal_tests

%inline %{

#include <csignal.h>

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

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      CPC_LOG_BUFFER_FLOAT32  (
        CPC_LOG_LEVEL_TRACE,
        "Coefficients:",
        filter->coefficients,
        filter->number_of_taps,
        8
                              );
    }
    else
    {
      CPC_ERROR( "Could not initialize kaiser filter: 0x%x.", return_value );

      filter = NULL;
    }
  }

  return( filter );
}
/*! \fn     static PyObject* python_get_gold_code (
                            gold_code*  in_gold_code,
                            int         in_number_of_bits
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
                        int         in_number_of_bits
                      )
{
  if( 0 > in_number_of_bits )
  {
    Py_RETURN_NONE;
  }
  else
  {
    if( NULL == in_gold_code )
    {
      Py_RETURN_NONE;
    }
    else
    {
      UINT32 size = 0;
      UCHAR* code = NULL;

      csignal_error_code return_value =
        csignal_get_gold_code (
          in_gold_code,
          in_number_of_bits,
          &size,
          &code
                              );

      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        PyObject* code_list = PyList_New( size );

        if( NULL == code_list )
        {
          Py_RETURN_NONE;
        }
        else
        {
          for( UINT32 i = 0; i < size; i++ )
          {
            if( 0 != PyList_SetItem( code_list, i, Py_BuildValue( "B", code[ i ] ) ) )
            {
              PyErr_Print();
            }
          }

          if( PyList_Check( code_list ) )
          {
            return( code_list );
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );

            Py_RETURN_NONE;
          }
        }
      }
      else
      {
        CPC_ERROR( "Could not get code: 0x%x.", return_value );

        Py_RETURN_NONE;
      }
    }
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
                            int             in_number_of_bits
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
                            int             in_number_of_bits
                          )
{
  if( 0 > in_number_of_bits )
  {
    Py_RETURN_NONE;
  }
  else
  {
    if( NULL == in_spreading_code )
    {
      Py_RETURN_NONE;
    }
    else
    {
      UINT32 size = 0;
      UCHAR* code = NULL;

      csignal_error_code return_value =
        csignal_get_spreading_code  (
          in_spreading_code,
          in_number_of_bits,
          &size,
          &code
                                    );

      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        PyObject* code_list = PyList_New( size );

        if( NULL == code_list )
        {
          Py_RETURN_NONE;
        }
        else
        {
          for( UINT32 i = 0; i < size; i++ )
          {
            if( 0 != PyList_SetItem( code_list, i, Py_BuildValue( "B", code[ i ] ) ) )
            {
              PyErr_Print();
            }
          }

          if( PyList_Check( code_list ) )
          {
            return( code_list );
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );

            Py_RETURN_NONE;
          }
        }
      }
      else
      {
        CPC_ERROR( "Could not get code: 0x%x.", return_value );

        Py_RETURN_NONE;
      }
    }
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
                        int         in_chip_duration,
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
                        int         in_chip_duration,
                        PyObject*   in_signal
                      )
{
  if( NULL == io_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code is null." );

    Py_RETURN_NONE;
  }
  else if( 0 >= in_chip_duration )
  {
    CPC_ERROR (
      "Chip duration (0x%x) must be strictly be positive.",
      in_chip_duration
              ); 

    Py_RETURN_NONE;
  }
  else if( ! PyList_Check( in_signal ) || PyList_Size( in_signal ) == 0 )
  {
    CPC_LOG_STRING  (
      CPC_LOG_LEVEL_ERROR,
      "Signal must be a list with elements."
                    );

    Py_RETURN_NONE;
  }
  else
  {
    SSIZE size    = PyList_Size( in_signal ); 
    INT16* signal = NULL;

    csignal_error_code return_value =
      cpc_safe_malloc( ( void** ) &signal, sizeof( INT16 ) * size );

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      for( UINT32 i = 0; i < size; i++ )
      {
        if( PyInt_Check( PyList_GetItem( in_signal, i ) ) )
        {
          signal[ i ] = PyInt_AsLong( PyList_GetItem( in_signal, i ) );
        }
        else
        {
          CPC_ERROR( "Entry 0x%x is not an integer.", i );

          return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
        }
      }

      CPC_LOG_BUFFER  (
        CPC_LOG_LEVEL_TRACE,
        "Signal",
        ( UCHAR* ) signal,
        sizeof( INT16 ) * size,
        8
                      );
    }
    else
    {
      CPC_ERROR( "Could not malloc signal: 0x%x.", return_value );
    }

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_spread_signal (
          io_gold_code,
          in_chip_duration,
          size,
          signal
                              );

      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        PyObject* new_signal = PyList_New( size );

        CPC_LOG_BUFFER  (
          CPC_LOG_LEVEL_TRACE,
          "Signal",
          ( UCHAR* ) signal,
          sizeof( INT16 ) * size,
          8
                        );

        for( UINT32 i = 0; i < size; i++ )
        {
          if  (
                0 !=  PyList_SetItem  (
                        new_signal,
                        i,
                        Py_BuildValue( "h", signal[ i ] )
                                      )
              )
          {
            PyErr_Print();

            return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
          }
        }

        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          return( new_signal );
        }
      }
      else
      {
        CPC_ERROR( "Could not spread signal: 0x%x.", return_value );
      }
    }
  }

  Py_RETURN_NONE;
}

/*! \fn     int python_write_LPCM_wav (
                        PyObject* in_file_name,
                        int       in_number_of_channels,
                        int       in_sample_rate,
                        int       in_number_of_samples,
                        PyObject* in_samples
                                      )
    \brief  The python wrapper for the csignal_write_LPCM_wav function. For
            a complete description of the parameters please see the
            documentation for csignal_write_LPCM_wav.

    \return CPC_TRUE if the WAV file was successfully created, CPC_FALSE
            otherwise.
 */
int
python_write_LPCM_wav (
                        PyObject* in_file_name,
                        int       in_number_of_channels,
                        int       in_sample_rate,
                        int       in_number_of_samples,
                        PyObject* in_samples
                      )
{
  INT16** samples       = NULL;
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
          for( UINT32 i = 0; i < in_number_of_channels && return_value; i++ )
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
              for( UINT32 j = 0; j < in_number_of_samples; j++ )
              {
                PyObject* number =
                  PyList_GetItem( PyList_GetItem( in_samples, i ), j );
  
                if( ! PyInt_Check( number ) )
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
        sizeof( INT16* ) * in_number_of_channels
                      );

    if( CPC_ERROR_CODE_NO_ERROR == error )
    {
      for( UINT32 i = 0; i < in_number_of_channels; i++ )
      {
        error =
          cpc_safe_malloc (
            ( void** ) &( samples[ i ] ),
            sizeof( INT16 ) * in_number_of_samples
                          );

        if( CPC_ERROR_CODE_NO_ERROR == error )
        {
          for( UINT32 j = 0; j < in_number_of_samples; j++ )
          {
            PyObject* number =
              PyList_GetItem( PyList_GetItem( in_samples, i ), j );

            samples[ i ][ j ] = PyInt_AsLong( number );
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
      csignal_write_LPCM_wav  (
        PyString_AsString( in_file_name ),
        in_number_of_channels,
        in_sample_rate,
        in_number_of_samples,
        samples
                              );

    if( error )
    {
      CPC_ERROR( "Could not write LPCM WAV file: 0x%x.", error );

      return_value = CPC_FALSE;
    }
  }

  return( return_value );
}

/*! \fn     static PyObject* python_modulate_symbol  (
                          int   in_symbol,
                          int   in_constellation_size,
                          int   in_sample_rate,
                          int   in_symbol_duration,
                          int   in_baseband_pulse_amplitude,
                          float in_carrier_frequency
                        )
    \brief  Modulates in_symbol into a signal of length in_symbol_duration. For
            a complete description of each parameter and the function please
            see the documentation for csignal_modulate_symbol.

    \return An arrary of integers if no error occurs or None
  */
static PyObject*
python_modulate_symbol  (
                          int   in_symbol,
                          int   in_constellation_size,
                          int   in_sample_rate,
                          int   in_symbol_duration,
                          int   in_baseband_pulse_amplitude,
                          float in_carrier_frequency
                        )
{
  if  (
        0 > in_symbol
        || 0 >= in_constellation_size
        || 0 >= in_sample_rate
        || 0 > in_symbol_duration
        || 0 >= in_carrier_frequency
      )
  {
    CPC_ERROR (
                "Invalid input: m=0x%x, M=0x%x, T=0x%x, sr=0x%x"
                ", |g|=0x%x, f_c=%.2f",
                in_symbol,
                in_constellation_size,
                in_symbol_duration,
                in_sample_rate,
                in_baseband_pulse_amplitude,
                in_carrier_frequency
              );

    Py_RETURN_NONE;
  }
  else
  {
    INT16* signal = NULL;

    csignal_error_code return_value =
      cpc_safe_malloc (
        ( void** ) &signal,
        in_symbol_duration * sizeof( INT16 )
                      );

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_modulate_symbol (
                                  in_symbol,
                                  in_constellation_size,
                                  in_sample_rate,
                                  in_symbol_duration,
                                  in_baseband_pulse_amplitude,
                                  in_carrier_frequency,
                                  signal
                                );

      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        PyObject* list = PyList_New( in_symbol_duration );

        if( NULL != list )
        {
          for( UINT32 i = 0; i < in_symbol_duration; i++ )
          {
            if( 0 != PyList_SetItem( list, i, Py_BuildValue( "h", signal[ i ] ) ) )
            {
              PyErr_Print();
            }
          }

          if( PyList_Check( list ) )
          {
            return( list );
          }
          else
          {
            CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "List not created." );

            Py_RETURN_NONE;
          }
        }
        else
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not create list." );

          Py_RETURN_NONE;
        }
      }
      else
      {
        CPC_ERROR( "Could not modulate symbol: 0x%x.", return_value );

        Py_RETURN_NONE;
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not malloc signal array." );

      Py_RETURN_NONE;
    }
  }
}

/*! \fn     static PyObject* python_get_symbol (
              csignal_symbol_tracker* in_symbol_tracker,
              int                     in_number_of_bits
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
                    int                     in_number_of_bits
                  )
{
  UINT32 symbol = 0;

  if( 0 >= in_number_of_bits )
  {
    CPC_ERROR( "Invalid input for number of bits: %d.", in_number_of_bits );

    Py_RETURN_NONE;
  }
  else
  {
    UINT32 number_of_bits = in_number_of_bits;

    csignal_error_code return_value = csignal_get_symbol  (
      in_symbol_tracker,
      number_of_bits,
      &symbol
                                                          );

    if( CPC_ERROR_CODE_NO_ERROR != return_value )
    {
      CPC_ERROR( "Could not get symbol: 0x%x.", return_value );

      Py_RETURN_NONE;
    }
    else
    {
      return( Py_BuildValue( "I", symbol ) );
    }
  }
}

/*! \fn     csignal_symbol_tracker* python_intialize_symbol_tracker (
              PyObject* in_data
            ) 
    \brief  Creates a new symbol tracker object that points to the data in
            in_data. Symbols of variable bit-length will be read from in_data.

    \param  in_data The data buffer to read symbols from.
    \return A newly created csignal_symbol_tracker if one could be created.
            NULL otherwise.
  */
csignal_symbol_tracker*
python_intialize_symbol_tracker (
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
