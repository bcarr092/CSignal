/*! \file   spreading_code.c
    \brief  This file contains all the implementation of generating both the
            spreading code and spreading signal (i.e. the c(t) function in SS
            modulation).
 
    \author Brent Carrara
 */

#include "spreading_code.h"

/*! \var    polynomial_masks
    \brief  The bit-masks used to filter out unneeded bit positions for variable
            degree calculations. The first entry in the array is for r = 2, that
            is only the 2 MSBs from the state and generator polynomial are ever
            used.
 */
UINT32 polynomial_masks[] =
{
  0xC0000000,
  0xE0000000,
  0xF0000000,
  0xF8000000,
  0xFC000000,
  0xFE000000,
  0xFF000000,
  0xFF800000,
  0xFFC00000,
  0xFFE00000,
  0xFFF00000,
  0xFFF80000,
  0xFFFC0000,
  0xFFFE0000,
  0xFFFF0000,
  0xFFFF8000,
  0xFFFFC000,
  0xFFFFE000,
  0xFFFFF000,
  0xFFFFF800,
  0xFFFFFC00,
  0xFFFFFE00,
  0xFFFFFF00,
  0xFFFFFF80,
  0xFFFFFFC0,
  0xFFFFFFE0,
  0xFFFFFFF0,
  0xFFFFFFF8,
  0xFFFFFFFC,
  0xFFFFFFFE,
  0xFFFFFFFF
};

/*! \fn     csignal_error_code csignal_get_spreading_code_bit  (
              spreading_code*  in_spreading_code,
              UCHAR*           out_bit
            )
    \brief  Executes one interation of the LFSR in in_spreading_code to generate
            one code bit. A single state change is done by bit-wise anding the
            generator with the state, and summing the result modulo-2 to get the
            next input bit. The output bit is simply the LSB of the state.
 
    \param  in_spreading_code The state and generator of the LFSR
    \param  out_bit The code bit generated by the LFSR
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc and csignal_get_spreading_code_bit for other
            possible errors):

            CPC_ERROR_CODE_NULL_POINTER If in_spreading_code or out_bit is NULL
 */
csignal_error_code
csignal_get_spreading_code_bit  (
                                 spreading_code*  in_spreading_code,
                                 UCHAR*           out_bit
                                 );

/*! \fn     UCHAR csignal_sum_ones  (
              UINT32 in_input
            )
    \brief  Sumes the ones in the in_input vector modulo-2
 
    \param  in_input  The vector whose ones are to be summed modulo-2
    \return The count of the number of '1' bits in in_input modulo-2
 */
UCHAR
csignal_sum_ones  (
                   UINT32 in_input
                   );

