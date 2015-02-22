import csignal_tests
import unittest
import string
import random
import types

class TestsCSignal( unittest.TestCase ):
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
