import csignal_tests                                                              
import unittest                                                                 
                                                                                
from test_csignal import TestsCSignal

csignal_tests.cpc_log_set_log_level( csignal_tests.CPC_LOG_LEVEL_NO_LOGGING )

alltests = unittest.TestSuite ( [                                                   \
 unittest.TestLoader().loadTestsFromTestCase( TestsCSignal ),                       \
                                ] )

unittest.TextTestRunner( verbosity=2 ).run( alltests )
