/*! \file   fir_filter.c
    \brief  This is the implementation of the helper functions for the FIR
            filter struct.
 
    \author Brent Carrara
 */

#include "kaiser_filter.h"

/*! \fn     csignal_error_code csignal_set_kaiser_lowpass_weights  (
              FLOAT64              in_centre_frequency,
              FLOAT64              in_alpha,
              fir_passband_filter* io_filter
            )
    \brief  Calculates and sets the weights of the impulse response for a low
            pass Kaiser filter
 
    \param  in_centre_frequency The cutoff frequency. This is the frequency
                                value (in radians) in the transition region
                                between passband and stopband.
    \param  in_alpha  The alpha value is derived from the input parameters to
                      the kaiser initialization function and is used to set the
                      window coefficients.
    \param  io_filter If no error is detected the impulse response coefficients
                      will be set.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):

            CPC_ERROR_CODE_NULL_POINTER If io_filter is null.
 */
csignal_error_code
csignal_set_kaiser_lowpass_weights  (
                                     FLOAT64              in_centre_frequency,
                                     FLOAT64              in_alpha,
                                     fir_passband_filter* io_filter
                                     );

/*! \fn     csignal_error_code csignal_set_kaiser_lowpass_impulse_response  (
              FLOAT32              in_passband,
              FLOAT32              in_stopband,
              FLOAT32              in_passband_attenuation,
              FLOAT32              in_stopband_attenuation,
              UINT32               in_sampling_frequency,
              fir_passband_filter* out_filter
            )
    \brief  This function largely calculates all the required parameters to
            generate the Kaiser low pass filter coefficients and then calls
            set_kaiser_weights to actually calculate the filter and window
            weights and set the impulse response values.
 
    \param  in_passband The cutoff frequency, i.e., upper passband frequency.
                        The passband range is [0,passband]. Units are Hz.
    \param  in_stopband The stopband start frequency. The range of the stopband
                        is [stopband, sampling_frequency/2]. Units are Hz.
    \param  in_passband_attenuation The amount of "ripple" in magnitude (dB),
                                    i.e deviation from 0 dB in the passband
                                    region. Units are dB.
    \param  in_stopband_attenuation The amount of attenuation in magnitude (dB)
                                    from the passband region to the stopband
                                    region. Units are dB.
    \param  in_sampling_frequency The sampling frequency used to generate the
                                  filter. Units are Hz.
    \param  out_filter  A fir_passband_filter struct with the impulse response
                        coefficients populate if no error was detected.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):

            CPC_ERROR_CODE_NULL_POINTER If out_filter is null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If the number of taps in io_filter
                                              is even.
 */
csignal_error_code
csignal_set_kaiser_lowpass_impulse_response  (
                                  FLOAT32              in_passband,
                                  FLOAT32              in_stopband,
                                  FLOAT32              in_passband_attenuation,
                                  FLOAT32              in_stopband_attenuation,
                                  UINT32               in_sampling_frequency,
                                  fir_passband_filter* out_filter
                                              );

/*! \fn     csignal_error_code csignal_set_kaiser_impulse_response  (
              FLOAT32              in_first_stopband,
              FLOAT32              in_first_passband,
              FLOAT32              in_second_passband,
              FLOAT32              in_second_stopband,
              FLOAT32              in_passband_attenuation,
              FLOAT32              in_stopband_attenuation,
              UINT32               in_sampling_frequency,
              fir_passband_filter* out_filter
            )
    \brief  This function largely calculates all the required parameters to
            generate the Kaiser filter coefficients and then calls
            set_kaiser_weights to actually calculate the filter and window
            weights and set the impulse response values.
 
    \param  in_first_stopband The upper range value in the first stopband. The
                              range of the first stopband is [0,first_stopband].
                              Units are Hz.
    \param  in_first_passband The lower range value in the passband. The
                              passband range is [first,second]. Units are Hz.
    \param  in_second_passband  The upper range value in the passband. The
                                passband range is [ first, second ]. Units are
                                Hz.
    \param  in_second_stopband  The lower range value in the second stopband.
                                The range of the second stopband is [second,
                                sampling_frequency/2]. Units are Hz.
    \param  in_passband_attenuation The amount of "ripple" in magnitude (dB),
                                    i.e deviation from 0 dB in the passband
                                    region. Units are dB.
    \param  in_stopband_attenuation The amount of attenuation in magnitude (dB)
                                    from the passband region to the stopband
                                    region. Units are dB.
    \param  in_sampling_frequency The sampling frequency used to generate the
                                  filter. Units are Hz.
    \param  out_filter  A fir_passband_filter struct with the impulse response
                        coefficients populate if no error was detected.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):

            CPC_ERROR_CODE_NULL_POINTER If out_filter is null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If the number of taps in io_filter
                                              is even.
 */
