cmake_minimum_required( VERSION 3.0 )

set( PROJECT_NAME "csignal" )

project( ${PROJECT_NAME} )

message( STATUS "Project: ${PROJECT_NAME}" )
message( STATUS "Project directory: ${PROJECT_SOURCE_DIR}" )

set( SOURCE_DIR "${PROJECT_SOURCE_DIR}/src" )
set( INCLUDE_DIR "${SOURCE_DIR}/include" )
set( INSTALL_DIR "${CMAKE_INSTALL_PREFIX}" )
set (
    INSTALL_LIB_DIR
    "${INSTALL_DIR}/lib/${PLATFORM}/${TARGET}/${ARCHITECTURE}"
    )

message( STATUS "Source directory: ${SOURCE_DIR}" )
message( STATUS "Include directory: ${INCLUDE_DIR}" )
message( STATUS "Install directory: ${INSTALL_DIR}" )

set( SOURCES "${SOURCE_DIR}/csignal.c" )
list( APPEND SOURCES "${SOURCE_DIR}/bit_packer.c" )
list( APPEND SOURCES "${SOURCE_DIR}/bit_stream.c" )
list( APPEND SOURCES "${SOURCE_DIR}/wav.c" )
list( APPEND SOURCES "${SOURCE_DIR}/gold_code.c" )
list( APPEND SOURCES "${SOURCE_DIR}/spreading_code.c" )
list( APPEND SOURCES "${SOURCE_DIR}/kaiser_filter.c" )
list( APPEND SOURCES "${SOURCE_DIR}/fir_filter.c" )
list( APPEND SOURCES "${SOURCE_DIR}/fft.c" )
list( APPEND SOURCES "${SOURCE_DIR}/conv.c" )
list( APPEND SOURCES "${SOURCE_DIR}/detect.c" )

set( HEADERS "${INCLUDE_DIR}/csignal.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/bit_packer.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/bit_stream.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/wav.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/gold_code.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/spreading_code.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/fir_filter.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/kaiser_filter.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/fft.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/conv.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/detect.h" )
list( APPEND HEADERS "${INCLUDE_DIR}/csignal_error_codes.h" )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
  find_library( OBJC_FRAMEWORK objc )

  set (
        EXTRA_LIBS
        ${OBJC_FRAMEWORK}
      )

  message( STATUS "Framework libraries: ${EXTRA_LIBS}" )

  set( DARWINHELPER_INCLUDE_DIR "${darwinhelper_SOURCE_DIR}/src/include" )

  message (
    STATUS
    "Working with darwinhelper source directory: ${DARWINHELPER_INCLUDE_DIR}"
          )
  
  include_directories( ${DARWINHELPER_INCLUDE_DIR} )
elseif( ${PLATFORM} STREQUAL "Generic" AND ${TARGET} MATCHES "^android" )
  find_library  (
                  MATH_LIB
                  m
                  HINTS "${CMAKE_FIND_ROOT_PATH}/usr/lib"
                )

  set (
        EXTRA_LIBS
        ${OPENSLES_LIB}
        ${MATH_LIB}
      )

  message( STATUS "Android libraries: ${EXTRA_LIBS}" )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )

else()
  message( FATAL_ERROR "Unsupported platform: ${PLATFORM}" )
endif()

message( STATUS "C source files found: ${SOURCES}" )
message( STATUS "C header files found: ${HEADERS}" )

set( CPCOMMON_INCLUDE_DIR "${cpcommon_SOURCE_DIR}/src/include" )

message (
  STATUS
  "Working with cpcommon source directory: ${CPCOMMON_INCLUDE_DIR}"
        )

include_directories( ${INCLUDE_DIR} )
include_directories( ${CPCOMMON_INCLUDE_DIR} )

add_library( ${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} )

if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
  target_link_libraries( ${PROJECT_NAME} darwinhelper )
endif()

target_link_libraries( ${PROJECT_NAME} cpcommon )
target_link_libraries( ${PROJECT_NAME} ${EXTRA_LIBS} )

install (
          TARGETS ${PROJECT_NAME}
          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
          LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        )
install( FILES ${HEADERS} DESTINATION "${INSTALL_DIR}/include/${PROJECT_NAME}" )

# Build test suite as well
add_subdirectory( test )

enable_testing()

find_package( Doxygen )

if( DOXYGEN_FOUND )
  set( DOXYGEN_OUTPUT "${INSTALL_DIR}/doc/${PROJECT_NAME}" )

  configure_file  (
    ${PROJECT_SOURCE_DIR}/Doxyfile.in
    ${PROJECT_BINARY_DIR}/Doxyfile
    @ONLY
                  )
  add_custom_target (
    doc_csignal
    ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
                    )

  file( MAKE_DIRECTORY ${DOXYGEN_OUTPUT} )

endif(DOXYGEN_FOUND)

