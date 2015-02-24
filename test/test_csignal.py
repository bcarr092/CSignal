import csignal_tests
import unittest
import string
import random
import types

class TestsCSignal( unittest.TestCase ):
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
