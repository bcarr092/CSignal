/*! \file   detect.c
    \brief  Implementation of signal detection routines.
 */
#include "detect.h"

/*! \def    csignal_error_code detect_despread_calculate_energy  (
              UINT32               in_signal_lengths,
              FLOAT64*             in_signal_one,
              FLOAT64*             in_signal_two,
              fir_passband_filter* in_filter,
              FLOAT64*             out_energy
            )
    \brief  Helper function that despreads, filters, and calculates the energy
            in the despread signal and stores it in out_energy.
 
    \param  in_signal_lengths The length of signal one and two.
    \param  in_signal_one The reference spreading signal to be despread.
    \param  in_signal_two The spreading signal whose energy is to be calculated.
    \param  in_filter The filter to apply after despreading.
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
    
    return_value =
      csignal_filter_signal (
                             in_wideband_filter,
                             in_signal_length,
                             in_signal,
                             &wideband_filtered_signal_length,
                             &wideband_filtered_signal
                             );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      INT32 num_tests =
        wideband_filtered_signal_length - in_spreading_code_length;
      
      *out_thresholds_length =
        ( USIZE )
        CPC_CEIL_FLOAT64( ( num_tests * 1.0 ) / ( in_decimation * 1.0 ) );
      
      return_value =
        cpc_safe_malloc (
                         ( void** ) out_thresholds,
                         sizeof( FLOAT64 ) * *out_thresholds_length
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        if( 0 < num_tests )
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
    
      cpc_safe_free( ( void** ) &wideband_filtered_signal );
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
                                   FLOAT64*             out_energy
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal_one
       || NULL == in_signal_two
       || NULL == in_filter
       || NULL == out_energy
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x), signal two (0x%x), filter (0x%x),"
               " or energy (0x%x) are null.",
               in_signal_one,
               in_signal_two,
               in_filter,
               out_energy
               );
  }
  else
  {
    USIZE     ouput_signal_length = 0;
    FLOAT64*  output_signal       = NULL;
      
    return_value =
    csignal_multipliy_signal  (
                               in_signal_lengths,
                               in_signal_one,
                               in_signal_lengths,
                               in_signal_two,
                               &ouput_signal_length,
                               &output_signal
                               );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      USIZE narrowband_filtered_signal_length = 0;
      FLOAT64* narrowband_filtered_signal     = NULL;
      
      return_value =
      csignal_filter_signal (
                             in_filter,
                             ouput_signal_length,
                             output_signal,
                             &narrowband_filtered_signal_length,
                             &narrowband_filtered_signal
                             );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        return_value =
        csignal_calculate_energy  (
                                   narrowband_filtered_signal_length,
                                   narrowband_filtered_signal,
                                   out_energy
                                   );
        
        if( CPC_ERROR_CODE_NO_ERROR != return_value )
        {
          CPC_ERROR (
                     "Could not calculate signal energy: 0x%x.",
                     return_value
                     );
        }
        
        cpc_safe_free( ( void** ) &narrowband_filtered_signal );
      }
      else
      {
        CPC_ERROR( "Could not filter signal: 0x%x.", return_value );
      }
      
      cpc_safe_free( ( void** ) &output_signal );
    }
    else
    {
      CPC_ERROR( "Could not multiply signals: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}
