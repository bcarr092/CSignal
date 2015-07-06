/*! \file   bit_stream.c
    \brief  Implementation of functions to read bits from a byte buffer.

    \author Brent Carrara
 */
#include "bit_stream.h"

csignal_error_code
bit_stream_initialize  (
                        CPC_BOOL      in_circular,
                        UCHAR*        in_data,
                        USIZE         in_data_length,
                        bit_stream**  out_bit_stream
                        )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  if( NULL == in_data || NULL == out_bit_stream )
  {
  
    CPC_ERROR (
               "Data (0x%x) or bit stream (0x%x) are null.",
               in_data,
               out_bit_stream
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =
      cpc_safe_malloc( ( void** ) out_bit_stream, sizeof( bit_stream ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      ( *out_bit_stream )->bit_offset   = 0;
      ( *out_bit_stream )->byte_offset  = 0;
      ( *out_bit_stream )->dirty_bit    = CPC_FALSE;
      ( *out_bit_stream )->circular     = in_circular;
      ( *out_bit_stream )->packer       = NULL;

      return_value = bit_packer_initialize( &( ( *out_bit_stream )->packer ) );

      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        bit_packer_add_bytes( in_data, in_data_length, ( *out_bit_stream )->packer );

        if( CPC_ERROR_CODE_NO_ERROR != return_value )
        {
          CPC_ERROR( "Could not add bytes to packer: 0x%x.", return_value );
        }
      }
      else
      {
        CPC_ERROR( "Could not create bit_packer: 0x%x.", return_value );
      }
    }
  }
  
  return( return_value );
}

csignal_error_code
bit_stream_initialize_from_bit_packer (
                                       CPC_BOOL     in_circular,
                                       bit_packer*  in_bit_packer,
                                       bit_stream** out_bit_stream
                                       )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_bit_packer || NULL == out_bit_stream )
  {
    CPC_ERROR (
               "Packer (0x%x) or bit stream (0x%x) are null.",
               in_bit_packer,
               out_bit_stream
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =
      cpc_safe_malloc( ( void** ) out_bit_stream, sizeof( bit_stream ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      ( *out_bit_stream )->bit_offset   = 0;
      ( *out_bit_stream )->byte_offset  = 0;
      ( *out_bit_stream )->dirty_bit    = CPC_TRUE;
      ( *out_bit_stream )->circular     = in_circular;
      ( *out_bit_stream )->packer       = in_bit_packer;
    }
  }
  
  return( return_value );
}

csignal_error_code
bit_stream_destroy  (
                     bit_stream* io_bit_stream
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_bit_stream )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit stream is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    if( NULL != io_bit_stream->packer && ! io_bit_stream->dirty_bit )
    {
      bit_packer_destroy( io_bit_stream->packer );
    }
    
    cpc_safe_free( ( void** ) &io_bit_stream );
  }
  
  return( return_value );
}

