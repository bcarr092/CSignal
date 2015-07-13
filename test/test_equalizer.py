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
      csignal_destroy_gold_code( self.spreadingCode ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( self.carrierInphaseStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( self.carrierQuadratureStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( self.spreadingCodeStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( self.spreadingSignalStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( self.dataStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( self.carrierInphasePacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )
                      
    self.assertEquals (
      bit_packer_destroy( self.carrierQuadraturePacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( self.spreadingCodePacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( self.spreadingSignalPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( self.dataPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

  def setUp( self ):
    self.bitsPerSymbol     = 1
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

    #self.threshold         = 7.25 * 10 ** 10
    #self.SNR               = 0 

    #self.threshold         = 2.2 * 10 ** 11
    #self.SNR               = -10 

    self.SNR               = 10 

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
    self.codePeriod       = 2 ** self.generatorDegree - 1

    self.numberOfTrainingSymbols  = 8
    self.numberOfDataSymbols      = 8
    #self.numberOfTrainingSymbols =  \
      #int (
        #math.ceil (
          #self.codePeriod / ( self.symbolDuration / self.chipDuration )
                  #)
          #)

    self.spreadingCode = \
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

    self.delay = 0
    #self.delay                  = \
      #random.randint( 0, self.numberOfTrainingSymbols * self.symbolDuration )
    #self.initialChannelImpulseResponse = [ 1.0, 0.0, 0.8, 0.0, 0.6, 0.0 ]
    self.initialChannelImpulseResponse = [ 1.0 ]
    self.channelImpulseResponse = \
      [ 0.0 ] * self.delay \
      + self.initialChannelImpulseResponse

    #self.readImpulseResponse()

    self.carrierInphasePacker     = None
    self.carrierQuadraturePacker  = None

    self.carrierInphaseStream     = None
    self.carrierQuadratureStream  = None

    self.spreadingCodePacker  = None
    self.spreadingCodeStream  = None

    self.spreadingSignalPacker  = None
    self.spreadingSignalStream  = None

    self.numberOfFeedforwardTaps  = self.numberOfTrainingSymbols
    self.numberOfFeedbackTaps     = 5

    self.equalizerStepSize        = 0.01
    self.equalizerIterations      = 100

    self.dataPacker = None
    self.dataStream = None

    self.populateCarrierStream()

    self.generateSpreadingSequence()

    self.generateSpreadingSignal()

    self.generateSymbolSignals()

    self.generateDataSymbols()

  def readImpulseResponse( self ):
    file = open( "/Users/user/work/AudioSimulation/echoProfiles/unprocessed_profile_1.dat", 'r' )

    self.channelImpulseResponse = []

    lineCount = 0

    if( file ):
      for line in file:
        value = float( line )

        if( 1.0 == value ):
          self.delay = lineCount

        self.channelImpulseResponse.append( value )

        lineCount += 1

      file.close()

  def generateSymbolSignals( self ):
    self.symbolSignals  = []

    for symbol in range( self.constellationSize ):
      result =  \
        python_csignal_modulate_BFSK_symbol (
          symbol,
          self.symbolDuration,
          self.sampleRate,
          self.carrierFrequency
                                            )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      self.assertNotEquals( None, result[ 0 ] )
      self.assertNotEquals( None, result[ 1 ] )

      self.assertEquals( len( result[ 0 ] ), self.symbolDuration )
      self.assertEquals( len( result[ 1 ] ), self.symbolDuration )

      self.symbolSignals.append( [ result[ 0 ], result[ 1 ] ] )

    signal = []

    for i in range( self.symbolDuration ):
      signal.append( self.symbolSignals[ 0 ][ 0 ][ i ] )
    for i in range( self.symbolDuration ):
      signal.append( self.symbolSignals[ 1 ][ 0 ][ i ] )

    self.outputSignal( "symbolSignals.WAV", signal )

  def generateSpreadingSignal( self ):
    self.assertEquals (
      bit_stream_reset( self.spreadingCodeStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.spreadingSignalPacker = python_bit_packer_initialize()

    self.assertNotEquals( None, self.spreadingSignalPacker )

    self.spreadingSignalStream = \
      python_bit_stream_initialize_from_bit_packer  (
        True,
        self.spreadingSignalPacker
                                                    )

    self.assertNotEquals( None, self.spreadingSignalStream )

    spreadingSignal = []

    for i in range( self.codePeriod ):
      result = python_bit_stream_get_bits( self.spreadingCodeStream, 8 )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 8 )
      self.assertEquals( len( buffer ), 1 )

      chipValue = struct.unpack( "b", buffer )[ 0 ]

      for j in range( self.chipDuration ):
        self.assertEquals (
          python_bit_packer_add_bytes (
            struct.pack( "d", chipValue * 1.0 ),
            self.spreadingSignalPacker
                                      ),
          CPC_ERROR_CODE_NO_ERROR
                          )

        spreadingSignal.append( chipValue * 1.0 )

    self.outputSignal( "spreadingSignal.WAV", spreadingSignal )

  def populateCarrierStream( self ):
    result = \
      python_generate_carrier_signal( self.sampleRate, self.carrierFrequency )

    self.assertNotEquals( None, result )

    ( inphase, quadrature ) = result

    self.assertNotEquals( None, inphase )
    self.assertNotEquals( None, quadrature )

    self.assertTrue( len( inphase ) > 0 )
    self.assertTrue( len( quadrature ) > 0 )

    self.carrierInphasePacker     = python_bit_packer_initialize()
    self.carrierQuadraturePacker  = python_bit_packer_initialize()

    self.assertNotEquals( None, self.carrierInphasePacker )
    self.assertNotEquals( None, self.carrierQuadraturePacker )

    self.carrierInphaseStream     = \
      python_bit_stream_initialize_from_bit_packer  (
        True,
        self.carrierInphasePacker
                                                    )
    self.carrierQuadratureStream  = \
      python_bit_stream_initialize_from_bit_packer  (
        True,
        self.carrierQuadraturePacker
                                                    )

    self.assertNotEquals( None, self.carrierInphaseStream )
    self.assertNotEquals( None, self.carrierQuadratureStream )

    for i in range( len( inphase ) ):
      inphaseValue    = struct.pack( "d", inphase[ i ] )
      quadratureValue = struct.pack( "d", quadrature[ i ] )

      self.assertEquals (
        python_bit_packer_add_bytes (
          inphaseValue,
          self.carrierInphasePacker
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )

      self.assertEquals (
        python_bit_packer_add_bytes (
          quadratureValue,
          self.carrierQuadraturePacker
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )

  def generateSpreadingSequence( self ):
    self.assertEquals (
      csignal_reset_gold_code( self.spreadingCode ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    codeSequence = python_get_gold_code( self.spreadingCode, self.codePeriod )

    self.assertNotEquals( None, codeSequence )
    self.assertTrue (
      len( codeSequence ),
      int( math.ceil( self.codePeriod / 8 ) )
                    )

    codeStream = python_bit_stream_initialize( False, codeSequence )

    self.assertNotEquals( None, codeStream )

    self.spreadingCodePacker = python_bit_packer_initialize()

    self.assertNotEquals( None, self.spreadingCodePacker )

    self.spreadingCodeStream =  \
      python_bit_stream_initialize_from_bit_packer  (
        True,
        self.spreadingCodePacker
                                                    )

    self.assertNotEquals( None, self.spreadingCodeStream )

    chipSequence = []

    for i in range( self.codePeriod ):
      result = \
        python_bit_stream_get_bits( codeStream, 1 )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 1 )
      self.assertEquals( len( buffer ), 1 )

      chipValue = struct.unpack( "B", buffer )[ 0 ] >> 7

      self.assertEquals (
        python_bit_packer_add_bytes  (
          struct.pack( "b", ( 1 if( chipValue ) else -1 ) ),
          self.spreadingCodePacker
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )

      chipSequence.append( chipValue )

    self.assertEquals (
      bit_stream_destroy( codeStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.outputSequence( "chips.dat", chipSequence ) 

  def generateTrainingSignal( self ):
    signal      = []

    for i in range( self.numberOfTrainingSymbols ):
      result =  \
        python_bit_stream_get_bits  (
          self.spreadingSignalStream,
          64 * self.symbolDuration
                                    )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 64 * self.symbolDuration )
      self.assertEquals( len( buffer ), 8 * self.symbolDuration )

      chipSamples = list( struct.unpack( "d" * self.symbolDuration, buffer ) )

      self.assertEquals( len( chipSamples ), self.symbolDuration )

      result =  \
        python_bit_stream_get_bits  (
          self.carrierInphaseStream,
          64 * self.symbolDuration
                                    )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 64 * self.symbolDuration )
      self.assertEquals( len( buffer ), 8 * self.symbolDuration )

      inphaseCarrier =  \
        list( struct.unpack( "d" * self.symbolDuration, buffer ) )

      self.assertEquals( len( inphaseCarrier ), self.symbolDuration )

      result =  \
        python_bit_stream_get_bits  (
          self.carrierQuadratureStream,
          64 * self.symbolDuration
                                    )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 64 * self.symbolDuration )
      self.assertEquals( len( buffer ), 8 * self.symbolDuration )

      quadratureCarrier = \
        list( struct.unpack( "d" * self.symbolDuration, buffer ) )

      self.assertEquals( len( quadratureCarrier ), self.symbolDuration )

      self.assertEquals( len( inphaseCarrier ), len( quadratureCarrier ) )

      inphaseSamples =  \
        python_csignal_multiply_signals( chipSamples, inphaseCarrier )

      self.assertNotEquals( None, inphaseSamples )
      self.assertEquals( len( inphaseSamples ), self.symbolDuration )

      quadratureSamples = \
        python_csignal_multiply_signals( chipSamples, quadratureCarrier )

      self.assertNotEquals( None, quadratureSamples )
      self.assertEquals( len( quadratureSamples ), self.symbolDuration )

      inphaseSymbolSignal     = self.symbolSignals[ 1 ][ 0 ]
      quadratureSymbolSignal  = self.symbolSignals[ 1 ][ 1 ]

      self.assertEquals( len( inphaseSymbolSignal ), self.symbolDuration )
      self.assertEquals( len( quadratureSymbolSignal ), self.symbolDuration )

      inphase = \
        python_csignal_multiply_signals( inphaseSamples, inphaseSymbolSignal )
        #python_csignal_multiply_signals( inphaseCarrier, inphaseSymbolSignal )

      self.assertNotEquals( None, inphase )
      self.assertEquals( len( inphase ), self.symbolDuration )

      quadrature =  \
        python_csignal_multiply_signals (
          quadratureSamples,
          quadratureSymbolSignal
                                        )
        #python_csignal_multiply_signals (
          #quadratureCarrier,
          #quadratureSymbolSignal
                                        #)

      self.assertNotEquals( None, quadrature )
      self.assertEquals( len( quadrature ), self.symbolDuration )

      for j in range( self.symbolDuration ):
        sampleValue = inphase[ j ] - quadrature[ j ]

        signal.append( sampleValue )

    self.assertNotEquals( None, signal )
    self.assertEquals (
      len( signal ),
      self.numberOfTrainingSymbols * self.symbolDuration
                      )

    self.outputSignal( "trainingSignal.WAV", signal )

    return( signal )

  def demodulateData( self, signal ):
    self.assertEquals (
      bit_stream_reset( self.carrierInphaseStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.carrierQuadratureStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    result =  \
      python_bit_stream_get_bits  (
        self.carrierInphaseStream,
        64 * len( signal )
                                  )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * len( signal ) )
    self.assertEquals( len( buffer ), 8 * len( signal ) )

    inphaseCarrier = list( struct.unpack( "d" * len( signal ), buffer ) )

    self.assertEquals( len( inphaseCarrier ), len( signal ) )

    result =  \
      python_bit_stream_get_bits  (
        self.carrierQuadratureStream,
        64 * len( signal )
                                  )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * len( signal ) )
    self.assertEquals( len( buffer ), 8 * len( signal ) )

    quadratureCarrier = \
      list( struct.unpack( "d" * len( signal ), buffer ) )

    self.assertEquals( len( quadratureCarrier ), len( signal ) )

    self.assertEquals( len( inphaseCarrier ), len( quadratureCarrier ) )

    inphaseSamples =  \
      python_csignal_multiply_signals( signal, inphaseCarrier )

    self.assertNotEquals( None, inphaseSamples )
    self.assertEquals( len( inphaseSamples ), len( signal ) )

    quadratureSamples = \
      python_csignal_multiply_signals( signal, quadratureCarrier )

    self.assertNotEquals( None, quadratureSamples )
    self.assertEquals( len( quadratureSamples ), len( signal) )

    demodulatedSignal = []

    for i in range( len( signal ) ):
      sampleValue = inphaseSamples[ i ] + quadratureSamples[ i ]

      demodulatedSignal.append( sampleValue )

    self.assertNotEquals( None, demodulatedSignal )
    self.assertEquals( len( demodulatedSignal ), len( signal ) )

    return( demodulatedSignal )

  def modulateData( self, inphaseSignal, quadratureSignal ):
    signal = []

    result =  \
      python_bit_stream_get_bits  (
        self.spreadingSignalStream,
        64 * self.symbolDuration
                                  )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * self.symbolDuration )
    self.assertEquals( len( buffer ), 8 * self.symbolDuration )

    chipSamples = list( struct.unpack( "d" * self.symbolDuration, buffer ) )

    self.assertEquals( len( chipSamples ), self.symbolDuration )

    result =  \
      python_bit_stream_get_bits  (
        self.carrierInphaseStream,
        64 * self.symbolDuration
                                  )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * self.symbolDuration )
    self.assertEquals( len( buffer ), 8 * self.symbolDuration )

    inphaseCarrier = list( struct.unpack( "d" * self.symbolDuration, buffer ) )

    self.assertEquals( len( inphaseCarrier ), self.symbolDuration )

    result =  \
      python_bit_stream_get_bits  (
        self.carrierQuadratureStream,
        64 * self.symbolDuration
                                  )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * self.symbolDuration )
    self.assertEquals( len( buffer ), 8 * self.symbolDuration )

    quadratureCarrier = \
      list( struct.unpack( "d" * self.symbolDuration, buffer ) )

    self.assertEquals( len( quadratureCarrier ), self.symbolDuration )

    self.assertEquals( len( inphaseCarrier ), len( quadratureCarrier ) )

    inphaseSamples =  \
      python_csignal_multiply_signals( chipSamples, inphaseCarrier )

    self.assertNotEquals( None, inphaseSamples )
    self.assertEquals( len( inphaseSamples ), self.symbolDuration )

    quadratureSamples = \
      python_csignal_multiply_signals( chipSamples, quadratureCarrier )

    self.assertNotEquals( None, quadratureSamples )
    self.assertEquals( len( quadratureSamples ), self.symbolDuration )

    inphase = \
      python_csignal_multiply_signals( inphaseSignal, chipSamples )

    self.assertNotEquals( None, inphase )
    self.assertEquals( len( inphase ), self.symbolDuration )

    quadrature =  \
      python_csignal_multiply_signals( quadratureSignal, quadratureSamples )

    self.assertNotEquals( None, quadrature )
    self.assertEquals( len( quadrature ), self.symbolDuration )

    for i in range( self.symbolDuration ):
      #sampleValue = inphase[ i ] - quadrature[ i ]
      #sampleValue = inphaseSignal[ i ]
      sampleValue = inphase[ i ]

      signal.append( sampleValue )

    self.assertNotEquals( None, signal )
    self.assertEquals( len( signal ), self.symbolDuration )

    return( signal )

  def generateDataSymbols( self ):
    self.dataPacker = python_bit_packer_initialize()

    self.assertNotEquals( None, self.dataPacker )

    self.dataStream = \
      python_bit_stream_initialize_from_bit_packer  (
        False,
        self.dataPacker 
                                                    )

    self.assertNotEquals( None, self.dataStream )

    for i in range( self.numberOfTrainingSymbols ):
      symbol = struct.unpack( 'B', '\x00' )[ 0 ]

      bit_packer_add_bits( symbol, 1, self.dataPacker )

    for i in range( self.numberOfDataSymbols ):
      symbol = random.randint( 0, 1 )

      if( symbol ):
        symbol = struct.unpack( 'B', '\x01' )[ 0 ]
      else:
        symbol = struct.unpack( 'B', '\x00' )[ 0 ]

      bit_packer_add_bits( symbol, 1, self.dataPacker )

  def generateTransmitSignal( self ):
    self.assertEquals (
      csignal_reset_gold_code( self.spreadingCode ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.carrierInphaseStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.carrierQuadratureStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.spreadingSignalStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    signal = []

    for i in range( self.numberOfTrainingSymbols + self.numberOfDataSymbols ):
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, 1 ) 

      self.assertEquals( numberOfBits, 1 )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> 7 

      self.assertNotEquals( symbol, None )

      inphaseSignal    = self.symbolSignals[ symbol ][ 0 ]
      quadratureSignal = self.symbolSignals[ symbol ][ 1 ]

      part = self.modulateData( inphaseSignal, quadratureSignal )

      self.assertNotEquals( part, None )

      signal = signal + part

    signal = python_filter_signal( self.widebandFilter, signal )

    self.assertNotEquals( signal, None )

    return( signal )

  def perturbSignal( self, signal, SNRdB ):
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
    self.assertEquals (
      bit_stream_reset( self.spreadingSignalStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    nTrainingSamples = self.numberOfTrainingSymbols * self.symbolDuration

    result =  \
      python_bit_stream_get_bits  (
        self.spreadingSignalStream,
        64 * nTrainingSamples
                                    )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * nTrainingSamples )
    self.assertEquals( len( buffer ), 8 * nTrainingSamples )

    chipSamples = list( struct.unpack( "d" * nTrainingSamples, buffer ) )

    self.assertEquals( len( chipSamples ), nTrainingSamples )

    startTime = time.time()

    nTests = len( signal ) - len( chipSamples )

    energy = []

    for i in range( 0, nTests, self.decimationFactor ):
      despread  = \
        python_csignal_multiply_signals (
          chipSamples,
          signal[ i : i + len( chipSamples ) ]
                                        )
      filtered  = python_filter_signal( self.narrowbandFilter, despread )
      squared   = python_csignal_multiply_signals( filtered, filtered )
      filtered  = python_filter_signal( self.lowpassFilter, squared )
      signalE   = python_csignal_sum_signal( filtered, 1.0 )

      energy.append( signalE )

    maxValue = 0
    maxIndex = 0

    for i in range( len( energy ) ):
      if( energy[ i ] >= maxValue ):
        maxValue = energy[ i ]
        maxIndex = i

    energyTime = time.time() - startTime

    self.assertNotEquals( None, energy )

    print "Energy detection time:\t%.04f" %( energyTime )

    self.outputSequence( "energy.dat", energy )

    return( maxIndex * self.decimationFactor )

  def receiveSignal( self, signal ):
    self.assertEquals (
      bit_stream_reset( self.carrierInphaseStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.carrierQuadratureStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    startTime = time.time()

    bandpassFilteredSignal =  \
      python_filter_signal( self.widebandFilter, signal )

    self.assertNotEquals( None, bandpassFilteredSignal )
    self.assertTrue( len( bandpassFilteredSignal ) > 0 )

    self.outputSignal( "filtered.WAV", bandpassFilteredSignal )

    widebandFilterTime = time.time()

    print "Wideband filter time:\t%.04f" %( widebandFilterTime - startTime )

    #demodulatedSignal = self.demodulateData( bandpassFilteredSignal, 0 )

    #[ self.assertNotEquals( None, value ) for value in demodulatedSignal ]

    #self.outputSignal( "demodulated.WAV", demodulatedSignal )

    #demodulateTime = time.time()

    #print "Demodulate time:\t%.04f" %( demodulateTime - widebandFilterTime )

    #filteredSignal =  \
      #python_filter_signal( self.lowpassFilter, demodulatedSignal )

    #[ self.assertNotEquals( None, value ) for value in filteredSignal ]

    #self.outputSignal( "filtered.WAV", filteredSignal )

    #narrowbandFilterTime = time.time()

    #print "Narrowband filter time:\t%.04f"  \
      #%( narrowbandFilterTime - demodulateTime )

    #print "LPF delay: %d." %( python_filter_get_group_delay( self.lowpassFilter ) )

    #return( filteredSignal, bandpassFilteredSignal )
    return( bandpassFilteredSignal )

  def despreadSignal( self, signal ):
    self.assertEquals (
      bit_stream_reset( self.spreadingSignalStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    nSamples = len( signal )

    result =  \
      python_bit_stream_get_bits  (
        self.spreadingSignalStream,
        64 * nSamples
                                    )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 * nSamples )
    self.assertEquals( len( buffer ), 8 * nSamples )

    chipSamples = list( struct.unpack( "d" * nSamples, buffer ) )

    self.assertEquals( len( chipSamples ), nSamples )

    startTime = time.time()

    despreadSignal = python_csignal_multiply_signals( chipSamples, signal )

    self.assertNotEquals( None, despreadSignal )
    self.assertEquals( len( despreadSignal ), len( signal ) )

    filteredSignal = python_filter_signal( self.narrowbandFilter, despreadSignal )

    return( filteredSignal )
    #demodulatedSignal = self.demodulateData( despreadSignal )
    #demodulatedSignal = self.demodulateData( signal )

    [ self.assertNotEquals( None, value ) for value in demodulatedSignal ]

    self.outputSignal( "demodulated.WAV", demodulatedSignal )

    demodulateTime = time.time()

    print "Demodulate time:\t%.04f" %( demodulateTime - startTime )

    filteredSignal =  \
      python_filter_signal( self.lowpassFilter, demodulatedSignal )

    [ self.assertNotEquals( None, value ) for value in filteredSignal ]

    self.outputSignal( "filtered.WAV", filteredSignal )

    narrowbandFilterTime = time.time()

    print "Narrowband filter time:\t%.04f"  \
      %( narrowbandFilterTime - demodulateTime )

    return( filteredSignal )

  def correctPhase( self, signal ):
    self.outputSignal( "phaseCorrection.WAV", signal )

  def test_equalizer( self ):
    transmittedSignal = self.generateTransmitSignal()

    self.outputSignal( "transmitted.WAV", transmittedSignal )

    receivedSignal = self.perturbSignal( transmittedSignal, self.SNR )
    #receivedSignal = transmittedSignal

    self.outputSignal( "received.WAV", receivedSignal )

    #( filteredSignal, bandpassSignal ) = self.receiveSignal( receivedSignal )
    filteredSignal = self.receiveSignal( receivedSignal )

    self.outputSignal( "filtered.WAV", filteredSignal )

    print "Delay from wideband filter: %d samples." %( python_filter_get_group_delay( self.widebandFilter ) )

    startOffset = self.findStartOffset( filteredSignal )
    #startOffset = 296

    if( None != startOffset ):
      print "Start offset:\t\t%d (Delay: %d)" %( startOffset, self.delay )

      despreadSignal = self.despreadSignal( filteredSignal[ startOffset : ] )
      #despreadSignal = self.despreadSignal( receivedSignal[ startOffset : ] )

      startOffset = python_filter_get_group_delay( self.narrowbandFilter )

      self.outputSignal( "despreadSignal.WAV", despreadSignal[ startOffset : ] )
      #self.outputSignal( "despreadSignal.WAV", despreadSignal )

      self.determineFrequencies( despreadSignal[ startOffset : ] )
      #self.determineFrequencies( despreadSignal )

      #phaseCorrectedSignal =  \
        #self.correctPhase( bandpassSignal[ startOffset : ] )

      #initialCodeSequence =  \
        #self.getInitialCodeSequence( despreadSignal )

      #benchmarkNoEq = self.benchmarkCodeSequence( initialCodeSequence )

      #print "No equalization P_e:\t%.02f" %( 1.0 - benchmarkNoEq )

      #codeSequence = self.getEqualizedCodeSequence( initialCodeSequence )

      #benchmarkEq = self.benchmarkCodeSequence( codeSequence )

      #print "Equalization P_e:\t%.02f" %( 1.0 - benchmarkEq )

  def determineFrequencies( self, signal ):
    symbol0 = self.symbolSignals[ 0 ][ 0 ]
    symbol1 = self.symbolSignals[ 1 ][ 0 ]
    #symbol0 = map( lambda x, y: x - y, self.symbolSignals[ 0 ][ 0 ], self.symbolSignals[ 0 ][ 1 ] )
    #symbol1 = map( lambda x, y: x - y, self.symbolSignals[ 1 ][ 0 ], self.symbolSignals[ 1 ][ 1 ] )

    e_s0 = python_csignal_calculate_energy( symbol0 )
    e_s1 = python_csignal_calculate_energy( symbol1 )

    symbol0 = map( lambda x: x / e_s0, symbol0 )
    symbol1 = map( lambda x: x / e_s1, symbol1 )

    for i in range( 0, len( signal ), self.symbolDuration ):
      if( len( signal ) - i < self.symbolDuration ):
        break

      e_s = python_csignal_calculate_energy( signal[ i : i + self.symbolDuration ] )

      r = map( lambda x: x / e_s, signal[ i : i + self.symbolDuration ] )

      c_s0 = python_csignal_multiply_signals( symbol0, r )
      c_s1 = python_csignal_multiply_signals( symbol1, r )

      c_s0 = python_csignal_sum_signal( c_s0, 1 )
      c_s1 = python_csignal_sum_signal( c_s1, 1 )

      print "Correlation %d: 0:%.04f\t1:%.04f\tDecision:%d" %( ( i / self.symbolDuration ), c_s0, c_s1, ( 0 if( c_s0 > c_s1 ) else 1 ) )

    for i in range( 0, len( signal ), self.symbolDuration ):
      testSignal    = signal[ i : i + self.symbolDuration ] * 10
      fft           = python_calculate_FFT( testSignal )
      fftMagnitudes = map( lambda x: abs( x ), fft )

      N     = len( fftMagnitudes )
      delta = 1.0 / self.sampleRate

      maxValue = -1
      maxIndex = 0

      for i in range( 1, N / 2 ):
        if( fftMagnitudes[ i ] > maxValue ):
          maxValue = fftMagnitudes[ i ]
          maxIndex = i
      
      #fft_mag = map( lambda x: 10**-12 if x == 0 else x, fft_mag )
      #fft_mag = map( lambda x: 10 * math.log10( x / max_value ), fft_mag )

      #frequencies = []
      #magnitudes  = []

      print "Index magnitudes: 21050=%03.04f\t20950=%03.04f" %( fftMagnitudes[ int( 21050.0 * delta * N ) ], fftMagnitudes[ int( 20950.0 * delta * N ) ] )

      #frequency = maxIndex / ( delta * N )

      #print "Max frequency is %.02f" %( frequency )

      #for n in range( N / 2 ):
        #frequency = n / ( delta * N )

        #frequencies.append( frequency )
        #magnitudes.append( fft_mag[ n ] )

      #self.outputDependantSequence( "symbol_%d.dat" %( int( i / self.symbolDuration ) ), frequencies, magnitudes )

  def getTrainingSequence( self ):
    self.assertEquals (
      bit_stream_reset( self.spreadingCodeStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    trainingSequence = []

    for i in range( self.numberOfTrainingChips ):
      result = python_bit_stream_get_bits( self.spreadingCodeStream, 8 )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 8 )
      self.assertEquals( len( buffer ), 1 )

      chipValue = struct.unpack( "b", buffer )[ 0 ]     

      trainingSequence.append( chipValue )

    return( trainingSequence )

  def initializeEqualizer( self ):
    feedforwardWeights  = \
      [ 0.0 for i in range( self.numberOfFeedforwardTaps ) ]
    feedbackWeights     = \
      [ 0.0 for i in range(  self.numberOfFeedbackTaps ) ]

    feedforwardBufferPacker = python_bit_packer_initialize()

    self.assertNotEquals( None, feedforwardBufferPacker )

    feedforwardBufferStream = \
      python_bit_stream_initialize_from_bit_packer  (
        False,
       feedforwardBufferPacker 
                                                    )

    self.assertNotEquals( None, feedforwardBufferStream )

    for i in range( self.numberOfFeedforwardTaps ):
      self.assertEquals (
        python_bit_packer_add_bytes (
          struct.pack( "i", 0 ),
          feedforwardBufferPacker
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )     

    feedbackBufferPacker = python_bit_packer_initialize()

    self.assertNotEquals( None, feedbackBufferPacker )

    feedbackBufferStream = \
      python_bit_stream_initialize_from_bit_packer  (
        False,
       feedbackBufferPacker 
                                                    )

    self.assertNotEquals( None, feedbackBufferStream )

    for i in range( self.numberOfFeedbackTaps ):
      self.assertEquals (
        python_bit_packer_add_bytes (
          struct.pack( "i", 0 ),
          feedbackBufferPacker
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )

    return  (
      feedforwardBufferPacker, feedforwardBufferStream,
      feedbackBufferPacker, feedbackBufferStream,
      feedforwardWeights, feedbackWeights
            )

  def destroyEqualizer  (
    self, feedforwardBufferPacker, feedforwardBufferStream,
    feedbackBufferPacker, feedbackBufferStream
                        ):

    self.assertEquals (
      bit_stream_destroy( feedforwardBufferStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( feedforwardBufferPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_destroy( feedbackBufferStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( feedbackBufferPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

  def equalizerAddAndIncrement( self, packer, stream, value ):
    self.assertEquals (
        python_bit_packer_add_bytes (
          struct.pack( "i", value ),
          packer
                                    ),
        CPC_ERROR_CODE_NO_ERROR
                        )

    result = python_bit_stream_get_bits( stream, 32 )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 32 )
    self.assertEquals( len( buffer ), 4 )

    value = struct.unpack( "i", buffer )[ 0 ]

    return( value )

  def equalizerGetTapValues( self, stream, numberOfTaps ):
    result = python_bit_stream_peak( stream )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 3 )

    ( readPointer, writePointer, buffer ) = result

    self.assertNotEquals( None, readPointer )
    self.assertNotEquals( None, writePointer )
    self.assertNotEquals( None, buffer )

    self.assertEquals( len( buffer ), 4 * numberOfTaps )

    values = list( struct.unpack( "i" * numberOfTaps, buffer ) )

    self.assertNotEquals( None, values )
    self.assertEquals( len( values ), numberOfTaps )

    return( values )

  def equalizerDetermineSymbol  (
    self, feedforwardWeights, feedforwardValues, feedbackWeights,
    feedbackValues, expectedSymbol
                                ):
    feedforwardValue  = 0.0
    feedbackValue     = 0.0

    self.assertEquals (
      len( feedforwardWeights ),
      self.numberOfFeedforwardTaps
                      )
    self.assertEquals( len( feedforwardWeights ), len( feedforwardValues ) )

    self.assertEquals (
      len( feedbackWeights ),
      self.numberOfFeedbackTaps
                      )
    self.assertEquals( len( feedbackWeights ), len( feedbackValues ) )

    for i in range( self.numberOfFeedforwardTaps ):
      feedforwardValue =  \
        feedforwardValue  \
        + ( feedforwardWeights[ i ] * ( feedforwardValues[ i ] * 1.0 ) )

    for i in range( self.numberOfFeedbackTaps ):
      feedbackValue = \
        feedbackValue \
        + ( feedbackWeights[ i ] * ( feedbackValues[ i ] * 1.0 ) )

    symbolEstimate = feedforwardValue - feedbackValue

    decision = 1.0 if( symbolEstimate >= 0 ) else -1.0

    error = 0.0

    if( None == expectedSymbol ):
      error = decision - symbolEstimate
    else:
      error = expectedSymbol - symbolEstimate

    return( int( decision ), error )

  def getEqualizedCodeSequence( self, initialCodeSequence ):
    trainingSymbol    = None
    trainingSequence  = self.getTrainingSequence()

    (
      feedforwardBufferPacker, feedforwardBufferStream,
      feedbackBufferPacker, feedbackBufferStream,
      feedforwardWeights, feedbackWeights
    ) = self.initializeEqualizer()

    codeSequence = []

    for i in range( len( initialCodeSequence ) ):
      self.equalizerAddAndIncrement (
        feedforwardBufferPacker,
        feedforwardBufferStream,
        initialCodeSequence[ i ]
                                    ) 
      feedforwardValues =  \
        self.equalizerGetTapValues  (
          feedforwardBufferStream,
          self.numberOfFeedforwardTaps
                                    )

      feedbackValues =  \
        self.equalizerGetTapValues  (
          feedbackBufferStream,
          self.numberOfFeedbackTaps
                                    )

      if( i < len( trainingSequence ) ):
        trainingSymbol = 1.0 * trainingSequence[ i ]
      else:
        trainingSymbol = None

      for j in range( self.equalizerIterations ):
        ( symbol, error ) = \
          self.equalizerDetermineSymbol ( 
            feedforwardWeights, feedforwardValues,
            feedbackWeights, feedbackValues,
            trainingSymbol
                                      )

        #print "Before:"
        #print "Feedforward weights:", feedforwardWeights
        #print "Feedforward values:", feedforwardValues
        #print "Feedback weights:", feedbackWeights
        #print "Feedback values:", feedbackValues

        ( feedforwardWeights, feedbackWeights ) = \
          self.equalizerUpdateWeights (
            error, feedforwardWeights, feedforwardValues,
            feedbackWeights, feedbackValues
                                      )

        #print "After (Error = %.04f):" %( error )
        #print "Feedforward weights:", feedforwardWeights
        #print "Feedforward values:", feedforwardValues
        #print "Feedback weights:", feedbackWeights
        #print "Feedback values:", feedbackValues

      codeSequence.append( symbol )

      self.equalizerAddAndIncrement (
        feedbackBufferPacker,
        feedbackBufferStream,
        symbol
                                    )

    print "Calculated:", codeSequence

    self.destroyEqualizer ( 
      feedforwardBufferPacker, feedforwardBufferStream,
      feedbackBufferPacker, feedbackBufferStream,
                          )

    return( codeSequence )

  def equalizerUpdateWeights  (
    self, error, feedforwardWeights, feedforwardValues, feedbackWeights,
    feedbackValues
                              ):

    feedforwardWeights  = \
      map (
        lambda x, y: x + error * self.equalizerStepSize * y,
        feedforwardWeights,
        feedforwardValues
          )

    feedbackWeights = \
      map (
        lambda x, y: x + error * self.equalizerStepSize * y,
        feedbackWeights,
        feedbackValues
          )

    return( feedforwardWeights, feedbackWeights )

  def getInitialCodeSequence( self, signal ):
    codeSequence = []

    self.outputSignal( "equalize.WAV", signal )

    for i in range( 0, len( signal ), self.chipDuration ):
      binaryPAMSymbol = \
        python_csignal_demodulate_binary_PAM  (
          signal[ i : i + self.chipDuration ]
                                              )

      self.assertNotEquals( None, binaryPAMSymbol )
      self.assertEquals( abs( binaryPAMSymbol ), 1 )

      codeSequence.append( binaryPAMSymbol )

    return( codeSequence )

  def benchmarkCodeSequence( self, codeSequence ):
    self.assertEquals (
      bit_stream_reset( self.spreadingCodeStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    hitCount        = 0
    desiredSequence = []

    for i in range( len( codeSequence ) ):
      result = python_bit_stream_get_bits( self.spreadingCodeStream, 8 )

      self.assertNotEquals( None, result )
      self.assertEquals( len( result ), 2 )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( None, numberOfBits )
      self.assertNotEquals( None, buffer )

      self.assertEquals( numberOfBits, 8 )
      self.assertEquals( len( buffer ), 1 )

      chipValue = struct.unpack( "b", buffer )[ 0 ]     

      desiredSequence.append( chipValue )

      if( chipValue == codeSequence[ i ] ):
        hitCount += 1

    print "Expected:", desiredSequence

    hitRate = ( hitCount + 1.0 ) / ( len( codeSequence ) * 1.0 )

    return( hitRate )

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

  def outputDependantSequence( self, fileName, variable, sequence ):
    file = open( fileName, 'w' )

    if( file ):
      for i in range( len( sequence ) ):
        file.write( "%.02f\t%.02f\n" %( variable[ i ], sequence[ i ] ) )

      file.close() 

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
