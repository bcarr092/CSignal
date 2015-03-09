import csignal_tests
import unittest
import string
import random
import types
import os
import tempfile
import re
import math

def touch_random_file():
  file_handle = tempfile.NamedTemporaryFile()

  file_name = file_handle.name

  return( file_handle, file_name )

class TestsCSignal( unittest.TestCase ):
  def test_fft( self ):
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32767
    carrier_frequency   = 21000
    symbol_duration     = 24000
    chip_duration       = 24 

    first_stopband  = 19000
    first_passband  = 20000
    second_passband = 22000
    second_stopband = 23000

    passband_attenuation = 0.1
    stopband_attenuation = 80

    gold_code = csignal_tests.python_initialize_gold_code( 7, 0x12000000, 0x1E000000, 0x12345678, 0x12345678 )
    filter    = csignal_tests.python_initialize_kaiser_filter( first_stopband, first_passband, second_passband, second_stopband, passband_attenuation, stopband_attenuation, sample_rate )

    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 10 ) )
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    self.assertNotEquals( symbol_tracker, None )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    self.assertNotEquals( symbol, None )

    signal = []

    while( symbol != None ):
      part = csignal_tests.python_modulate_symbol (
          symbol,
          constellation_size,
          sample_rate,
          symbol_duration,
          baseband_amplitude,
          carrier_frequency
                                                            )

      self.assertNotEquals( part, None )
      self.assertEquals( len( part ), symbol_duration )

      part = csignal_tests.python_spread_signal (
          gold_code,
          chip_duration,
          part
                                                )
    
      self.assertNotEquals( part, None )
      self.assertEquals( len( part ), symbol_duration )

      signal = signal + part

      self.assertNotEquals( signal, None )

      symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    signal = csignal_tests.python_filter_signal( filter, signal )

    self.assertNotEquals( signal, None )

    signal_max = -1;

    for sample in signal:
      if( abs( sample ) > signal_max ):
        signal_max = abs( sample )

    signal = map( lambda x: x / signal_max, signal )

    file_handle = open( 'signal_after.dat', 'w' )

    for sample in signal:
      file_handle.write( "%e\n" %( sample ) )

    file_handle.close()

    fft = csignal_tests.python_calculate_FFT( signal )

    self.assertNotEquals( fft, None )

    fft_mag = map( lambda x: abs( x ), fft )

    N = len( fft_mag )
    delta = 1.0 / sample_rate

    max_value = -1

    for magnitude in fft_mag:
      if( magnitude > max_value ):
        max_value = magnitude
    
    fft_mag = map( lambda x: 10**-12 if x == 0 else x, fft_mag )
    fft_mag = map( lambda x: 10 * math.log10( x / max_value ), fft_mag )

    print "Length of signal is %d, N is %d." %( len( signal ), N )
    print "Length of fft is %d, sampling rate is %d Hz." %( len( fft_mag ), sample_rate )
    print "Delta (sample interval) is %.6f s, frequency interval is %.2f Hz." %( delta, 1.0 / ( delta * N ) )
    print "Max magnitude is %.2f. Max frequency is %.2f Hz" %( max_value, ( N / 2 ) * ( 1.0 / ( delta * N ) ) )

    file_handle = open( 'test.dat', 'w' )

    for index in range( len( fft_mag ) ):
      if( index > ( N / 2 ) ):
        n = index - N
      else:
        n = index

      file_handle.write( "%e\t%e\n" %( n / ( delta * N ), fft_mag[ index ] ) )

    file_handle.close()

    samples = [ signal ]

    if( os.path.exists( '/tmp/test.WAV' ) ):
      os.unlink( '/tmp/test.WAV' )

    error = csignal_tests.python_write_FLOAT_wav (
      '/tmp/test.WAV',
      len( samples ), 
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

  def test_filter( self ):
    sample_rate     = 48000

    first_stopband  = 19000
    first_passband  = 20000
    second_passband = 22000
    second_stopband = 23000

    passband_attenuation = 0.1
    stopband_attenuation = 80

    signal = []

    for i in range( 100 ):
      part = []

      for j in range( 200 ):
        part.append( 32767 * random.normalvariate( 0, 1 ) )

      #part = csignal_tests.python_fft_filter( first_passband, second_passband, sample_rate, part )

      #self.assertNotEquals( part, None )

      signal = signal + part

      self.assertNotEquals( signal, None )

    signal_max = -1;

    for sample in signal:
      if( abs( sample ) > signal_max ):
        signal_max = abs( sample )


    signal = map( lambda x: x / signal_max, signal )

    filter    = csignal_tests.python_initialize_kaiser_filter( first_stopband, first_passband, second_passband, second_stopband, passband_attenuation, stopband_attenuation, sample_rate )

    signal = csignal_tests.python_filter_signal( filter, signal )

    self.assertNotEquals( signal, None )

    file_handle = open( 'signal.dat', 'w' )

    for sample in signal:
      file_handle.write( "%e\n" %( sample ) )

    file_handle.close()

  def test_filter_signal( self ): 
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32767 
    carrier_frequency   = 12000
    symbol_duration     = 1000
    chip_duration       = 10

    first_stopband  = 10000
    first_passband  = 11000
    second_passband = 13000
    second_stopband = 14000

    passband_attenuation = 0.1
    stopband_attenuation = 80

    ( file_handle, file_name ) = touch_random_file()

    file_handle.close()

    filter = csignal_tests.python_initialize_kaiser_filter( first_stopband, first_passband, second_passband, second_stopband, passband_attenuation, stopband_attenuation, sample_rate )

    self.assertNotEquals( filter, None )

    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    gold_code = csignal_tests.python_initialize_gold_code( 7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000 )

    signal = []

    while( symbol != None ):
      part = csignal_tests.python_modulate_symbol (
          symbol,
          constellation_size,
          sample_rate,
          symbol_duration,
          baseband_amplitude,
          carrier_frequency
                                                  )
  
      self.assertNotEquals( part, None )

      part = csignal_tests.python_spread_signal (
          gold_code,
          chip_duration,
          part
                                                )

      self.assertNotEquals( part, None )

      signal = signal + part

      self.assertNotEquals( signal, None )

      symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    self.assertNotEquals( signal, None )

    self.assertNotEqual( len( signal ), None )

    signal = csignal_tests.python_filter_signal( filter, signal )

    self.assertNotEquals( signal, None )

    self.assertNotEquals( len( signal ), 0 )

    empty_signal = []

    for _ in range( len( signal )  ):
     empty_signal.append( 0 )

    samples = [ signal, signal ]

    error = csignal_tests.python_write_FLOAT_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

    self.assertEquals( csignal_tests.csignal_destroy_passband_filter( filter ), csignal_tests.CPC_ERROR_CODE_NO_ERROR )

    self.assertEquals( csignal_tests.csignal_destroy_symbol_tracker( symbol_tracker ), csignal_tests.CPC_ERROR_CODE_NO_ERROR )

    self.assertEquals( csignal_tests.csignal_destroy_gold_code( gold_code ), csignal_tests.CPC_ERROR_CODE_NO_ERROR )

    if( os.path.exists( file_name ) ):
      os.unlink( file_name )

  def test_spread_signal( self ):
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32000
    carrier_frequency   = 12000
    symbol_duration     = 1000
    chip_duration       = 10

    ( file_handle, file_name ) = touch_random_file()

    file_handle.close()

    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    gold_code = csignal_tests.python_initialize_gold_code( 7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000 )

    signal = []

    while( symbol != None ):
      part = csignal_tests.python_modulate_symbol (
          symbol,
          constellation_size,
          sample_rate,
          symbol_duration,
          baseband_amplitude,
          carrier_frequency
                                                  )
  
      self.assertNotEquals( part, None )

      part = csignal_tests.python_spread_signal (
          gold_code,
          chip_duration,
          part
                                                )

      self.assertNotEquals( part, None )

      signal = signal + part

      self.assertNotEquals( signal, None )

      symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    self.assertNotEquals( signal, None )

    samples = [ signal, signal ]

    error = csignal_tests.python_write_FLOAT_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

    if( os.path.exists( file_name ) ): 
      os.unlink( file_name )

  def test_initialize_kaiser_filter( self ):
    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 6000, 5000, 0.1, 80, 0 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 6000, 5000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 6000, 6000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 9000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 8000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 2000, 6000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 3000, 6000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 5000, 4000, 6000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 4000, 4000, 6000, 8000, 0.1, 80, 20000 )

    self.assertEquals( filter, None )

    filter = csignal_tests.python_initialize_kaiser_filter( 3000, 4000, 6000, 8000, 0.1, 80, 20000 )

    self.assertNotEquals( filter, None )

    csignal_tests.csignal_destroy_passband_filter( filter )

  def test_get_gold_code_degree_7( self ):
    gold_code = csignal_tests.python_initialize_gold_code( 7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000 )

    self.assertNotEquals( gold_code, None )

    codes = csignal_tests.python_get_gold_code( gold_code, 80000 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 10000 )

    index = 0

    with open( 'GoldSequenceTestVector_7_89_8F_20_20.dat' ) as file_pointer:
      for line in file_pointer:
        expression = re.compile( '^(\d+)$' )

        result = re.match( expression, line )

        if( result ):
          self.assertEquals( codes[ index ], int( result.group( 1 ) ) )

          index += 1

    result = csignal_tests.csignal_destroy_gold_code( gold_code )

    self.assertEquals( result, csignal_tests.CPC_ERROR_CODE_NO_ERROR )

  def test_get_gold_code_size( self ):
    gold_code = csignal_tests.python_initialize_gold_code( 2, 0xC0000000, 0xC0000000, 0x40000000, 0x80000000 )

    self.assertNotEquals( gold_code, None )

    codes = csignal_tests.python_get_gold_code( gold_code, 0 )

    self.assertEquals( codes, None )

    codes = csignal_tests.python_get_gold_code( gold_code, 8 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 1 )

    codes = csignal_tests.python_get_gold_code( gold_code, 16 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 2 )

    result = csignal_tests.csignal_destroy_gold_code( gold_code )

    self.assertEquals( result, csignal_tests.CPC_ERROR_CODE_NO_ERROR )

    gold_code = csignal_tests.python_initialize_gold_code( 7, 0x12000000, 0x1E000000, 0x40000000, 0x40000000 )

    self.assertNotEquals( gold_code, None )

    codes = csignal_tests.python_get_gold_code( gold_code, 0 )

    self.assertEquals( codes, None )

    codes = csignal_tests.python_get_gold_code( gold_code, 8 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 1 )

    codes = csignal_tests.python_get_gold_code( gold_code, 16 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 2 )

    result = csignal_tests.csignal_destroy_gold_code( gold_code )

    self.assertEquals( result, csignal_tests.CPC_ERROR_CODE_NO_ERROR )

  def test_initialize_gold_code( self ):
    gold_code = csignal_tests.python_initialize_gold_code( 1, 0x10000000, 0x10000000, 0x10000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 33, 0x10000000, 0x10000000, 0x10000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 2, 0x80000000, 0x10000000, 0xC0000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 2, 0xC0000000, 0x10000000, 0x80000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 2, 0xE0000000, 0x10000000, 0xC0000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 2, 0xC0000000, 0x10000000, 0xE0000000, 0x10000000 )

    self.assertEquals( gold_code, None )

    gold_code = csignal_tests.python_initialize_gold_code( 2, 0xC0000000, 0x40000000, 0xC0000000, 0x40000000 )

    self.assertNotEquals( gold_code, None )

    result = csignal_tests.csignal_destroy_gold_code( gold_code )

    self.assertEquals( result, csignal_tests.CPC_ERROR_CODE_NO_ERROR )

  def test_get_spreading_code_degree_32( self ):
    spreading_code = csignal_tests.python_initialize_spreading_code( 32, 0x00200007, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 80000 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 10000 )

    index = 0

    with open( 'PNSequenceTestVector_32_100200007_40000000.dat' ) as file_pointer:
      for line in file_pointer:
        expression = re.compile( '^(\d+)$' )

        result = re.match( expression, line )

        if( result ):
          self.assertEquals( codes[ index ], int( result.group( 1 ) ) )

          index += 1

  def test_get_spreading_code_degree_7( self ):
    spreading_code = csignal_tests.python_initialize_spreading_code( 7, 0x12000000, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 80000 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 10000 )

    index = 0

    with open( 'PNSequenceTestVector_7_89_20.dat' ) as file_pointer:
      for line in file_pointer:
        expression = re.compile( '^(\d+)$' )

        result = re.match( expression, line )

        if( result ):
          self.assertEquals( codes[ index ], int( result.group( 1 ) ) )

          index += 1

  def test_get_spreading_code_degree_2( self ):
    spreading_code = csignal_tests.python_initialize_spreading_code( 2, 0xC0000000, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 80000 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 10000 )

    index = 0

    with open( 'PNSequenceTestVector_2_7_1.dat' ) as file_pointer:
      for line in file_pointer:
        expression = re.compile( '^(\d+)$' )

        result = re.match( expression, line )

        if( result ):
          self.assertEquals( codes[ index ], int( result.group( 1 ) ) )

          index += 1

  def test_initialize_spreading_code( self ):
    spreading_code = csignal_tests.python_initialize_spreading_code( 1, 0x10000000, 0x10000000 )

    self.assertEquals( spreading_code, None )

    spreading_code = csignal_tests.python_initialize_spreading_code( 33, 0x10000000, 0x10000000 )

    self.assertEquals( spreading_code, None )

    spreading_code = csignal_tests.python_initialize_spreading_code( 2, 0x80000000, 0x10000000 )

    self.assertEquals( spreading_code, None )

    spreading_code = csignal_tests.python_initialize_spreading_code( 2, 0xE0000000, 0x10000000 )

    self.assertEquals( spreading_code, None )

    spreading_code = csignal_tests.python_initialize_spreading_code( 2, 0xC0000000, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

  def test_get_spreading_code_size( self ):
    spreading_code = csignal_tests.python_initialize_spreading_code( 2, 0xC0000000, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 0 )

    self.assertEquals( codes, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 8 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 1 )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 16 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 2 )

    spreading_code = csignal_tests.python_initialize_spreading_code( 7, 0x12000000, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 0 )

    self.assertEquals( codes, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 8 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 1 )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 16 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 2 )

    spreading_code = csignal_tests.python_initialize_spreading_code( 32, 0x00200007, 0x40000000 )

    self.assertNotEquals( spreading_code, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 0 )

    self.assertEquals( codes, None )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 8 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 1 )

    codes = csignal_tests.python_get_spreading_code( spreading_code, 16 )

    self.assertNotEquals( codes, None )
    self.assertEquals( len( codes ), 2 )

  def test_write_wav_random( self ):
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32000
    carrier_frequency   = 22000

    ( file_handle, file_name ) = touch_random_file()

    file_handle.close()

    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    signal = []

    while( symbol != None ):
      part = csignal_tests.python_modulate_symbol (
          symbol,
          constellation_size,
          sample_rate,
          sample_rate,
          baseband_amplitude,
          carrier_frequency
                                                  )
  
      self.assertNotEquals( part, None )

      signal = signal + part

      symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    samples = [ signal, signal ]

    error = csignal_tests.python_write_FLOAT_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

  def test_write_wav_basic( self ):
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32000
    carrier_frequency   = 22000

    data = '\x12'
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    signal = csignal_tests.python_modulate_symbol (
        symbol,
        constellation_size,
        sample_rate,
        sample_rate,
        baseband_amplitude,
        carrier_frequency
                                                  )

    self.assertNotEquals( signal, None )

    samples = [ signal, signal ]

    ( file_handle, file_name ) = touch_random_file()

    error = csignal_tests.python_write_FLOAT_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )
    
    file_handle.close()

    self.assertEquals( error, csignal_tests.CPC_FALSE )

    error = csignal_tests.python_write_FLOAT_wav (
      "/test.WAV",
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_FALSE )

  def test_write_wav( self ):
    bits_per_symbol     = 8
    constellation_size  = 2 ** bits_per_symbol
    sample_rate         = 48000
    baseband_amplitude  = 32000
    carrier_frequency   = 22000

    ( file_handle, file_name ) = touch_random_file()

    file_handle.close()

    data = '\x12'
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, bits_per_symbol ) 

    signal = csignal_tests.python_modulate_symbol (
        symbol,
        constellation_size,
        sample_rate,
        sample_rate,
        baseband_amplitude,
        carrier_frequency
                                                  )

    self.assertNotEquals( signal, None )

    samples = [ signal, signal ]

    error = csignal_tests.python_write_FLOAT_wav (
      file_name,
      len( samples ),
      sample_rate,
      sample_rate,
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

    if( os.path.exists( file_name ) ):
      os.unlink( file_name )

  def test_generate_signal_random( self ):
    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )

    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 8 )

    while( symbol != None ):
      signal = csignal_tests.python_modulate_symbol (
        symbol,
        2**8,
        48000,
        100,
        16000,
        22000
                                                    )

      self.assertNotEquals( signal, None )

      symbol = csignal_tests.python_get_symbol( symbol_tracker, 8 )

  def test_generate_signal( self ):
    self.assertEquals (
      csignal_tests.python_modulate_symbol( -1, 8, 48000, 10, 16000, 22000 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, 0, 48000, 10, 16000, 22000 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, -1, 48000, 10, 16000, 22000 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, 8, 0, 10, 16000, 22000 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, 8, -1, 10, 16000, 22000 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, 8, 48000, 10, 16000, -1 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 1, 8, 48000, 10, 16000, 0 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 4, 2, 48000, 10, 16000, 0 ),
      None
                      )

    self.assertEquals (
      csignal_tests.python_modulate_symbol( 2, 2, 48000, 10, 16000, 0 ),
      None
                      )

  def test_generate_signal_basic( self ):
    data = '\x12\x34'
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 ) 

    while( symbol != None ):
      signal = csignal_tests.python_modulate_symbol (
        symbol,
        2**4,
        48000,
        10,
        16000,
        22000
                                                    )

      self.assertNotEquals( signal, None )

      symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 ) 

  def test_csignal_initialize_destroy_symbol_tracker( self ):
    data = "This is my data!"

    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    self.assertNotEqual( symbol_tracker, None )

    error = csignal_tests.csignal_destroy_symbol_tracker( symbol_tracker )

    self.assertEqual( error, csignal_tests.CPC_ERROR_CODE_NO_ERROR )

    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( None )

    self.assertEqual( symbol_tracker, None )

    error = csignal_tests.csignal_destroy_symbol_tracker( None )

    self.assertEqual( error, csignal_tests.CPC_ERROR_CODE_NULL_POINTER )

  def test_get_symbol( self ):
    self.assertEqual( csignal_tests.python_get_symbol( None, 4 ), None )

    data = ''

    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    self.assertEqual( csignal_tests.python_get_symbol( symbol_tracker, 4 ), None )

  def test_reading_data( self ):
    # Two bytes (in binary): 0001 0010 0011 0100
    data = '\x12\x34'
    
    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 )

    self.assertEqual( symbol, 1 )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 )

    self.assertEqual( symbol, 2 )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 )

    self.assertEqual( symbol, 3 )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 4 )

    self.assertEqual( symbol, 4 )

  def test_reading_data_random( self ):
    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 100 ) )

    symbol_tracker = csignal_tests.python_intialize_symbol_tracker( data )

    symbol = csignal_tests.python_get_symbol( symbol_tracker, 8 )

    index = 0

    while( symbol != None ):
      self.assertEqual( symbol, ord( data[ index ] ) )

      index += 1

      symbol = csignal_tests.python_get_symbol( symbol_tracker, 8 )

if __name__ == '__main__':
  csignal_tests.cpc_log_set_log_level( csignal_tests.CPC_LOG_LEVEL_TRACE )

  unittest.main()