csignal_error_code
csignal_initialize_spreading_code (
                                   UCHAR            in_degree,
                                   UINT32           in_generator_polynomial,
                                   UINT32           in_initial_state,
                                   spreading_code** out_spreading_code
                                   )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_spreading_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Spreading code is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( in_degree < 2 || in_degree > 32 )
  {
    CPC_ERROR( "Degree (%d) must be between 2 and 32.", in_degree );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else if (
           ! (
              ( 0x00000001 << ( sizeof( UINT32 ) * 8 - in_degree ) )
              & in_generator_polynomial
              )
           )
  {
    CPC_ERROR (
               "Generator polynomial (0x%x) must have a '1'"
               " bit in the g_0 position (0x%x)",
               in_generator_polynomial,
               0x00000001 << ( sizeof( UINT32 ) * 8 - in_degree )
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else if( in_generator_polynomial & ~ polynomial_masks[ in_degree - 2 ] )
  {
    CPC_ERROR (
               "Generator polynomial (0x%x) has bits set"
               " in positions other than g_(r-1) (0x%x)... g_0 (0x%x).",
               in_generator_polynomial,
               0x80000000,
               0x00000001 << ( sizeof( UINT32 ) * 8 - in_degree )
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_spreading_code,
                       sizeof( spreading_code )
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      ( *out_spreading_code )->degree               = in_degree;
      ( *out_spreading_code )->generator_polynomial = in_generator_polynomial;
      ( *out_spreading_code )->state                = in_initial_state;
      
      CPC_LOG (
               CPC_LOG_LEVEL_TRACE,
               "Created a new LFSR with degree 0x%x, generator 0x%x,"
               " and initial state 0x%x.",
               ( *out_spreading_code )->degree,
               ( *out_spreading_code )->generator_polynomial,
               ( *out_spreading_code )->state
               );
    }
    else
    {
      CPC_ERROR( "Could not malloc spreading code: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_get_spreading_code  (
                             spreading_code*  in_spreading_code,
                             USIZE            in_number_of_code_bits,
                             USIZE*           out_size,
                             UCHAR**          out_code
                             )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_size || NULL == out_code )
  {
    CPC_ERROR( "Size (0x%x) or code (0x%x) are null.", out_size, out_code );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( NULL == in_spreading_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Spreading code is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 == in_number_of_code_bits )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Number of bits is zero." );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    USIZE byte_index = 0;
    USIZE bit_index  = 0;
    
    *out_size = 0;
    *out_code = NULL;
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Number of code bits is 0x%x.",
             in_number_of_code_bits
             );
    
    *out_size = in_number_of_code_bits / ( sizeof( UCHAR ) * 8 );
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Out size is: 0x%x.", *out_size );
    
    if( 0 != in_number_of_code_bits % ( sizeof( UCHAR ) * 8 ) )
    {
      *out_size = *out_size + 1;
    }
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Out size is: 0x%x.", *out_size );
    
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_code,
                       sizeof( UCHAR ) * *out_size
                      );

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      for( UINT32 i = 0; i < in_number_of_code_bits; i++ )
      {
        UCHAR bit = 0;
        
        return_value =
          csignal_get_spreading_code_bit( in_spreading_code, &bit );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          CPC_LOG (
                   CPC_LOG_LEVEL_TRACE,
                   "Inserting bit 0x%x at bit index 0x%x"
                   " of byte 0x%x (max 0x%x).",
                   bit,
                   bit_index,
                   byte_index,
                   *out_size
                   );
          
          CPC_LOG (
                   CPC_LOG_LEVEL_TRACE,
                   "Current byte value before set is 0x%x.",
                   ( *out_code )[ byte_index ]
                   );
          
          ( *out_code )[ byte_index ] |= ( bit << ( 7 - bit_index ) );
          
          CPC_LOG (
                   CPC_LOG_LEVEL_TRACE,
                   "Current byte value before set is 0x%x.",
                   ( *out_code )[ byte_index ]
                   );
          
          bit_index = ( bit_index + 1 ) % 8;
          
          if( 0 == bit_index )
          {
            byte_index++;
          }
        }
        else
        {
          break;
        }
      }
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_get_spreading_code_bit  (
                                 spreading_code*  in_spreading_code,
                                 UCHAR*           out_bit
                                 )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_spreading_code || NULL == out_bit )
  {
    CPC_ERROR (
               "Spreading code (0x%x) or bit (0x%x) are null.",
               in_spreading_code,
               out_bit
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    UCHAR new_input =
      csignal_sum_ones  (
                         in_spreading_code->state
                         & in_spreading_code->generator_polynomial
                         & polynomial_masks[ in_spreading_code->degree - 2 ]
                         );
    
    *out_bit =
      (
       in_spreading_code->state >>
        (
         ( sizeof( UINT32 ) * 8 )
         - in_spreading_code->degree
         )
       )
      & 0x00000001;
    
    in_spreading_code->state =
      ( in_spreading_code->state >> 1 )
      & polynomial_masks[ in_spreading_code->degree - 2 ];
    
    if( 0 != new_input )
    {
      in_spreading_code->state  |= 0x80000000;
    }
  }
  
  return( return_value );
}

UCHAR
csignal_sum_ones  (
                   UINT32 in_input
                   )
{
  UCHAR sum = 0;
  
  if( 0 != in_input )
  {
    for( UINT32 i = 0; i < sizeof( UINT32 ) * 8; i++ )
    {
      UINT32 index = ( 0x00000001 << i );
      
      if( index & in_input )
      {
        sum++;
      }
    }
  }
  
  return( ( sum % 2 ) );
}

csignal_error_code
csignal_set_spreading_signal  (
                               UCHAR    in_bit,
                               USIZE    in_signal_size,
                               FLOAT64* out_signal
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_signal )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Signal is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    for( USIZE i = 0; i < in_signal_size; i++ )
    {
      if( in_bit )
      {
        out_signal[ i ] = SPREADING_WAVEFORM_POSITIVE;
      }
      else
      {
        out_signal[ i ] = SPREADING_WAVEFORM_NEGATIVE;
      }
    }
  }
  
  return( return_value );
}
