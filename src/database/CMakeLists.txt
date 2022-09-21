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

add_definitions(-DWARHEAD_API_EXPORT_DATABASE)

add_library(database
  ${PRIVATE_SOURCES})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

target_include_directories(database
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(database
  PRIVATE
    warhead-core-interface
    mysql
  PUBLIC
    common)

add_dependencies(database revision_data.h)

set_target_properties(database
  PROPERTIES
    FOLDER
      "src")

if(BUILD_SHARED_LIBS)
  if(UNIX)
    install(TARGETS database
      LIBRARY
        DESTINATION lib)
  elseif(WIN32)
    install(TARGETS database
      RUNTIME
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()
