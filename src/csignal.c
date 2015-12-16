/*! \file   csignal.c
    \brief  This file contains the implementation for generating data modulated
            audio signals. Currently, implementations for BFSK and DSSS are
            supported.
 
    \author Brent Carrara
 */

#include "csignal.h"

/*! \fn     UINT32 csignal_greatest_common_divisor (
              UINT32 in_u,
              UINT32 in_v
            )
    \brief  Calcualtes the greatest common divisor (GCD) of in_u and in_v and
            returns the result.
 
    \note   The implementation is the Binary GCD Algorithm (or Stein's Algorithm)
            and the source was copied from Wikipedia at the following URL:
 
            https://en.wikipedia.org/wiki/Binary_GCD_algorithm
 
    \param  in_u  First of two input factors to find the GCD of.
    \param  in_v  Second of two input factors to find the GCD of.
    \return GCD( in_u, in_v )
 */
UINT32
csignal_greatest_common_divisor (
                                 UINT32 in_u,
                                 UINT32 in_v
                                 );

/*! \fn     UINT32 csignal_gray_code_encode  (
              UINT32 in_input
            )
    \brief  Converts the input value, in_input, to a Gray Code value. A Gray
            Code changes only one bit in adjacent values, ex. 00 01 11 10 is a
            2-bit Gray Code.
 
    \param  in_input  The number to converted to Gray Code.
    \return A Gray Code encoded value of in_input
 */
UINT32
csignal_gray_code_encode  (
                           UINT32 in_input
                           );
void
csignal_initialize( void )
{
  if( !cpc_is_initialized() )
  {
    cpc_initialize();
  }
}

void
csignal_terminate( void )
{
  if( cpc_is_initialized() )
  {
    cpc_terminate();
  }
}

csignal_error_code
csignal_BFSK_determine_frequencies  (
                                     UINT32   in_samples_per_symbol,
                                     UINT32   in_sample_rate,
                                     FLOAT32  in_carrier_frequency,
                                     FLOAT64* out_symbol_0_frequency,
                                     FLOAT64* out_symbol_1_frequency,
                                     FLOAT64* out_delta_frequency
                                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == out_symbol_0_frequency
       || NULL == out_symbol_1_frequency
       || NULL == out_delta_frequency
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Symbol 0 (0x%x), 1 (0x%x), or delta (0x%x) are null.",
               out_symbol_0_frequency,
               out_symbol_1_frequency,
               out_delta_frequency
               );
  }
  else if( 0 == in_samples_per_symbol || 0 == in_sample_rate )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Samples per symbol (%d) and sample rate (%d) must"
               " be greater than zero.",
               in_samples_per_symbol,
               in_sample_rate
               );
  }
  else
  {
    FLOAT64 integer_part = 0.0;
    
    *out_delta_frequency =
      ( in_sample_rate / in_samples_per_symbol );
    
    modf( ( in_carrier_frequency / *out_delta_frequency ), &integer_part );
    
    *out_symbol_0_frequency =
      ( integer_part * *out_delta_frequency ) - in_carrier_frequency;
    *out_symbol_1_frequency = *out_symbol_0_frequency + *out_delta_frequency;
    
    CPC_LOG (
             CPC_LOG_LEVEL_DEBUG,
             "Symbol 0 frequency is %.02f. Symbol 1 frequency is %.02f."
             " Delta frequency is %.02f",
             *out_symbol_0_frequency,
             *out_symbol_1_frequency,
             *out_delta_frequency
             );
  }
  
  return( return_value );
}

