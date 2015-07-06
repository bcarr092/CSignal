from csignal_tests import *

import time
import os
import math
import unittest
import random
import struct
import string

import wave

class TestsEqualizer( unittest.TestCase ):
  def tearDown( self ):
    self.assertEquals (
      bit_stream_destroy( self.carrierStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( self.carrierPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      csignal_destroy_passband_filter( self.widebandFilter ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      csignal_destroy_passband_filter( self.narrowbandFilter ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      csignal_destroy_passband_filter( self.lowpassFilter ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      csignal_destroy_gold_code( self.inphaseCode ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      csignal_destroy_gold_code( self.quadratureCode ),
      CPC_ERROR_CODE_NO_ERROR
                      )

  def setUp( self ):
    self.bitsPerSymbol     = 8
    self.constellationSize = 2 ** self.bitsPerSymbol
    self.sampleRate        = 48000
    self.bitDepth          = 16
    self.basebandAmplitude = 2 ** ( self.bitDepth - 2 ) - 1
    self.carrierFrequency  = 21000
    self.symbolDuration    = 480
    self.chipDuration      = 48
    self.testsPerChip      = 1
    self.decimationFactor  = int( self.chipDuration / self.testsPerChip )

    self.widebandStopbandGap    = 1000
    self.narrowbandStopbandGap  = 2000

    self.widebandFirstPassband   =  \
      self.carrierFrequency - ( self.sampleRate / self.chipDuration )
    self.widebandSecondPassband  =  \
      self.carrierFrequency + ( self.sampleRate / self.chipDuration )
    
    self.widebandFirstStopband   =  \
      self.widebandFirstPassband - self.widebandStopbandGap
    self.widebandSecondStopband  =  \
      self.widebandSecondPassband + self.widebandStopbandGap

    self.narrowbandFirstPassband   =  \
      self.carrierFrequency - ( self.sampleRate / self.symbolDuration )
    self.narrowbandSecondPassband  =  \
      self.carrierFrequency + ( self.sampleRate / self.symbolDuration )

    self.narrowbandFirstStopband  = \
      self.narrowbandFirstPassband - self.narrowbandStopbandGap
    self.narrowbandSecondStopband = \
      self.narrowbandSecondPassband + self.narrowbandStopbandGap

    self.lowpassPassband = ( self.sampleRate / self.symbolDuration )
    self.lowpassStopband = self.lowpassPassband + self.narrowbandStopbandGap

    self.passbandAttenuation = 0.1
    self.stopbandAttenuation = 80

    self.inphaseCode = \
      python_initialize_gold_code (
        7,
        0x12000000,
        0x1E000000,
        0x12345678,
        0x12345678
                                  )
    self.quadratureCode =  \
      python_initialize_gold_code (
        7,
        0x12000000,
        0x1E000000,
        0x12345678,
        0x12345678
                                  )

    self.widebandFilter =  \
      python_initialize_kaiser_filter (
        self.widebandFirstStopband,
        self.widebandFirstPassband,
        self.widebandSecondPassband,
        self.widebandSecondStopband,
        self.passbandAttenuation,
        self.stopbandAttenuation,
        self.sampleRate
                                      )
    self.narrowbandFilter =  \
      python_initialize_kaiser_filter (
        self.narrowbandFirstStopband,
        self.narrowbandFirstPassband,
        self.narrowbandSecondPassband,
        self.narrowbandSecondStopband,
        self.passbandAttenuation,
        self.stopbandAttenuation,
        self.sampleRate
                                      )
    
    self.lowpassFilter = \
      python_initialize_kaiser_lowpass_filter (
        self.lowpassPassband,
        self.lowpassStopband,
        self.passbandAttenuation,
        self.stopbandAttenuation,
        self.sampleRate
                                              )

    self.channelImpulseResponse = [ 0.0, 0.0, 1.0, 0.0, 0.8, 0.0, 0.6, 0.0 ]

    carrierSignal =  \
      python_generate_carrier_signal( self.sampleRate, self.carrierFrequency )

    self.carrierPacker  = python_bit_packer_initialize()
    self.carrierStream  = \
      python_bit_stream_initialize_from_bit_packer( True, self.carrierPacker )

    for value in carrierSignal:
      sampleValue = int( self.basebandAmplitude * value )

      sampleValue = struct.pack( "i", sampleValue )

      python_bit_packer_add_bytes( sampleValue, self.carrierPacker )   

    self.numberOfTrainingSymbols = 13

    self.generate_training_sequence()

  def generate_training_sequence( self ):
    data = '\x00' * self.numberOfTrainingSymbols
    
    tracker = python_bit_stream_initialize( False, data )

    ( numberOfBits, buffer ) = \
      python_bit_stream_get_bits( tracker, self.bitsPerSymbol ) 

    symbol = struct.unpack( "B", buffer )[ 0 ]

    self.spreadingCode = []

    while( symbol != None ):
      components = python_modulate_symbol (
          symbol,
          self.constellationSize,
          self.sampleRate,
          self.symbolDuration,
          self.basebandAmplitude,
          self.carrierFrequency
                                          )

      I = python_spread_signal (
          self.inphaseCode,
          self.chipDuration,
          components[ 0 ]
                                          )

      Q = python_spread_signal  (
          self.quadratureCode,
          self.chipDuration,
          components[ 1 ]
                                )

      part = []

      for index in range( self.symbolDuration ):
        sampleValue = I[ index ] - Q[ index ]

        part.append( sampleValue )

      self.spreadingCode = self.spreadingCode + part

      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( tracker, self.bitsPerSymbol ) 

      if( 0 == numberOfBits ):
        symbol = None
      else:
        symbol = struct.unpack( "B", buffer )[ 0 ]

  def generate_transmit_signal( self ):
    trainingData  = '\x00' * self.numberOfTrainingSymbols
    data          = \
      ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )

    transmitData = trainingData + data
    
    tracker = python_bit_stream_initialize( False, transmitData )

    self.assertNotEquals( tracker, None )

    ( numberOfBits, buffer ) = \
      python_bit_stream_get_bits( tracker, self.bitsPerSymbol ) 

    self.assertEquals( numberOfBits, self.bitsPerSymbol )
    self.assertNotEquals( buffer, None )

    symbol = struct.unpack( "B", buffer )[ 0 ]

    self.assertNotEquals( symbol, None )

    signal = []

    csignal_reset_gold_code( self.inphaseCode )
    csignal_reset_gold_code( self.quadratureCode )

    while( symbol != None ):
      components = python_modulate_symbol (
          symbol,
          self.constellationSize,
          self.sampleRate,
          self.symbolDuration,
          self.basebandAmplitude,
          self.carrierFrequency
                                          )

      self.assertNotEquals( components, None )
      self.assertEquals( len( components ), 2 )
      self.assertNotEquals( components[ 0 ], None )
      self.assertNotEquals( components[ 1 ], None )
      self.assertEquals( len( components[ 0 ] ), self.symbolDuration )
      self.assertEquals( len( components[ 1 ] ), self.symbolDuration )

      inphase = python_spread_signal (
          self.inphaseCode,
          self.chipDuration,
          components[ 0 ]
                                     )

      self.assertNotEquals( inphase, None )

      quadrature = python_spread_signal  (
          self.quadratureCode,
          self.chipDuration,
          components[ 1 ]
                                              )

      self.assertNotEquals( quadrature, None )

      part = []

      for index in range( self.symbolDuration ):
        part.append( inphase[ index ] - quadrature[ index ] )

      self.assertNotEquals( part, None )
      self.assertEquals( len( part ), self.symbolDuration )

      signal = signal + part

      self.assertNotEquals( signal, None )

      ( numberOfBits, buffer ) =  \
        python_bit_stream_get_bits( tracker, self.bitsPerSymbol ) 

      if( 0 == numberOfBits ):
        symbol = None
      else:
        symbol = struct.unpack( "B", buffer )[ 0 ]

    signal = python_filter_signal( self.widebandFilter, signal )

    self.assertNotEquals( signal, None )

    return( signal )

  def perturb_signal( self, signal, SNRdB ):
    signal = python_convolve( signal, self.channelImpulseResponse )

    self.assertNotEquals( None, signal )

    [ self.assertNotEquals( None, value ) for value in signal ]

    signalPower = \
      python_csignal_calculate_energy( signal ) \
      / ( 1.0 * len( signal ) )

    powerRatio    = 10 ** ( SNRdB / 10 )
    noiseVariance = signalPower / powerRatio

    signalPlusNoise = \
      map (
        lambda x: x + random.normalvariate( 0, noiseVariance ),
        signal
          )

    [ self.assertNotEquals( None, value ) for value in signal ]

    return( signal )

  def findStartOffset( self, signal ):
    startTime = time.time()

    thresholds =  \
      python_csignal_calculate_thresholds ( 
        self.spreadingCode,
        self.widebandFilter,
        self.narrowbandFilter,
        signal,
        self.decimationFactor
                                          )

    self.assertNotEquals( None, thresholds )

    thresholdsTime = time.time()

    print "Threshold time:\t\t%.04f" %( thresholdsTime - startTime )

  def receiveSignal( self, signal ):
    startTime = time.time()

    bandpassFilteredSignal =  \
      python_filter_signal( self.widebandFilter, signal )

    widebandFilterTime = time.time()

    print "Wideband filter time:\t%.04f" %( widebandFilterTime - startTime )

    demodulatedSignal = []

    for sampleValue in bandpassFilteredSignal:
      ( numberOfBits, buffer ) =  \
        python_bit_stream_get_bits( self.carrierStream, 32 )

      carrierValue  = struct.unpack( "i", buffer )[ 0 ]

      demodulatedSignal.append( sampleValue * carrierValue )

    [ self.assertNotEquals( None, value ) for value in demodulatedSignal ]

    demodulateTime = time.time()

    print "Demodulate time:\t%.04f" %( demodulateTime - widebandFilterTime )

    filteredSignal =  \
      python_filter_signal( self.lowpassFilter, demodulatedSignal )

    [ self.assertNotEquals( None, value ) for value in filteredSignal ]

    self.outputSignal( "filtered.WAV", filteredSignal )

    narrowbandFilterTime = time.time()

    print "Narrowband filter time:\t%.04f"  \
      %( narrowbandFilterTime - demodulateTime )

  def test_equalizer( self ):
    transmittedSignal = self.generate_transmit_signal()

    self.outputSignal( "transmitted.WAV", transmittedSignal )

    receivedSignal = self.perturb_signal( transmittedSignal, 20 )

    self.outputSignal( "received.WAV", receivedSignal )

    startOffset = self.findStartOffset( receivedSignal )

    #self.receiveSignal( receivedSignal )

  def outputSignal( self, fileName, signal ):
    maxValue = -1

    for value in signal:
      if( abs( value ) > maxValue ):
        maxValue = abs( value )

    wavSignal = \
      map( lambda x: ( x * 1.0 ) / ( maxValue * 1.0 ), signal )

    samples = [ wavSignal ]

    if( os.path.exists( fileName ) ):
      os.unlink( fileName )

    error = python_write_FLOAT_wav  (
      fileName,
      len( samples ), 
      self.sampleRate,
      len( wavSignal ),
      samples
                                    )

    self.assertEquals( error, CPC_TRUE )

if __name__ == '__main__':
  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()
