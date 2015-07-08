/*! \file   detect.c
    \brief  Implementation of signal detection routines.
 */
#include "detect.h"

/*! \def    csignal_error_code detect_despread_calculate_energy  (
              UINT32               in_signal_lengths,
              FLOAT64*             in_signal_one,
              FLOAT64*             in_signal_two,
              fir_passband_filter* in_filter,
              USIZE                in_multiply_buffer_length,
              FLOAT64*             io_multiply_buffer,
              USIZE                in_filter_buffer_length,
              FLOAT64*             io_filter_buffer,
              FLOAT64*             out_energy
            )
    \brief  Helper function that despreads, filters, and calculates the energy
            in the despread signal and stores it in out_energy. io_buffer is
            used as a scratch space for efficiency (i.e., to reduce mallocs).
 
    \param  in_signal_lengths The length of signal one and two.
    \param  in_signal_one The reference spreading signal to be despread.
    \param  in_signal_two The spreading signal whose energy is to be calculated.
    \param  in_filter The filter to apply after despreading.
    \param  in_multiply_buffer_length The number of elements in
                                      io_multiply_buffer.
    \param  io_multiply_buffer  A malloc'd buffer to be used as scratch space for
                                the multiplication operation to reduce the
                                number of mallocs.
    \param  in_filter_buffer_length  The number of elements in io_filter_buffer.
    \param  io_filter_buffer  A malloc'd buffer to be used as scratch space for
                              the filter operation to reduce the number of
                              mallocs.
    \param  out_energy The energy in the signal after despreading.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
 */
csignal_error_code
detect_despread_calculate_energy  (
                                 UINT32               in_signal_lengths,
                                 FLOAT64*             in_signal_one,
                                 FLOAT64*             in_signal_two,
                                 fir_passband_filter* in_filter,
                                 USIZE                in_multiply_buffer_length,
                                 FLOAT64*             io_multiply_buffer,
                                 USIZE                in_filter_buffer_length,
                                 FLOAT64*             io_filter_buffer,
                                 FLOAT64*             out_energy
                                   );

