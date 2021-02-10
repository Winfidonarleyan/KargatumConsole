#
# This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

# Require a min version Cmake
cmake_minimum_required(VERSION 3.16...3.19)

message(STATUS "CMake version: ${CMAKE_VERSION}")

# CMake policies (can not be handled elsewhere)
cmake_policy(SET CMP0005 NEW)

# Set projectname (must be done AFTER setting configurationtypes)
project(WarheadApps)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 0)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# set macro-directory
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros")

include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

# set default buildoptions and print them
include(cmake/options.cmake)

include(ConfigureBaseTargets)
include(CheckPlatform)
include(GroupSources)
include(AutoCollect)
include(PocoMacros)
include(GenerateApp)

find_package(Git)

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)

# print out the results before continuing
include(cmake/showoptions.cmake)

# find packages
find_package(Threads REQUIRED)

# add dependencies
add_subdirectory(dep)

# add sources
add_subdirectory(src)
