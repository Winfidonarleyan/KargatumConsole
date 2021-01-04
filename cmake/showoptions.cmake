# output generic information about the core and buildtype chosen
message("")
message("* WH Console revision    : ${rev_hash} ${rev_date} (${rev_branch} branch)")
message("")

# Show infomation about the options selected during configuration
if( WITH_WARNINGS )
  message("* Show all warnings      : Yes")
else()
  message("* Show compile-warnings  : No  (default)")
endif()

if( NOT WITH_SOURCE_TREE STREQUAL "no" )
  message("* Show source tree       : Yes (${WITH_SOURCE_TREE})")
else()
  message("* Show source tree       : No")
endif()

if (BUILD_SHARED_LIBS)
  message("")
  message(" *** WITH_DYNAMIC_LINKING - INFO!")
  message(" *** Will link against shared libraries!")
  message(" *** Please note that this is an experimental feature!")
  add_definitions(-DKARGATUM_API_USE_DYNAMIC_LINKING)
endif()

message("")
