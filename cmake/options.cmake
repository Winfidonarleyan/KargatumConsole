#
# Copyright (C) 
#

option(WITH_DYNAMIC_LINKING "Enable dynamic library linking."                         0)
option(WITH_WARNINGS    "Show all warnings during compile"                            0)

if (WITH_DYNAMIC_LINKING)
  set(BUILD_SHARED_LIBS ON)
else()
  set(BUILD_SHARED_LIBS OFF)
endif()

set(WITH_SOURCE_TREE    "hierarchical" CACHE STRING "Build the source tree for IDE's.")
set_property(CACHE WITH_SOURCE_TREE PROPERTY STRINGS no flat hierarchical hierarchical-folders)
