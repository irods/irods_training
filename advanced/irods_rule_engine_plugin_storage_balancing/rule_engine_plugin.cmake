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
  "${CMAKE_CURRENT_SOURCE_DIR}/src/libirods_rule_engine_plugin-cpp-storage-balancing.cpp"
  )

target_include_directories(
  ${PLUGIN}
  PRIVATE
  "${IRODS_EXTERNALS_FULLPATH_BOOST}/include"
  "${IRODS_EXTERNALS_FULLPATH_FMT}/include"
  "${IRODS_EXTERNALS_FULLPATH_SPDLOG}/include"
  )

target_link_libraries(
  ${PLUGIN}
  PRIVATE
  irods_server
  irods_common
  "${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_coroutine.so"
  "${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so"
  "${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so"
  "${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_regex.so"
  "${IRODS_EXTERNALS_FULLPATH_FMT}/lib/libfmt.so"
  ${CMAKE_DL_LIBS}
  )

target_compile_definitions(
  ${PLUGIN}
  PRIVATE
  ${IRODS_COMPILE_DEFINITIONS_PRIVATE}
  ENABLE_RE
  IRODS_ENABLE_SYSLOG
)

set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

install(
  TARGETS
  ${PLUGIN}
  LIBRARY
  DESTINATION "${IRODS_PLUGINS_DIRECTORY}/rule_engines"
  )
