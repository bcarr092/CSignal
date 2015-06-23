from csignal_tests import *

import unittest

class TestsConv( unittest.TestCase ):
  def test_conv_basic( self ):
    input   = [ 1.0, 2.0, 3.0 ]
    impulse = [ 1.0 ]

    output = python_convolve( input, impulse )

    self.assertNotEquals( output, None )

    self.assertEquals( len( input ) + len( impulse ), len( output ) )

    for index in range( len( input ) ):
      self.assertEquals( input[ index ], output[ index ] )

if __name__ == '__main__':
  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()