csignal_error_code
csignal_modulate_BFSK_symbol  (
                               UINT32     in_symbol,
                               UINT32     in_samples_per_symbol,
                               UINT32     in_sample_rate,
                               FLOAT32    in_carrier_frequency,
                               USIZE*     out_signal_length,
                               FLOAT64**  out_signal_inphase,
                               FLOAT64**  out_signal_quadrature
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == out_signal_length
       || NULL == out_signal_inphase
       || NULL == out_signal_quadrature
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Lenght (0x%x), inphase (0x%x), or quadrature (0x%x) are NULL.",
               out_signal_length,
               out_signal_inphase,
               out_signal_quadrature
               );
  }
  else if( 2 <= in_symbol )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR( "Symbol (%d) must be 0 or 1.", in_symbol );
  }
  else if( 0 == in_samples_per_symbol || 0 == in_sample_rate )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Samples per symbol (%d) and sample rate (%d) must"
               " be greater than zero.",
               in_samples_per_symbol,
               in_sample_rate
               );
  }
  else
  {
    *out_signal_length = in_samples_per_symbol;
    
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_signal_inphase,
                       sizeof( FLOAT64 ) * *out_signal_length
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        cpc_safe_malloc (
                         ( void** ) out_signal_quadrature,
                         sizeof( FLOAT64 ) * *out_signal_length
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        FLOAT64 symbol_0_frequency  = 0.0;
        FLOAT64 symbol_1_frequency  = 0.0;
        FLOAT64 delta_frequency     = 0.0;
        
        return_value =
          csignal_BFSK_determine_frequencies  (
                                               in_samples_per_symbol,
                                               in_sample_rate,
                                               in_carrier_frequency,
                                               &symbol_0_frequency,
                                               &symbol_1_frequency,
                                               &delta_frequency
                                               );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          FLOAT64 frequency =
            ( in_symbol ? symbol_1_frequency : symbol_0_frequency );
          
          CPC_LOG (
                   CPC_LOG_LEVEL_ERROR,
                   "Symbol: %d\tFrequency: %.02f",
                   in_symbol,
                   frequency
                   );
          
          for( UINT32 i = 0; i < *out_signal_length; i++ )
          {
            ( *out_signal_inphase )[ i ] =
            cos (
                 2.0 * M_PI * in_carrier_frequency
                 * ( i * 1.0 ) / ( in_sample_rate * 1.0 )
                 )
            * cos (
                   2.0 * M_PI * frequency
                   * ( i * 1.0 ) / ( in_sample_rate * 1.0 )
                   );
            
            ( *out_signal_quadrature )[ i ] =
            sin (
                 2.0 * M_PI * in_carrier_frequency
                 * ( i * 1.0 ) / ( in_sample_rate * 1.0 )
                 )
            * sin (
                   2.0 * M_PI * frequency
                   * ( i * 1.0 ) / ( in_sample_rate * 1.0 )
                   );
          }
        }
        else
        {
          CPC_ERROR( "Could not determine frequencies: 0x%x.", return_value );
        }
      }
      else
      {
        CPC_ERROR( "Could not malloc quadrature: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc inphase: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_modulate_symbol (
                         UINT32   in_symbol,
                         UINT32   in_constellation_size,
                         FLOAT64* out_symbol_inphase,
                         FLOAT64* out_symbol_quadrature
                         )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  CPC_LOG (
           CPC_LOG_LEVEL_TRACE,
           "m=0x%x, M=0x%x",
           in_symbol,
           in_constellation_size
           );
  
  if( NULL == out_symbol_inphase || NULL == out_symbol_quadrature )
  {
    CPC_ERROR (
               "Inphase signal (0x%x) or quadrature signal (0x%x) are null.",
               out_symbol_inphase,
               out_symbol_quadrature
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if ( in_symbol >= in_constellation_size )
  {
    CPC_ERROR (
               "Symbol (0x%x) must be strictly smaller "
               "than constellation size (0x%x)",
               in_symbol,
               in_constellation_size
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    UINT32 gray_code_symbol = csignal_gray_code_encode( in_symbol );
    FLOAT64 phase           =
      ( 1.0 * gray_code_symbol ) / ( 1.0 * in_constellation_size );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Encoded symbol 0x%x as 0x%x. Phase is %.2f",
             in_symbol,
             gray_code_symbol,
             phase
             );
    
    *out_symbol_inphase     = cos( 2.0 * M_PI * phase );
    *out_symbol_quadrature  = sin( 2.0 * M_PI * phase );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "I=%.2f, Q=%.2f",
             *out_symbol_inphase,
             *out_symbol_quadrature
             );
  }
  
  return( return_value );
}

UINT32
csignal_gray_code_encode  (
                           UINT32 in_input
                           )
{
  return( ( in_input >> 1 ) ^ in_input );
}

csignal_error_code
csignal_spread_signal (
                       gold_code* io_gold_code,
                       UINT32     in_chip_duration,
                       USIZE      in_signal_size,
                       FLOAT64*   io_signal
                       )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_gold_code || NULL == io_signal )
  {
    CPC_ERROR (
               "Gold code (0x%x) or signal (0x%x) are null.",
               io_gold_code,
               io_signal
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 != ( in_signal_size % in_chip_duration ) )
  {
    CPC_ERROR (
               "Signal size must be evenly divisble by chip duration.",
               in_signal_size,
               in_chip_duration
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else if( 0 == in_chip_duration )
  {
    CPC_ERROR (
               "Chip duration (0x%x) must be strictly postiive.",
               in_chip_duration
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    USIZE number_of_code_bits   = in_signal_size / in_chip_duration;
    USIZE size                  = 0;
    UCHAR* code                 = NULL;
    
    return_value =
      csignal_get_gold_code( io_gold_code, number_of_code_bits, &size, &code );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      FLOAT64* spreading_signal = NULL;
      USIZE signal_offset       = 0;
      
      CPC_LOG_BUFFER( CPC_LOG_LEVEL_DEBUG, "Code:", code, size, 8 );
      
      return_value =
        cpc_safe_malloc (
                         ( void** ) &spreading_signal,
                         sizeof( FLOAT64 ) * in_chip_duration
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        for( USIZE i = 0; i < number_of_code_bits; i++ )
        {
          USIZE bit_offset  = i % ( sizeof( UCHAR ) * 8 );
          USIZE byte_offset = i / ( sizeof( UCHAR ) * 8 );
          
          UCHAR mask = ( 0x80 >> bit_offset );
          UCHAR bit   = code[ byte_offset ] & mask;
          
          return_value =
            csignal_set_spreading_signal  (
                                           bit,
                                           in_chip_duration,
                                           spreading_signal
                                           );
          
          for( USIZE j = 0; j < in_chip_duration; j++ )
          {
            io_signal[ signal_offset++ ] *= spreading_signal[ j ];
          }
        }
        
        cpc_safe_free( ( void** ) &spreading_signal );
      }
      else
      {
        CPC_ERROR( "Could not malloc spreading signal: 0x%x.", return_value );
      }
    }
    
    if( NULL != code )
    {
      cpc_safe_free( ( void** ) &code );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_multiply_signal  (
                           USIZE       in_signal_one_length,
                           FLOAT64*    in_signal_one,
                           USIZE       in_signal_two_length,
                           FLOAT64*    in_signal_two,
                           USIZE*      out_signal_length,
                           FLOAT64**   out_signal
                           )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal_one
       || NULL == in_signal_two
       || NULL == out_signal
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x), two (0x%x), or out (0x%x) are null.",
               in_signal_one,
               in_signal_two,
               out_signal
               );
  }
  else if( in_signal_one_length != in_signal_two_length )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Signal one length (%d) is not equal to signal two length (%d).",
               in_signal_one_length,
               in_signal_two_length
               );
  }
  else if (
           *out_signal_length != 0
           && ( in_signal_one_length > *out_signal_length )
           )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Out signal length (%d) must be greater or equal to"
               " the length of signal one (%d).",
               *out_signal_length,
               in_signal_one_length
               );
  }
  else if( *out_signal_length != 0 && NULL == *out_signal )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Out signal length (%d) is set, but out signal (0x%x) is null.",
               *out_signal_length,
               *out_signal
               );
  }
  else
  {
    if( NULL == *out_signal )
    {
      return_value =
        cpc_safe_malloc (
                         ( void** ) out_signal,
                         sizeof( FLOAT64 )
                         * in_signal_one_length
                         );
    }
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      *out_signal_length = in_signal_one_length;
      
      for( SSIZE i = 0; i < *out_signal_length; i++ )
      {
        FLOAT64 value = in_signal_one[ i ] * in_signal_two[ i ];
        
        ( *out_signal )[ i ] = value;
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc new signal: 0x%x.", return_value );
      
      *out_signal_length = 0;
      *out_signal        = NULL;
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_calculate_energy  (
                           USIZE    in_signal_length,
                           FLOAT64* in_signal,
                           FLOAT64* out_energy
                           )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == out_energy
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Signal one (0x%x) or energy (0x%x) are null.",
               in_signal,
               out_energy
               );
  }
  else
  {
    *out_energy = 0.0;
    
    for( SSIZE i = 0; i < in_signal_length; i++ )
    {
      *out_energy += CPC_POW_FLOAT64( in_signal[ i ], 2 );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_sum_signal(
                   USIZE    in_signal_length,
                   FLOAT64* in_signal,
                   FLOAT64  in_scalar,
                   FLOAT64* out_sum
                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_signal || NULL == out_sum )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR( "Signal (0x%x) or sum (0x%x) are null.", in_signal, out_sum );
  }
  else
  {
    *out_sum = 0.0;
    
    for( UINT32 i = 0; i < in_signal_length; i++ )
    {
      *out_sum += in_scalar * in_signal[ i ];
    }
  }
  
  return( return_value );
}

UINT32
csignal_greatest_common_divisor (
                                 UINT32 in_u,
                                 UINT32 in_v
                                 )
{
  // simple cases (termination)
  if( in_u == in_v )
  {
    return( in_u );
  }
  
  if( 0 == in_u )
  {
    return( in_v );
  }
  
  if( 0 == in_v )
  {
    return( in_u );
  }
  
  // look for factors of 2
  if( ~in_u & 1 ) // u is even
  {
    if( in_v & 1 )
    {
      // v is odd
      return( csignal_greatest_common_divisor( in_u >> 1, in_v ) );
    }
    else
    {
      // both u and v are even
      return( csignal_greatest_common_divisor( in_u >> 1, in_v >> 1 ) << 1 );
    }
  }
  
  if( ~in_v & 1 )
  {
    // u is odd, v is even
    return( csignal_greatest_common_divisor( in_u, in_v >> 1 ) );
  }
  
  // reduce larger argument
  if( in_u > in_v )
  {
    return( csignal_greatest_common_divisor( ( in_u - in_v ) >> 1, in_v ) );
  }
  else
  {
    return( csignal_greatest_common_divisor( ( in_v - in_u ) >> 1, in_u ) );
  }
}
