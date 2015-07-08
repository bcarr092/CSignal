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
    self.testsPerChip      = 4
    self.decimationFactor  = int( self.chipDuration / self.testsPerChip )

    #self.threshold         = 8 * 10 ** 9
    #self.SNR               = 20

    #self.threshold         = 2.25 * 10 ** 10
    #self.SNR               = 10 

    self.threshold         = 7.25 * 10 ** 10
    self.SNR               = 0 

    #self.threshold         = 2.2 * 10 ** 11
    #self.SNR               = -10 

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

    self.generatorDegree  = 7

    self.inphaseCode = \
      python_initialize_gold_code (
        self.generatorDegree,
        0x12000000,
        0x1E000000,
        0x12345678,
        0x12345678
                                  )
    self.quadratureCode =  \
      python_initialize_gold_code (
        self.generatorDegree,
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

    carrierSignal =  \
      python_generate_carrier_signal( self.sampleRate, self.carrierFrequency )

    self.carrierPacker  = python_bit_packer_initialize()
    self.carrierStream  = \
      python_bit_stream_initialize_from_bit_packer( True, self.carrierPacker )

    for value in carrierSignal:
      sampleValue = int( self.basebandAmplitude * value )

      sampleValue = struct.pack( "i", sampleValue )

      python_bit_packer_add_bytes( sampleValue, self.carrierPacker )   

    self.numberOfTrainingChips = 2 ** 7 - 1

    self.generate_training_sequence()

    self.silenceSamples           = \
      self.numberOfTrainingSymbols * self.symbolDuration * 4
    self.propagationDelaySamples  = int( math.ceil( self.sampleRate / 340 ) )
    self.channelImpulseResponse   = [ 1.0, 0.0, 0.8, 0.0, 0.6, 0.0 ]
    self.channelImpulseResponse   = \
      [ 0.0 ] * ( self.silenceSamples + self.propagationDelaySamples )  \
      + self.channelImpulseResponse

    print "Number of delay samples: %d."  \
      %( self.silenceSamples + self.propagationDelaySamples )

  def generate_training_sequence( self ):
    data = '\x00' * self.numberOfTrainingSymbols
    
    tracker = python_bit_stream_initialize( False, data )

    ( numberOfBits, buffer ) = \
      python_bit_stream_get_bits( tracker, self.bitsPerSymbol ) 

    symbol = struct.unpack( "B", buffer )[ 0 ]

    self.spreadingCode = []

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
  def generate_transmit_signal( self ):
    trainingData  = '\x00' * self.numberOfTrainingSymbols
    data          = \
      ''.join( random.choice( string.ascii_lowercase ) for _ in range( 2 * self.numberOfTrainingSymbols ) )

    transmitData = trainingData + data
    #transmitData = trainingData
    
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
    noiseVariance = math.sqrt( signalPower / powerRatio )

    signal = \
      map (
        lambda x: x + random.normalvariate( 0, noiseVariance ),
        signal
          )

    [ self.assertNotEquals( None, value ) for value in signal ]

    return( signal )

  def findStartOffset( self, signal ):
    print "Signal length is %d." %( len( signal ) )

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

    self.outputSequence( "thresholds.dat", thresholds )

    thresholdIndex = -1

    for i in range( len( thresholds ) ):
      if( thresholds[ i ] > self.threshold ):
        thresholdIndex = i

        break

    if( -1 != thresholdIndex ):
      print "Threshold index is: %d." %( thresholdIndex )

      adjustedThresholdIndex = thresholdIndex * self.decimationFactor

      print "Adjusted threshold index is: %d." %( adjustedThresholdIndex )

      searchStartIndex =  \
        int( adjustedThresholdIndex + len( self.spreadingCode )  \
        + python_filter_get_group_delay( self.widebandFilter ) \
        - 0.5 * len( self.spreadingCode ) )
        #- self.chipDuration )

      if( 0 > searchStartIndex ):
        searchStartIndex = 0

      endSearchIndex =  \
        int( searchStartIndex + ( 1.5 * len( self.spreadingCode ) ) )
        #int( searchStartIndex + self.chipDuration + len( self.spreadingCode ) )

      if( endSearchIndex  >= len( signal) ):
        endSearchIndex = len( signal )

      print "Search space is [ %d, %d )." \
        %( searchStartIndex, endSearchIndex )

      thresholds =  \
        python_csignal_calculate_thresholds (
          self.spreadingCode,
          self.widebandFilter,
          self.narrowbandFilter,
          signal[ searchStartIndex : endSearchIndex ],
          1
                                            )

      maxValue  = -1
      maxOffset = 0

      for i in range( len( thresholds ) ):
        if( thresholds[ i ] > maxValue ):
          maxOffset = i
          maxValue  = thresholds[ i ]

      print "Max offset is %d (%.04f)." %( ( maxOffset + searchStartIndex ), maxValue )

      return( searchStartIndex + maxOffset )

  def receiveSignal( self, signal ):
    print "Received signal is %d samples long." %( len( signal ) )

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

    print "Group delay: %d."  \
      %( python_filter_get_group_delay( self.widebandFilter ) \
        + python_filter_get_group_delay( self.lowpassFilter ) )

  def test_equalizer( self ):
    transmittedSignal = self.generate_transmit_signal()

    self.outputSignal( "transmitted.WAV", transmittedSignal )

    receivedSignal = self.perturb_signal( transmittedSignal, self.SNR )

    self.outputSignal( "received.WAV", receivedSignal )

    #startOffset = self.findStartOffset( receivedSignal )

    #if( None != startOffset ):
      #print "Start offset is %d." %( startOffset )

      #self.receiveSignal( receivedSignal[ startOffset : ] )

    self.receiveSignal( receivedSignal )

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

  def outputSequence( self, fileName, sequence ):
    file = open( fileName, 'w' )

    if( file ):
      for i in range( len( sequence ) ):
        file.write( "%.02f\t%.02f\n" %( i, sequence[ i ] ) )

      file.close() 

if __name__ == '__main__':
  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()
