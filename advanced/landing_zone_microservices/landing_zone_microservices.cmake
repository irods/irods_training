set(
  MICROSERVICES
  msifilesystem_rename
  msiget_filepaths_from_glob
  msiget_image_meta
  msiput_dataobj_or_coll
  )

set(
  IRODS_COMPILE_DEFINITIONS_LANDING_ZONE
  RODS_SERVER
  ENABLE_RE
  )

foreach(MICROSERVICE ${MICROSERVICES})
  add_library(
    ${MICROSERVICE}
    MODULE
    ${CMAKE_SOURCE_DIR}/${MICROSERVICE}/lib${MICROSERVICE}.cpp
    )

  target_include_directories(
    ${MICROSERVICE}
    PRIVATE
    #/usr/include/ImageMagick
    ${IRODS_INCLUDE_DIRS}
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
    ${IRODS_EXTERNALS_FULLPATH_JANSSON}/include
    )

  target_link_libraries(
    ${MICROSERVICE}
    PRIVATE
    irods_server
    irods_client
    irods_common
    #/usr/local/lib/libMagick++-7.Q16HDRI.so.0.0.0
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so
    ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
    )

  target_compile_definitions(${MICROSERVICE} PRIVATE ${IRODS_COMPILE_DEFINITIONS_LANDING_ZONE} ${IRODS_COMPILE_DEFINITIONS} BOOST_SYSTEM_NO_DEPRECATED)
  set_property(TARGET ${MICROSERVICE} PROPERTY CXX_STANDARD ${IRODS_CXX_STANDARD})

  install(
    TARGETS
    ${MICROSERVICE}
    LIBRARY
    DESTINATION usr/lib/irods/plugins/microservices
    )
endforeach()

install(
  FILES
  ${CMAKE_SOURCE_DIR}/landing_zone.r
  DESTINATION var/lib/irods
  )
