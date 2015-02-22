/*! \file   csignal.h
    \brief  This is the main include file for the csignal library. This is the
            only header file that needs to be included by external libraries/
            binaries.
 */
#ifndef __CSIGNAL_H__
#define __CSIGNAL_H__

#include <cpcommon.h>

#include "csignal_error_codes.h"

/*! \var    csignal_symbol_tracker
    \brief  Struct definition for symbol tracker. A symbol tracker is used to
            iterate through the symbols in a buffer. Symbols of variable bit-
            length are read from index 0 to data_length.
 */
typedef struct csignal_symbol_tracker_t
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
  UINT32 byte_offset;
  
  /*! \var    bit_offset
      \brief  A pointer to the bit in current_byte that will be read from in
              the next invocation of csignal_read_symbol
   */
  UCHAR bit_offset;
  
} csignal_symbol_tracker;

/*! \fn     cpc_error_code csignal_initialize_symbol_tracker  (
              UCHAR*                    in_data,
              USIZE                     in_data_length,
              csignal_symbol_tracker ** out_symbol_tracker
            )
    \brief  Initializes the symbol_tracker struct with a data buffer and sets
            both bit and byte offsets to 0.
 
    \note   in_data must not be freed until the destroy_symbol_tracker function
            is called. For efficiency purposes, i.e. not mallocing a new buffer
            and copying the contents of in_data into it, in_data is simply
            pointed to and not copied.
 
    \param  in_data The data buffer that contains the bits to read as symbols.
    \param  in_data_length  The length of in_data
    \param  out_symbol_tracker  If this function successfully completes this
                                parameter will point to a newly created
                                csignal_symbol_tracker. It will be NULL
                                otherwise.
    \return Returns NO_ERROR upon successful execution and an error code if a
            problem occurs.
 */
csignal_error_code
csignal_initialize_symbol_tracker  (
                                    UCHAR*                    in_data,
                                    USIZE                     in_data_length,
                                    csignal_symbol_tracker ** out_symbol_tracker
                                    );

/*! \fn     cpc_error_code csignal_destroy_symbol_tracker  (
              csignal_symbol_tracker* io_symbol_tracker
            )
    \brief  Frees io_symbol_tracker
 
    \note   The buffer pointed to by the data member is not freed. It is the
            responsibility of the creator of data to destroy the buffer.
 
    \return Returns NO_ERROR upon succesful execution or an error code if a
            problem occurs.
 */
csignal_error_code
csignal_destroy_symbol_tracker  (
                                 csignal_symbol_tracker* io_symbol_tracker
                                 );

csignal_error_code
csignal_get_symbol  (
                     csignal_symbol_tracker*  in_symbol_tracker,
                     UINT32                   in_number_of_bits,
                     UINT32*                  out_symbol
                     );


#endif  /*  __CSIGNAL_H__ */
