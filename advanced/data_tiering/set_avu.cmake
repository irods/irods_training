set(
  IRODS_SET_AVU_PLUGIN_COMPILE_DEFINITIONS
  RODS_SERVER
  ENABLE_RE
  )

set(
  PLUGIN
  msiset_avu
  )

  add_library(
      ${PLUGIN}
      MODULE
      ${CMAKE_SOURCE_DIR}/src/msiset_avu.cpp
      )

  target_include_directories(
      ${PLUGIN}
      PRIVATE
      /usr/include/irods
      ${IRODS_EXTERNALS_FULLPATH_QPID}/include
      ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
      ${IRODS_EXTERNALS_FULLPATH_JANSSON}/include
      ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/include
      )

  target_link_libraries(
      ${PLUGIN}
      PRIVATE
      irods_server
      irods_common
      ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so
      ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
      ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/lib/libarchive.so
      ${OPENSSL_CRYPTO_LIBRARY}
      )

  target_compile_definitions(${PLUGIN} PRIVATE ${IRODS_SET_AVU_PLUGIN_COMPILE_DEFINITIONS} ${IRODS_COMPILE_DEFINITIONS} BOOST_SYSTEM_NO_DEPRECATED)
  target_compile_options(${PLUGIN} PRIVATE -Wno-write-strings)
  set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

install(
  TARGETS
  ${PLUGIN}
  LIBRARY
    DESTINATION usr/lib/irods/plugins/microservices
  )


