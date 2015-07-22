/*! \file   detect.c
    \brief  Implementation of signal detection routines.
 */
#include "detect.h"

typedef struct detect_range_t
{
  SSIZE start_index;
  
  SSIZE end_index;
  
} detect_range;

csignal_error_code
detect_exhaustive_find_max  (
                             USIZE                in_signal_length,
                             FLOAT64*             in_signal,
                             USIZE                in_spreading_signal_length,
                             FLOAT64*             in_spread_signal,
                             SSIZE                in_start_index,
                             SSIZE                in_end_index,
                             UINT32               in_exhaustive_decimation,
                             fir_passband_filter* in_narrowband_filter,
                             fir_passband_filter* in_lowpass_filter,
                             SSIZE*               out_offset
                             );

csignal_error_code
detect_find_max (
                 USIZE                in_signal_length,
                 FLOAT64*             in_signal,
                 USIZE                in_spreading_signal_length,
                 FLOAT64*             in_spread_signal,
                 SSIZE                in_start_index,
                 SSIZE                in_end_index,
                 FLOAT64              in_exhaustive_difference,
                 UINT32               in_exhaustive_decimation,
                 fir_passband_filter* in_narrowband_filter,
                 fir_passband_filter* in_lowpass_filter,
                 SSIZE*               out_offset
                 );

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
                                 USIZE                in_multiply_buffer_length,
                                 FLOAT64*             io_multiply_buffer,
                                 FLOAT64*             out_energy
                                   );

