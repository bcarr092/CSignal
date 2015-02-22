%module csignal_tests

%inline %{

#include <csignal.h>

static PyObject*
python_get_symbol (
                    csignal_symbol_tracker* in_symbol_tracker,
                    int                     in_number_of_bits
                  )
{
  UINT32 symbol           = 0;

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