csignal_error_code
csignal_calculate_thresholds  (
                               UINT32               in_spreading_code_length,
                               FLOAT64*             in_spreading_code,
                               fir_passband_filter* in_wideband_filter,
                               fir_passband_filter* in_narrowband_filter,
                               USIZE                in_signal_length,
                               FLOAT64*             in_signal,
                               UINT32               in_decimation,
                               USIZE*               out_thresholds_length,
                               FLOAT64**            out_thresholds
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_spreading_code
       || NULL == in_wideband_filter
       || NULL == in_narrowband_filter
       || NULL == in_signal
       || NULL == out_thresholds_length
       || NULL == out_thresholds
      )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Spreading code (0x%x), wideband filter (0x%x), narrowband"
               " filter (0x%x), signal (0x%x), thresholds length (0x%x),"
               " or thresholds (0x%x) are null.",
               in_spreading_code,
               in_wideband_filter,
               in_narrowband_filter,
               in_signal,
               out_thresholds_length,
               out_thresholds
               );
  }
  else
  {
    USIZE     wideband_filtered_signal_length  = 0;
    FLOAT64*  wideband_filtered_signal         = NULL;
    
//    return_value =
//      csignal_filter_signal (
//                             in_wideband_filter,
//                             in_signal_length,
//                             in_signal,
//                             &wideband_filtered_signal_length,
//                             &wideband_filtered_signal
//                             );
    
    wideband_filtered_signal_length = in_signal_length;
    wideband_filtered_signal        = in_signal;
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      INT32 num_tests =
        wideband_filtered_signal_length - in_spreading_code_length;
      
      *out_thresholds_length =
        ( USIZE )
        CPC_CEIL_FLOAT64( ( num_tests * 1.0 ) / ( in_decimation * 1.0 ) );
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Signal length is %d, filtered signal lenght is %d."
               " Number of tests is %d.",
               in_signal_length,
               wideband_filtered_signal_length,
               *out_thresholds_length
               );
      
      return_value =
        cpc_safe_malloc (
                         ( void** ) out_thresholds,
                         sizeof( FLOAT64 ) * *out_thresholds_length
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        if( 0 < num_tests )
        {
          FLOAT64* multiply_buffer  = NULL;
          FLOAT64* filter_buffer    = NULL;
          
          USIZE multiply_buffer_length  = in_spreading_code_length;
          USIZE filter_buffer_length    =
            csignal_calculate_closest_power_of_two( in_spreading_code_length )
            * 2;
          
          return_value =
            cpc_safe_malloc (
                             ( void** ) &multiply_buffer,
                             sizeof( FLOAT64 ) * multiply_buffer_length
                             );
          
          if( CPC_ERROR_CODE_NO_ERROR == return_value )
          {
            return_value =
              cpc_safe_malloc (
                               ( void** ) &filter_buffer,
                               sizeof( FLOAT64 ) * filter_buffer_length
                               );
            
            if( CPC_ERROR_CODE_NO_ERROR == return_value )
            {
              for( INT32 i = 0; i < num_tests; i += in_decimation )
              {
                FLOAT64 energy = 0.0;
                
                return_value =
                  detect_despread_calculate_energy  (
                                             in_spreading_code_length,
                                             &( wideband_filtered_signal[ i ] ),
                                             in_spreading_code,
                                             in_narrowband_filter,
                                             multiply_buffer_length,
                                             multiply_buffer,
                                             filter_buffer_length,
                                             filter_buffer,
                                             &energy
                                                     );
                
                if( CPC_ERROR_CODE_NO_ERROR == return_value )
                {
                  INT32 index = ( INT32 ) ( i / in_decimation );
                  
                  ( *out_thresholds )[ index ] = energy;
                }
                else
                {
                  CPC_ERROR (
                             "Could not calculate despread signal energy: 0x%x.",
                             return_value
                             );
                  
                  *out_thresholds_length = 0;
                  
                  cpc_safe_free( ( void** ) out_thresholds );
                  
                  break;
                }
              }
              
              cpc_safe_free( ( void** ) &filter_buffer );
            }
            else
            {
              *out_thresholds_length = 0;
              
              cpc_safe_free( ( void** ) out_thresholds );
            }
            
            cpc_safe_free( ( void** ) &multiply_buffer );
          }
          else
          {
            *out_thresholds_length = 0;
            
            cpc_safe_free( ( void** ) out_thresholds );
          }
        }
        else
        {
          return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
          
          *out_thresholds_length = 0;
          
          cpc_safe_free( ( void** ) out_thresholds );
          
          CPC_ERROR (
                     "Filtered signal is of length %d, spreading code length"
                     " is %d. Cannot calculate thresholds.",
                     wideband_filtered_signal_length,
                     in_spreading_code_length
                     );
        }
      }
    
