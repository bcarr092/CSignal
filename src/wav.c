/*! \file   cwav.c
    \brief  This file contains the implementation of the read/write methods for
            WAV files.
*/

#include "wav.h"

/*! \fn     csignal_error_code
              csignal_write_RIFF_header (
              FILE*  in_file_pointer,
              UINT16 in_number_of_channels,
              UINT32 in_number_of_samples
            )
    \brief  This function will write a new RIFF header to the file pointed to
            by in_file_pointer.
 
    \param  in_file_pointer A valid file pointer opened for write.
    \param  in_number_of_channels The number of channels in the LPCM WAV file
                                  being created.
    \param  in_number_of_samples  The number of samples encoded in each channel.
    \return Returns NO_ERROR upon succesful execution or one of these errors:

            CPC_ERROR_CODE_NULL_POINTER If in_file_pointer is NULL.
            CSIGNAL_ERROR_CODE_WRITE_ERROR  If data could not be written to
                                            in_file_pointer.
 */
csignal_error_code
csignal_write_RIFF_header (
                           FILE*  in_file_pointer,
                           UINT16 in_number_of_channels,
                           UINT32 in_number_of_samples
                           );

/*! \fn     csignal_error_code csignal_write_WAVE_header (
              FILE*   in_file_pointer,
              UINT16  in_number_of_channels,
              UINT32  in_sample_rate
            )
    \brief  This function will write a new WAVE format header to the file
            pointed to by in_file_pointer.
 
    \param  in_file_pointer A valid file pointer opened for write.
    \param  in_number_of_channels The number of channels in the LPCM WAV file
                                  being created.
    \param  in_sample_rate  The sample rate the data in the file was encoded
                            at.
    \return Returns NO_ERROR upon succesful execution or one of these errors:

            CPC_ERROR_CODE_NULL_POINTER If in_file_pointer is NULL.
            CSIGNAL_ERROR_CODE_WRITE_ERROR  If data could not be written to
            in_file_pointer.
 */
csignal_error_code
csignal_write_WAVE_header (
                           FILE*   in_file_pointer,
                           UINT16  in_number_of_channels,
                           UINT32  in_sample_rate
                           );

/*! \fn     csignal_error_code csignal_write_data (
              FILE*    in_file_pointer,
              UINT16   in_number_of_channels,
              UINT32   in_number_of_samples,
              INT16**  in_samples
            )
    \brief  This function will write all the data in in_samples in interleaved
            form to the WAVE file pointed to in_file_pointer.
 
    \param  in_file_pointer A valid file pointer opened for write.
    \param  in_number_of_channels The number of channels in the LPCM WAV file
            being created.
    \param  in_number_of_samples  The number of samples encoded in each channel.
    \param  in_samples  The samples array it is an in_number_of_channels X
            in_samples data buffer.
    \return Returns NO_ERROR upon succesful execution or one of these errors:

            CPC_ERROR_CODE_NULL_POINTER If in_file_pointer is NULL.
            CSIGNAL_ERROR_CODE_WRITE_ERROR  If data could not be written to
                                            in_file_pointer.
 */
csignal_error_code
csignal_write_data (
                    FILE*    in_file_pointer,
                    UINT16   in_number_of_channels,
                    UINT32   in_number_of_samples,
                    INT16**  in_samples
                    );

/*! \fn     UINT16 csignal_convert_short_to_little_endian  (
              UINT16 in_number
            )
    \brief  Converts in_number to little endian. If in_number is already in
            little endian format nothing is done.
    
    \param  in_number The number to convert to little endian.
    \return in_number converted to little endian, or in_number if the number
            was already in that format.
 */
UINT16
csignal_convert_short_to_little_endian  (
                                         UINT16 in_number
                                         );


/*! \fn     UINT32 csignal_convert_int_to_little_endian  (
              UINT32 in_number
            )
    \brief  Converts in_number to little endian. If in_number is already in
            little endian format nothing is done.

    \param  in_number The number to convert to little endian.
    \return in_number converted to little endian, or in_number if the number
            was already in that format.
    */
UINT32
csignal_convert_int_to_little_endian  (
                                       UINT32 in_number
                                       );

