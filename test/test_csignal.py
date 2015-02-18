import csignal_tests
import unittest
import string
import types

class TestsCSignal( unittest.TestCase ):
  def test_test( self ):
    csignal_tests.test_csignal()

if __name__ == '__main__':
  csignal_tests.cpc_log_set_log_level( csignal_tests.CPC_LOG_LEVEL_ERROR )

  unittest.main()