csignal_error_code
csignal_set_kaiser_impulse_response  (
                                  FLOAT32              in_first_stopband,
                                  FLOAT32              in_first_passband,
                                  FLOAT32              in_second_passband,
                                  FLOAT32              in_second_stopband,
                                  FLOAT32              in_passband_attenuation,
                                  FLOAT32              in_stopband_attenuation,
                                  UINT32               in_sampling_frequency,
                                  fir_passband_filter* out_filter
                                      );

/*! \fn     csignal_error_code csignal_set_kaiser_weights  (
              FLOAT64              in_first_cutoff_frequency,
              FLOAT64              in_second_cutoff_frequency,
              FLOAT64              in_alpha,
              fir_passband_filter* io_filter
            )
    \brief  Calculates and sets the weights of the impulse response.
 
    \param  in_first_cutoff_frequency The lower cutoff frequency. This is the
                                      frequency value (in radians) in the
                                      transition region below first_passband.
    \param  in_second_cutoff_frequency  The upper cutoff frequency. This is the
                                        frequency value (in radians) in the
                                        transition region above second_passband.
    \param  in_alpha  The alpha value is derived from the input parameters to
                      the kaiser initialization function and is used to set the
                      window coefficients.
    \param  io_filter If no error is detected the impulse response coefficients
                      will be set.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):

            CPC_ERROR_CODE_NULL_POINTER If io_filter is null.
 */
csignal_error_code
csignal_set_kaiser_weights  (
                             FLOAT64              in_first_cutoff_frequency,
                             FLOAT64              in_second_cutoff_frequency,
                             FLOAT64              in_alpha,
                             fir_passband_filter* io_filter
                             );

