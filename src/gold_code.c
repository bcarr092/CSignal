/*! \file   gold_code.c
    \brief  The impementation of Gold Codes. The implementation simply uses two
            m-sequences of the same degree that meet the "preferred pair"
            criteria to generate Gold Code sequences.
 
    \author Brent Carrara
 */
#include "gold_code.h"

csignal_error_code
csignal_initialize_gold_code  (
                               UCHAR        in_degree,
                               UINT32       in_generator_polynomial_1,
                               UINT32       in_generator_polynomial_2,
                               UINT32       in_initial_state_1,
                               UINT32       in_initial_state_2,
                               gold_code**  out_gold_code
                               )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    return_value =
      cpc_safe_malloc( ( void** ) out_gold_code, sizeof( gold_code ) );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_initialize_spreading_code (
                                       in_degree,
                                       in_generator_polynomial_1,
                                       in_initial_state_1,
                                       &( ( *out_gold_code )->polynomials[ 0 ] )
                                           );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        return_value =
        csignal_initialize_spreading_code (
                                       in_degree,
                                       in_generator_polynomial_2,
                                       in_initial_state_2,
                                       &( ( *out_gold_code )->polynomials[ 1 ] )
                                           );

        if( CPC_ERROR_CODE_NO_ERROR != return_value )
        {
          cpc_safe_free( ( void** )&( ( *out_gold_code )->polynomials[0] ) );
          cpc_safe_free( ( void** )out_gold_code );
        }
      }
      else
      {
        cpc_safe_free( ( void** )out_gold_code );
      }
    }
    else
    {
      CPC_ERROR( "Could not malloc gold code: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_destroy_gold_code (
                           gold_code* io_gold_code
                           )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code was null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else
  {
    if( NULL != io_gold_code->polynomials[ 0 ] )
    {
      return_value =
        cpc_safe_free( ( void** ) &( io_gold_code->polynomials[ 0 ] ) );
    }
    
    if  (
         CPC_ERROR_CODE_NO_ERROR == return_value
         && NULL != io_gold_code->polynomials[ 1 ]
         )
    {
      return_value =
        cpc_safe_free( ( void** ) &( io_gold_code->polynomials[ 1 ] ) );
    }

    cpc_safe_free( ( void** )&io_gold_code );
  }
  
  return( return_value );
}

csignal_error_code
csignal_get_gold_code  (
                        gold_code*  in_gold_code,
                        USIZE       in_number_of_code_bits,
                        USIZE*      out_size,
                        UCHAR**     out_code
                        )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == out_size || NULL == out_code )
  {
    CPC_ERROR( "Size (0x%x) or code (0x%x) are null.", out_size, out_code );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( NULL == in_gold_code )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code is null." );
    
    return_value = CPC_ERROR_CODE_NULL_POINTER;
  }
  else if( 0 == in_number_of_code_bits )
  {
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Number of bits is zero." );
    
    return_value = CPC_ERROR_CODE_INVALID_PARAMETER;
  }
  else
  {
    UCHAR* code_1 = NULL;
    UCHAR* code_2 = NULL;
    
    USIZE size_1 = 0;
    USIZE size_2 = 0;
    
    return_value =
      csignal_get_spreading_code  (
                                   in_gold_code->polynomials[ 0 ],
                                   in_number_of_code_bits,
                                   &size_1,
                                   &code_1
                                   );

    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_get_spreading_code  (
                                     in_gold_code->polynomials[ 1 ],
                                     in_number_of_code_bits,
                                     &size_2,
                                     &code_2
                                     );
      
      if( CPC_ERROR_CODE_NO_ERROR == return_value )
      {
        if( size_1 == size_2 )
        {
          *out_size = size_1;
          
          return_value =
            cpc_safe_malloc( ( void** ) out_code, sizeof( UCHAR ) * *out_size );
          
          if( CPC_ERROR_CODE_NO_ERROR == return_value )
          {
            for( UINT32 i = 0; i < *out_size; i++ )
            {
              ( *out_code )[ i ] = code_1[ i ] ^ code_2[ i ];
            }
          }
          else
          {
            CPC_ERROR( "Could not malloc buffer: 0x%x.", return_value );
          }
        }
        else
        {
          CPC_ERROR (
                     "Sizes from primitive polynomials do not match:"
                     " 0x%x != 0x%x.",
                     size_1,
                     size_2
                     );
          
          return_value = CSIGNAL_ERROR_CODE_LENGTH_MISMATCH;
        }
    
        cpc_safe_free( ( void** ) &code_2 );
      }
      
      cpc_safe_free( ( void** ) &code_1 );
    }
  }
  
  return( return_value );
}

csignal_error_code
csignal_reset_gold_code (
                         gold_code* io_gold_code
                         )
{
  csignal_error_code return_value = CPC_ERROR_CODE_NO_ERROR;
  
  if( NULL == io_gold_code )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_LOG_STRING( CPC_LOG_LEVEL_ERROR, "Gold code is null." );
  }
  else if (
           NULL == io_gold_code->polynomials[ 0 ]
           || NULL == io_gold_code->polynomials[ 1 ]
           )
  {
    return_value = CPC_ERROR_CODE_NULL_POINTER;
    
    CPC_ERROR (
               "Polynomial one (0x%x) or two (0x%x) are null.",
               io_gold_code->polynomials[ 0 ],
               io_gold_code->polynomials[ 1 ]
               );
  }
  else
  {
    return_value =
      csignal_reset_spreading_code( io_gold_code->polynomials[ 0 ] );
    
    if( CPC_ERROR_CODE_NO_ERROR == return_value )
    {
      return_value =
        csignal_reset_spreading_code( io_gold_code->polynomials[ 1 ] );
      
      if( CPC_ERROR_CODE_NO_ERROR != return_value )
      {
        CPC_ERROR( "Could not reset polynomial two: 0x%x.", return_value );
      }
    }
    else
    {
      CPC_ERROR( "Could not reset polynomial one: 0x%x.", return_value );
    }
  }
  
  return( return_value );
}
