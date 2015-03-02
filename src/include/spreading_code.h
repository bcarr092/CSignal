/*! \file   spreading_code.h
    \brief  This file contains all functions related to creating a spreading
            code (i.e. the c(t) function in spread spectrum modulation). Gold
            Codes are employed to allow for multiple access.
 */
#ifndef __SPREADING_CODE_H__
#define __SPREADING_CODE_H__

#include <cpcommon.h>

#include "csignal_error_codes.h"

/*! \var    spreading_code
    \brief  Spreading codes are generated using a linear feedback shift register
            (LFSR) and should be used to generate m-sequences. That is maximum
            length sequences. A maximum length sequence will generate a code
            of length N = 2^n - 1 before repeating, where n is the number of
            stages in the LFSR. We only model 32 stage LFSR in this library.
            
            A LFSR is composed of stages (think flip-flops) and taps. A tap can
            be opened or closed. A '1' bit in the generator_polynomial indicates
            the tap is closed in that bit position. Each time a bit is generated
            by the spreading code all bits in the state vector are shifted and
            all bits that have closed taps associated with them are summed
            modulo-2 to determine the next input bit. The output bit of the
            spreading code generator is the right-most bit in the state
            variable.
 
            For an introductory discussion on LFSRs, m-sequences and Gold Codes
            see 'Introduction to Spread Spectrum Communications' by Peterson,
            et al., Chapter 3.
 */
typedef struct spreading_code_t
{
  /*! \var    generator_polynomial
      \brief  The generator polynomial must be a primitive polynomial to ensure
              an m-sequence is generated. The generator polynomial has a '1'
              bit set if there is a closed tap in that position in the LFSR. If
              a 4 stage LFSR was being used with the generator polynomial bit
              vector [ 1 0 1 0 ] it would indicate that the 2nd and 4th taps are
              closed.
   
      \note   The LSB in the polynomial MUST BE '1'. The MSB is implicitly '1'.
   */
  UINT32 generator_polynomial;
  
  /*! \var    state
      \brief  This is the current state of the LFSR, i.e. the configuration of
              all the stages. Each bit in the code in the sequence that is
              generated bit-shifts the state variable 1 position to the right.
   
      \note   If the state is initialized to 0, the LFSR will always generate
              '0' bits.
   */
  UINT32 state;
  
} spreading_code;

/*! \fn     csignal_error_code csignal_initialize_spreading_code (
              UINT32           in_generator_polynomial,
              UINT32           in_initial_state,
              spreading_code** out_spreading_code
            )
    \brief  Initializes the sequence generator struct, spreading_code. It
            also ensures that the generator polynomial is valid.
 
    \note   The LSB of the polynomial must be a '1'
 
    \param  in_generator_polynomial The generator polynomial for the LFSR.
    \param  in_initial_state  The initial state of the LFSR. If this value is 0
                              the generator will only ever generate 0s.
    \param  out_spreading_code  If no errors are detected a new spreading_code
                                struct will be created and returned. The caller
                                must free this structure.
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc for other possible errors):
 
            CPC_ERROR_CODE_INVALID_PARAMETER  If the LSB in the polynomial is
                                              not '1'
 */
csignal_error_code
csignal_initialize_spreading_code (
                                   UINT32           in_generator_polynomial,
                                   UINT32           in_initial_state,
                                   spreading_code** out_spreading_code
                                   );

/*! \fn     csignal_error_code csignal_get_spreading_code  (
              spreading_code*  in_spreading_code,
              UINT32           in_number_of_code_bits,
              UINT32*          out_size,
              UCHAR**          out_code
            )
    \brief  Gets the next in_number_of_bits from the sequence generator
            configured in in_spreading_code. The generated bits are returned
            as a byte array in out_code. The size of the byte array is in
            out_size.
 
    \param  in_spreading_code The generator and initial state of the LFSR.
    \param  in_number_of_bits The number of code bits to get from the LFSR.
    \param  out_size  The size of the byte array containing the code bits.
    \param  out_code  The code bits generated
    \return Returns NO_ERROR upon succesful execution or one of these errors
            (see cpc_safe_malloc and csignal_get_spreading_code_bit for other
            possible errors):

            CPC_ERROR_CODE_NULL_POINTER If in_spreading_code is NULL
            CPC_ERROR_CODE_INVALID_PARAMETER  If in_number_of_bits is zero
 */
csignal_error_code
csignal_get_spreading_code  (
                             spreading_code*  in_spreading_code,
                             UINT32           in_number_of_code_bits,
                             UINT32*          out_size,
                             UCHAR**          out_code
                             );

#endif  /*  __SPREADING_CODE_H__  */
