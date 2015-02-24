%module csignal_tests

%inline %{

#include <csignal.h>

/*! \fn     static PyObject* python_modulate_symbol  (
                          int   in_symbol,
                          int   in_constellation_size,
                          float in_sample_rate,
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
                          float in_sample_rate,
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
                "Invalid input: m=0x%x, M=0x%x, T=0x%x, sr=%.2f"
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

// These have to be included because we don't recursively parse headers
%include <csignal_error_codes.h>
%include <types.h>
%include <cpcommon_error_codes.h>
%include <log_definitions.h>
%include <log_functions.h>
