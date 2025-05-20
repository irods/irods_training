cmake_minimum_required(VERSION 3.12...3.18 FATAL_ERROR)
if (POLICY CMP0109)
  # find_program requires permission to execute but not to read
  cmake_policy(SET CMP0109 NEW)
endif()
if (POLICY CMP0121)
  # Detect invalid indices in list()
  cmake_policy(SET CMP0121 NEW)
endif()
if (POLICY CMP0125)
  # Consistent behavior for cache variables managed by find_*()
  cmake_policy(SET CMP0125 NEW)
endif()
if (POLICY CMP0130)
  # Diagnose condition evaluation errors in while()
  cmake_policy(SET CMP0130 NEW)
endif()
if (POLICY CMP0132)
  # Consistent handling of compiler environment variables
  cmake_policy(SET CMP0132 NEW)
endif()
if (POLICY CMP0140)
  # Check validity of return() params
  cmake_policy(SET CMP0140 NEW)
endif()

set(IRODS_MINIMUM_VERSION "4.3.1")
set(IRODS_MAXIMUM_VERSION "6.0.0")
find_package(IRODS "${IRODS_MINIMUM_VERSION}...<${IRODS_MAXIMUM_VERSION}" REQUIRED)

include(IrodsCXXCompiler)

set(CMAKE_CXX_STANDARD ${IRODS_CXX_STANDARD})

include(IrodsCXXCompiler)

project(irods_rule_engine_plugin-cpp-storage-balancing
  LANGUAGES C CXX
)

include(${IRODS_TARGETS_PATH})

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build {Debug, Release}." FORCE)
  message(STATUS "Setting unspecified CMAKE_BUILD_TYPE to '${CMAKE_BUILD_TYPE}'")
endif()

if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/" CACHE STRING "Install root path. Change from \"/\" for run-in-place installations." FORCE)
  message(STATUS "Setting unspecified CMAKE_INSTALL_PREFIX to '${CMAKE_INSTALL_PREFIX}'. This is the correct setting for normal builds.")
endif()

if (NOT CPACK_PACKAGING_INSTALL_PREFIX)
  set(CPACK_PACKAGING_INSTALL_PREFIX "/" CACHE STRING "Package root path. \"/\" is correct for normal package builds.." FORCE)
  message(STATUS "Setting unspecified CPACK_PACKAGING_INSTALL_PREFIX to '${CPACK_PACKAGING_INSTALL_PREFIX}'. This is the correct setting for normal builds.")
endif()

set(CMAKE_EXE_LINKER_FLAGS_INIT "-rdynamic -Wl,--export-dynamic -Wl,--enable-new-dtags -Wl,--as-needed")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-Wl,--enable-new-dtags -Wl,--as-needed -Wl,-z,defs")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
include(IrodsRunpathDefaults)

find_package(nlohmann_json "3.6.1" REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
find_package(Threads REQUIRED)
find_package(fmt "8.1.1" REQUIRED
  HINTS "${IRODS_EXTERNALS_FULLPATH_FMT}")

include(rule_engine_plugin.cmake)

set(CPACK_PACKAGE_FILE_NAME "irods_rule_engine_plugin-cpp-storage-balancing${IRODS_PACKAGE_FILE_NAME_SUFFIX}")

include(IrodsCPackCommon)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENTS_GROUPING IGNORE)
set(CPACK_PACKAGE_VERSION ${IRODS_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${IRODS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${IRODS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${IRODS_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The integrated Rule-Oriented Data System")

set(CPACK_DEB_COMPONENT_INSTALL OFF)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_RPM_COMPONENT_INSTALL OFF)
set(CPACK_RPM_PACKAGE_LICENSE "BSD-3-Clause")
set(CPACK_RPM_PACKAGE_AUTOREQ 0)
set(CPACK_RPM_PACKAGE_AUTOPROV 0)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${CPACK_PACKAGING_INSTALL_PREFIX}${IRODS_PLUGINS_DIRECTORY}")

set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)

set(CPACK_DEBIAN_PACKAGE_NAME "irods_rule_engine_plugin-cpp-storage-balancing")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${IRODS_PACKAGE_DEPENDENCIES_STRING}, irods-runtime (= ${IRODS_VERSION}), irods-server (= ${IRODS_VERSION}), libc6")

set(CPACK_RPM_PACKAGE_NAME "irods_rule_engine_plugin-cpp-storage-balancing")
set(CPACK_RPM_PACKAGE_REQUIRES "${IRODS_PACKAGE_DEPENDENCIES_STRING}, irods-runtime (= ${IRODS_VERSION}), irods-server (= ${IRODS_VERSION}), openssl")

if (NOT CPACK_GENERATOR)
  set(CPACK_GENERATOR ${IRODS_CPACK_GENERATOR} CACHE STRING "CPack generator to use, e.g. {DEB, RPM, TGZ}." FORCE)
  message(STATUS "Setting unspecified CPACK_GENERATOR to ${CPACK_GENERATOR}. This is the correct setting for normal builds.")
endif()

include(CPack)
