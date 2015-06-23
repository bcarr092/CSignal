/*! \file   conv.c
    \brief  Impelementation of the convolution algorithm.
 */
#include "conv.h"

csignal_error_code
convolve  (
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
  else
  {
    cpc_safe_malloc (
                     ( void** ) out_signal,
                     sizeof( FLOAT64 )
                     * ( in_signal_one_length + in_signal_two_length )
                     );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      *out_signal_length = in_signal_one_length + in_signal_two_length;
      
      for( SSIZE i = 0; i < *out_signal_length; i++ )
      {
        FLOAT64 value = 0;
        
        SSIZE min =
          ( i >= in_signal_two_length - 1 )
          ? i - ( in_signal_two_length - 1 ) : 0;
        
        SSIZE max =
          ( i < in_signal_one_length - 1 ) ? i : in_signal_one_length - 1;
        
        for( SSIZE j = min; j <= max; j++ )
        {
          value += in_signal_one[ j ] * in_signal_two[ i - j ];
        }
        
        ( *out_signal )[ i ] = value;
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc convolved signal: 0x%x.", return_value );
      
      *out_signal_length = 0;
      *out_signal        = NULL;
    }
  }
  
  return( return_value );
}
