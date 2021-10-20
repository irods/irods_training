set(
  PLUGIN
  ${CMAKE_PROJECT_NAME}
  )

set(
  IRODS_COMPILE_DEFINITIONS_${PLUGIN}
  RODS_SERVER
  ENABLE_RE
  )

add_library(
  ${PLUGIN}
  MODULE
  ${CMAKE_SOURCE_DIR}/src/libirods_rule_engine_plugin-cpp-storage-balancing.cpp
  )

target_include_directories(
  ${PLUGIN}
  PRIVATE
  ${IRODS_INCLUDE_DIRS}
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
  ${IRODS_EXTERNALS_FULLPATH_JANSSON}/include
  ${IRODS_EXTERNALS_FULLPATH_JSON}/include
  )

target_link_libraries(
  ${PLUGIN}
  PRIVATE
  irods_server
  irods_common
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_coroutine.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
  )

target_compile_definitions(${PLUGIN} PRIVATE ${IRODS_COMPILE_DEFINITIONS_${PLUGIN}} ${IRODS_COMPILE_DEFINITIONS} BOOST_SYSTEM_NO_DEPRECATED)
set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

install(
  TARGETS
  ${PLUGIN}
  LIBRARY
  DESTINATION usr/lib/irods/plugins/rule_engines
  )