//      cpc_safe_free( ( void** ) &wideband_filtered_signal );
    }
    else
    {
      CPC_ERROR( "Could not filter signal: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_despread_calculate_energy  (
                                 UINT32               in_signal_lengths,
                                 FLOAT64*             in_signal_one,
                                 FLOAT64*             in_signal_two,
                                 fir_passband_filter* in_filter,
                                 USIZE                in_multiply_buffer_length,
                                 FLOAT64*             io_multiply_buffer,
                                 USIZE                in_filter_buffer_length,
                                 FLOAT64*             io_filter_buffer,
                                 FLOAT64*             out_energy
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal_one
       || NULL == in_signal_two
       || NULL == in_filter
       || NULL == io_multiply_buffer
       || NULL == io_filter_buffer
       || NULL == out_energy
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x), signal two (0x%x), filter (0x%x),"
               " multiply buffer (0x%x), filter buffer (0x%x), or"
               " energy (0x%x) are null.",
               in_signal_one,
               in_signal_two,
               in_filter,
               io_multiply_buffer,
               io_filter_buffer,
               out_energy
               );
  }
  else if (
           in_multiply_buffer_length != in_signal_lengths
           )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Multiply buffer length (%d) must be equal to signal"
               " length (%d).",
               in_multiply_buffer_length,
               in_signal_lengths
               );
  }
  else if (
           in_filter_buffer_length
           != csignal_calculate_closest_power_of_two( in_signal_lengths ) * 2
           )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Filter buffer lenght (%d) must be equal to two times the next"
               " power of 2 larger than signal length (%d).",
               in_filter_buffer_length,
               csignal_calculate_closest_power_of_two( in_signal_lengths )
               );
  }
  else
  {
    CPC_MEMSET  (
                 io_multiply_buffer,
                 0x0,
                 sizeof( FLOAT64 ) * in_multiply_buffer_length
                 );
    
    return_value =
      csignal_multiply_signal  (
                                in_signal_lengths,
                                in_signal_one,
                                in_signal_lengths,
                                in_signal_two,
                                &in_multiply_buffer_length,
                                &io_multiply_buffer
                                );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      CPC_LOG_BUFFER_FLOAT64  (
                               CPC_LOG_LEVEL_TRACE,
                               "Multiply buffer:",
                               io_multiply_buffer,
                               200,
                               8
                               );
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Buffer length: %d, fft length: %d.",
               in_multiply_buffer_length,
               in_filter_buffer_length
               );
      
      CPC_MEMSET  (
                   io_filter_buffer,
                   0x0,
                   sizeof( FLOAT64 ) * in_filter_buffer_length
                   );
      
      return_value =
        csignal_calculate_FFT (
                               in_multiply_buffer_length,
                               io_multiply_buffer,
                               &in_filter_buffer_length,
                               &io_filter_buffer
                               );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        CPC_LOG_BUFFER_FLOAT64  (
                                 CPC_LOG_LEVEL_TRACE,
                                 "Filter buffer:",
                                 io_filter_buffer,
                                 200,
                                 8
                                 );
        
        FLOAT64 delta = 1.0 / ( in_filter->sampling_frequency * 1.0 );
        
        UINT32 N = ( UINT32 ) ( in_filter_buffer_length * 1.0 ) / 2.0;
        
        UINT32 positive_lowerbound_index =
          CPC_FLOOR_FLOAT64( in_filter->first_passband * delta * N );
        UINT32 positive_upperbound_index =
          CPC_CEIL_FLOAT64( in_filter->second_passband * delta * N );
        
        UINT32 negative_upperbound_index =
          CPC_FLOOR_FLOAT64 (
                             -1.0 * in_filter->second_passband * delta
                             * N + N
                             );
        UINT32 negative_lowerbound_index =
          CPC_CEIL_FLOAT64  (
                             -1.0 * in_filter->first_passband * delta
                             * N + N
                             );
        
        CPC_LOG (
                 CPC_LOG_LEVEL_TRACE,
                 "Summing from %d to %d and %d to %d.",
                 positive_lowerbound_index,
                 positive_upperbound_index,
                 negative_upperbound_index,
                 negative_lowerbound_index
                 );
        
        *out_energy = 0.0;
        
        for (
             UINT32 i = positive_lowerbound_index;
             i <= positive_upperbound_index;
             i += 2
             )
        {
          *out_energy +=
            CPC_POW_FLOAT64( io_filter_buffer[ i ], 2 )
            + CPC_POW_FLOAT64( io_filter_buffer[ i + 1 ], 2 );
        }
        
        for (
             UINT32 i = negative_upperbound_index;
             i <= negative_lowerbound_index;
             i += 2
             )
        {
          *out_energy +=
            CPC_POW_FLOAT64( io_filter_buffer[ i ], 2 )
            + CPC_POW_FLOAT64( io_filter_buffer[ i + 1 ], 2 );
        }
        
        *out_energy = CPC_SQRT_FLOAT64( *out_energy );
      }
      else
      {
        CPC_ERROR( "Could not calculate FFT: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_ERROR( "Could not multiply signals: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}
