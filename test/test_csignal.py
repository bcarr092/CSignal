import csignal_tests
import unittest
import string
import random
import types
import os
import tempfile
import re

class TestsCSignal( unittest.TestCase ):
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

    ( file_handle, file_name ) = self.touch_random_file()

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

    error = csignal_tests.python_write_LPCM_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

  def touch_random_file( self ):
    file_handle = tempfile.NamedTemporaryFile()

    file_name = file_handle.name

    return( file_handle, file_name )

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

    ( file_handle, file_name ) = self.touch_random_file()

    error = csignal_tests.python_write_LPCM_wav (
      file_name,
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )
    
    file_handle.close()

    self.assertEquals( error, csignal_tests.CPC_FALSE )

    error = csignal_tests.python_write_LPCM_wav (
      "/test.WAV",
      len( samples ),
      sample_rate,
      len( signal ),
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_FALSE )

  def test_write_wav( self ):
    file_name           = "/tmp/test.WAV"
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

    error = csignal_tests.python_write_LPCM_wav (
      file_name,
      len( samples ),
      sample_rate,
      sample_rate,
      samples
                                                )

    self.assertEquals( error, csignal_tests.CPC_TRUE )

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
      csignal_tests.python_modulate_symbol( 1, 8, 48000, -1, 16000, 22000 ),
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
