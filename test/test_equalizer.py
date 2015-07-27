from csignal_tests import *

import re
import sys
import time
import os
import cmath
import math
import unittest
import random
import struct
import string

import wave

dataFile = None

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
    self.carrierFrequency  = 18000
    self.symbolDuration    = 1500
    self.chipDuration      = 48
    self.testsPerChip      = 4
    self.samplesPerSymbol  = 1

    self.chipDecimationFactor  = int( self.chipDuration / self.testsPerChip )
    self.symbolDecimationFactor = int( self.symbolDuration / self.samplesPerSymbol )

    #self.threshold         = 8 * 10 ** 9
    #self.SNR               = 20

    #self.threshold         = 2.25 * 10 ** 10
    #self.SNR               = 10 

    #self.threshold         = 7.25 * 10 ** 10
    #self.SNR               = 0 

    #self.threshold         = 2.2 * 10 ** 11
    #self.SNR               = -10 

    self.numberOfStartOffsets = 2
    self.threshold            = 2.0
    self.SNR                  = 20

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

    self.dataSequence         = [ 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 ]
    self.numberOfDataSymbols  = len( self.dataSequence )
    #self.dataSequence = None

    self.numberOfTrainingSymbols  = 32
    #self.numberOfDataSymbols      = 20
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

    #self.delay = 800
    #self.delay                  = \
      #random.randint( 0, self.numberOfTrainingSymbols * self.symbolDuration )
    #self.initialChannelImpulseResponse = [ 1.0, 0.0, 0.8, 0.0, 0.6, 0.0 ]
    #self.initialChannelImpulseResponse = [ 1.0 ]

    #for i in range( 100 ):
      #self.initialChannelImpulseResponse.append( random.uniform( 0, 1 ) )
    #for i in range( 200 ):
      #self.initialChannelImpulseResponse.append( random.uniform( 0, 0.5 ) )
    #for i in range( 1000 ):
      #self.initialChannelImpulseResponse.append( random.uniform( 0, 0.1 ) )
    #for i in range( 10000 ):
      #self.initialChannelImpulseResponse.append( random.uniform( 0, 0.01 ) )

    #self.channelImpulseResponse = \
      #[ 0.0 ] * self.delay \
      #+ self.initialChannelImpulseResponse

    self.readImpulseResponse()

    self.carrierInphasePacker     = None
    self.carrierQuadraturePacker  = None

    self.carrierInphaseStream     = None
    self.carrierQuadratureStream  = None

    self.spreadingCodePacker  = None
    self.spreadingCodeStream  = None

    self.spreadingSignalPacker  = None
    self.spreadingSignalStream  = None

    self.numberOfFeedforwardTaps  = 2
    self.numberOfFeedbackTaps     = 2

    #self.equalizerStepSize        = 0.2
    self.equalizerStepSize        = 0.1
    #self.equalizerStepSize        = \
      #1.0 \
      #/ \
      #(
        #5.0
        #* ( 2.0 * ( self.numberOfFeedforwardTaps + self.numberOfFeedbackTaps ) + 1 )
        #* ( 10.0 ** ( self.SNR / 10.0 ) )
      #)
    self.equalizerIterations      = 200

    print "Step size: %.04f" %( self.equalizerStepSize )

    self.dataPacker = None
    self.dataStream = None

    self.populateCarrierStream()

    self.generateSpreadingSequence()

    self.generateSpreadingSignal()

    #self.generateSymbolSignals()

    self.generateDataSymbols()

  def readImpulseResponse( self ):
    file = open( "unprocessed_profile_1.dat", 'r' )

    self.channelImpulseResponse = \
      [ 0.0 for i in range( self.numberOfTrainingSymbols * self.symbolDuration ) ]

    lineCount = 0

    if( file ):
      for line in file:
        value = float( line )

        if( 1.0 == value ):
          self.delay =  \
            lineCount + ( self.numberOfTrainingSymbols * self.symbolDuration )

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

  def demodulateData( self, signal, phaseOffset ):
    self.assertEquals (
      bit_stream_reset( self.carrierInphaseStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_stream_reset( self.carrierQuadratureStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    inphaseOffset     = math.cos( phaseOffset )
    quadratureOffset  = math.sin( phaseOffset )

    amplitude = 0

    for sampleValue in signal:
      if( abs( sampleValue ) > amplitude ):
        amplitude = abs( sampleValue )

    signal = map( lambda x: x / amplitude, signal )

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
      sampleValue = \
        inphaseOffset * inphaseSamples[ i ] \
        + quadratureOffset * quadratureSamples[ i ]

      demodulatedSignal.append( sampleValue )

    self.assertNotEquals( None, demodulatedSignal )
    self.assertEquals( len( demodulatedSignal ), len( signal ) )

    return( demodulatedSignal )

  def modulateBFSKData( self, symbol ):
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

    result = \
      python_csignal_modulate_BFSK_symbol( symbol, self.symbolDuration, self.sampleRate, self.carrierFrequency )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( inphaseSignal, quadratureSignal ) = result

    self.assertNotEquals( None, inphaseSignal )
    self.assertNotEquals( None, quadratureSignal )

    self.assertEquals( len( inphaseSignal ), self.symbolDuration )
    self.assertEquals( len( quadratureSignal ), self.symbolDuration )

    inphase =  \
      python_csignal_multiply_signals( chipSamples, inphaseSignal )

    self.assertNotEquals( None, inphase )
    self.assertEquals( len( inphase ), self.symbolDuration )

    quadrature = \
      python_csignal_multiply_signals( chipSamples, quadratureSignal )

    self.assertNotEquals( None, quadrature )
    self.assertEquals( len( quadrature ), self.symbolDuration )

    for i in range( self.symbolDuration ):
      sampleValue = inphase[ i ] - quadrature[ i ]

      signal.append( sampleValue )

    self.assertNotEquals( None, signal )
    self.assertEquals( len( signal ), self.symbolDuration )

    return( signal )

  def modulateData( self, inphaseSymbol, quadratureSymbol ):
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

    #inphase = \
      #python_csignal_multiply_signals( inphaseSignal, chipSamples )

    #self.assertNotEquals( None, inphase )
    #self.assertEquals( len( inphase ), self.symbolDuration )

    #quadrature =  \
      #python_csignal_multiply_signals( quadratureSignal, quadratureSamples )

    #self.assertNotEquals( None, quadrature )
    #self.assertEquals( len( quadrature ), self.symbolDuration )

    for i in range( self.symbolDuration ):
      sampleValue =                                     \
        ( inphaseSymbol * inphaseSamples[ i ] )         \
        - ( quadratureSymbol * quadratureSamples[ i ] )
      #sampleValue = inphase[ i ] - quadrature[ i ]
      #sampleValue = inphaseSignal[ i ]
      #sampleValue = inphase[ i ]

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
      symbol = 0
      #symbol = i % self.constellationSize
      symbol = struct.unpack( 'B', chr( symbol ) )[ 0 ]

      bit_packer_add_bits( symbol, self.bitsPerSymbol, self.dataPacker )

    if( self.dataSequence ):
      for symbol in self.dataSequence:
        bit_packer_add_bits( symbol, self.bitsPerSymbol, self.dataPacker )
    else:
      for i in range( self.numberOfDataSymbols ):
        #symbol = i % self.constellationSize
        symbol = random.randint( 0, self.constellationSize )
        symbol = struct.unpack( 'B', chr( symbol ) )[ 0 ]
  
        bit_packer_add_bits( symbol, self.bitsPerSymbol, self.dataPacker )

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

    for i in range( self.numberOfTrainingSymbols ):
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, self.bitsPerSymbol ) 

      self.assertEquals( numberOfBits, self.bitsPerSymbol )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> ( 8 - self.bitsPerSymbol )

      self.assertNotEquals( symbol, None )

      ( inphaseSymbol, quadratureSymbol ) = \
        python_modulate_symbol( symbol, self.constellationSize )

      part = self.modulateData( inphaseSymbol, quadratureSymbol )

      self.assertNotEquals( part, None )

      signal = signal + part

    synchronizationBuffer = self.numberOfTrainingSymbols * self.symbolDuration

    signal = signal + [ 0.0 for i in range( synchronizationBuffer ) ]

    self.assertEquals (
      bit_stream_reset( self.spreadingSignalStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    for i in range( self.numberOfDataSymbols ):
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, self.bitsPerSymbol ) 

      self.assertEquals( numberOfBits, self.bitsPerSymbol )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> ( 8 - self.bitsPerSymbol )

      self.assertNotEquals( symbol, None )

      part = self.modulateBFSKData( symbol )

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

  def exhaustiveFindMax( self, signal, sample, startIndex, endIndex ):
    energy = []

    print "Performing exhaustive search between %d and %d." \
      %( startIndex, endIndex )

    for i in range( startIndex, endIndex ):
      signalEnergy =  \
        self.calculateEnergy  (
          signal[ i : i + len( sample ) ],
          sample
                              )

      energy.append( signalEnergy )

    maxIndex = 0

    print "Energy: %d" %( len( energy ) )

    for i in range( 1, len( energy ) ):
      if( energy[ i ] > energy[ maxIndex ] ):
        maxIndex = i

        print "Max energy: %.04f\tIndex: %d" %( energy[ maxIndex ], startIndex + maxIndex )

    return( startIndex + maxIndex )

  def findMax( self, signal, sample, startIndex, endIndex ):
    if( endIndex - startIndex < 200 ):
      return( self.exhaustiveFindMax( signal, sample, startIndex, endIndex ) )

    print "Start: %d\tEnd: %d" %( startIndex, endIndex )

    midIndex = int( ( 1.0 * startIndex + 1.0 * endIndex ) / 2.0 )

    hiIndex = int( ( 1.0 * midIndex + 1.0 * endIndex ) / 2.0 )
    loIndex = int( ( 1.0 * startIndex + 1.0 * midIndex ) / 2.0 )

    print "Low: %d\tMid: %d\tHigh: %d" %( loIndex, midIndex, hiIndex )

    hiEnergy =  \
      self.calculateEnergy  (
        signal[ hiIndex : hiIndex + len( sample ) ],
        sample
                            )
    loEnergy =  \
      self.calculateEnergy  (
        signal[ loIndex : loIndex + len( sample ) ],
        sample
                            )

    print "Low energy: %.04f\tHi energy: %.04f" %( loEnergy, hiEnergy )

    if( hiEnergy > loEnergy ):
      return( self.findMax( signal, sample, midIndex, endIndex ) )
    else:
      return( self.findMax( signal, sample, startIndex, midIndex ) )

  def findPilot( self, signal ):
    datFileName = "pilot_%s.dat" %( versionInfo )

    print "Saving test data to %s." %( datFileName )

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

    synchronizationPeriod = \
      int( 2.0 * ( self.numberOfTrainingSymbols * self.symbolDuration ) )

    nTests    = synchronizationPeriod - len( chipSamples )
    maxNTests = len( signal ) - len( chipSamples )

    if( nTests > maxNTests ):
      nTests = maxNTests

    print "Number of tests: %d" %( nTests )
    print "Length of signal: %d" %( len( signal ) )

    energy = []

    foundPilot  = False
    startIndex  = -1

    startTime = time.time()

    ranges = []

    startIndex = -1

    decimationFactor = 10

    #for i in range( 0, nTests, nTests / decimationFactor ):
      #signalEnergy =  \
        #self.calculateEnergy  (
          #signal[ i : i + len( chipSamples ) ],
          #chipSamples
                              #)

      #print signalEnergy

      #if( signalEnergy >= self.threshold ):
        #if( startIndex == -1 ):
          #startIndex = i
      #else:
        #if( startIndex != -1 ):
          #ranges.append( [ startIndex, i ] )

          #startIndex = -1


    #if( startIndex != -1 ):
      #ranges.append( [ startIndex, i ] )

    #print "Scanned %d items in %.04f seconds." %( decimationFactor, runTime )
    #print "Ranges: ", ranges

    startTime = time.time()

    offset = python_detect_find_highest_energy_offset  (
      signal,
      chipSamples,
      nTests,
      ( nTests / decimationFactor ),
      self.narrowbandFilter,
      self.lowpassFilter,
      0.1,
      self.chipDecimationFactor,
      self.threshold
                                                        )

    runTime = time.time() - startTime

    print "Peak: ", offset
    print "Peak detection took %.04f seconds." %( runTime )

    #for offsets in ranges:
      #maxOffset = self.findMax( signal, chipSamples, offsets[ 0 ], offsets[ 1 ] )

      #print "Max offset between %d and %d is %d." \
        #%( offsets[ 0 ], offsets[ 1 ], maxOffset )

    maxOffset = 0

    startTime = time.time()

    for i in range( 0, nTests, self.chipDecimationFactor ):
    #for i in range( 0, nTests ):
      signalEnergy =  \
        self.calculateEnergy  (
          signal[ i : i + len( chipSamples ) ],
          chipSamples
                              )
    
      energy.append( signalEnergy )

      if( signalEnergy > energy[ maxOffset ] ):
        maxOffset = i / self.chipDecimationFactor
        #maxOffset = i

    runTime = time.time() - startTime

    self.outputSequence( datFileName, energy )

    maxOffset = maxOffset * self.chipDecimationFactor

    print "Exhaustive peak: %d" %( maxOffset )
    print "Exhaustive peak detection took %.04f seconds." %( runTime )

    return( offset )

    #if( offset != maxOffset ):
      #print "ERROR: Mismatch offset detected!"

    #if( foundPilot ):
      #print "Start: %d" %( startIndex )

      #sortedEnergy = sorted( energy[ startIndex : ], None, None, True )

      #searchIndices = []

      #for i in range( startIndex, len( energy ) ):
        #if( energy[ i ] >= sortedEnergy[ self.numberOfStartOffsets - 1 ] ):
          #searchIndices.append( i * self.chipDecimationFactor )

      #return( searchIndices )
    #else:
      #return( None )

  def calculateEnergy( self, signal, chipSignal ):
    signalEnergy =  \
      python_detect_calculate_energy  (
        signal,
        chipSignal,
        self.narrowbandFilter,
        self.lowpassFilter
                                      )

    #despread  = \
      #python_csignal_multiply_signals (
        #chipSignal,
        #signal
                                      #)

    #filtered  = python_filter_signal( self.narrowbandFilter, despread )
    #squared   = python_csignal_multiply_signals( filtered, filtered )
    #filtered  = python_filter_signal( self.lowpassFilter, squared )
    #signalE   = python_csignal_sum_signal( filtered, 1.0 )

    return( signalEnergy )

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

    print "Looking for start of signal..."

    startIndex = None

    for i in range( 0, nTests, self.chipDecimationFactor ):
      signalEnergy =  \
        self.calculateEnergy  (
          signal[ i : i + len( chipSamples ) ],
          chipSamples
                              )

      if( signalEnergy > self.threshold ):
        startIndex = i 

        break

    print "Found threshold exceeded at index: %d" %( startIndex )

    energy = []

    endIndex = int( startIndex + 1.5 * len( chipSamples ) )

    if( endIndex > len( signal ) - len( chipSamples ) ):
      endIndex = len( signal ) - len( chipSamples )

    print "Searching for top %d peaks between %d and %d"  \
      %( self.numberOfStartOffsets, startIndex, endIndex )

    for i in range( startIndex, endIndex, self.chipDecimationFactor ):
      signalEnergy =  \
        self.calculateEnergy  (
          signal[ i : i + len( chipSamples ) ],
          chipSamples
                              )

      energy.append( signalEnergy )

    sortedEnergy = sorted( energy, None, None, True )

    searchIndices = []

    for i in range( len( energy ) ):
      if( energy[ i ] >= sortedEnergy[ self.numberOfStartOffsets - 1 ] ):
        searchIndices.append( startIndex + i * self.chipDecimationFactor )

    energyTime = time.time() - startTime

    print "Energy detection time:\t%.04f" %( energyTime )

    self.outputSequence( "energy.dat", energy )

    return( searchIndices )

  def receiveSignal( self, signal ):
    #attenuationFactor = random.uniform( 0, 1 )

    #print "Attenuation factor: %.04f" %( attenuationFactor )

    #signal = map( lambda x: x * attenuationFactor, signal )

    startTime = time.time()

    bandpassFilteredSignal =  \
      python_filter_signal( self.widebandFilter, signal )

    self.assertNotEquals( None, bandpassFilteredSignal )
    self.assertTrue( len( bandpassFilteredSignal ) > 0 )

    self.outputSignal( "widebandFiltered.WAV", bandpassFilteredSignal )

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

  def findOffset( self, inphaseSymbol, quadratureSymbol, signal ):
    sum     = python_csignal_sum_signal( signal, 1 )
    symbol  = sum / self.symbolDuration

    print "Symbol: %.04f" %( symbol )
    print "I-Symbol: %.04f\tQ-Symbol: %.04f" %( inphaseSymbol, quadratureSymbol )

    previousDiff = 0
    offsetPhase  = 0

    nTests = 1000

    for i in range( nTests + 1 ):
      angle = i * ( 180.0 / nTests );
      phase = ( 2.0 * math.pi * angle ) / 360.0

      estimate =  \
        0.5 *   \
        ( ( inphaseSymbol * math.cos( phase ) ) \
        - ( quadratureSymbol * math.sin( phase ) ) )


      diff = symbol - estimate

      if( diff * previousDiff < 0 ):
        offsetPhase = phase

        break

      previousDiff = diff

      #diffs.append( symbol - ( 0.5 * estimate ) )
      #diffs.append( symbol - estimate )

      #print "Diff: %.04f\tSymbol: %.04f\tPhase: %.04f\tPhase offset: %.04f\tI-Symbol: %.04f\tI-Phase: %.04f\tQ-Symbol: %.04f\tQ-Phase: %.04f"  \
        #%( ( symbol - estimate ), symbol, phase, estimate, inphaseSymbol, math.cos( phase ), quadratureSymbol, math.sin( phase ) )

    #crossingIndices = self.findZeroCrossings( diffs )

    return( offsetPhase )
    
  def determinePhaseOffset( self, signal ):
    self.assertEquals (
      bit_stream_reset( self.dataStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    symbolCount = 0
    offsetSum   = 0

    for i in range( 0, len( signal ), self.symbolDuration ):
      if( len( signal ) - i < self.symbolDuration ):
        break
      if( symbolCount >= self.numberOfTrainingSymbols ):
        break

      symbolCount += 1 

      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, self.bitsPerSymbol ) 

      self.assertEquals( numberOfBits, self.bitsPerSymbol )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> ( 8 - self.bitsPerSymbol )

      self.assertNotEquals( symbol, None )

      ( inphaseSymbol, quadratureSymbol ) = \
        python_modulate_symbol( symbol, self.constellationSize )

      symbolSignal = signal[ i : i + self.symbolDuration  ]

      offset = self.findOffset( inphaseSymbol, quadratureSymbol, symbolSignal )

      print "Offset: %.04f" %( offset )

      offsetSum += offset

    print "Estimated offset: %.04f" %( offsetSum / self.numberOfTrainingSymbols )

    return( offsetSum / self.numberOfTrainingSymbols )

  def findZeroCrossings( self, values ):
    previousValue   = None
    crossingPoints  = []

    for i in range( len( values ) ):
      if( None != previousValue and previousValue * values[ i ] <= 0 ):
        crossingPoints.append( ( i + ( i - 1 ) ) / 2 )

      previousValue = values[ i ]

    return( crossingPoints )

  def scoreMatch( self, values, symbols ):
    self.assertEquals (
      bit_stream_reset( self.dataStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    symbolCount = 0
    offsetSum   = 0

    expectedValues  = []
    expectedSymbols = []

    for i in range( self.numberOfTrainingSymbols ): 
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, self.bitsPerSymbol ) 

      self.assertEquals( numberOfBits, self.bitsPerSymbol )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> ( 8 - self.bitsPerSymbol )

      self.assertNotEquals( symbol, None )

      ( inphaseSymbol, quadratureSymbol ) = \
        python_modulate_symbol( symbol, self.constellationSize )

      expectedValues.append( 0.5 * ( inphaseSymbol + quadratureSymbol ) )
      expectedSymbols.append( symbol )

    valueDistance = map( lambda x, y: ( x - y ) ** 2, values[ 0 : self.numberOfTrainingSymbols ], expectedValues )
    symbolDistance = map( lambda x, y : ( x - y ) ** 2, symbols[ 0 : self.numberOfTrainingSymbols ], expectedSymbols )

    valueScore = math.sqrt( sum( valueDistance ) )
    symbolScore = math.sqrt( sum( symbolDistance ) )

    return( valueScore, symbolScore )

  def getReceivedSignal( self ):
    #wavPath = "/Users/user/Downloads/data.WAV"
    #wavPath = "/Users/user/Downloads/pilot.WAV"
    #wavPath = "/Users/user/Downloads/noise.WAV"

    signal = []

    try:
      wav = wave.open( dataFile, 'r' )

      if( wav ):
        print "Received file: %s" %( dataFile )
        print "Channels: %d" %( wav.getnchannels() )
        print "Sample width: %d" %( wav.getsampwidth() )
        print "Sample rate: %d" %( wav.getframerate() )
        print "Number of samples: %d" %( wav.getnframes() )

        if( wav.getsampwidth() == 1 ):
          signal = \
            list  (
              struct.unpack (
                "b" * wav.getnframes(), wav.readframes( wav.getnframes() )
                            )
                  )
        elif( wav.getsampwidth() == 2 ):
          signal = \
            list  (
              struct.unpack (
                "h" * wav.getnframes(), wav.readframes( wav.getnframes() )
                            )
                  )
        elif( wav.getsampwidth() == 4 ):
          signal = \
            list  (
              struct.unpack (
                "i" * wav.getnframes(), wav.readframes( wav.getnframes() )
                            )
                  )
        else:
          print "ERROR: Could not handle sample width %d."  \
            %( wav.getsampwidth() )

          print "Value: %d" %( sampleValue )

        average = sum( signal ) / ( 1.0 * len( signal ) )

        print "Max: %d\tMin: %d\tAverage: %.04f" %( max( signal ), min( signal ), average )

        signal = map( lambda x: ( x * 1.0 ) - average, signal )

        maxValue  = max( [ max( signal ), abs( min( signal ) ) ] )
        signal    = map( lambda x: ( x * 1.0 ) / ( maxValue * 1.0 ), signal )

        average = sum( signal ) / ( 1.0 * len( signal ) )

        print "Max: %+.04f\tMin: %+.04f\tAverage: %+.04f" %( max( signal ), min( signal ), average )
        
        wav.close()
    except wave.Error as e:
      print "ERROR: Could not open '%s': %s" %( dataFile, str( e ) )

    return( signal )

  def findOptimalStartOffset( self, startIndices, signal ):
    narrowbandOffset = python_filter_get_group_delay( self.narrowbandFilter )

    print "Delay from narrowband filter: %d samples." %( narrowbandOffset )

    lowpassOffset = python_filter_get_group_delay( self.lowpassFilter )

    print "Delay from lowpass filter: %d samples." %( lowpassOffset )

    offsetScores  = []
    offsetPhase   = []

    for startOffset in startIndices:
      despreadSignal = self.despreadSignal( signal[ startOffset : ] )

      self.outputSignal (
        "despreadSignal_%d.WAV" %( startOffset ),
        despreadSignal[ narrowbandOffset : ]
                        )

      bestValues    = None
      bestDecisions = None
      bestScore     = sys.maxint
      delayGuess    = -1

      for i in range( self.chipDecimationFactor ):
        phaseOffset =                           \
          2.0 * math.pi * self.carrierFrequency \
          * ( ( 1.0 * i ) / self.sampleRate )

        demodulatedSignal = \
          self.demodulateData (
            despreadSignal[ narrowbandOffset : ],
            phaseOffset
                              )
  
        lowpassSignal =  \
          python_filter_signal( self.lowpassFilter, demodulatedSignal )

        self.outputSignal (
          "lowpassFiltered_phase_%d_%d_%.04f.WAV" \
            %( startOffset, i, phaseOffset ),
          lowpassSignal[ lowpassOffset : ]
                          )

        ( values, decisions ) = \
          self.determineSymbols( lowpassSignal[ lowpassOffset : ] )

        #print "Offset: %d\tDelay: %d" %( startOffset, i )
        #print "Values: ", values
        #print "Decisions: ", decisions

        ( valueScore, decisionScore ) = self.scoreMatch( values, decisions )

        #print "Value score: %.04f" %( valueScore )
        #print "Decision score: %.04f" %( decisionScore )

        score = \
          map (
            lambda x, y: ( x - y ) ** 2,
            [ 0, 0 ],
            [ valueScore, decisionScore ]
              )

        self.assertEquals( len( score ), 2 )

        score = \
          math.sqrt( score[ 0 ] + score[ 1 ] * self.numberOfTrainingSymbols )

        if( score < bestScore ):
          bestValues    = values
          bestDecisions = decisions
          bestScore     = score
          delayGuess    = i

      phaseOffset =                             \
          2.0 * math.pi * self.carrierFrequency \
          * ( ( 1.0 * delayGuess ) / self.sampleRate )

      print "Offset: %d\tDelay: %d\tScore: %.04f\tPhase: %.04f\t" \
        %( startOffset, delayGuess, bestScore, phaseOffset ),
      print "Decisions: ", bestDecisions, "\tValues: ", bestValues

      offsetScores.append( bestScore )
      offsetPhase.append( phaseOffset )

    bestScore = sys.maxint
    bestIndex = -1

    for i in range( len( offsetScores ) ):
      if( offsetScores[ i ] < bestScore ):
        bestScore = offsetScores[ i ]
        bestIndex = i
     
    print "Best offset: %d\tScore: %.04f" %( startIndices[ bestIndex ], bestScore ) 

    return( startIndices[ bestIndex ], offsetPhase[ bestIndex ] )

  def test_equalizer( self ):
    transmittedSignal = self.generateTransmitSignal()

    self.outputSignal( "transmitted.WAV", transmittedSignal )

    #receivedSignal = self.perturbSignal( transmittedSignal, self.SNR )

    receivedSignal = self.getReceivedSignal()

    self.outputSignal( "received.WAV", receivedSignal )

    #( filteredSignal, bandpassSignal ) = self.receiveSignal( receivedSignal )
    filteredSignal = self.receiveSignal( receivedSignal )

    widebandDelay = python_filter_get_group_delay( self.widebandFilter )

    print "Delay from wideband filter: %d samples." %( widebandDelay )

    pilotOffset = self.findPilot( filteredSignal[ widebandDelay : ] )

    if( pilotOffset ):
      print "Start offset: %d" %( pilotOffset )

      synchronizationPeriod = \
        int( 2.0 * ( self.numberOfTrainingSymbols * self.symbolDuration ) )

      dataStartOffset = widebandDelay + pilotOffset + synchronizationPeriod

      print "Data start offset is %d." %( dataStartOffset )
  
      self.outputSignal( "filteredOffsetSignal_%d.WAV" %( dataStartOffset ), filteredSignal[ dataStartOffset : ] )

      despreadSignal = self.despreadSignal( filteredSignal[ dataStartOffset : ] )
  
      despreadOffset = python_filter_get_group_delay( self.narrowbandFilter )

      print "Delay from narrowband filter: %d samples." %( despreadOffset )

      self.outputSignal( "despreadSignal_%d.WAV" %( dataStartOffset ), despreadSignal[ despreadOffset : ] )

    self.determineFrequencies( despreadSignal[ despreadOffset : ] )

    sys.exit()

    print "Sample delay is %d." %( self.delay )

    startOffsets = self.findStartOffset( filteredSignal )

    if( None != startOffsets ):
      print "Delay: %d\tStart offsets: " %( self.delay ), startOffsets

      ( startOffset, phaseOffset ) =  \
        self.findOptimalStartOffset( startOffsets, filteredSignal )

      print "Start offset: %d\tPhase offset: %.04f" \
        %( startOffset, phaseOffset )

      despreadSignal = self.despreadSignal( filteredSignal[ startOffset : ] )

      despreadOffset = python_filter_get_group_delay( self.narrowbandFilter )

      print "Delay from narrowband filter: %d samples." %( despreadOffset )

      self.outputSignal( "despreadSignal.WAV", despreadSignal[ despreadOffset : ] )

      demodulatedSignal = self.demodulateData( despreadSignal[ despreadOffset : ], phaseOffset )

      filteredSignal = \
        python_filter_signal( self.lowpassFilter, demodulatedSignal )

      filterOffset = python_filter_get_group_delay( self.lowpassFilter )

      self.outputSignal( "correctedLowpassFiltered.WAV", filteredSignal[ filterOffset : ] )

      downsampledSignal = self.downsampleSignal( filteredSignal[ filterOffset : ] )

      self.outputSignal( "downsampledSignal.WAV", downsampledSignal, self.sampleRate / self.symbolDecimationFactor )

      ( values, decisions ) = self.determineSymbols( filteredSignal[ filterOffset : ] )

      print "Final values:"
      print "[ ",
      for value in values:
       print "%f " %( value ),
      print "]"
        
      print "Final decisions: ", decisions
      print "Start offset:\t\t%d (Delay: %d)" %( startOffset, self.delay )

      initialSymbols = self.getInitialSymbols()

      expected = []

      print "Symbols:\t",
      for i in range( min( len( initialSymbols ), self.numberOfTrainingSymbols + self.numberOfDataSymbols ) ):
        print "%+d " %( initialSymbols[ i ] ),

        symbol = initialSymbols[ i ] if( initialSymbols[ i ] == 1 ) else 0

        expected.append( symbol )
      print

      print "Expected values:"
      print "[ ",
      for value in expected:
        print "%d " %( value ),
      print "]"

      decisions = map( lambda x: x if( x ) else -1, decisions )

      print "Initial:\t",
      for i in range( min( len( decisions ), self.numberOfTrainingSymbols + self.numberOfDataSymbols ) ):
        print "%+d " %( decisions[ i ] ),
      print

      benchmarkNoEq = self.benchmarkDemodulation( initialSymbols[ self.numberOfTrainingSymbols : ], decisions[ self.numberOfTrainingSymbols : ] )

      equalizedDecisions = self.getEqualizedSymbols( downsampledSignal )

      print "Number of downsampled: %d" %( len( downsampledSignal ) )

      print "Equalized:\t",
      for i in range( min( len( equalizedDecisions ), self.numberOfTrainingSymbols + self.numberOfDataSymbols ) ):
        print "%+d " %( equalizedDecisions[ i ] ),
      print

      print "No equalization P_e:\t%.02f" %( 1.0 - benchmarkNoEq )

      benchmarkEq = \
        self.benchmarkDemodulation( initialSymbols[ self.numberOfTrainingSymbols : ], equalizedDecisions[ self.numberOfTrainingSymbols : ] )

      print "Equalization P_e:\t%.02f" %( 1.0 - benchmarkEq )

  def downsampleSignal( self, signal ):
    downsampledSignal = []

    for i in range( 0, len( signal ), self.symbolDecimationFactor ):
      if( len( signal ) - i < self.symbolDuration ):
        break

      average = python_csignal_sum_signal( signal[ i : i + self.symbolDecimationFactor ], 1 ) / self.symbolDecimationFactor

      downsampledSignal.append( average )

    return( downsampledSignal )

  def determineSymbols( self, signal ):
    rawValues           = []
    demodulatedSymbols  = []

    for i in range( 0, len( signal ), self.symbolDuration ):
      if( len( signal ) - i < self.symbolDuration ):
        break

      sum     = \
        python_csignal_sum_signal( signal[ i : i + self.symbolDuration ], 1 )
      symbol  = sum / self.symbolDuration

      rawValues.append( symbol )

      distance          = sys.maxint
      demodulatedSymbol = -1

      for i in range( self.constellationSize ):
        ( inphaseValue, quadratureValue ) = \
          python_modulate_symbol( i, self.constellationSize )

        if( abs( inphaseValue - symbol ) < distance ):
          demodulatedSymbol = i
          distance          = abs( inphaseValue - symbol )
         
      demodulatedSymbols.append( demodulatedSymbol )

    return( rawValues, demodulatedSymbols )

  def determineFrequencies( self, signal ):
    #decisions = []

    #symbol0 = self.symbolSignals[ 0 ][ 0 ]
    #symbol1 = self.symbolSignals[ 1 ][ 0 ]
    #symbol0 = map( lambda x, y: x - y, self.symbolSignals[ 0 ][ 0 ], self.symbolSignals[ 0 ][ 1 ] )
    #symbol1 = map( lambda x, y: x - y, self.symbolSignals[ 1 ][ 0 ], self.symbolSignals[ 1 ][ 1 ] )

    #e_s0 = python_csignal_calculate_energy( symbol0 )
    #e_s1 = python_csignal_calculate_energy( symbol1 )

    #symbol0 = map( lambda x: x / e_s0, symbol0 )
    #symbol1 = map( lambda x: x / e_s1, symbol1 )

    #for i in range( 0, len( signal ), self.symbolDuration ):
      #if( len( signal ) - i < self.symbolDuration ):
        #break

      #e_s = python_csignal_calculate_energy( signal[ i : i + self.symbolDuration ] )

      #r = map( lambda x: x / e_s, signal[ i : i + self.symbolDuration ] )

      #c_s0 = python_csignal_multiply_signals( symbol0, r )
      #c_s1 = python_csignal_multiply_signals( symbol1, r )

      #c_s0 = python_csignal_sum_signal( c_s0, 1 )
      #c_s1 = python_csignal_sum_signal( c_s1, 1 )

      #print "Correlation %02d: 0:%+01.04f\t1:%+01.04f\tDecision:%d" %( ( i / self.symbolDuration ), c_s0, c_s1, ( 0 if( c_s0 > c_s1 ) else 1 ) )

    count = 0

    deltaFrequency  = ( 1.0 * self.sampleRate ) / ( 1.0 * self.symbolDuration )
    lowFrequency    = ( 1.0 * self.carrierFrequency ) - ( deltaFrequency / 2.0 )
    hiFrequency     = ( 1.0 * self.carrierFrequency ) + ( deltaFrequency / 2.0 )

    print "Delta: %.04f" %( deltaFrequency )

    for i in range( 0, len( signal ), self.symbolDuration ):
      if( len( signal ) - i < self.symbolDuration ):
        break

      count = count + 1

      testSignal    = signal[ i : i + self.symbolDuration ] * 10
      fft           = python_calculate_FFT( testSignal )
      fftMagnitudes = map( lambda x: abs( x ), fft )

      N     = 1.0 * len( fftMagnitudes )
      delta = 1.0 / ( 1.0 * self.sampleRate )

      lowIndex  = int( math.floor( lowFrequency * delta * N ) )
      hiIndex   = int( math.ceil( hiFrequency * delta * N ) )

      maxIndex = 0

      for j in range( 1, int( N / 2 ) ):
        frequency = ( 1.0 * j ) / ( delta * N )

        if( int( frequency ) != self.carrierFrequency ):
          if( fftMagnitudes[ j ] > fftMagnitudes[ maxIndex ] ):
            maxIndex = j

      lowDistance = abs( maxIndex - lowIndex )
      hiDistance  = abs( maxIndex - hiIndex )

      symbol = 1 if( hiDistance <= lowDistance ) else 0

      print "Max index: %d\tFrequency: %.04f\tMagnitude: %.04f\tSymbol: %d" \
        %( maxIndex, ( maxIndex / ( delta * N ) ), fftMagnitudes[ maxIndex ], symbol )
      
      #fft_mag = map( lambda x: 10**-12 if x == 0 else x, fft_mag )
      #fft_mag = map( lambda x: 10 * math.log10( x / max_value ), fft_mag )

      frequencies = []
      magnitudes  = []

      #one  = fftMagnitudes[ int( 21050.0 * delta * N ) ]
      #zero   = fftMagnitudes[ int( 20950.0 * delta * N ) ]

      #print "Index magnitudes %02d: 0=%03.04f\t1=%03.04f\tDecision:%d" %( ( i / self.symbolDuration ), zero, one, 0 if( zero > one ) else 1  )

      #decisions.append( -1 if( zero > one ) else 1 )

      #frequency = maxIndex / ( delta * N )

      #print "Max frequency is %.02f" %( frequency )

      for n in range( int( N / 2 ) ):
        frequency = ( 1.0 * n ) / ( delta * N )

        frequencies.append( frequency )
        #magnitudes.append( fft_mag[ n ] )
        magnitudes.append( fftMagnitudes[ n ] )

      self.outputDependantSequence( "symbol_%d.dat" %( int( i / self.symbolDuration ) ), frequencies, magnitudes )

    #return( decisions )

  def getTrainingSequence( self ):
    self.assertEquals (
      bit_stream_reset( self.dataStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    symbols = []

    for i in range( self.numberOfTrainingSymbols ):
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, self.bitsPerSymbol ) 

      self.assertEquals( numberOfBits, 1 )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> ( 8 - self.bitsPerSymbol )

      self.assertNotEquals( symbol, None )

      ( inphaseSymbol, quadratureSymbol ) = \
        python_modulate_symbol( symbol, self.constellationSize )

      symbols.append( 0.5 * complex( inphaseSymbol, quadratureSymbol ) )

    return( symbols )

  def initializeEqualizer( self ):
    feedforwardWeights  = \
      [ complex( 1.0, 0.0 ) for i in range( self.numberOfFeedforwardTaps * self.samplesPerSymbol ) ]
    feedbackWeights     = \
      [ complex( 1.0, 0.0 ) for i in range(  self.numberOfFeedbackTaps ) ]

    feedforwardBufferPacker = python_bit_packer_initialize()

    self.assertNotEquals( None, feedforwardBufferPacker )

    feedforwardBufferStream = \
      python_bit_stream_initialize_from_bit_packer  (
        False,
       feedforwardBufferPacker 
                                                    )

    self.assertNotEquals( None, feedforwardBufferStream )

    for i in range( self.numberOfFeedforwardTaps * self.samplesPerSymbol ):
      self.assertEquals (
        python_bit_packer_add_bytes (
          struct.pack( "d", 0.0 ),
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
          struct.pack( "d", 0.0 ),
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
        struct.pack( "d", value ),
        packer
                                  ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    result = python_bit_stream_get_bits( stream, 64 )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 2 )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( None, numberOfBits )
    self.assertNotEquals( None, buffer )

    self.assertEquals( numberOfBits, 64 )
    self.assertEquals( len( buffer ), 8 )

    value = struct.unpack( "d", buffer )[ 0 ]

    return( value )

  def equalizerGetTapValues( self, stream, numberOfTaps ):
    result = python_bit_stream_peak( stream )

    self.assertNotEquals( None, result )
    self.assertEquals( len( result ), 3 )

    ( readPointer, writePointer, buffer ) = result

    self.assertNotEquals( None, readPointer )
    self.assertNotEquals( None, writePointer )
    self.assertNotEquals( None, buffer )

    self.assertEquals( len( buffer ), 8 * numberOfTaps )

    values = list( struct.unpack( "d" * numberOfTaps, buffer ) )

    self.assertNotEquals( None, values )
    self.assertEquals( len( values ), numberOfTaps )

    return( values )

  def equalizerDetermineSymbol  (
    self, feedforwardWeights, feedforwardValues, feedbackWeights,
    feedbackValues, expectedSymbol
                                ):
    feedforwardValue  = complex( 0.0, 0.0 )
    feedbackValue     = complex( 0.0, 0.0 )

    self.assertEquals (
      len( feedforwardWeights ),
      self.numberOfFeedforwardTaps * self.samplesPerSymbol
                      )
    self.assertEquals( len( feedforwardWeights ), len( feedforwardValues ) )

    self.assertEquals (
      len( feedbackWeights ),
      self.numberOfFeedbackTaps
                      )
    self.assertEquals( len( feedbackWeights ), len( feedbackValues ) )

    for i in range( self.numberOfFeedforwardTaps * self.samplesPerSymbol ):
      feedforwardValue =  \
        feedforwardValue  \
        + ( feedforwardWeights[ i ] * feedforwardValues[ i ] )

    for i in range( self.numberOfFeedbackTaps ):
      feedbackValue = \
        feedbackValue \
        + ( feedbackWeights[ i ] * feedbackValues[ i ] )

    valueEstimate = feedforwardValue + feedbackValue

    #print "FeedforwardValue: (%+.04e, %+.04ej)" %( feedforwardValue.real, feedforwardValue.imag )
    #print "FeedbackValue: %+.04e, %+.04ej)" %( feedbackValue.real, feedbackValue.imag )
    #print "Estimate: (%+.04e, %+.04ej)" %( valueEstimate.real, valueEstimate.imag )

    bestSymbol    = None
    minDistance   = sys.maxint

    for i in range( self.constellationSize ):
      ( inphaseSymbol, quadratureSymbol ) = \
        python_modulate_symbol( i, self.constellationSize )

      symbolValue = 0.5 * complex( inphaseSymbol, quadratureSymbol )

      difference = symbolValue - valueEstimate

      #print "Difference: (%+.04e, %+.04ej)" %( difference.real, difference.imag )

      distance = math.sqrt( difference.real ** 2 + difference.imag ** 2 )

      if( distance < minDistance ):
        minDistance = distance
        bestSymbol = symbolValue

    decision  = bestSymbol
    error     = None

    if( None == expectedSymbol ):
      error = bestSymbol - valueEstimate
      #print "Best symbol: (%+.04e, %+.04ej)" %( bestSymbol.real, bestSymbol.imag )
    else:
      error = expectedSymbol - valueEstimate
      #print "Expected symbol: (%+.04e, %+.04ej)" %( expectedSymbol.real, expectedSymbol.imag )

    return( decision, error )

  def getEqualizedSymbols( self, initialValues ):
    trainingSymbol    = None
    trainingSequence  = self.getTrainingSequence()

    print "Training sequence: "
    print "[ ",
    for value in trainingSequence:
      print "%+f%+fj " %( value.real, value.imag ),
    print "]"

    (
      feedforwardBufferPacker, feedforwardBufferStream,
      feedbackBufferPacker, feedbackBufferStream,
      feedforwardWeights, feedbackWeights
    ) = self.initializeEqualizer()

    codeSequence = []

    #print "Initial value length: %d" %( len( initialValues ) )

    for i in range( 0, len( initialValues ), self.samplesPerSymbol ):
      if( len( initialValues ) - i < self.samplesPerSymbol ):
        break

      print "Symbol: %d" %( i / self.samplesPerSymbol + 1 )

      for j in range( self.samplesPerSymbol ):
        self.equalizerAddAndIncrement (
          feedforwardBufferPacker,
          feedforwardBufferStream,
          initialValues[ i + j ]
                                      ) 

      feedforwardValues =  \
        self.equalizerGetTapValues  (
          feedforwardBufferStream,
          self.numberOfFeedforwardTaps
           * self.samplesPerSymbol
                                    )

      feedbackValues =  \
        self.equalizerGetTapValues  (
          feedbackBufferStream,
          self.numberOfFeedbackTaps
                                    )

      trainingIndex = int( i / self.samplesPerSymbol )

      if( trainingIndex < len( trainingSequence ) ):
        trainingSymbol = trainingSequence[ trainingIndex ]
      else:
        trainingSymbol = None

      print "Before:"
      print "Feedforward weights:\t",
      for value in feedforwardWeights:
        print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
      print
      print "Feedback weights:\t",
      for value in feedbackWeights:
        print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
      print

      for j in range( self.equalizerIterations ):
        ( symbol, error ) = \
          self.equalizerDetermineSymbol ( 
            feedforwardWeights, feedforwardValues,
            feedbackWeights, feedbackValues,
            trainingSymbol
                                        )

        #print "Before:"
        #print "Feedforward weights:\t",
        #for value in feedforwardWeights:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedforward values:\t",
        #for value in feedforwardValues:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedback weights:\t",
        #for value in feedbackWeights:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedback values:\t",
        #for value in feedbackValues:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print

        ( feedforwardWeights, feedbackWeights ) = \
          self.equalizerUpdateWeights (
            error, feedforwardWeights, feedforwardValues,
            feedbackWeights, feedbackValues
                                      )

        #print "After (Error=(%+.04e, %+.04ej)\t|Error|=%+.04e" %( error.real, error.imag, abs( error ) )
        #print "Feedforward weights:\t",
        #for value in feedforwardWeights:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedforward values:\t",
        #for value in feedforwardValues:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedback weights:\t",
        #for value in feedbackWeights:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print
        #print "Feedback values:\t",
        #for value in feedbackValues:
          #print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
        #print

      print "After:"
      print "Feedforward weights:\t",
      for value in feedforwardWeights:
        print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
      print
      print "Feedback weights:\t",
      for value in feedbackWeights:
        print "(%+.04e, %+.04ej) " %( value.real, value.imag ),
      print

      print "Symbol: (%+.04f,%+.04f)" %( symbol.real, symbol.imag )

      detectedSymbol = None

      for j in range( self.constellationSize ):
        ( inphaseValue, quadratureValue ) = \
          python_modulate_symbol( j, self.constellationSize )

        if( 0.5 * complex( inphaseValue, quadratureValue ) == symbol ):
          detectedSymbol = j

          break

      print "Detected symbol: %d" %( detectedSymbol )
      print "Error: (%+.04e, %+.04ej)\t|Error|=%+.04e" %( error.real, error.imag, abs( error ) )

      codeSequence.append( 1 if( detectedSymbol ) else -1 )

      self.equalizerAddAndIncrement (
        feedbackBufferPacker,
        feedbackBufferStream,
        symbol.real
                                    )

    self.destroyEqualizer ( 
      feedforwardBufferPacker, feedforwardBufferStream,
      feedbackBufferPacker, feedbackBufferStream,
                          )

    return( codeSequence )

  def equalizerUpdateWeights  (
    self, error, feedforwardWeights, feedforwardValues, feedbackWeights,
    feedbackValues
                              ):

    feedforwardMagnitude = \
      python_csignal_multiply_signals( feedforwardValues, feedforwardValues )
    feedforwardMagnitude = 1.0 * python_csignal_sum_signal( feedforwardMagnitude, 1.0 )

    feedbackMagnitude = \
      python_csignal_multiply_signals( feedbackValues, feedbackValues )
    feedbackMagnitude = 1.0 * python_csignal_sum_signal( feedbackMagnitude, 1.0 )

    if( 0 == feedbackMagnitude ):
      feedbackMagnitude = 1.0

    if( 0 == feedforwardMagnitude ):
      feedforwardMagnitude = 1.0

    feedforwardWeights  = \
      map (
        lambda x, y:  \
          x + error * self.equalizerStepSize * y / feedforwardMagnitude,
        feedforwardWeights,
        feedforwardValues
          )

    feedbackWeights = \
      map (
        lambda x, y:  \
          x + error * self.equalizerStepSize * y / feedbackMagnitude,
        feedbackWeights,
        feedbackValues
          )

    return( feedforwardWeights, feedbackWeights )

  def getInitialSymbols( self ):
    self.assertEquals (
      bit_stream_reset( self.dataStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    symbols = []

    for i in range( self.numberOfTrainingSymbols + self.numberOfDataSymbols ):
      ( numberOfBits, buffer ) = \
        python_bit_stream_get_bits( self.dataStream, 1 ) 

      self.assertEquals( numberOfBits, 1 )
      self.assertNotEquals( buffer, None )

      symbol = struct.unpack( "B", buffer )[ 0 ] >> 7 

      self.assertNotEquals( symbol, None )

      symbols.append( 1 if( symbol ) else -1 )

    return( symbols )

  def benchmarkDemodulation( self, initialSymbols, demodulatedSymbols ):
    nTests = min( len( initialSymbols ), len( demodulatedSymbols ) )

    hitCount = 0.0

    for i in range( nTests ):
      if( initialSymbols[ i ] == demodulatedSymbols[ i ] ):
        hitCount += 1.0

    hitRate = hitCount / ( len( initialSymbols ) * 1.0 )

    return( hitRate )

  def outputSignal( self, fileName, signal, sampleRate=48000 ):
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
      sampleRate,
      len( wavSignal ),
      samples
                                    )

    self.assertEquals( error, CPC_TRUE )

  def outputDependantSequence( self, fileName, variable, sequence ):
    file = open( fileName, 'w' )

    if( file ):
      for i in range( len( sequence ) ):
        file.write( "%f\t%f\n" %( variable[ i ], sequence[ i ] ) )

      file.close() 

  def outputSequence( self, fileName, sequence ):
    file = open( fileName, 'w' )

    if( file ):
      for i in range( len( sequence ) ):
        file.write( "%f\t%f\n" %( i, sequence[ i ] ) )

      file.close() 

if __name__ == '__main__':
  print "Num arguments: %d" %( len( sys.argv ) )

  if( len( sys.argv ) != 2 ):
    print "ERROR: Script requires an input WAV file."

    sys.exit()
  else:
    dataFile = sys.argv[ 1 ]

    match = re.search( ".*\/data_(.*?)\.wav$", dataFile )

    if( not match ):
      print "ERROR: Could not extract info from file name."
    else:
      versionInfo = match.group( 1 )

    sys.argv.remove( dataFile )

  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()