csignal_error_code
csignal_calculate_thresholds  (
                               USIZE                in_spreading_code_length,
                               FLOAT64*             in_spreading_code,
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
       || NULL == in_signal
       || NULL == out_thresholds_length
       || NULL == out_thresholds
      )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Spreading code (0x%x), signal (0x%x), thresholds length (0x%x),"
               " or thresholds (0x%x) are null.",
               in_spreading_code,
               in_signal,
               out_thresholds_length,
               out_thresholds
               );
  }
  else
  {
    INT32 num_tests =
      in_signal_length - in_spreading_code_length;
    
    *out_thresholds_length =
      ( USIZE )
      CPC_CEIL_FLOAT64( ( num_tests * 1.0 ) / ( in_decimation * 1.0 ) );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Signal length is %d, spreading code length is %d."
             " Number of tests is %d.",
             in_signal_length,
             in_spreading_code_length,
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
        
        USIZE multiply_buffer_length  = in_spreading_code_length;
        
        return_value =
          cpc_safe_malloc (
                           ( void** ) &multiply_buffer,
                           sizeof( FLOAT64 ) * multiply_buffer_length
                           );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          for( INT32 i = 0; i < num_tests; i += in_decimation )
          {
            FLOAT64 energy = 0.0;
            
            return_value =
              detect_despread_calculate_energy  (
                                         in_spreading_code_length,
                                         in_spreading_code,
                                         &( in_signal[ i ] ),
                                         multiply_buffer_length,
                                         multiply_buffer,
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
          
          cpc_safe_free( ( void** ) &multiply_buffer );
        }
        else
        {
          CPC_ERROR( "Could not malloc buffer: 0x%x.", return_value );
          
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
                   "Signal is of length %d, spreading code length"
                   " is %d. Cannot calculate thresholds.",
                   in_signal_length,
                   in_spreading_code_length
                   );
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc buffer: 0x%x.", return_value );
      
      *out_thresholds_length = 0;
      
      cpc_safe_free( ( void** ) out_thresholds );
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_despread_calculate_energy  (
                                   UINT32               in_signal_lengths,
                                   FLOAT64*             in_signal_one,
                                   FLOAT64*             in_signal_two,
                                   USIZE                in_multiply_buffer_length,
                                   FLOAT64*             io_multiply_buffer,
                                   FLOAT64*             out_energy
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal_one
       || NULL == in_signal_two
       || NULL == io_multiply_buffer
       || NULL == out_energy
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x), signal two (0x%x), multiply buffer (0x%x),"
               " or energy (0x%x) are null.",
               in_signal_one,
               in_signal_two,
               io_multiply_buffer,
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
  else
  {
    CPC_MEMSET  (
                 io_multiply_buffer,
                 0x0,
                 sizeof( FLOAT64 ) * in_multiply_buffer_length
                 );
    
    USIZE buffer_length = in_multiply_buffer_length;
    
    return_value =
      csignal_multiply_signal  (
                                in_signal_lengths,
                                in_signal_one,
                                in_signal_lengths,
                                in_signal_two,
                                &buffer_length,
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
        
      *out_energy = 0.0;
      
      return_value =
        csignal_calculate_energy  (
                                   buffer_length,
                                   io_multiply_buffer,
                                   out_energy
                                   );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR( "Could not calculate signal energy: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_ERROR( "Could not multiply signals: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_calculate_energy (
                         USIZE                in_signal_length,
                         FLOAT64*             in_signal,
                         USIZE                in_spread_signal_length,
                         FLOAT64*             in_spread_signal,
                         fir_passband_filter* in_narrowband_filter,
                         fir_passband_filter* in_lowpass_filter,
                         FLOAT64*             out_energy
)
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == in_spread_signal
       || NULL == in_narrowband_filter
       || NULL == in_lowpass_filter
       || NULL == out_energy
      )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), narrowband (0x%x),"
               " lowpass (0x%x) or energy (0x%x) are null.",
               in_signal,
               in_spread_signal,
               in_narrowband_filter,
               in_lowpass_filter,
               out_energy
               );
  }
  else
  {
    FLOAT64* multiply_buffer  = NULL;
    FLOAT64* filter_buffer    = NULL;
    USIZE max_buffer_length   =
      in_spread_signal_length + in_narrowband_filter->number_of_taps
      + in_lowpass_filter->number_of_taps;
    
    return_value =
      cpc_safe_malloc (
                       ( void ** ) &multiply_buffer,
                       sizeof( FLOAT64 ) * max_buffer_length
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        cpc_safe_malloc (
                         ( void** ) &filter_buffer,
                         sizeof( FLOAT64 ) * max_buffer_length
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        USIZE multiply_buffer_length = max_buffer_length;
        
        return_value =
          csignal_multiply_signal (
                                   in_signal_length,
                                   in_signal,
                                   in_spread_signal_length,
                                   in_spread_signal,
                                   &multiply_buffer_length,
                                   &multiply_buffer
                                   );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          USIZE filter_buffer_length = max_buffer_length;
          
          return_value =
            csignal_filter_signal (
                                   in_narrowband_filter,
                                   multiply_buffer_length,
                                   multiply_buffer,
                                   &filter_buffer_length,
                                   &filter_buffer
                                   );
          
          if( CPC_ERROR_CODE_NO_ERROR == return_value )
          {
            multiply_buffer_length = max_buffer_length;
            
            CPC_MEMSET  (
                         multiply_buffer,
                         0x0,
                         sizeof( FLOAT64 ) * max_buffer_length
                         );
            
            return_value =
              csignal_multiply_signal (
                                       filter_buffer_length,
                                       filter_buffer,
                                       filter_buffer_length,
                                       filter_buffer,
                                       &multiply_buffer_length,
                                       &multiply_buffer
                                       );
            
            if( CPC_ERROR_CODE_NO_ERROR == return_value )
            {
              filter_buffer_length = max_buffer_length;
              
              CPC_MEMSET  (
                           filter_buffer,
                           0x0,
                           sizeof( FLOAT64 ) * max_buffer_length
                           );
              
              return_value =
                csignal_filter_signal (
                                       in_lowpass_filter,
                                       multiply_buffer_length,
                                       multiply_buffer,
                                       &filter_buffer_length,
                                       &filter_buffer
                                       );
              
              if( CPC_ERROR_CODE_NO_ERROR == return_value )
              {
                *out_energy = 0x0;
                
                return_value =
                  csignal_sum_signal  (
                                       filter_buffer_length,
                                       filter_buffer,
                                       1.0,
                                       out_energy
                                       );
                
                if( CPC_ERROR_CODE_NO_ERROR != return_value )
                {
                  *out_energy = 0.0;
                  
                  CPC_ERROR( "Could not sum signal: 0x%x.", return_value );
                }
              }
              else
              {
                CPC_ERROR (
                           "Could not lowpass filter signal: 0x%x.",
                           return_value
                           );
              }
            }
            else
            {
              CPC_ERROR( "Could not square signal: 0x%x.", return_value );
            }
          }
          else
          {
            CPC_ERROR (
                       "Could not narrowband filter signal: 0x%x.",
                       return_value
                       );
          }
        }
        else
        {
          CPC_ERROR( "Could not despread signal: 0x%x.", return_value );
        }
      }
      else
      {
        CPC_ERROR( "Could not malloc filter buffer: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc multiply buffer: 0x%x", return_value );
    }
    
    if( NULL != multiply_buffer )
    {
      cpc_safe_free( ( void** ) &multiply_buffer );
    }
    
    if( NULL != filter_buffer )
    {
      cpc_safe_free( ( void** ) &filter_buffer );
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_find_highest_energy_offset (
                               USIZE                in_signal_length,
                               FLOAT64*             in_signal,
                               USIZE                in_spreading_signal_length,
                               FLOAT64*             in_spread_signal,
                               USIZE                in_number_of_tests,
                               USIZE                in_step_size,
                               fir_passband_filter* in_narrowband_filter,
                               fir_passband_filter* in_lowpass_filter,
                               FLOAT64              in_threshold,
                               FLOAT64              in_exhaustive_difference,
                               UINT32               in_exhaustive_decimation,
                               USIZE*               out_offset
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == in_spread_signal
       || NULL == in_narrowband_filter
       || NULL == in_lowpass_filter
       || NULL == out_offset
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), narrowband (0x%x),"
               " lowpass (0x%x), or offset (0x%x) are null.",
               in_signal,
               in_spread_signal,
               in_narrowband_filter,
               in_lowpass_filter,
               out_offset
               );
  }
  else if( in_signal_length < in_spreading_signal_length )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Signal length (0x%x) must be larger or equal than"
               " spread signal length (0x%x).",
               in_signal_length,
               in_spreading_signal_length
               );
  }
  else
  {
    USIZE max_number_of_ranges =
      CPC_CEIL_FLOAT64  (
                         (
                          ( 1.0 * in_number_of_tests ) / ( 1.0 * in_step_size )
                          )
                         / 2.0
                         );
    detect_range* ranges = NULL;
    
    return_value =
      cpc_safe_malloc (
                       ( void** ) &ranges,
                       sizeof( detect_range ) * max_number_of_ranges
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      SSIZE i                 = 0;
      UINT32 number_of_ranges = 0;
      SSIZE start_index       = -1;
      
      for( i = 0; i < in_number_of_tests; i += in_step_size )
      {
        FLOAT64 signal_energy = 0.0;
        
        return_value =
          detect_calculate_energy (
                                   in_spreading_signal_length,
                                   &( in_signal[ i ] ),
                                   in_spreading_signal_length,
                                   in_spread_signal,
                                   in_narrowband_filter,
                                   in_lowpass_filter,
                                   &signal_energy
                                   );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          CPC_ERROR( "%.06f", signal_energy );
          
          if( signal_energy >= in_threshold )
          {
            if( -1 == start_index )
            {
              start_index = i;
            }
          }
          else
          {
            if( -1 != start_index )
            {
              ranges[ number_of_ranges ].start_index  = start_index;
              ranges[ number_of_ranges ].end_index    = i;
              
              number_of_ranges++;
              
              start_index = -1;
            }
          }
        }
        else
        {
          CPC_ERROR( "Could not calculate energy: 0x%x.", return_value );
        }
      }
      
      if( -1 != start_index )
      {
        ranges[ number_of_ranges ].start_index  = start_index;
        ranges[ number_of_ranges ].end_index    = i;
        
        number_of_ranges++;
      }
      
      CPC_LOG (
               CPC_LOG_LEVEL_ERROR,
               "Number of ranges is %d.",
               number_of_ranges
               );
      
      if( 0 == number_of_ranges )
      {
        return_value = CSIGNAL_ERROR_CODE_NO_RESULT;
        
        CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not find signal." );
      }
      else
      {
        for( i = 0; i < number_of_ranges; i++ )
        {
          CPC_LOG (
                   CPC_LOG_LEVEL_ERROR,
                   "Range %d: %d to %d.",
                   ( i + 1 ),
                   ranges[ i ].start_index,
                   ranges[ i ].end_index
                   );
          
          SSIZE offset = 0;
          
          detect_find_max (
                           in_signal_length,
                           in_signal,
                           in_spreading_signal_length,
                           in_spread_signal,
                           ranges[ i ].start_index,
                           ranges[ i ].end_index,
                           in_exhaustive_difference,
                           in_exhaustive_decimation,
                           in_narrowband_filter,
                           in_lowpass_filter,
                           &offset
                           );
          
          CPC_ERROR( "Max offset is %d.", offset );
          
          if( offset >= 0 )
          {
            *out_offset = offset;
          }
          else
          {
            return_value = CSIGNAL_ERROR_CODE_NO_RESULT;
            
            CPC_ERROR( "Offest (%d) was found to be negative.", offset );
          }
        }
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc range array: 0x%x.", return_value );
    }
    
    if( NULL != ranges )
    {
      cpc_safe_free( ( void** ) &ranges );
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_exhaustive_find_max  (
                             USIZE                in_signal_length,
                             FLOAT64*             in_signal,
                             USIZE                in_spreading_signal_length,
                             FLOAT64*             in_spread_signal,
                             SSIZE                in_start_index,
                             SSIZE                in_end_index,
                             UINT32               in_exhaustive_decimation,
                             fir_passband_filter* in_narrowband_filter,
                             fir_passband_filter* in_lowpass_filter,
                             SSIZE*               out_offset
                             )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == in_spread_signal
       || NULL == out_offset
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), or offset (0x%x).",
               in_signal,
               in_spread_signal,
               out_offset
               );
  }
  else if( in_end_index < in_start_index )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "End index (0x%x) must be greater than or equal"
               " to start index (0x%x).",
               in_end_index,
               in_start_index
               );
  }
  else
  {
    FLOAT64 max_energy = 0.0;
    
    *out_offset = 0;
    
    CPC_LOG (
             CPC_LOG_LEVEL_ERROR,
             "Performing exhaustive search between %d and %d.",
             in_start_index,
             in_end_index
             );
    
    for (
         SSIZE i = in_start_index;
         i < in_end_index;
         i += in_exhaustive_decimation
         )
    {
      FLOAT64 energy = 0.0;
      
      return_value =
        detect_calculate_energy (
                                 in_spreading_signal_length,
                                 &( in_signal[ i ] ),
                                 in_spreading_signal_length,
                                 in_spread_signal,
                                 in_narrowband_filter,
                                 in_lowpass_filter,
                                 &energy
                                 );
      
      if( energy > max_energy )
      {
        max_energy  = energy;
        *out_offset = i;
        
        CPC_LOG (
                 CPC_LOG_LEVEL_ERROR,
                 "Max energy: %.04f\tIndex: %d",
                 max_energy,
                 *out_offset
                 );
      }
    }
  }
  
  return( return_value );
}

