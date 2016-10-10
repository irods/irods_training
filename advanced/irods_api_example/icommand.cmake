add_executable(
  iapi_example
  ${CMAKE_SOURCE_DIR}/src/iapi_example.cpp
  )
target_link_libraries(
  iapi_example
  PRIVATE
  irods_client
  irods_common
  irods_plugin_dependencies
  ${IRODS_EXTERNALS_FULLPATH_AVRO}/lib/libavrocpp.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_program_options.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
  ${IRODS_EXTERNALS_FULLPATH_JANSSON}/lib/libjansson.so
  ${IRODS_EXTERNALS_FULLPATH_ZMQ}/lib/libzmq.so
  )
target_include_directories(
  iapi_example
  PRIVATE
  ${IRODS_INCLUDE_DIRS}
  ${IRODS_EXTERNALS_FULLPATH_AVRO}/include
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
  ${IRODS_EXTERNALS_FULLPATH_JANSSON}/include
  ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/include
  ${IRODS_EXTERNALS_FULLPATH_CPPZMQ}/include
  ${IRODS_EXTERNALS_FULLPATH_ZMQ}/include
  )
target_compile_definitions(iapi_example PRIVATE RODS_SERVER ${IRODS_COMPILE_DEFINITIONS} BOOST_SYSTEM_NO_DEPRECATED)
target_compile_options(iapi_example PRIVATE -Wno-write-strings)
set_property(TARGET iapi_example PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

install(
  TARGETS
  iapi_example
  RUNTIME
  DESTINATION usr/bin
  )
