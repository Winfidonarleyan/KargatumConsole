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
# User has manually chosen to ignore the git-tests, so throw them a warning.
# This is done EACH compile so they can be alerted about the consequences.
#

# basic packagesearching and setup
# (further support will be needed, this is a preliminary release!)
set(OPENSSL_EXPECTED_VERSION 1.0.0)

find_package(OpenSSL REQUIRED)

add_library(openssl INTERFACE)

target_link_libraries(openssl
  INTERFACE
    ${OPENSSL_LIBRARIES})

target_include_directories(openssl
  INTERFACE
    ${OPENSSL_INCLUDE_DIR})