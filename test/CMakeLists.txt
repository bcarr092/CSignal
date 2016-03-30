cmake_minimum_required( VERSION 3.0 )

set( PROJECT_NAME "csignal_tests" )

project( ${PROJECT_NAME} )

message( STATUS "Project: ${PROJECT_NAME}" )
message( STATUS "Project directory: ${PROJECT_SOURCE_DIR}" )

set( SOURCE_DIR "${PROJECT_SOURCE_DIR}/src" )
set( INCLUDE_DIR "${SOURCE_DIR}/include" )
set( INSTALL_DIR "${CMAKE_INSTALL_PREFIX}" )
set( INSTALL_TEST_DIR "${INSTALL_DIR}/tests/${PROJECT_NAME}" )
set( INSTALL_TEST_LIB_DIR "${INSTALL_TEST_DIR}/lib" )
set (
    INSTALL_TEST_PLATFORM_LIB_DIR
    "${INSTALL_TEST_DIR}/lib/${PLATFORM}/${TARGET}/${ARCHITECTURE}"
    )

message( STATUS "Source directory: ${SOURCE_DIR}" )
message( STATUS "Include directory: ${INCLUDE_DIR}" )
message( STATUS "Install directory: ${INSTALL_DIR}" )

set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH )

find_package( SWIG REQUIRED )

set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

include( ${SWIG_USE_FILE} )

set( CSIGNAL_INCLUDE_DIRECTORY "${csignal_SOURCE_DIR}/src/include" )
set( CPCOMMON_INCLUDE_DIRECTORY "${cpcommon_SOURCE_DIR}/src/include" )

if(
    DEFINED ENV{PYTHON_INCLUDE}
    AND DEFINED ENV{PYTHON_LIB}
    AND DEFINED ENV{PYTHON_BIN}
  )
  set( PYTHON_INCLUDE $ENV{PYTHON_INCLUDE}  )
  set( PYTHON_LIB     $ENV{PYTHON_LIB}      )
  set( PYTHON_BIN     $ENV{PYTHON_BIN}      )
else()
  message (
    FATAL_ERROR
    "Environment variables PYTHON_INCLUDE, PYTHON_LIB, PYTHON_BIN must be set."
          )
endif()

if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
  set (
        PYTHON_LIBRARIES
        "${PYTHON_LIB}/libpython.dylib"
      )
  set (
        PYTHON_BINARY
        "${PYTHON_BIN}/python"
      )
elseif( ${PLATFORM} STREQUAL "Generic" AND ${TARGET} MATCHES "^android" )
  set (
        PYTHON_LIBRARIES
        "${PYTHON_LIB}/libpython.so"
      )
  set (
        PYTHON_BINARY
        "${PYTHON_BIN}/python"
      )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
  string( REPLACE "\\" "/" PYTHON_INCLUDE ${PYTHON_INCLUDE} )
  string( REPLACE "\\" "/" PYTHON_LIB ${PYTHON_LIB} )
  string( REPLACE "\\" "/" PYTHON_BIN ${PYTHON_BIN} )

  set (
        PYTHON_LIBRARIES
        "${PYTHON_LIB}/python27.lib"
      )
  set (
        PYTHON_BINARY
        "${PYTHON_BIN}/python.exe"
      )
endif()

include_directories( CSIGNAL_INCLUDE_DIRECTORY )
include_directories( ${CPCOMMON_INCLUDE_DIRECTORY} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${PYTHON_INCLUDE} )
include_directories( ${INCLUDE_DIR} )

message( STATUS "Python includes: ${PYTHON_INCLUDE}" )
message( STATUS "Python binary: ${PYTHON_BINARY}" )
message( STATUS "Python library: ${PYTHON_LIBRARIES}" )

set( SOURCES "${SOURCE_DIR}/csignal_wrapper.c" )

set( HEADERS "${INCLUDE_DIR}/csignal_wrapper.h" )

message( STATUS "C source files found: ${SOURCES}" )
message( STATUS "C header files found: ${HEADERS}" )

swig_add_module (
  ${PROJECT_NAME}
  python
  "${PROJECT_NAME}.i"
  ${SOURCES}
  ${HEADERS}
                )

swig_link_libraries( ${PROJECT_NAME} ${PYTHON_LIBRARIES} cpcommon csignal )

set( SCRIPTS "${PROJECT_SOURCE_DIR}/test_driver.py" )
list( APPEND SCRIPTS "${PROJECT_SOURCE_DIR}/PNSequenceTestVector_2_7_1.dat" )
list( APPEND SCRIPTS "${PROJECT_SOURCE_DIR}/PNSequenceTestVector_7_89_20.dat" )
list  (
  APPEND
  SCRIPTS
  "${PROJECT_SOURCE_DIR}/PNSequenceTestVector_32_100200007_40000000.dat"
      )
list  (
  APPEND
  SCRIPTS
  "${PROJECT_SOURCE_DIR}/GoldSequenceTestVector_7_89_8F_20_20.dat"
      )

set( LIBS "${PROJECT_SOURCE_DIR}/test_csignal.py" )
list( APPEND LIBS "${PROJECT_SOURCE_DIR}/test_bit_packer.py" )
list( APPEND LIBS "${PROJECT_SOURCE_DIR}/test_bit_stream.py" )
list( APPEND LIBS "${PROJECT_SOURCE_DIR}/test_conv.py" )
list( APPEND LIBS "${PROJECT_SOURCE_DIR}/test_signal_operations.py" )

set( WRAPPERS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.py" )

message( STATUS "Copying scripts: ${SCRIPTS}" )
message( STATUS "Copying libs: ${LIBS}" )
message( STATUS "Copying wrappers: ${WRAPPERS}" )

foreach( FILE_TO_COPY IN ITEMS ${SCRIPTS} )
    message( STATUS "Copying script ${FILE_TO_COPY} to ${PROJECT_BINARY_DIR}" )
    
    add_custom_command( TARGET "_${PROJECT_NAME}" PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${FILE_TO_COPY}" "${PROJECT_BINARY_DIR}" )
                        
endforeach( FILE_TO_COPY )

install(  TARGETS "_${PROJECT_NAME}" DESTINATION
          "${INSTALL_TEST_PLATFORM_LIB_DIR}" )
install(  PROGRAMS ${SCRIPTS} DESTINATION
          "${INSTALL_TEST_DIR}" )
install(  FILES ${LIBS} DESTINATION
          "${INSTALL_TEST_LIB_DIR}" )
install(  FILES ${WRAPPERS} DESTINATION
          "${INSTALL_TEST_PLATFORM_LIB_DIR}" )

enable_testing()

add_test  (
  ${PROJECT_NAME}
  ${PYTHON_BINARY}
  "${PROJECT_BINARY_DIR}/test_driver.py"
          )

if( ${PLATFORM} STREQUAL "Generic" AND ${TARGET} MATCHES "^android" )
  find_library  (
                  MATH_LIB
                  m  
                  HINTS "${CMAKE_FIND_ROOT_PATH}/usr/lib"
                )

  set (
        EXTRA_LIBS
        ${MATH_LIB}
      )

  message( STATUS "Android libraries: ${EXTRA_LIBS}" )

  swig_link_libraries( ${PROJECT_NAME} ${EXTRA_LIBS} )
endif()