csignal_error_code
csignal_write_LPCM_wav  (
                         CHAR*    in_file_name,
                         USIZE    in_number_of_channels,
                         UINT32   in_sample_rate,
                         USIZE    in_number_of_samples,
                         INT16**  in_samples
                         )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( cpc_check_if_file_exists( in_file_name ) )
  {
    CPC_ERROR( "File (%s) exists, cannot overwrite.", in_file_name );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else if (
           in_number_of_channels > MAX_UINT16
           || in_number_of_samples > MAX_UINT32
           )
  {
    CPC_ERROR (
               "Number of channels (0x%x) must be smaller than 0x%x and"
               " number of samples (0x%x) must be smaller than 0x$x.",
               in_number_of_channels,
               MAX_UINT16,
               in_number_of_samples,
               MAX_UINT32
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    FILE* file_pointer = fopen( in_file_name, "wb" );
    
    if( NULL != file_pointer )
    {
      return_value =
        csignal_write_RIFF_header (
                                file_pointer,
                                in_number_of_channels,
                                in_number_of_samples
                                );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        return_value =
          csignal_write_WAVE_header  (
                                   file_pointer,
                                   in_number_of_channels,
                                   in_sample_rate
                                   );
        
        if( CPC_ERROR_CODE_NO_ERROR == return_value )
        {
          return_value =
            csignal_write_data (
                             file_pointer,
                             in_number_of_channels,
                             in_number_of_samples,
                             in_samples
                             );
        }
      }
      
      fclose( file_pointer );
    }
    else
    {
      CPC_ERROR( "File (%s) could not be created.", in_file_name );
      
      return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_write_data (
                    FILE*    in_file_pointer,
                    UINT16   in_number_of_channels,
                    UINT32   in_number_of_samples,
                    INT16**  in_samples
                    )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_file_pointer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "File pointer is NULL." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    UINT32 chunk_size =
      csignal_convert_int_to_little_endian (
        CSIGNAL_SHORT_SAMPLE_SIZE * in_number_of_channels * in_number_of_samples
                                            );
    
    if  (
         4 != fwrite( CSIGNAL_WAVE_HEADER_CHUNK_DATA_ID, 1, 4, in_file_pointer )
         || 1 != fwrite( &chunk_size, sizeof( UINT32 ), 1, in_file_pointer )
        )
    {
      CPC_LOG_STRING  (
                       CPC_LOG_LEVEL_ERROR,
                       "Error writing WAVE header to file."
                       );
      
      return_value = CSIGNAL_ERROR_CODE_WRITE_ERROR;
    }
    else
    {
      for( UINT32 i = 0; i < in_number_of_samples; i++ )
      {
        for( UINT32 j = 0; j < in_number_of_channels; j++ )
        {
          INT16 sample =
            csignal_convert_short_to_little_endian( in_samples[ j ][ i ] );
          
          if( 1 != fwrite( &sample, sizeof( INT16 ), 1, in_file_pointer ) )
          {
            CPC_ERROR (
                       "Could not write sample 0x%x, index=0x%x, channel=0x%x.",
                       sample,
                       i,
                       j
                       );
            
            return_value = CSIGNAL_ERROR_CODE_WRITE_ERROR;
          }
        }
      }
      
      if( chunk_size % 2 != 0 )
      {
        INT16 padding = 0;
        
        if( 1 != fwrite( &padding, sizeof( INT16 ), 1, in_file_pointer ) )
        {
          CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Could not write padding." );
          
          return_value = CSIGNAL_ERROR_CODE_WRITE_ERROR;
        }
      }
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_write_WAVE_header (
                           FILE* in_file_pointer,
                           UINT16  in_number_of_channels,
                           UINT32  in_sample_rate
                           )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_file_pointer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "File pointer is NULL." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    UINT32 chunk_size       =
      csignal_convert_int_to_little_endian  (
                                          CSIGNAL_WAVE_HEADER_CHUNK_FORMAT_SIZE
                                             );
    UINT16 format_tag       =
      csignal_convert_short_to_little_endian( CSIGNAL_WAVE_LPCM_FORMAT_CODE );
    UINT16 number_of_channels =
      csignal_convert_short_to_little_endian( in_number_of_channels );
    UINT32 sample_rate =
      csignal_convert_int_to_little_endian( in_sample_rate );
    UINT32 bytes_per_second =
      csignal_convert_int_to_little_endian  (
                                             in_sample_rate
                                             * CSIGNAL_SHORT_SAMPLE_SIZE
                                             * in_number_of_channels
                                             );
    UINT16 bytes_per_block  =
      csignal_convert_short_to_little_endian  (
                                               CSIGNAL_SHORT_SAMPLE_SIZE
                                               * in_number_of_channels
                                               );
    UINT16 bits_per_sample  =
      csignal_convert_short_to_little_endian( CSIGNAL_SHORT_SAMPLE_SIZE * 8 );
    
    if  (
         4 != fwrite( CSIGNAL_WAVE_HEADER_ID, 1, 4, in_file_pointer )
         || 4 != fwrite (
                         CSIGNAL_WAVE_HEADER_CHUNK_FORMAT_ID,
                         1,
                         4,
                         in_file_pointer
                         )
         || 1 != fwrite( &chunk_size, sizeof( UINT32 ), 1, in_file_pointer )
         || 1 != fwrite( &format_tag, sizeof( UINT16 ), 1, in_file_pointer )
         || 1 != fwrite (
                         &number_of_channels,
                         sizeof( UINT16 ),
                         1,
                         in_file_pointer
                         )
         || 1 != fwrite( &sample_rate, sizeof( UINT32 ), 1, in_file_pointer )
         || 1 != fwrite (
                         &bytes_per_second,
                         sizeof( UINT32 ),
                         1,
                         in_file_pointer
                         )
         || 1 != fwrite (
                         &bytes_per_block,
                         sizeof( UINT16 ),
                         1,
                         in_file_pointer
                         )
         || 1 != fwrite (
                         &bits_per_sample,
                         sizeof( UINT16 ),
                         1,
                         in_file_pointer
                         )
        )
    {
      CPC_LOG_STRING  (
                       CPC_LOG_LEVEL_ERROR,
                       "Error writing WAVE header to file."
                       );
      
      return_value = CSIGNAL_ERROR_CODE_WRITE_ERROR;
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_write_RIFF_header (
                           FILE*  in_file_pointer,
                           UINT16 in_number_of_channels,
                           UINT32 in_number_of_samples
                           )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_file_pointer )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "File pointer is NULL" );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    UINT32 data_size  =
      in_number_of_samples * in_number_of_channels * CSIGNAL_SHORT_SAMPLE_SIZE;
    UINT32 size       =
      csignal_convert_int_to_little_endian  (
                                             CSIGNAL_RIFF_FILE_SIZE_BASE
                                             + data_size
                                             + ( data_size % 2 == 0 ? 0 : 1 )
                                             );
    
    if  (
         4 != fwrite( CSIGNAL_RIFF_HEADER_ID, 1, 4, in_file_pointer )
         || 1 != fwrite( &size, sizeof( UINT32 ), 1, in_file_pointer )
         )
    {
      CPC_LOG_STRING  (
                       CPC_LOG_LEVEL_ERROR,
                       "Error writing RIFF header to file."
                       );
      
      return_value = CSIGNAL_ERROR_CODE_WRITE_ERROR;
    }
  }
  
  return( return_value );
}

UINT16
csignal_convert_short_to_little_endian  (
                                         UINT16 in_number
                                         )
{
  UINT16 little_endian = in_number;
  
  if( CPC_HTONL( in_number ) == in_number )
  {
    little_endian = ( in_number >> 8 ) || ( in_number << 8 );
  }
  
  return( little_endian );
}

UINT32
csignal_convert_int_to_little_endian  (
                                       UINT32 in_number
                                       )
{
  UINT32 little_endian = in_number;
  
  if( CPC_HTONL( in_number ) == in_number )
  {
    little_endian =
      in_number >> 24
      || ( in_number & 0x00FF0000 ) >> 8
      || ( in_number & 0x0000FF00 ) << 8
      || in_number << 24;
  }
  
  return( little_endian );
}