csignal_error_code
detect_find_max (
                 USIZE                in_signal_length,
                 FLOAT64*             in_signal,
                 USIZE                in_spreading_signal_length,
                 FLOAT64*             in_spread_signal,
                 SSIZE                in_start_index,
                 SSIZE                in_end_index,
                 FLOAT64              in_exhaustive_difference,
                 UINT32               in_exhaustive_decimation,
                 fir_passband_filter* in_narrowband_filter,
                 fir_passband_filter* in_lowpass_filter,
                 SSIZE*               out_offset
                 )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == in_spread_signal
       || NULL == out_offset
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal (0x%x), spread signal (0x%x), or offset (0x%x).",
               in_signal,
               in_spread_signal,
               out_offset
               );
  }
  else if( in_end_index < in_start_index )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "End index (0x%x) must be greater than or equal"
               " to start index (0x%x).",
               in_end_index,
               in_start_index
               );
  }
  else
  {
    FLOAT64 high_index_energy = 0.0;
    FLOAT64 low_index_energy  = 0.0;
    
    CPC_LOG (
             CPC_LOG_LEVEL_ERROR,
             "Start index: %d\tEnd index: %d.",
             in_start_index,
             in_end_index
             );
    
    SSIZE mid_index   =
      ( SSIZE) ( 1.0 * in_start_index + 1.0 * in_end_index ) / 2.0;
    SSIZE high_index  =
      ( SSIZE ) ( 1.0 * mid_index + 1.0 * in_end_index ) / 2.0;
    SSIZE low_index   =
      ( SSIZE ) ( 1.0 * in_start_index + 1.0 * mid_index ) / 2.0;
    
    CPC_LOG (
             CPC_LOG_LEVEL_ERROR,
             "Low: %d\tMid: %d\tHigh: %d.",
             low_index, 
             mid_index,
             high_index
             );
    
    return_value =
      detect_calculate_energy (
                               in_spreading_signal_length,
                               &( in_signal[ high_index ] ),
                               in_spreading_signal_length,
                               in_spread_signal,
                               in_narrowband_filter,
                               in_lowpass_filter,
                               &high_index_energy
                               );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        detect_calculate_energy (
                                 in_spreading_signal_length,
                                 &( in_signal[ low_index ] ),
                                 in_spreading_signal_length,
                                 in_spread_signal,
                                 in_narrowband_filter,
                                 in_lowpass_filter,
                                 &low_index_energy
                                 );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        CPC_LOG (
                 CPC_LOG_LEVEL_ERROR,
                 "Low energy: %.04f\tHi energy: %.04f",
                 low_index_energy,
                 high_index_energy
                 );
        
        if  (
             fabs( high_index_energy - low_index_energy )
             / high_index_energy <= in_exhaustive_difference
             )
        {
          return_value =
            detect_exhaustive_find_max  (
                                         in_signal_length,
                                         in_signal,
                                         in_spreading_signal_length,
                                         in_spread_signal,
                                         in_start_index,
                                         in_end_index,
                                         in_exhaustive_decimation,
                                         in_narrowband_filter,
                                         in_lowpass_filter,
                                         out_offset
                                         );
          
          if( CPC_ERROR_CODE_NO_ERROR != return_value )
          {
            CPC_ERROR (
                       "Could not exhaustively find max: 0x%x.",
                       return_value
                       );
          }
        }
        else if( high_index_energy > low_index_energy )
        {
          return  (
                   detect_find_max  (
                                     in_signal_length,
                                     in_signal,
                                     in_spreading_signal_length,
                                     in_spread_signal,
                                     mid_index,
                                     in_end_index,
                                     in_exhaustive_difference,
                                     in_exhaustive_decimation,
                                     in_narrowband_filter,
                                     in_lowpass_filter,
                                     out_offset
                                     )
                   );
        }
        else
        {
          return  (
                   detect_find_max  (
                                     in_signal_length,
                                     in_signal,
                                     in_spreading_signal_length,
                                     in_spread_signal,
                                     in_start_index,
                                     mid_index,
                                     in_exhaustive_difference,
                                     in_exhaustive_decimation,
                                     in_narrowband_filter,
                                     in_lowpass_filter,
                                     out_offset
                                     )
                   );
        }
      }
      else
      {
        CPC_ERROR (
                   "Could not calculate energy for index %d: 0x%x.",
                   low_index,
                   return_value
                   );
      }
    }
    else
    {
      CPC_ERROR (
                 "Could not calculate energy for index %d: 0x%x.",
                 high_index,
                 return_value
                 );
    }
  }
  
  return( return_value );
}
