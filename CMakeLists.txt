cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")

include(CheckIncludeFiles)
include(CheckStructHasMember)
include(CheckSymbolExists)

PROJECT(epgdump)

SET(serial "0.2.1 + patch 1.0")

find_package (Iconv REQUIRED)
IF( ICONV_FOUND )
  INCLUDE_DIRECTORIES( ${ICONV_INCLUDE_DIR} )
  SET( HAVE_ICONV_H 1 )
  SET( ICONV_REQUIRES_CONST ${ICONV_SECOND_ARGUMENT_IS_CONST} )
ENDIF( ICONV_FOUND )

add_definitions(-D_LARGEFILE64_SOURCE)
add_definitions(-D_FILE_OFFSET_BITS=64)

include_directories (${CMAKE_SOURCE_DIR}/)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_SOURCE_DIR}/config.h )

ADD_EXECUTABLE(epgdump 
aribstr.c
eit.c
epgdump.c
nit.c
sdt.c
bit.c
ts.c
tdt.c
tail.c
util.c)


target_link_libraries (epgdump ${ICONV_LIBRARIES})

install(TARGETS epgdump RUNTIME DESTINATION "bin")
