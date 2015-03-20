/*! \file   fir_filter.c
    \brief  The implementation of a passband FIR filter and filter (correlation)
            algorithm.
 
    \author Brent Carrara
 */
#include "fir_filter.h"

csignal_error_code
csignal_initialize_passband_filter (
                                    FLOAT32               in_first_passband,
                                    FLOAT32               in_second_passband,
                                    UINT32                in_sampling_frequency,
                                    USIZE                 in_number_of_taps,
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
           || 0 == in_sampling_frequency
           )
  {
    CPC_ERROR (
               "First passband (%.2f) must be positive and strictly less than"
               " second passband (%.2f). The sampling frequency (%d Hz) must be"
               " positive.",
               in_first_passband,
               in_second_passband,
               in_sampling_frequency
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    return_value =
      cpc_safe_malloc (
                       ( void** ) &( out_filter->coefficients ),
                       sizeof( FLOAT64 ) * in_number_of_taps
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

csignal_error_code
csignal_filter_signal (
                       fir_passband_filter* in_filter,
                       USIZE                in_signal_length,
                       FLOAT64*             in_signal,
                       USIZE*               out_filtered_signal_length,
                       FLOAT64**            out_filtered_signal
                       )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_filter
       || NULL == in_signal
       || NULL == out_filtered_signal_length
       || NULL == out_filtered_signal
       )
  {
    CPC_ERROR (
               "Filter (0x%x), signal (0x%x), filtered signal (0x%x),"
               " or filtered signal length (0x%x) are null.",
               in_filter,
               in_signal,
               out_filtered_signal,
               out_filtered_signal_length
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 == in_signal_length || 0 == in_filter->number_of_taps )
  {
    CPC_LOG_STRING  (
                     CPC_LOG_LEVEL_ERROR,
                     "Signal length must be greater than 0."
                     );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    *out_filtered_signal_length = 0;
    *out_filtered_signal        = NULL;
  }
  else
  {
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_filtered_signal,
                       sizeof( FLOAT64 )
                        * ( in_filter->number_of_taps + in_signal_length )
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      *out_filtered_signal_length =
        in_signal_length + in_filter->number_of_taps;
      
      for( SSIZE i = 0; i < *out_filtered_signal_length; i++ )
      {
        FLOAT64 value = 0;
        
        SSIZE min =
          ( i >= in_filter->number_of_taps - 1 )
          ? i - ( in_filter->number_of_taps - 1 ) : 0;
        
        SSIZE max = ( i < in_signal_length - 1 ) ? i : in_signal_length - 1 ;
        
        for( SSIZE j = min; j <= max; j++ )
        {
          value += in_signal[ j ] * in_filter->coefficients[ i - j ];
        }
        
        ( *out_filtered_signal )[ i ] = value;
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc filtered signal: 0x%x.", return_value );
      
      *out_filtered_signal_length = 0;
      *out_filtered_signal        = NULL;
    }
  }
  
  return( return_value );
}
