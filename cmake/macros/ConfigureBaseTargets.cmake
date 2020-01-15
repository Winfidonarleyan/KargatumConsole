#
# Copyright (C) 
#

add_library(kargatum-compile-option-interface INTERFACE)

# Use -std=c++11 instead of -std=gnu++11
set(CXX_EXTENSIONS OFF)

# Enable support С++17
set(CMAKE_CXX_STANDARD 17)
message(STATUS "Enabled С++17 support")

# An interface library to make the target features available to other targets
add_library(kargatum-feature-interface INTERFACE)

target_compile_features(kargatum-feature-interface
  INTERFACE
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_decltype
    cxx_decltype_auto
    cxx_final
    cxx_lambdas
    cxx_generic_lambdas
    cxx_variadic_templates
    cxx_defaulted_functions
    cxx_nullptr
    cxx_trailing_return_types
    cxx_return_type_deduction)

# An interface library to make the warnings level available to other targets
# This interface taget is set-up through the platform specific script
add_library(kargatum-warning-interface INTERFACE)

# An interface used for all other interfaces
add_library(kargatum-default-interface INTERFACE)
target_link_libraries(kargatum-default-interface
  INTERFACE
    kargatum-compile-option-interface
    kargatum-feature-interface)

# An interface used for silencing all warnings
add_library(kargatum-no-warning-interface INTERFACE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(kargatum-no-warning-interface
    INTERFACE
      /W0)
else()
  target_compile_options(kargatum-no-warning-interface
    INTERFACE
      -w)
endif()

# An interface library to change the default behaviour
# to hide symbols automatically.
add_library(kargatum-hidden-symbols-interface INTERFACE)

# An interface amalgamation which provides the flags and definitions
# used by the dependency targets.
add_library(kargatum-dependency-interface INTERFACE)
target_link_libraries(kargatum-dependency-interface
  INTERFACE
    kargatum-default-interface
    kargatum-no-warning-interface
    kargatum-hidden-symbols-interface)

# An interface amalgamation which provides the flags and definitions
# used by the core targets.
add_library(kargatum-core-interface INTERFACE)
target_link_libraries(kargatum-core-interface
  INTERFACE
    kargatum-default-interface
    kargatum-warning-interface)
