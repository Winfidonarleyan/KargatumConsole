#
# Copyright (C) 
#

include(CheckSymbolExists)

set(strtod_l_headers stdlib.h)

if(WIN32)
  check_symbol_exists(open io.h HAVE_OPEN)
  check_symbol_exists(_strtod_l "${strtod_l_headers}" HAVE_STRTOD_L)
endif()

function(add_headers VAR)
  set(headers ${${VAR}})
  foreach (header ${ARGN})
    set(headers ${headers} fmt/${header})
  endforeach()
  set(${VAR} ${headers} PARENT_SCOPE)
endfunction()

# Define the fmt library, its includes and the needed defines.
add_headers(FMT_HEADERS 
  chrono.h 
  color.h 
  compile.h 
  core.h 
  format.h 
  format-inl.h
  locale.h 
  ostream.h 
  printf.h 
  ranges.h
  safe-duration-cast.h)

set(FMT_SOURCES fmt/format.cc)

if(HAVE_OPEN)
  add_headers(FMT_HEADERS posix.h)
  set(FMT_SOURCES ${FMT_SOURCES} fmt/posix.cc)
endif()

add_library(fmt STATIC 
  ${FMT_SOURCES}
  ${FMT_HEADERS})

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

if (HAVE_STRTOD_L)
  target_compile_definitions(fmt 
    PUBLIC 
      FMT_LOCALE)
endif()

target_include_directories(fmt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(fmt
  PRIVATE
    kargatum-dependency-interface)

set_target_properties(fmt
  PROPERTIES
    FOLDER
      "dep")
