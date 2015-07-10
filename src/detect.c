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
