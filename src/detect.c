/*! \file   detect.c
    \brief  Implementation of signal detection routines.
 */
#include "detect.h"

/*! \var    detect_range
    \brief  Struct used to hold range information. This struct is used in the
            find_max algorithm to store the offsets that are likely to contain
            a hill that needs to be climbed to find a peak.
 */
typedef struct detect_range_t
{
  /*! \var    start_index
      \brief  The start index in the range, must be <= to end_index.
   */
  SSIZE start_index;
  
  /*! \var    end_index
      \brief  The end index in the range, must be >= start_index.
   */
  SSIZE end_index;
  
} detect_range;

/*! \fn     csignal_error_code detect_exhaustive_find_max  (
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
    \brief  Performs an exhaustive search for the offset in in_signal between
            in_start_index and in_end_index that has the highest energy.
 
    \param  in_signal_length  The number of elements in in_signal.
    \param  in_signal The signal whose energy is to be calculated.
    \param  in_spreading_signal_length The number of elements in in_spread_signal.
    \param  in_spread_signal  The signal used to despread in_signal.
    \param  in_start_index  The index to begin the search at (the first index
                            in the range to search).
    \param  in_end_index  The index to end the search at (the last index in the
                          range to search).
    \param  in_exhaustive_decimation  The decimation factor (step size) to use
                                      when the algorithm enters the exhaustive
                                      search mode.
    \param  in_narrowband_filter  A BPF applied after in_signal is despread.
    \param  in_lowpass_filter A LPF applied to remove the double frequency
                              term created after squaring the signal.
    \param  out_offset  The offset in in_signal with the highest energy.
    \return Returns NO_ERROR upon succesful exection or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If in_start_index > in_end_index.
 */
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

/*! \fn     csignal_error_code detect_find_max (
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
    \brief  Performs a hill climbing algorithm until the energy difference on
            both sides of the hill are within in_exhaustive_differnece
            (percentage) of one another. Once this occurrs the algorithm switches
            to exhaustive search.
 
    \note The hill climbing alorithm works by cutting the list of candidates for
          peak energy in half each iteration. Each iteration two offsets are
          calculated, one in the first half of the range and one in the second.
          Their energy is calculated and compared. If one energy is much higher
          than the other the next iteration will look at that half, otherwise
          exhaustive search is performed.
    \note This algorithm will recursively call itself on smaller and smaller
          ranges.
 
    \param  in_signal_length  The number of elements in in_signal.
    \param  in_signal The signal whose energy is to be calculated.
    \param  in_spreading_signal_length The number of elements in in_spread_signal.
    \param  in_spread_signal  The signal used to despread in_signal.
    \param  in_start_index  The index to begin the search at (the first index
                            in the range to search).
    \param  in_end_index  The index to end the search at (the last index in the
                          range to search).
    \param  in_exhaustive_difference  When performing the hill climbing
                                      algorithm the search strategy will switch
                                      to exhaustive search when the energy on
                                      both sides of the hill are within this
                                      difference (note that this is a percentage
                                      difference).
    \param  in_exhaustive_decimation  The decimation factor (step size) to use
                                      when the algorithm enters the exhaustive
                                      search mode.
    \param  in_narrowband_filter  A BPF applied after in_signal is despread.
    \param  in_lowpass_filter A LPF applied to remove the double frequency
                              term created after squaring the signal.
    \param  out_offset  The offset in in_signal with the highest energy.
    \return Returns NO_ERROR upon succesful exection or one of these errors:
 
            CPC_ERROR_CODE_NULL_POINTER If any of the input parameters are null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If in_start_index > in_end_index.
 */
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
            SSIZE start_index = offset - in_exhaustive_decimation;
            SSIZE end_index   = offset + in_exhaustive_decimation;
            
            CPC_LOG (
                     CPC_LOG_LEVEL_ERROR,
                     "Secondary exhaustive search between %d and %d.",
                     start_index,
                     end_index
                     );
            
            return_value =
              detect_exhaustive_find_max  (
                                           in_signal_length,
                                           in_signal,
                                           in_spreading_signal_length,
                                           in_spread_signal,
                                           start_index,
                                           end_index,
                                           1,
                                           in_narrowband_filter,
                                           in_lowpass_filter,
                                           &offset
                                           );
            
            if( CPC_ERROR_CODE_NO_ERROR == return_value )
            {
              *out_offset = offset;
            }
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
