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

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DWARHEAD_API_EXPORT_COMMON)

add_library(common
  ${PRIVATE_SOURCES})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

target_include_directories(common
  PUBLIC
    ${CMAKE_BINARY_DIR}
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(common
  PRIVATE
    warhead-core-interface
  PUBLIC
    Poco::Foundation
    boost
    fmt
    indicators::indicators
    # json
    openssl
    utf8cpp
    WarheadRegex)

add_dependencies(common revision_data.h)

set_target_properties(common
  PROPERTIES
    FOLDER
      "src")

if(BUILD_SHARED_LIBS)
  if(UNIX)
    install(TARGETS common
      LIBRARY
        DESTINATION lib)
  elseif(WIN32)
    install(TARGETS common
      RUNTIME
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()
