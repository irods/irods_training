set(
  IRODS_API_PLUGIN_SOURCES_api_example_server
  ${CMAKE_SOURCE_DIR}/src/libapi-example.cpp
  )

set(
  IRODS_API_PLUGIN_SOURCES_api_example_client
  ${CMAKE_SOURCE_DIR}/src/libapi-example.cpp
  )

set(
  IRODS_API_PLUGIN_COMPILE_DEFINITIONS_api_example_server
  RODS_SERVER
  ENABLE_RE
  )

set(
  IRODS_API_PLUGIN_COMPILE_DEFINITIONS_api_example_client
  )

set(
  IRODS_API_PLUGIN_LINK_LIBRARIES_api_example_server
  irods_client
  irods_server
  irods_common
  irods_plugin_dependencies
  )

set(
  IRODS_API_PLUGIN_LINK_LIBRARIES_api_example_client
  irods_client
  irods_common
  irods_plugin_dependencies
  )

set(
  IRODS_API_PLUGINS
  api_example_server
  api_example_client
  )

foreach(PLUGIN ${IRODS_API_PLUGINS})
  add_library(
    ${PLUGIN}
    MODULE
    ${IRODS_API_PLUGIN_SOURCES_${PLUGIN}}
    )

  target_include_directories(
    ${PLUGIN}
    PRIVATE
    ${IRODS_INCLUDE_DIRS}
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
    ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/include
    )

  target_link_libraries(
    ${PLUGIN}
    PRIVATE
    ${IRODS_API_PLUGIN_LINK_LIBRARIES_${PLUGIN}}
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
    ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/lib/libarchive.so
    ${OPENSSL_CRYPTO_LIBRARY}
    )

  target_compile_definitions(${PLUGIN} PRIVATE ${IRODS_API_PLUGIN_COMPILE_DEFINITIONS_${PLUGIN}} ${IRODS_COMPILE_DEFINITIONS} BOOST_SYSTEM_NO_DEPRECATED)
  target_compile_options(${PLUGIN} PRIVATE -Wno-write-strings)
  set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

  install(
    TARGETS
    ${PLUGIN}
    LIBRARY
    DESTINATION usr/lib/irods/plugins/api
    )
endforeach()
