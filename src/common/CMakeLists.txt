#
# Copyright (C)
#

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DKARGATUM_API_EXPORT_COMMON)

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
    kargatum-core-interface
  PUBLIC
    fmt
    Poco::Foundation)

add_dependencies(common revision_data.h)

set_target_properties(common
  PROPERTIES
    FOLDER
      "src")

if( BUILD_SHARED_LIBS )
  if( UNIX )
    install(TARGETS common
      LIBRARY
        DESTINATION lib)
  elseif( WIN32 )
    install(TARGETS common
      RUNTIME
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()
