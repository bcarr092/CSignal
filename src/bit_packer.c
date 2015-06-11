/*! \file   bit_packer.c
    \brief  Implementation of storate class for bits.

    \author Brent Carrara
 */

#include "bit_packer.h"

/*! \fn     csignal_error_code bit_packer_grow (
              bit_packer* io_bit_packer
            )
    \brief  Grows the internal buffer stored in io_bit_packer by a factor of
            BIT_PACKER_GROWTH_RATE.
 
    \param  io_bit_packer The bit packer that contains the data buffer that will
                          be grown.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_realloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If io_bit_packer is null
 */
csignal_error_code
bit_packer_grow (
                 bit_packer* io_bit_packer
                 );

csignal_error_code
bit_packer_initialize (
                       bit_packer** out_bit_packer
                       )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_bit_packer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit packer is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =
      cpc_safe_malloc( ( void** ) out_bit_packer, sizeof( bit_packer ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      ( *out_bit_packer )->bit_offset   = 0;
      ( *out_bit_packer )->byte_offset  = 0;
      ( *out_bit_packer )->data_length  = BIT_PACKER_INITIAL_SIZE;
      
      return_value =
        cpc_safe_malloc (
                         ( void** ) &( ( *out_bit_packer )->data ),
                         sizeof( UCHAR ) * ( *out_bit_packer )->data_length
                         );
    }
  }
  
  return( return_value );
}

csignal_error_code
bit_packer_destroy  (
                     bit_packer* in_bit_packer
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_bit_packer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit packer is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    if( NULL != in_bit_packer->data )
    {
      cpc_safe_free( ( void** ) &( in_bit_packer->data ) );
    }
    
    cpc_safe_free( ( void** ) &in_bit_packer );
  }
  
  return( return_value );
}

csignal_error_code
bit_packer_add_bits (
                     UCHAR        in_data,
                     USIZE        in_num_bits,
                     bit_packer*  io_bit_packer
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_bit_packer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit packer is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( in_num_bits > ( sizeof( UCHAR ) * 8 ) )
  {
    CPC_ERROR (
               "Invalid number of bits (0x%x), must be less than 8.",
               in_num_bits
               );
    
    return_value = CSIGNAL_ERROR_CODE_LENGTH_MISMATCH;
  }
  else
  {
    UINT16 mask   = 0;
    UINT16 bytes  =
      0xFF00 & ( io_bit_packer->data[ io_bit_packer->byte_offset ] << 8 );
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Bytes are 0x%04x.", bytes );
    
    for( UINT32 i = 0; i < in_num_bits; i++ )
    {
      mask |= ( 1 << i );
    }
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE, 
             "Mask is 0x%04x, data is 0x%02x.",
             mask,
             in_data
             );
    
    bytes |=
      ( in_data & mask )
      << ( sizeof( UINT16 ) * 8 - io_bit_packer->bit_offset - in_num_bits );
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Bytes are now 0x%04x.", bytes );
    
    UCHAR hi_part = ( UCHAR ) ( bytes >> ( sizeof( UCHAR ) * 8 ) );
    UCHAR lo_part = ( UCHAR ) ( bytes & 0x00FF );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Hi part: 0x%02x. Lo part: 0x%02x.",
             hi_part,
             lo_part
             );
    
    io_bit_packer->data[ io_bit_packer->byte_offset ] = hi_part;
    
    if( ( sizeof( UCHAR ) * 8 <= ( io_bit_packer->bit_offset + in_num_bits ) ) )
    {
      if( ( io_bit_packer->byte_offset + 1 ) >= io_bit_packer->data_length )
      {
        return_value = bit_packer_grow( io_bit_packer );
      }
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        io_bit_packer->byte_offset++;
        
        io_bit_packer->data[ io_bit_packer->byte_offset ] = lo_part;
      }
    }
    
    io_bit_packer->bit_offset =
      ( io_bit_packer->bit_offset + in_num_bits ) % ( sizeof( UCHAR ) * 8 );
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Bit offset is 0x%x, byte offset is 0x%x.",
             io_bit_packer->bit_offset,
             io_bit_packer->byte_offset
             );
    
    CPC_LOG_BUFFER  (
                     CPC_LOG_LEVEL_TRACE, "Bit packer data:",
                     io_bit_packer->data,
                     io_bit_packer->byte_offset,
                     8
                     );
  }
  
  return( return_value );
}

csignal_error_code
bit_packer_grow (
                 bit_packer* io_bit_packer
                 )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_bit_packer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit packer is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =
      cpc_safe_realloc  (
           ( void** ) &( io_bit_packer->data ),
           sizeof( UCHAR ) * io_bit_packer->data_length,
           sizeof( UCHAR ) * io_bit_packer->data_length * BIT_PACKER_GROWTH_RATE
                         );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      io_bit_packer->data_length *= BIT_PACKER_GROWTH_RATE;
    }
  }
  
  return( return_value );
}

csignal_error_code
bit_packer_add_bytes (
                      UCHAR*        in_data,
                      USIZE         in_data_length,
                      bit_packer*   io_bit_packer
                      )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_data )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Data is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    for( UINT32 i = 0; i < in_data_length; i++ )
    {
      return_value =
        bit_packer_add_bits( in_data[ i ], sizeof( UCHAR ) * 8, io_bit_packer );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR (
                   "Could not add bits (0x%x,0x%x): 0x%x.",
                   in_data[ i ],
                   i,
                   return_value
                   );
        
        break;
      }
    }
  }
  
  return( return_value );
}

csignal_error_code
bit_packer_get_bytes (
                     bit_packer*  in_bit_packer,
                     UCHAR**      out_data,
                     USIZE*       out_data_length
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_bit_packer || NULL == out_data || NULL == out_data_length )
  {
    CPC_ERROR (
             "Bit packer (0x%x), data (0x%x), or data length (0x%x) are null.",
             in_bit_packer,
             out_data,
             out_data_length
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    *out_data_length = in_bit_packer->byte_offset;
    
    if( in_bit_packer->bit_offset != 0 )
    {
      *out_data_length = *out_data_length + 1;
    }
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is 0x%x.", *out_data_length );
    
    if( *out_data_length > in_bit_packer->data_length )
    {
      CPC_ERROR (
           "Calculated data length (0x%x) is larger than buffer length (0x%x).",
           *out_data_length,
           in_bit_packer->data_length
                 );
      
      return_value = CSIGNAL_ERROR_CODE_LENGTH_MISMATCH;
    }
    else
    {
      return_value = cpc_safe_malloc( ( void** ) out_data, *out_data_length );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        CPC_MEMCPY( *out_data, in_bit_packer->data, *out_data_length );
      }
    }
  }
  
  return( return_value );
}
