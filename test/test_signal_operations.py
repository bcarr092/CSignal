from csignal_tests import *

import unittest
import random
import struct
import copy

class TestsSignalOperations( unittest.TestCase ):
  def test_calculate_thresholds( self ):
    bitsPerSymbol         = 8
    constellationSize     = 2 ** bitsPerSymbol
    sampleRate            = 48000
    basebandAmplitude     = 1
    carrierFrequency      = 21000
    symbolDuration        = 480
    chipDuration          = 48 
    widebandStopbandGap   = 1000
    narrowbandStopbandGap = 1000
    testsPerChip          = 2
    decimationFactor      = int( chipDuration / testsPerChip )

    widebandFirstPassband  = carrierFrequency - ( sampleRate / chipDuration )
    widebandSecondPassband = carrierFrequency + ( sampleRate / chipDuration )

    widebandFirstStopband  = widebandFirstPassband - widebandStopbandGap
    widebandSecondStopband = widebandSecondPassband + widebandStopbandGap

    narrowbandFirstPassband = \
      carrierFrequency - ( sampleRate / symbolDuration )
    narrowbandSecondPassband = \
      carrierFrequency + ( sampleRate / symbolDuration )

    narrowbandFirstStopband  = narrowbandFirstPassband - narrowbandStopbandGap
    narrowbandSecondStopband = narrowbandSecondPassband + narrowbandStopbandGap

    attenuationPassband = 0.1
    attenuationStopband = 80

    widebandFilter = \
      python_initialize_kaiser_filter (
        widebandFirstStopband,
        widebandFirstPassband,
        widebandSecondPassband,
        widebandSecondStopband,
        attenuationPassband,
        attenuationStopband,
        sampleRate
                                      )
    narrowbandFilter = \
      python_initialize_kaiser_filter (
        narrowbandFirstStopband,
        narrowbandFirstPassband,
        narrowbandSecondPassband,
        narrowbandSecondStopband,
        attenuationPassband,
        attenuationStopband,
        sampleRate
                                      )

    inphaseCode = \
      python_initialize_gold_code (
        7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000
                                  )
    quadratureCode =  \
      python_initialize_gold_code (
        7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000
                                  )

    data = '\x00' * 13 # Want to go through one iteration of the code sequence
    
    tracker = python_bit_stream_initialize( data )

    self.assertNotEqual( tracker, None )

    ( numberOfBits, buffer ) = \
      python_bit_stream_get_bits( tracker, bitsPerSymbol ) 

    self.assertEquals( numberOfBits, bitsPerSymbol )
    self.assertNotEquals( buffer, None )

    symbol = struct.unpack( "B", buffer )[ 0 ]

    self.assertNotEquals( symbol, None )

    spreadingCode = []

    while( symbol != None ):
      components = python_modulate_symbol (
          symbol,
          constellationSize,
          sampleRate,
          symbolDuration,
          basebandAmplitude,
          carrierFrequency
                                          )
  
      self.assertNotEquals( components, None )
      self.assertEquals( len( components ), 2 )
      self.assertNotEquals( components[ 0 ], None )
      self.assertNotEquals( components[ 1 ], None )
      self.assertEquals( len( components[ 0 ] ), symbolDuration )
      self.assertEquals( len( components[ 1 ] ), symbolDuration )

      I = python_spread_signal (
          inphaseCode,
          chipDuration,
          components[ 0 ]
                                          )

      self.assertNotEquals( I, None )

      Q = python_spread_signal  (
          quadratureCode,
          chipDuration,
          components[ 1 ]
                                )

      self.assertNotEquals( Q, None )

      part = []

      for index in range( symbolDuration ):
        sampleValue = I[ index ] - Q[ index ]

        self.assertTrue( abs( sampleValue ) <= basebandAmplitude )

        part.append( sampleValue )

      self.assertNotEquals( part, None )
      self.assertEquals( len( part ), symbolDuration )

      spreadingCode = spreadingCode + part

      self.assertNotEquals( spreadingCode, None )

      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( tracker, bitsPerSymbol ) 

      if( 0 == numberOfBits ):
        symbol = None
      else:
        symbol = struct.unpack( "B", buffer )[ 0 ] 

    signal = copy.copy( spreadingCode )

    noise = [ basebandAmplitude * random.normalvariate( 0, 1 ) for i in range( len( spreadingCode ) ) ]

    self.assertEquals( len( signal ), len( spreadingCode ) )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        None,
        widebandFilter,
        narrowbandFilter,
        signal,
        decimationFactor
                                          )

    self.assertEquals( None, thresholds )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        None,
        decimationFactor
                                          )

    self.assertEquals( None, thresholds )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        None,
        narrowbandFilter,
        signal,
        decimationFactor
                                          )

    self.assertEquals( None, thresholds )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        None,
        signal,
        decimationFactor
                                          )

    self.assertEquals( None, thresholds )

    shortTest = [ 1.0 ]

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        shortTest,
        decimationFactor
                                          )

    self.assertEquals( None, thresholds )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        signal,
        decimationFactor
                                          )

    self.assertNotEquals( None, thresholds )

    max = 0
    index = 0

    for i in range( len( thresholds ) ):
      value = thresholds[ i ]

      if( value > max ):
        max = value
        index = i

    self.assertTrue( max > 1.0 )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        noise + signal,
        decimationFactor
                                          )

    self.assertNotEquals( None, thresholds )

    max = 0
    index = 0

    for i in range( len( thresholds ) ):
      value = thresholds[ i ]

      if( value > max ):
        max = value
        index = i

    self.assertTrue( max > 1.0 )

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        noise,
        decimationFactor
                                          )

    self.assertNotEquals( None, thresholds )

    max = 0
    index = 0

    for i in range( len( thresholds ) ):
      value = thresholds[ i ]

      if( value > max ):
        max = value
        index = i

    self.assertTrue( max < 1.0 )

    noise = [ 0.0 for i in range( len( spreadingCode ) ) ]

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        spreadingCode,
        widebandFilter,
        narrowbandFilter,
        noise,
        decimationFactor
                                          )

    self.assertNotEquals( None, thresholds )

    [ self.assertEquals( thresholds[ i ], 0.0 ) for i in range( len( thresholds ) ) ]

    self.assertEquals( bit_stream_destroy( tracker ), CPC_ERROR_CODE_NO_ERROR )
    self.assertEquals( csignal_destroy_passband_filter( widebandFilter ), CPC_ERROR_CODE_NO_ERROR )
    self.assertEquals( csignal_destroy_passband_filter( narrowbandFilter ), CPC_ERROR_CODE_NO_ERROR )
    self.assertEquals( csignal_destroy_gold_code( inphaseCode ), CPC_ERROR_CODE_NO_ERROR )
    self.assertEquals( csignal_destroy_gold_code( quadratureCode ), CPC_ERROR_CODE_NO_ERROR )

