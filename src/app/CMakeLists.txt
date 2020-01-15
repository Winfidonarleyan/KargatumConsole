#
#
#

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES)

if( WIN32 )
  list(APPEND PRIVATE_SOURCES ${sources_windows})
  if ( MSVC )
    list(APPEND PRIVATE_SOURCES kargatum.rc)
  endif()
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(kargatum
  ${PRIVATE_SOURCES})

target_link_libraries(kargatum
  PRIVATE
    kargatum-core-interface
  PUBLIC
    common)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

target_include_directories(kargatum
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(kargatum
  PROPERTIES
    FOLDER
      "app")

if( WIN32 )
  if ( "${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild" )
    add_custom_command(TARGET kargatum
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Config.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif ( MINGW )
    add_custom_command(TARGET kargatum
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Config.conf.dist ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()

if( WIN32 )
  install(TARGETS kargatum DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES Config.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