csignal_error_code
bit_stream_get_bits (
                     bit_stream*  io_bit_stream,
                     USIZE*       io_num_bits,
                     UCHAR**      out_data,
                     USIZE*       out_data_length
                     )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == io_bit_stream
       || NULL == io_num_bits
       || NULL == out_data
       || NULL == out_data_length
       )
  {
    CPC_ERROR (
             "Bit stream (0x%x), num bits (0x%x), data (0x%x)"
              ", or data length (0x%x) are null.",
             io_bit_stream,
             io_num_bits,
             out_data,
             out_data_length
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    bit_packer* packer = NULL;
    
    return_value = bit_packer_initialize( &packer );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      USIZE num_bits_packed = *io_num_bits;
      
      while (
             0 != *io_num_bits
             && bit_stream_get_number_of_remaining_bits( io_bit_stream )
             )
      {
        USIZE bits_left_in_byte =
          ( sizeof( UCHAR ) * 8 ) - io_bit_stream->bit_offset;
        USIZE num_bits_to_pack  =
          CPC_MIN( USIZE, *io_num_bits, bits_left_in_byte );
        UCHAR byte              =
          io_bit_stream->packer->data[ io_bit_stream->byte_offset ]
          >>
          (
            ( sizeof( UCHAR ) * 8 )
            - ( io_bit_stream->bit_offset + num_bits_to_pack )
          );
        
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Bits left: 0x%x.", bits_left_in_byte );
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Num to pack: 0x%x.", num_bits_to_pack );
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Byte: 0x%x.", byte );
        
        *io_num_bits              -= num_bits_to_pack;
        io_bit_stream->bit_offset += num_bits_to_pack;
        
        if( 0 == ( io_bit_stream->bit_offset % ( sizeof( UCHAR ) * 8  ) ) )
        {
          io_bit_stream->bit_offset = 0;
          
          io_bit_stream->byte_offset++;
        }
        
        if  (
             io_bit_stream->bit_offset == io_bit_stream->packer->bit_offset
             && io_bit_stream->byte_offset == io_bit_stream->packer->byte_offset
             && io_bit_stream->circular
             )
        {
          io_bit_stream->bit_offset   = 0;
          io_bit_stream->byte_offset  = 0;
        }
        
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Num bits left: 0x%x.", *io_num_bits );
        CPC_LOG (
                 CPC_LOG_LEVEL_TRACE,
                 "Bit offset: 0x%x.", 
                 io_bit_stream->bit_offset
                 );
        CPC_LOG (
                 CPC_LOG_LEVEL_TRACE,
                 "Byte offset: 0x%x.",
                 io_bit_stream->byte_offset
                 );
        
        return_value = bit_packer_add_bits( byte, num_bits_to_pack, packer );
        
        if( CPC_ERROR_CODE_NO_ERROR != return_value )
        {
          CPC_ERROR( "Could not add bits: 0x%x.", return_value );
          
          break;
        }
      }
      
      *io_num_bits = num_bits_packed - *io_num_bits;
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        return_value = bit_packer_get_bytes( packer, out_data, out_data_length );
        
        if( CPC_ERROR_CODE_NO_ERROR != return_value )
        {
          CPC_ERROR( "Could not get packer bytes: 0x%x.", return_value );
        }
      }
      
      bit_packer_destroy( packer );
    }
  }
  
  return( return_value );
}

USIZE
bit_stream_get_number_of_remaining_bits (
                                         bit_stream* in_bit_stream
                                         )
{
  USIZE number_of_bits = 0;
  
  if( NULL != in_bit_stream )
  {
    if( CPC_TRUE == in_bit_stream->circular )
    {
      number_of_bits = MAX_USIZE;
    }
    else
    {
      USIZE total_number_of_bits =
        ( in_bit_stream->packer->byte_offset * sizeof( UCHAR ) * 8 )
        + in_bit_stream->packer->bit_offset;
      
      USIZE  current_position =
        ( in_bit_stream->byte_offset * sizeof( UCHAR ) * 8 )
        + in_bit_stream->bit_offset;
      
      number_of_bits = total_number_of_bits - current_position;
    }
  }
  else
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Bit stream is null." );
  }
  
  return( number_of_bits );
}

csignal_error_code
bit_stream_peak (
                 bit_stream*  in_bit_stream,
                 UCHAR*       out_read_bit_offset,
                 UCHAR*       out_write_bit_offset,
                 USIZE*       out_buffer_length,
                 UCHAR**      out_buffer
                 )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_bit_stream
       || NULL == in_bit_stream->packer
       || NULL == out_read_bit_offset
       || NULL == out_write_bit_offset
       || NULL == out_buffer_length
       || NULL == out_buffer
       )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Bit stream (0x%x), stream's packer (0x%x), read offset (0x%x),"
               " write offset (0x%x), length (0x%x), or buffer (0x%x)"
               " are null.",
               in_bit_stream,
               in_bit_stream->packer,
               out_read_bit_offset,
               out_write_bit_offset,
               out_buffer_length,
               out_buffer
               );
  }
  else
  {
    *out_read_bit_offset  = in_bit_stream->bit_offset;
    *out_write_bit_offset = in_bit_stream->packer->bit_offset;
    *out_buffer           =
      in_bit_stream->packer->data + in_bit_stream->byte_offset;
    *out_buffer_length    =
      in_bit_stream->packer->byte_offset - in_bit_stream->byte_offset;
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is: %d.", *out_buffer_length );
    
    if( 0 != *out_buffer_length && *out_write_bit_offset != 0 )
    {
      ( *out_buffer_length )++;
    }
    
    CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is: %d.", *out_buffer_length );
  }
  
  return( return_value );
}