csignal_error_code
csignal_initialize_kaiser_filter  (
                                 FLOAT32                in_first_stopband,
                                 FLOAT32                in_first_passband,
                                 FLOAT32                in_second_passband,
                                 FLOAT32                in_second_stopband,
                                 FLOAT32                in_passband_attenuation,
                                 FLOAT32                in_stopband_attenuation,
                                 UINT32                 in_sampling_frequency,
                                 fir_passband_filter**  out_filter
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if (
           0 > in_first_stopband
           || 0 >= in_first_passband
           || 0 >= in_second_passband
           || 0 >= in_second_stopband
           || in_first_stopband >= in_first_passband
           || in_second_passband >= in_second_stopband
           || 0 == in_sampling_frequency
           )
  {
    CPC_ERROR (
               "First passband (%.2f) must be positive and strictly larger than"
               " first stopband (%.2f). Second stopband (%.2f) must be strictly"
               " positive and larger than second passband (%.2f). Sampling"
               " frequency (%d Hz) must be positive.",
               in_first_passband,
               in_first_stopband,
               in_second_stopband,
               in_second_passband,
               in_sampling_frequency
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "First stopband is %.2f, first passband is %.2f,"
             " second passband is %.2f, second stop band is %.2f."
             " Passband attenuation is %.2f and stopband attenuation is %.2f."
             " Sampling frequency is %d Hz.",
             in_first_stopband,
             in_first_passband,
             in_second_passband,
             in_second_stopband,
             in_passband_attenuation,
             in_stopband_attenuation,
             in_sampling_frequency
             );
    
    return_value =
      cpc_safe_malloc( ( void**) out_filter, sizeof( fir_passband_filter ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_set_kaiser_impulse_response (
                                             in_first_stopband,
                                             in_first_passband,
                                             in_second_passband,
                                             in_second_stopband,
                                             in_passband_attenuation,
                                             in_stopband_attenuation,
                                             in_sampling_frequency,
                                             *out_filter
                                             );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        csignal_error_code destroy_return_code =
          csignal_destroy_passband_filter( *out_filter );
        
        if( CPC_ERROR_CODE_NO_ERROR != destroy_return_code )
        {
          CPC_ERROR( "Could not free filter: 0x%x.", destroy_return_code );
          
          *out_filter = NULL;
          
          return_value = destroy_return_code;
        }
      }
      else
      {
        ( *out_filter )->sampling_frequency = in_sampling_frequency;
      }
    }
    else
    {
      CPC_ERROR( "Coudl not malloc filter: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_set_kaiser_impulse_response  (
                                   FLOAT32              in_first_stopband,
                                   FLOAT32              in_first_passband,
                                   FLOAT32              in_second_passband,
                                   FLOAT32              in_second_stopband,
                                   FLOAT32              in_passband_attenuation,
                                   FLOAT32              in_stopband_attenuation,
                                   UINT32               in_sampling_frequency,
                                   fir_passband_filter* out_filter
                                      )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    FLOAT64 delta_f =
      CPC_MIN (
               FLOAT32,
               ( in_first_passband - in_first_stopband ),
               ( in_second_stopband - in_second_passband )
               );
    
    FLOAT64 cutoff_a = in_first_passband - ( 0.5 * delta_f );
    FLOAT64 cutoff_b = in_second_passband + ( 0.5 * delta_f );
    
    FLOAT64 w_a = ( 2 * M_PI * cutoff_a ) / ( 1.0 * in_sampling_frequency );
    FLOAT64 w_b = ( 2 * M_PI * cutoff_b ) / ( 1.0 * in_sampling_frequency );
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "pi=%.40f", M_PI );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "f_a=%.20f, w_a=%.20f, f_b=%.20f, w_b=%.20f",
             cutoff_a,
             w_a,
             cutoff_b,
             w_b
             );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Delta_f is %.2f. First cutoff frequency is %.2f Hz (%.4f rad)."
             " Second cutoff frequency is %.2f Hz (%.4f rad)",
             delta_f,
             cutoff_a,
             w_a,
             cutoff_b,
             w_b
             );
    
    FLOAT64 delta_passband =
      ( CPC_POW( FLOAT64, 10, in_passband_attenuation / 20.0 ) - 1 )
      / ( CPC_POW( FLOAT64, 10, in_passband_attenuation / 20.0 ) + 1 );
    FLOAT64 delta_stopband =
      CPC_POW( FLOAT64, 10, ( -1.0 * in_stopband_attenuation ) / 20.0 );
    
    FLOAT64 delta = CPC_MIN( FLOAT64, delta_passband, delta_stopband );
    
    FLOAT64 attenuation = -20 * CPC_LOGARITHM_10( FLOAT64, delta );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Delta passband is %e, delta stopband is %e."
             " Min delta is %e resulting in an attenuation of %.2f dB.",
             delta_passband,
             delta_stopband,
             delta,
             attenuation
             );
    
    FLOAT64 alpha             = 0;
    FLOAT64 transition_width  = 0;
    UINT32 number_of_taps     = 0;
    
    if( 50 <= attenuation )
    {
      alpha = 0.1102 * ( attenuation - 8.7 );
    }
    else if( 21 <= attenuation && 50 > attenuation )
    {
      alpha =
        0.5842 * CPC_POW( FLOAT64, attenuation - 21, 0.4 ) + 0.07886
        * ( attenuation - 21 );
    }
    
    if( 21 < attenuation )
    {
      transition_width = ( attenuation - 7.95 ) / 14.36;
    }
    else
    {
      transition_width = 0.922;
    }
    
    number_of_taps =
      ( UINT32 )
        CPC_CEIL  (
                   FLOAT64,
                   ( transition_width * in_sampling_frequency ) / delta_f + 1
                   );
    
    if( 0 == number_of_taps % 2 )
    {
      number_of_taps++;
    }
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Alpha is %.2f resulting in transition width of %.2f."
             " The number of taps is %d.",
             alpha,
             transition_width,
             number_of_taps
             );
    
    return_value =
      csignal_initialize_passband_filter  (
                                           in_first_passband,
                                           in_second_passband,
                                           in_sampling_frequency,
                                           number_of_taps,
                                           out_filter
                                           );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value = csignal_set_kaiser_weights( w_a, w_b, alpha, out_filter );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR (
                   "Could not set weights on kaiser filter: 0x%x.",
                   return_value
                   );
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not initialize filter." );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_set_kaiser_weights  (
                             FLOAT64              in_first_cutoff_frequency,
                             FLOAT64              in_second_cutoff_frequency,
                             FLOAT64              in_alpha,
                             fir_passband_filter* io_filter
                             )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 == io_filter->number_of_taps % 2 )
  {
    CPC_ERROR (
               "Number of taps (0x%x) must be odd.",
               io_filter->number_of_taps
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    INT32 middle_tap = ( io_filter->number_of_taps - 1 ) / 2;
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Middle tap is at index %d of %d taps."
             " First cutoff frequency is %.13e rad, second is %.13e rad.",
             middle_tap,
             io_filter->number_of_taps,
             in_first_cutoff_frequency,
             in_second_cutoff_frequency
             );
    
    for( INT32 i = 0; i < io_filter->number_of_taps; i++ )
    {
      FLOAT64 numerator =
        ( FLOAT64 )
          ( in_alpha * CPC_SQRT( FLOAT32, ( FLOAT32 ) i * ( 2 * middle_tap - i ) ) )
            / ( middle_tap * 1.0 );
      
      FLOAT64 window_coefficient =
        cpc_bessel_i0( numerator ) / cpc_bessel_i0( in_alpha );
      
      FLOAT64 filter_coefficient = 0;
      
      if( i != middle_tap )
      {
        filter_coefficient =
          (
           sin( in_second_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ) )
           - sin( in_first_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ) )
          ) / ( M_PI * ( ( i - middle_tap ) * 1.0 ) );
      }
      else
      {
        filter_coefficient =
          ( in_second_cutoff_frequency - in_first_cutoff_frequency ) / M_PI;
      }
      
      io_filter->coefficients[ i ] = window_coefficient * filter_coefficient;
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "%d: w=%.14e, d=%.14e, h=%.14e",
               i,
               window_coefficient,
               filter_coefficient,
               io_filter->coefficients[ i ]
               );
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "w_a=%.14e, w_b=%.14e",
               in_first_cutoff_frequency,
               in_second_cutoff_frequency
               );
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "%.14e -> %.14e, %.14e -> %.14e, %.14e",
               in_second_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ),
               sin( in_second_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ) ),
               in_first_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ),
               sin( in_first_cutoff_frequency * ( ( i - middle_tap ) * 1.0 ) ),
               M_PI * ( ( i - middle_tap ) * 1.0 )
               );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_inititalize_kaiser_lowpass_filter(
                                  FLOAT32               in_passband,
                                  FLOAT32               in_stopband,
                                  FLOAT32               in_passband_attenuation,
                                  FLOAT32               in_stopband_attenuation,
                                  UINT32                in_sampling_frequency,
                                  fir_passband_filter** out_filter
                                          )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if (
           0 >= in_passband
           || 0 >= in_stopband
           || in_passband >= in_stopband
           || 0 == in_sampling_frequency
           )
  {
    CPC_ERROR (
               "Passband (%.2f) must be positive and strictly smaller than"
               " stopband (%.2f). Stopband (%.2f) must be strictly"
               " positive. Sampling frequency (%d Hz) must be positive.",
               in_passband,
               in_stopband,
               in_stopband,
               in_sampling_frequency
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Passband is %.2f, stopband is %.2f, and sampling frequency"
             " is %d Hz.",
             in_passband,
             in_stopband,
             in_sampling_frequency
             );
    
    return_value =
    cpc_safe_malloc( ( void**) out_filter, sizeof( fir_passband_filter ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_set_kaiser_lowpass_impulse_response (
                                                     in_passband,
                                                     in_stopband,
                                                     in_passband_attenuation,
                                                     in_stopband_attenuation,
                                                     in_sampling_frequency,
                                                     *out_filter
                                                     );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        csignal_error_code destroy_return_code =
          csignal_destroy_passband_filter( *out_filter );
        
        if( CPC_ERROR_CODE_NO_ERROR != destroy_return_code )
        {
          CPC_ERROR( "Could not free filter: 0x%x.", destroy_return_code );
          
          *out_filter = NULL;
          
          return_value = destroy_return_code;
        }
      }
      else
      {
        ( *out_filter )->sampling_frequency = in_sampling_frequency;
      }
    }
    else
    {
      CPC_ERROR( "Coudl not malloc filter: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_set_kaiser_lowpass_impulse_response  (
                                  FLOAT32              in_passband,
                                  FLOAT32              in_stopband,
                                  FLOAT32              in_passband_attenuation,
                                  FLOAT32              in_stopband_attenuation,
                                  UINT32               in_sampling_frequency,
                                  fir_passband_filter* out_filter
                                              )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    FLOAT64 delta_f   = in_stopband - in_passband;
    FLOAT64 centre_f  = 0.5 * ( in_passband + in_stopband );
    
    FLOAT64 passband_frequency  = centre_f - ( 0.5 * delta_f );
    FLOAT64 stopband_frequency  = centre_f + ( 0.5 * delta_f );
    
    FLOAT64 w_centre    =
      ( 2 * M_PI * centre_f ) / ( 1.0 * in_sampling_frequency );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Delta_f is %.2f. Passband frequency is %.2f Hz."
             " Stopband frequency is %.2f Hz. Centre frequency is %.2f rad.",
             delta_f,
             passband_frequency,
             stopband_frequency,
             w_centre
             );
    
    FLOAT64 delta_passband =
    ( CPC_POW( FLOAT64, 10, in_passband_attenuation / 20.0 ) - 1 )
    / ( CPC_POW( FLOAT64, 10, in_passband_attenuation / 20.0 ) + 1 );
    FLOAT64 delta_stopband =
    CPC_POW( FLOAT64, 10, ( -1.0 * in_stopband_attenuation ) / 20.0 );
    
    FLOAT64 delta = CPC_MIN( FLOAT64, delta_passband, delta_stopband );
    
    FLOAT64 attenuation = -20 * CPC_LOGARITHM_10( FLOAT64, delta );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Delta passband is %e, delta stopband is %e."
             " Min delta is %e resulting in an attenuation of %.2f dB.",
             delta_passband,
             delta_stopband,
             delta,
             attenuation
             );
    
    FLOAT64 alpha             = 0;
    FLOAT64 transition_width  = 0;
    UINT32 number_of_taps     = 0;
    
    if( 50 <= attenuation )
    {
      alpha = 0.1102 * ( attenuation - 8.7 );
    }
    else if( 21 <= attenuation && 50 > attenuation )
    {
      alpha =
      0.5842 * CPC_POW( FLOAT64, attenuation - 21, 0.4 ) + 0.07886
      * ( attenuation - 21 );
    }
    
    if( 21 < attenuation )
    {
      transition_width = ( attenuation - 7.95 ) / 14.36;
    }
    else
    {
      transition_width = 0.922;
    }
    
    number_of_taps =
    ( UINT32 )
    CPC_CEIL  (
               FLOAT64,
               ( transition_width * in_sampling_frequency ) / delta_f + 1
               );
    
    if( 0 == number_of_taps % 2 )
    {
      number_of_taps++;
    }
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Alpha is %.2f resulting in transition width of %.2f."
             " The number of taps is %d.",
             alpha,
             transition_width,
             number_of_taps
             );
    
    return_value =
      csignal_initialize_passband_filter  (
                                           0,
                                           in_passband,
                                           in_sampling_frequency,
                                           number_of_taps,
                                           out_filter
                                           );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_set_kaiser_lowpass_weights( w_centre, alpha, out_filter );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR (
                   "Could not set weights on kaiser filter: 0x%x.",
                   return_value
                   );
      }
    }
    else
    {
      CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not initialize filter." );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_set_kaiser_lowpass_weights  (
                                     FLOAT64              in_centre_frequency,
                                     FLOAT64              in_alpha,
                                     fir_passband_filter* io_filter
                                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_filter )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Filter was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 == io_filter->number_of_taps % 2 )
  {
    CPC_ERROR (
               "Number of taps (0x%x) must be odd.",
               io_filter->number_of_taps
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    INT32 middle_tap = ( io_filter->number_of_taps - 1 ) / 2;
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Middle tap is at index %d of %d taps."
             " Cutoff frequency is at %.13e rad.",
             middle_tap,
             io_filter->number_of_taps,
             in_centre_frequency
             );
    
    for( INT32 i = 0; i < io_filter->number_of_taps; i++ )
    {
      FLOAT64 numerator =
        ( FLOAT64 )
          ( in_alpha * CPC_SQRT( FLOAT64, ( FLOAT64 ) i * ( 2.0 * middle_tap - i ) ) )
          / ( middle_tap * 1.0 );
      
      FLOAT64 window_coefficient =
        cpc_bessel_i0( numerator ) / cpc_bessel_i0( in_alpha );
      
      FLOAT64 filter_coefficient = 0;
      
      if( i != middle_tap )
      {
        filter_coefficient =
          ( sin( in_centre_frequency * ( ( i - middle_tap ) * 1.0 ) ) )
          / ( M_PI * ( ( i - middle_tap ) * 1.0 ) );
      }
      else
      {
        filter_coefficient = in_centre_frequency / M_PI;
      }
      
      io_filter->coefficients[ i ] = window_coefficient * filter_coefficient;
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "%d: w=%.14e, d=%.14e, h=%.14e",
               i,
               window_coefficient,
               filter_coefficient,
               io_filter->coefficients[ i ]
               );
    }
  }
  
  return( return_value );
}
