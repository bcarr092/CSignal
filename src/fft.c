/*! \file   fft.c
    \brief  The implementation of the FFT algorithm.
 
    \author Brent Carrara
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

/*! \fn     csignal_error_code csignal_convert_real_array_to_complex_array (
              USIZE    in_real_signal_length,
              FLOAT64* in_real_signal,
              USIZE    in_complex_signal_length,
              FLOAT64* in_complex_signal
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
                                             USIZE    in_real_signal_length,
                                             FLOAT64* in_real_signal,
                                             USIZE    in_complex_signal_length,
                                             FLOAT64* in_complex_signal
                                             );

/*! \fn     void csignal_fft (
              FLOAT64* io_data,
              USIZE    in_data_length,
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
             FLOAT64* io_data,
             USIZE    in_data_length,
             CHAR     in_sign
             );

csignal_error_code
csignal_calculate_FFT (
                       USIZE      in_signal_length,
                       FLOAT64*   in_signal,
                       USIZE*     out_fft_length,
                       FLOAT64**  out_fft
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
  else if (
           *out_fft_length != 0
           && (
               csignal_calculate_closest_power_of_two( in_signal_length ) * 2
               ) > *out_fft_length
           )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Out fft length (%d) must be greater or equal to the length of"
               " twice the next power of two larger than signal length (%d).",
               *out_fft_length,
               csignal_calculate_closest_power_of_two( in_signal_length )
               );
  }
  else if( *out_fft_length != 0 && NULL == *out_fft )
  {
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
    
    CPC_ERROR (
               "Out fft length (%d) is set, but out fft (0x%x) is null.",
               *out_fft_length,
               *out_fft
               );
  }
  else
  {
    *out_fft_length =
      csignal_calculate_closest_power_of_two( in_signal_length ) * 2;
    
    CPC_LOG (
             CPC_LOG_LEVEL_TRACE,
             "Power of 2 is %d (%d), length is %d.",
             csignal_calculate_closest_power_of_two( in_signal_length ),
             in_signal_length,
             *out_fft_length
             );
    
    if( NULL == *out_fft )
    {
      return_value =
        cpc_safe_malloc (
                         ( void** ) out_fft,
                         sizeof( FLOAT64 ) * *out_fft_length
                         );
    }
    
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
        CPC_LOG_BUFFER_FLOAT64  (
                                 CPC_LOG_LEVEL_TRACE,
                                 "Complex signal:",
                                 *out_fft,
                                 200,
                                 8
                                 );
        
        CPC_LOG( CPC_LOG_LEVEL_TRACE, "Length is %d.", *out_fft_length );
        
        //  The -1 is required for the first parameter because of the way the
        //  fft algorithm was written in Numerical Recipes.
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
             FLOAT64* io_data,
             USIZE    in_data_length,
             CHAR     in_sign
             )
{
  FLOAT64* data = io_data;
  USIZE nn      = in_data_length;
  FLOAT64 isign = ( in_sign * 1.0 );
  
  USIZE n,mmax,m,j,istep,i;
  FLOAT64 wtemp,wr,wpr,wpi,wi,theta;
  FLOAT64 tempr,tempi;
  
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
                                             USIZE    in_real_signal_length,
                                             FLOAT64* in_real_signal,
                                             USIZE    in_complex_signal_length,
                                             FLOAT64* in_complex_signal
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
         USIZE i = 0;
         i < in_complex_signal_length && ( i / 2 ) < in_real_signal_length;
         i += 2
         )
    {
      in_complex_signal[ i ] = in_real_signal[ ( i / 2 ) ];
    }
  }
  
  return( return_value );
}

USIZE
csignal_calculate_closest_power_of_two  (
                                         USIZE in_number
                                         )
{
  FLOAT64 number  = ( in_number * 1.0 );
  FLOAT64 exponent  =
    CPC_LOGARITHM_10( FLOAT64, number ) / CPC_LOGARITHM_10( FLOAT64, 2.0 );
  
  CPC_LOG (
           CPC_LOG_LEVEL_DEBUG,
           "Number is %.2lf, exponent is (%.6f / %.6f) %.6f.",
           number,
           CPC_LOGARITHM_10( FLOAT64, number ),
           CPC_LOGARITHM_10( FLOAT64, 2.0 ),
           exponent
           );
  
  if( CPC_POW( FLOAT64, 2.0, CPC_FLOOR( FLOAT64, exponent ) ) != number )
  {
    number = CPC_POW( FLOAT64, 2.0, CPC_CEIL( FLOAT64, exponent ) );
  }
  
  CPC_LOG (
           CPC_LOG_LEVEL_DEBUG,
           "Starting number is %d, exponent is %.2f, next power is %.2lf (%d).",
           in_number,
           exponent,
           number,
           ( USIZE ) number
           );
  
  return( ( USIZE ) number );
}
