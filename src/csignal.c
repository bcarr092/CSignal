/*! \fn     csignal.c
    \breif  This file contains the implementation for generating data modulated
            audio signals. Currently, implementations for QPSK and DSSS are
            supported.
 */

#include "csignal.h"

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

csignal_error_code
csignal_initialize_symbol_tracker  (
                                    UCHAR*                    in_data,
                                    USIZE                     in_data_length,
                                    csignal_symbol_tracker ** out_symbol_tracker
                                    )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  ( *out_symbol_tracker ) = NULL;
  
  if( NULL == in_data )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Data buffer is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =  cpc_safe_malloc (
                                     ( void ** ) out_symbol_tracker,
                                     sizeof( csignal_symbol_tracker )
                                     );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      ( *out_symbol_tracker )->data           = in_data;
      ( *out_symbol_tracker )->data_length    = in_data_length;
      ( *out_symbol_tracker )->byte_offset    = 0;
      ( *out_symbol_tracker )->bit_offset     = 0;
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_destroy_symbol_tracker  (
                                 csignal_symbol_tracker* io_symbol_tracker
                                 )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_symbol_tracker )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Symbol tracker is  null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value = cpc_safe_free( ( void** ) &io_symbol_tracker );
  }
  
  return( return_value );
}

csignal_error_code
csignal_get_symbol  (
                     csignal_symbol_tracker*  in_symbol_tracker,
                     UINT32                   in_number_of_bits,
                     UINT32*                  out_symbol
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  CPC_LOG (
           CPC_LOG_LEVEL_TRACE,
           "ist=0x%x, inob=0x%x, os=0x%x",
           in_symbol_tracker,
           in_number_of_bits,
           out_symbol
           );
  
  if( NULL == in_symbol_tracker || NULL == out_symbol )
  {
    CPC_ERROR (
               "Received a null pointer: tracker=0x%x, symbol=0x%x.",
               in_symbol_tracker,
               out_symbol
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    CPC_LOG_BUFFER  (
                     CPC_LOG_LEVEL_TRACE,
                     "Tracker data:",
                     in_symbol_tracker->data,
                     in_symbol_tracker->data_length,
                     8
                     );
    
    if( in_symbol_tracker->byte_offset == in_symbol_tracker->data_length )
    {
      return_value = CSIGNAL_ERROR_CODE_NO_DATA;
    }
    else
    {
      
      UINT32 mask       = 0;
      UINT32 new_offset = 0;
      UINT32 num_bytes  = CPC_MIN  (
                                    UINT32,
                                    in_symbol_tracker->data_length
                                    - in_symbol_tracker->byte_offset,
                                    sizeof( UINT32 )
                                    );
      
      *out_symbol = 0;
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Number of bytes: 0x%x.", num_bytes );
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Byte offset: 0x%x.",
               in_symbol_tracker->byte_offset
               );
      
      cpc_memcpy  (
                   out_symbol,
                   in_symbol_tracker->data
                    + ( in_symbol_tracker->byte_offset * sizeof( UCHAR ) ),
                   num_bytes
                   );
      
      *out_symbol = CPC_HTONL( *out_symbol );
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Symbol contains: 0x%x.", *out_symbol );
      
      *out_symbol >>= ( sizeof( UINT32 ) * 8 )
        - ( in_symbol_tracker->bit_offset + in_number_of_bits );
      
      for( UINT32 i = 0; i < in_number_of_bits; i++ )
      {
        mask <<= 1;
        
        mask |= 0x00000001;
      }
      
      *out_symbol &= mask;
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Symbol contains: 0x%x.", *out_symbol );
      
      new_offset = in_symbol_tracker->bit_offset + in_number_of_bits;
      
      CPC_LOG( CPC_LOG_LEVEL_TRACE, "Offset is: 0x%x.", new_offset );
      
      in_symbol_tracker->byte_offset  +=  new_offset / ( sizeof( UCHAR ) * 8 );
      in_symbol_tracker->bit_offset   =   new_offset % ( sizeof( UCHAR )  * 8 );
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Byte offset: 0x%x,",
               in_symbol_tracker->byte_offset
               );
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Bit offset: 0x%x,",
               in_symbol_tracker->bit_offset
               );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_modulate_symbol (
                         UINT32   in_symbol,
                         UINT32   in_constellation_size,
                         UINT32   in_sample_rate,
                         UINT32   in_symbol_duration,
                         INT16    in_baseband_pulse_amplitude,
                         FLOAT32  in_carrier_frequency,
                         INT16*   out_signal
                         )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  CPC_LOG (
           CPC_LOG_LEVEL_TRACE,
           "m=0x%x, M=0x%x, T=0x%x, sr=0x%x, |g|=0x%x, f_c=%.2f",
           in_symbol,
           in_constellation_size,
           in_symbol_duration,
           in_sample_rate,
           in_baseband_pulse_amplitude,
           in_carrier_frequency
           );
  
  if( NULL == out_signal )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Signal array is null." );
    
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
  else if( 0 >= in_carrier_frequency )
  {
    CPC_ERROR (
               "Carrier frequency (%.2f) must be strictly greater than 0.",
               in_carrier_frequency
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    UINT32 gray_code_symbol = csignal_gray_code_encode( in_symbol );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Encoded symbol 0x%x as 0x%x.",
             in_symbol,
             gray_code_symbol
             );
    
    FLOAT32 inphase_component =
      cos( 2 * M_PI * in_symbol / in_constellation_size );
    FLOAT32 quadrature_component =
      sin( 2 * M_PI * in_symbol / in_constellation_size );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "I=%.2f, Q=%.2f",
             inphase_component,
             quadrature_component
             );
    
    for( UINT32 i = 0; i < in_symbol_duration; i++ )
    {
      out_signal[ i ] =
        ( INT16 )
        ( in_baseband_pulse_amplitude * 1.0 ) * inphase_component
          * cos( 2 * M_PI * in_carrier_frequency * i / ( in_sample_rate * 1.0 ) )
        - ( in_baseband_pulse_amplitude * 1.0 ) * quadrature_component
          * sin( 2 * M_PI * in_carrier_frequency * i / ( in_sample_rate * 1.0 ) );
    }
    
    CPC_LOG_BUFFER  (
                     CPC_LOG_LEVEL_TRACE,
                     "Signal:",
                     ( UCHAR* ) out_signal,
                     in_symbol_duration * ( sizeof( INT16 ) / sizeof( UCHAR ) ),
                     8
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
                       UINT32     in_signal_size,
                       INT16*     io_signal
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
    UINT32 number_of_code_bits  = in_signal_size / in_chip_duration;
    UINT32 size                 = 0;
    UCHAR* code                 = NULL;
    
    return_value =
      csignal_get_gold_code( io_gold_code, number_of_code_bits, &size, &code );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      INT16* spreading_signal = NULL;
      UINT32 signal_offset    = 0;
      
      return_value =
        cpc_safe_malloc (
                         ( void** ) &spreading_signal,
                         sizeof( INT16 ) * in_chip_duration
                         );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        for( UINT32 i = 0; i < number_of_code_bits; i++ )
        {
          UINT32 bit_offset   = i % ( sizeof( UCHAR ) * 8 );
          UINT32 byte_offset  = i / ( sizeof( UCHAR ) * 8 );
          
          UCHAR mask = ( 0x80 >> bit_offset );
          UCHAR bit   = code[ byte_offset ] & mask;
          
          return_value =
            csignal_set_spreading_signal  (
                                           bit,
                                           in_chip_duration,
                                           spreading_signal
                                           );
          
          for( UINT32 j = 0; j < in_chip_duration; j++ )
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
  }
  
  return( return_value );
}