"""
  def test_energy( self ):
    test = [ 1.0 for i in range( 100 ) ]

    energy = python_csignal_calculate_energy( test )

    self.assertNotEquals( None, test )
    self.assertEquals( 100, energy )

    for _ in range( 1000 ):
      test = [ random.normalvariate( 0, 1 ) for i in range( 100 ) ]

      output = python_csignal_calculate_energy( test )

      self.assertNotEquals( output, None )

  def test_energy_negative( self ):
    energy = python_csignal_calculate_energy( None )

    self.assertEquals( None, energy )

  def test_multiply_signals_negative( self ):
    test = [ 1.0 ]

    output = python_csignal_multiply_signals( None, None )

    self.assertEquals( output, None )

    output = python_csignal_multiply_signals( test, None )

    self.assertEquals( output, None )

    output = python_csignal_multiply_signals( None, test )

    self.assertEquals( None, output )

    test2= [ 1.0, 2.0 ]

    output = python_csignal_multiply_signals( test, test2 )

    self.assertEquals( None, output )

  def test_multiply_signals( self ):
    test = [ 1.0 for i in range( 100 ) ]
    test2 = [ random.randint( 0, 10 ) * 1.0 for i in range( 100 ) ]

    output = python_csignal_multiply_signals( test, test2 )

    self.assertNotEquals( output, None )
    self.assertEquals( len( output ), len( test ) )
    self.assertEquals( len( output ), len( test2 ) )

    [ self.assertEquals( output[ i ], test2[ i ] ) for i in range( len( output ) ) ]

    for _ in range( 1000 ):
      test = [ random.normalvariate( 0, 1 ) for i in range( 100 ) ]
      test2 = [ random.normalvariate( 0, 1 ) for i in range( 100 ) ]

      output = python_csignal_multiply_signals( test, test2 )

      self.assertNotEquals( output, None )
      self.assertEquals( len( output ), len( test ) )
      self.assertEquals( len( output ), len( test2 ) )
"""

if __name__ == '__main__':
  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()