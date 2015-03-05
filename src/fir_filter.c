#include "fir_filter.h"

csignal_error_code
csignal_initialize_passband_filter (
                                    FLOAT32               in_first_passband,
                                    FLOAT32               in_second_passband,
                                    UINT32                in_sampling_frequency,
                                    UINT32                in_number_of_taps,
                                    fir_passband_filter*  out_filter
                                    )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if (
           0 >= in_first_passband
           || 0 >= in_second_passband
           || in_first_passband >= in_second_passband
           || 0 == in_number_of_taps
           || 0 == in_sampling_frequency
           )
  {
    CPC_ERROR (
               "First passband (%.2f) must be positive and strictly less than"
               " second passband (%.2f). The number of taps (0x%x) must be"
               " positive. The sampling frequency (%d Hz) must be positive.",
               in_first_passband,
               in_second_passband,
               in_number_of_taps,
               in_sampling_frequency
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    return_value =
      cpc_safe_malloc (
                       ( void** ) &( out_filter->coefficients ),
                       sizeof( FLOAT32 ) * in_number_of_taps
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      out_filter->first_passband  = in_first_passband;
      out_filter->second_passband = in_second_passband;
      out_filter->number_of_taps  = in_number_of_taps;
    }
    else
    {
      CPC_ERROR( "Could not malloc coefficients: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_destroy_passband_filter(
                                fir_passband_filter* io_filter
                                )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NULL_POINTER;
  
  if( NULL == io_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    if( NULL != io_filter->coefficients )
    {
      return_value =
        cpc_safe_free( ( void** ) &( io_filter->coefficients ) );
    }
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value = cpc_safe_free( ( void** ) &io_filter );
    }
  }
  
  return( return_value );
}
