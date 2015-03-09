/*! \file   fft.c
    \brief  The implementation of the FFT algorithm.
 */
#include "fft.h"

/*! \var    CALCULATE_FFT
    \brief  Flag used by the FFT algorithm from Numerical Recipes to indicate 
            that the FFT algorithm should calculated the DFT.
 */
#define CALCULATE_FFT   1

/*! \var    CALCULATE_IFFT
    \brief  Flag used by the FFT algorithm from Numerical Recipes to indicate
            that the FFT algorithm should calculated the IDFT.
 */
#define CALCULATE_IFFT  -1

/*! \def    SWAP
    \brief  Swaps the values in in_a and in_b. Used by the Numerical Recipes
            FFT algorithm.
 */
#define SWAP( in_a, in_b ) tempr=(in_a); (in_a)=(in_b); (in_b)=tempr;

/*! \fn     UINT32 csignal_calculate_closest_power_of_two  (
              UINT32 in_number
            )
    \brief  Finds the next power of 2 larger than or equal to in_number.
 
    \param  in_number The next power of 2 will be larger than or equal to this
                      parameter.
    \return A power of 2 larger than or equal to in_number.
 */
UINT32
csignal_calculate_closest_power_of_two  (
                                         UINT32 in_number
                                         );

/*! \fn     csignal_error_code csignal_convert_real_array_to_complex_array (
              UINT32   in_real_signal_length,
              INT16*   in_real_signal,
              UINT32   in_complex_signal_length,
              FLOAT32* in_complex_signal
            )
    \brief  Takes the elements in in_real_signal and copies their values to
            the even elements in in_complex_signal and sets the the odd elements
            to zero. The even elements are the real components of the complex
            numbers stored in in_complex_signal and the odd elements are the
            imaginary elements.
 
    \param  in_real_signal_length The number of elements in in_real_signal.
    \param  in_real_signal  The vector of real components to copy to the real
                            components of in_complex_signal.
    \param  in_complex_signal_length  The number of elements in complex_signal.
    \param  in_complex_signal The array returned with real components (even
                              indices) equal to the values in_real_signal and
                              complex components (odd indices) equal to zero.
    \return Returns NO_ERROR upon succesful exection or one of these errors
            (see cpc_safe_malloc for more error codes):
 
            CPC_ERROR_CODE_NULL_POINTER If in_real_signal or in_complex_signal
                                        are null.
            CPC_ERROR_CODE_INVALID_PARAMETER  If in_complex_signal_length is not
                                              2x larger than the next power of
                                              two larger than in_real_signal.
 */
csignal_error_code
csignal_convert_real_array_to_complex_array (
                                             UINT32   in_real_signal_length,
                                             INT16*   in_real_signal,
                                             UINT32   in_complex_signal_length,
                                             FLOAT32* in_complex_signal
                                             );

/*! \fn     void csignal_fft (
              FLOAT32* io_data,
              UINT32   in_data_length,
              CHAR     in_sign
            )
    \brief  The Numerical Recipes implementation of the FFT algorithm.
 
    \param  io_data The compex-valued signal whose FFT (or IFFT) will be
                    calculated.
    \param  in_data_length  The number of elements in io_data.
    \param  in_sign If +1 the FFT will be calculated, if -1 the IFFT will be
                    calculated.
 */
void
csignal_fft (
             FLOAT32* io_data,
             UINT32   in_data_length,
             CHAR     in_sign
             );

