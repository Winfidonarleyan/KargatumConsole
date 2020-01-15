#
#
#

# We require CMake >= 3.2
cmake_minimum_required(VERSION 3.15)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Set projectname (must be done AFTER setting configurationtypes)
project(KargatumConsole)

# CMake policies (can not be handled elsewhere)
cmake_policy(SET CMP0005 NEW)
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW) # Disable 'Ignore COMPILE_DEFINITIONS_<Config> properties'
endif()

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted - prevents intepreting if (SOME_STRING_VARIABLE MATCHES "MSVC") as if (SOME_STRING_VARIABLE MATCHES "1")
endif()

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW) # find_package() uses <PackageName>_ROOT variables
endif()

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

# find_package(PCHSupport)
find_package(Git)
find_package(MySQL REQUIRED)

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)

# print out the results before continuing
include(cmake/showoptions.cmake)

# find packages
find_package(Threads REQUIRED)

# add dependencies
add_subdirectory(dep)

# add core sources
add_subdirectory(src)
