/*! \file   bit_stream.h
    \brief  Functions to handle reading bits from a byte buffer.

    \author Brent Carrara
 */
#ifndef __BIT_STREAM_H__
#define __BIT_STREAM_H__

#include <cpcommon.h>

#include "bit_packer.h"

#include "csignal_error_codes.h"

/*! \var    bit_stream
    \brief  Struct definition for symbol tracker. A symbol tracker is used to
            iterate through the symbols in a buffer. Symbols of variable bit-
            length are read from index 0 to data_length.
 */
typedef struct bit_stream_t
{
  /*! \var    data
      \brief  The data buffer that contains the data bits
   */
  UCHAR* data;
  
  /*! \var    data_length
      \brief  The size of the data buffer
   */
  USIZE data_length;
  
  /*! \var    byte_offset
      \brief  A pointer to the buffer that will be read from in the next
              invocation of csignal_read_symbol
   */
  USIZE byte_offset;
  
  /*! \var    bit_offset
      \brief  A pointer to the bit in current_byte that will be read from in
              the next invocation of csignal_read_symbol
   */
  UCHAR bit_offset;
  
} bit_stream;

/*! \fn     cpc_error_code bit_stream_initialize  (
              UCHAR*        in_data,
              USIZE         in_data_length,
              bit_stream**  out_bit_stream
            )
    \brief  Initializes the bit_stream struct with a data buffer and sets
            both bit and byte offsets to 0.
 
    \param  in_data The data buffer that contains the bits to read.
    \param  in_data_length  The length of in_data
    \param  out_symbol_tracker  If this function successfully completes this
                                parameter will point to a newly created
                                bit_stream. It will be NULL otherwise.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If in_data or out_bit_stream is null
 */
csignal_error_code
bit_stream_initialize  (
                        UCHAR*        in_data,
                        USIZE         in_data_length,
                        bit_stream**  out_bit_stream
                        );

/*! \fn     csignal_error_code bit_stream_initialize_from_bit_packer (
              bit_packer* in_bit_packer,
              bit_stream** out_bit_stream
            )
    \brief  Creates a new bit stream whose data buffer is a copy of the buffer
            stored by in_bit_packer.
 
    \note The data buffer from the bit_packer will be copied to the newly
          created bit stream object.
 
    \param  in_bit_packer The packer whose data is to be copied.
    \param  out_bit_stream  A newly created bit stream or Null if an error
                            occurs.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc and bit_packer_get_bytes for other possible 
            errors):
 
            CPC_ERROR_CODE_NULL_POINTER If in_data or out_bit_stream is null
 */
csignal_error_code
bit_stream_initialize_from_bit_packer (
                                       bit_packer*  in_bit_packer,
                                       bit_stream** out_bit_stream
                                       );

/*! \fn     cpc_error_code bit_stream_destroy  (
              bit_stream* io_bit_stream
            )
    \brief  Frees io_bit_stream along with the data buffer pointed to.
 
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_free for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER if io_bit_stream is null
 */
csignal_error_code
bit_stream_destroy  (
                     bit_stream* io_bit_stream
                     );

/*! \fn     csignal_error_code bit_stream_get_bits (
              bit_stream*  io_bit_stream,
              USIZE*       io_num_bits,
              UCHAR**      out_data,
              USIZE*       out_data_length
            )
    \brief  Copies up to io_num_bits from io_bit_stream, copies them to out_data
            and returns the size of out_data in out_data_length and the number
            of copied bits in io_num_bits.
 
    \param  io_bit_stream The bit stream to get the bits from.
    \param  io_num_bits Originally contains the number of bits to copy from
                        io_bit_stream. After the call it contains the number
                        of bits actually copied to out_data.
    \param  out_data  A buffer of bits copied from io_bit_stream. Note the size
                      of this buffer is stored in out_data_length, but is also
                      equal to ceil( io_num_bits / 8 ).
    \param  out_data_length The size of out_data in bytes.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc and bit_packer.h for other possible errors):
 
            CPC_ERROR_CODE_NULL_POINTER If any of the inputs are null.
 */
csignal_error_code
bit_stream_get_bits (
                     bit_stream*  io_bit_stream,
                     USIZE*       io_num_bits,
                     UCHAR**      out_data,
                     USIZE*       out_data_length
                     );

#endif  /*  __BIT_STREAM_H__  */