csignal_error_code
csignal_calculate_FFT (
                       UINT32     in_signal_length,
                       INT16*     in_signal,
                       UINT32*    out_fft_length,
                       FLOAT32**  out_fft
                       )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if  (
       NULL == in_signal
       || NULL == out_fft_length
       || NULL == out_fft
       )
  {
    CPC_ERROR (
               "Signal (0x%x), fft length (0x%x), or fft (0x%x) are null.",
               in_signal,
               out_fft_length,
               out_fft
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    *out_fft_length =
      csignal_calculate_closest_power_of_two( in_signal_length ) * 2;
    
    return_value =
      cpc_safe_malloc (
                       ( void** ) out_fft,
                       sizeof( FLOAT32 ) * *out_fft_length
                       );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_convert_real_array_to_complex_array (
                                                     in_signal_length,
                                                     in_signal,
                                                     *out_fft_length,
                                                     *out_fft
                                                     );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        //  The -1 is required for the first parameter because of the way the
        //  fft algorithm was written.
        csignal_fft( *out_fft - 1, *out_fft_length / 2, CALCULATE_FFT );
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc fft: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

void
csignal_fft (
             FLOAT32* io_data,
             UINT32   in_data_length,
             CHAR     in_sign
             )
{
  FLOAT32*  data = io_data;
  UINT32    nn = in_data_length;
  INT32     isign = in_sign;
  
  UINT32  n,mmax,m,j,istep,i;
  FLOAT64 wtemp,wr,wpr,wpi,wi,theta;
  FLOAT32 tempr,tempi;
  
  n=nn << 1;
  j=1;
  
  for (i=1;i<n;i+=2) {
    if (j > i) {
      
      SWAP(data[j],data[i]);
      SWAP(data[j+1],data[i+1]);
    }
    m=nn;
    while (m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  
  mmax=2;
  while (n > mmax) {
    istep=mmax << 1;
    theta=isign*(6.28318530717959/mmax);
    wtemp=sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi=sin(theta);
    wr=1.0;
    wi=0.0;
    for (m=1;m<mmax;m+=2) {
      for (i=m;i<=n;i+=istep) {
        j=i+mmax;
        
        tempr=wr*data[j]-wi*data[j+1];
        tempi=wr*data[j+1]+wi*data[j];
        
        data[j]=data[i]-tempr;
        data[j+1]=data[i+1]-tempi;
        
        data[i] += tempr;
        data[i+1] += tempi;
      }
      wr=(wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
    }
    mmax=istep;
  }
}

csignal_error_code
csignal_convert_real_array_to_complex_array (
                                             UINT32   in_real_signal_length,
                                             INT16*   in_real_signal,
                                             UINT32   in_complex_signal_length,
                                             FLOAT32* in_complex_signal
                                             )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == in_real_signal || NULL == in_complex_signal )
  {
    CPC_ERROR (
               "Real signal (0x%x) or complex signal (0x%x) are null.",
               in_real_signal,
               in_complex_signal
               );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if (
           csignal_calculate_closest_power_of_two( in_real_signal_length ) * 2
           != in_complex_signal_length
           )
  {
    CPC_ERROR (
               "Closest power of two of the real signal length (0x%x-0x%x) must"
               " be half of complex signal length (0x%x).",
               in_real_signal_length,
               csignal_calculate_closest_power_of_two( in_real_signal_length ),
               in_complex_signal_length
               );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    for (
         UINT32 i = 0;
         i < in_complex_signal_length && ( i / 2 ) < in_real_signal_length;
         i += 2
         )
    {
      in_complex_signal[ i ] = in_real_signal[ ( i / 2 ) ];
    }
  }
  
  return( return_value );
}

UINT32
csignal_calculate_closest_power_of_two  (
                                         UINT32 in_number
                                         )
{
  FLOAT32 number  = ( in_number * 1.0 );
  UINT32 exponent = ( UINT32 )
    CPC_LOGARITHM_10( FLOAT32, in_number ) / CPC_LOGARITHM_10( FLOAT32, 2.0 );
  
  CPC_LOG (
           CPC_LOG_LEVEL_TRACE,
           "Number is 0x%x, exponent is 0x%x.",
           in_number,
           exponent
           );
  
  if( CPC_POW( FLOAT32, 2.0, exponent ) != number )
  {
    number = CPC_POW( FLOAT32, 2.0, ( exponent + 1 ) );
  }
  
  CPC_LOG (
           CPC_LOG_LEVEL_TRACE,
           "Number is 0x%x, exponent is 0x%x, next power is 0x%x-%e.",
           in_number,
           exponent,
           ( UINT32 ) number,
           number
           );
  
  return( ( UINT32 ) number );
}
