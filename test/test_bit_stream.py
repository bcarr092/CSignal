from csignal_tests import *
from array import array
import unittest
import struct
import socket
import random
import string
import sys

class TestsBitStream( unittest.TestCase ):
  def test_bit_stream_add_read( self ):
    data = ''
    
    for index in range( 1000 ):
      initialNumber = random.randint( 0, 2 ** 32 - 1 ) 

      data = data + struct.pack( "I", int( initialNumber ) )

    bitPacker = python_bit_packer_initialize()

    self.assertNotEquals( bitPacker, None )

    bitStream = python_bit_stream_initialize_from_bit_packer( bitPacker )

    self.assertNotEquals( bitStream, None )

    for index in range( 0, len( data ), 4 ):
      string  = data[ index : index + 4 ]
      value   = struct.unpack( "I", string )[ 0 ]

      self.assertEquals (
        python_bit_packer_add_bytes( string, bitPacker ),
        CPC_ERROR_CODE_NO_ERROR
                        )

      result = python_bit_stream_get_bits( bitStream, 32 )

      self.assertNotEquals( result, None )

      ( numberOfBits, buffer ) = result

      self.assertEquals( numberOfBits, 32 )
      self.assertEquals( len( buffer ), 4 )

      retrievedValue = struct.unpack( "I", buffer )[ 0 ]

      self.assertEquals( value, retrievedValue )


    self.assertEquals (
      bit_stream_destroy( bitStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )

    self.assertEquals (
      bit_packer_destroy( bitPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )

  def test_bit_stream_get_bits_int_random( self ):
    for index in range( 100 ):
      initialNumber = random.randint( 0, 2 ** 32 - 1 ) 

      data = struct.pack( "I", int( initialNumber ) )

      bitStream = python_bit_stream_initialize( data )

      self.assertNotEquals( bitStream, None )

      result = python_bit_stream_get_bits( bitStream, 32 )

      self.assertNotEquals( result, None )

      ( numberOfBits, buffer ) = result

      self.assertNotEquals( numberOfBits, None )
      self.assertNotEquals( buffer, None )

      self.assertEquals( numberOfBits, 32 )
      self.assertEquals( len( buffer ), 4 )

      retrievedNumber = struct.unpack( "I", buffer )[ 0 ]

      self.assertEquals( initialNumber, retrievedNumber )

      self.assertEquals (
        bit_stream_destroy( bitStream ),
        CPC_ERROR_CODE_NO_ERROR
                      )   
 
 
    for count in range( 100 ):
      data = ''

      for _ in range( 1000 ):
        initialNumber = random.randint( 0, 2 ** 32 - 1 ) 

        data = data + struct.pack( "I", int( initialNumber ) )

      bitStream = python_bit_stream_initialize( data )

      self.assertNotEquals( bitStream, None )

      for index in range( 1000 ):
        baseIndex = index * 4

        initialNumber = struct.unpack( "I", data[ baseIndex : baseIndex + 4 ] )[ 0 ]

        result = python_bit_stream_get_bits( bitStream, 32 )

        self.assertNotEquals( result, None )

        ( numberOfBits, buffer ) = result

        self.assertNotEquals( numberOfBits, None )
        self.assertNotEquals( buffer, None )

        self.assertEquals( numberOfBits, 32 )
        self.assertEquals( len( buffer ), 4 )

        retrievedNumber = struct.unpack( "I", buffer )[ 0 ]

        self.assertEquals( initialNumber, retrievedNumber )

      self.assertEquals (
        bit_stream_destroy( bitStream ),
        CPC_ERROR_CODE_NO_ERROR
                        )   

  def test_bit_stream_get_bits_int_basic( self ):
    initialNumber = 100

    data = struct.pack( "I", int( initialNumber ) )

    bitStream = python_bit_stream_initialize( data )

    self.assertNotEquals( bitStream, None )

    result = python_bit_stream_get_bits( bitStream, 32 )

    self.assertNotEquals( result, None )

    ( numberOfBits, buffer ) = result

    self.assertNotEquals( numberOfBits, None )
    self.assertNotEquals( buffer, None )

    self.assertEquals( numberOfBits, 32 )
    self.assertEquals( len( buffer ), 4 )

    retrievedNumber = struct.unpack( "I", buffer )[ 0 ]

    self.assertEquals( initialNumber, retrievedNumber )

    self.assertEquals (
      bit_stream_destroy( bitStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )   

  def test_bit_stream_get_bits_random( self ):
    data = ''.join( random.choice( string.ascii_lowercase ) for _ in range( 1000 ) )

    for numBits in [ 1, 2, 4, 8 ]:
      bitStream = python_bit_stream_initialize( data )

      self.assertNotEquals( bitStream, None )

      for index in range( 0, len( data ) * 8, numBits ):
        result  = python_bit_stream_get_bits( bitStream, numBits )

        self.assertNotEquals( result, None )

        ( numberOfBits, buffer ) = result

        self.assertNotEquals( numberOfBits, None )
        self.assertNotEquals( buffer, None )

        self.assertEquals( numberOfBits, numBits )
        self.assertEquals( len( buffer ), 1 )

        byte          = ( struct.unpack( "B", buffer )[ 0 ] >> ( 8 - numBits ) )
        byteToCompare =                                         \
          struct.unpack( "B", data[ int( index / 8 )  ] )[ 0 ]  \
          >> ( ( 8 - numBits ) - ( index % 8 ) )

        mask = 0

        for maskBit in range( numBits ):
          mask |= ( 1 << maskBit )

        self.assertEquals( ( byte ^ ( byteToCompare  & mask ) ), 0 )

      self.assertEquals (
        bit_stream_destroy( bitStream ),
        CPC_ERROR_CODE_NO_ERROR
                        )   

  def test_bit_stream_get_bits_basic( self ):
    data = "\x12"

    for numBits in [ 1, 2, 4, 8 ]:
      bitStream = python_bit_stream_initialize( data )

      self.assertNotEquals( bitStream, None )

      for index in range( 0, len( data ) * 8, numBits ):
        result  = python_bit_stream_get_bits( bitStream, numBits )

        self.assertNotEquals( result, None )

        ( numberOfBits, buffer ) = result

        self.assertNotEquals( numberOfBits, None )
        self.assertNotEquals( buffer, None )

        self.assertEquals( numberOfBits, numBits )
        self.assertEquals( len( buffer ), 1 )

        byte          = ( struct.unpack( "B", buffer )[ 0 ] >> ( 8 - numBits ) )
        byteToCompare =                                         \
          struct.unpack( "B", data[ int( index / 8 )  ] )[ 0 ]  \
          >> ( ( 8 - numBits ) - ( index % 8 ) )

        mask = 0

        for maskBit in range( numBits ):
          mask |= ( 1 << maskBit )

        self.assertEquals( ( byte ^ ( byteToCompare  & mask ) ), 0 )

      self.assertEquals (
        bit_stream_destroy( bitStream ),
        CPC_ERROR_CODE_NO_ERROR
                        )   

  def test_bit_stream_negative( self ):
    self.assertEquals( python_bit_stream_initialize( None ), None )
    self.assertEquals( python_bit_stream_initialize( 1 ), None )

    self.assertEquals( python_bit_stream_initialize_from_bit_packer( None ), None )

    self.assertNotEquals( bit_stream_destroy( None ), CPC_ERROR_CODE_NO_ERROR )

  def test_initialize_from_bit_packer( self ):
    bitPacker = python_bit_packer_initialize()

    self.assertNotEquals( bitPacker, None )

    data = "Hello"

    self.assertEquals (
      python_bit_packer_add_bytes (
        data,
        bitPacker
                                  ),
      CPC_ERROR_CODE_NO_ERROR
                    )

    bitStream = python_bit_stream_initialize_from_bit_packer( bitPacker )

    self.assertNotEquals( bitStream, None )

    self.assertEquals (
      bit_packer_destroy( bitPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )   

    self.assertEquals (
      bit_stream_destroy( bitStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )   

    bitPacker = python_bit_packer_initialize()

    self.assertNotEquals( bitPacker, None )

    data = ""

    self.assertEquals (
      python_bit_packer_add_bytes (
        data,
        bitPacker
                                  ),
      CPC_ERROR_CODE_NO_ERROR
                    )

    bitStream = python_bit_stream_initialize_from_bit_packer( bitPacker )

    self.assertNotEquals( bitStream, None )

    self.assertEquals (
      bit_packer_destroy( bitPacker ),
      CPC_ERROR_CODE_NO_ERROR
                      )   

    self.assertEquals (
      bit_stream_destroy( bitStream ),
      CPC_ERROR_CODE_NO_ERROR
                      )   

  def test_initialize( self ):
    data = "Hello"

    bitStream = python_bit_stream_initialize( data )

    self.assertNotEquals( bitStream, None )
    self.assertEquals( bit_stream_destroy( bitStream ), CPC_ERROR_CODE_NO_ERROR )

    data = struct.pack( "I", socket.htonl( int( 1722 ) ) )

    bitStream = python_bit_stream_initialize( data )

    self.assertNotEquals( bitStream, None )
    self.assertEquals( bit_stream_destroy( bitStream ), CPC_ERROR_CODE_NO_ERROR )

    data = ""

    for index in range( 100 ):
      value = 32767 * random.normalvariate( 0, 1 )
      value = struct.unpack( "I", struct.pack( "i", int( value ) ))
      data  = data + struct.pack( "I", socket.htonl( value[ 0 ] ) )

    bitStream = python_bit_stream_initialize( data )

    self.assertNotEquals( bitStream, None )
    self.assertEquals( bit_stream_destroy( bitStream ), CPC_ERROR_CODE_NO_ERROR )

  def test_initialize_destroy( self ):
    data = "1"

    bitStream = python_bit_stream_initialize( data )

    self.assertNotEquals( bitStream, None )

    self.assertEquals( bit_stream_destroy( bitStream ), CPC_ERROR_CODE_NO_ERROR )

if __name__ == '__main__':
  cpc_log_set_log_level( CPC_LOG_LEVEL_ERROR )

  csignal_initialize()

  unittest.main()

  csignal_terminate()
