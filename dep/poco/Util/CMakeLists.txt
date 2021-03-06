#
# This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

# Sources
file(GLOB SRCS_G "src/*.cpp")
POCO_SOURCES_AUTO( SRCS ${SRCS_G})

# Headers
file(GLOB_RECURSE HDRS_G "include/*.h" )
POCO_HEADERS_AUTO( SRCS ${HDRS_G})

POCO_SOURCES_AUTO_PLAT(SRCS WIN32
  src/WinRegistryConfiguration.cpp
  src/WinRegistryKey.cpp
  src/WinService.cpp)

add_library(Util ${SRCS})
add_library(Poco::Util ALIAS Util)

target_link_libraries(Util PUBLIC Poco::Foundation)
target_compile_definitions(Util 
  PUBLIC 
    POCO_UTIL_NO_XMLCONFIGURATION
    POCO_UTIL_NO_JSONCONFIGURATION)

target_include_directories(Util
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Util
  PRIVATE
    warhead-dependency-interface)
