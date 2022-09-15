# output generic information about the core and buildtype chosen
message("*")
message("* WH Console revision    : ${rev_hash} ${rev_date} (${rev_branch} branch)")

if (UNIX)
  message("* WH Console buildtype   : ${CMAKE_BUILD_TYPE}")
endif()

message("*")

message("* Install app to         : ${CMAKE_INSTALL_PREFIX}")
if( UNIX )
  message("* Install libs to        : ${LIBSDIR}")
endif()

message("* Install configs to     : ${CONF_DIR}")
add_definitions(-D_CONF_DIR=$<1:"${CONF_DIR}">)

message("*")

# Show infomation about the options selected during configuration
if( WITH_WARNINGS )
  message("* Show all warnings      : Yes")
else()
  message("* Show all warnings      : No  (default)")
endif()

if(WIN32)
  if(NOT WITH_SOURCE_TREE STREQUAL "no")
    message("* Show source tree       : Yes - \"${WITH_SOURCE_TREE}\"")
  else()
    message("* Show source tree       : No")
  endif()
else()
  message("* Show source tree       : No (For UNIX default)")
endif()

if (BUILD_SHARED_LIBS)
  message("")
  message(" *** WITH_DYNAMIC_LINKING - INFO!")
  message(" *** Will link against shared libraries!")
  message(" *** Please note that this is an experimental feature!")
  add_definitions(-DWARHEAD_API_USE_DYNAMIC_LINKING)
endif()

message("*")
