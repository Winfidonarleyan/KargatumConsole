#
# Copyright (C) 2019+     WarheadCore
#

# Sources
file(GLOB SRCS_G "src/*.cpp")
POCO_SOURCES_AUTO(SRCS ${SRCS_G})

# Headers
file(GLOB_RECURSE HDRS_G "include/*.h")
POCO_HEADERS_AUTO(SRCS ${HDRS_G})

# Platform Specific
POCO_SOURCES_AUTO_PLAT(SRCS UNIX src/SyslogChannel.cpp)
POCO_HEADERS_AUTO(SRCS include/Poco/SyslogChannel.h)

# For Windows CE we need to disable these
if(WINCE)
POCO_SOURCES_AUTO_PLAT(SRCS OFF
  src/WindowsConsoleChannel.cpp
  src/EventLogChannel.cpp)
else()
POCO_SOURCES_AUTO_PLAT(SRCS WIN32
	src/WindowsConsoleChannel.cpp
	src/EventLogChannel.cpp)
endif()

# Messages
POCO_MESSAGES(SRCS Logging src/pocomsg.mc)

# pcre
POCO_SOURCES(SRCS pcre
  src/pcre_config.c
  src/pcre_byte_order.c
  src/pcre_chartables.c
  src/pcre_compile.c
  src/pcre_exec.c
  src/pcre_fullinfo.c
  src/pcre_globals.c
  src/pcre_maketables.c
  src/pcre_newline.c
  src/pcre_ord2utf8.c
  src/pcre_study.c
  src/pcre_tables.c
  src/pcre_dfa_exec.c
  src/pcre_get.c
  src/pcre_jit_compile.c
  src/pcre_refcount.c
  src/pcre_string_utils.c
  src/pcre_version.c
  src/pcre_ucd.c
  src/pcre_valid_utf8.c
  src/pcre_xclass.c)

# zlib
POCO_HEADERS(SRCS zlib
  include/Poco/zconf.h
  include/Poco/zlib.h)

POCO_SOURCES(SRCS zlib
  src/adler32.c
  src/compress.c
  src/crc32.c
  src/deflate.c
  src/infback.c
  src/inffast.c
  src/inflate.c
  src/inftrees.c
  src/trees.c
  src/zutil.c)

# Create static Library
add_library(Foundation STATIC 
  ${SRCS})

add_library(Poco::Foundation ALIAS Foundation)

set_target_properties(Foundation PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(Foundation
  PRIVATE
    kargatum-dependency-interface)

target_compile_definitions(Foundation
  PUBLIC
    # $<$<CONFIG:Debug>:_DEBUG>
    POCO_ENABLE_CPP14)

target_compile_features(Foundation
  PUBLIC
    cxx_defaulted_move_initializers)
  
if(WIN32)
  target_compile_definitions(Foundation PUBLIC POCO_NO_UNWINDOWS POCO_NO_AUTOMATIC_LIBS)
  target_link_libraries(Foundation PRIVATE iphlpapi)
endif()

target_include_directories(Foundation
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(Foundation
  PUBLIC
    POCO_STATIC)
