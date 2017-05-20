
launch_thumbnail_compute(
    *src_obj_path ) {


    # TODO - ensure image is on image compute resource




    split_path(*src_obj_path, "/", *col_name, *obj_name)



    *thumb_coll_name = "NULL"
    get_thumbnail_collection_name(*col_name, *obj_name, *thumb_coll_name);

    *err = errormsg(msiCollCreate(*thumb_coll_name, 1, *out), *msg)
    if( *err < 0 ) {
        writeLine("serverLog", "msiCollCreate failed: [*err] [*msg] [*out]")
        failmsg(*err, *msg)
    }

    *guid_str = "NULL"
    msiget_uuid(*guid_str)
    *dst_dir_name = "/tmp/" ++ *obj_name ++ "-" ++ *guid

    # capture configuration parameters
    *image_compute_type = "NULL"
    get_image_compute_type(*image_compute_type)
    if("NULL" == *image_compute_type) {
	    failmsg(-1,"get_image_compute_type failed")
    }
    writeLine("serverLog", "image_compute_type [*image_compute_type]")

    *lts_compute_type = "NULL"
    get_long_term_storage_type(*lts_compute_type)
    if("NULL" == *lts_compute_type) {
	    failmsg(-1,"get_long_term_storage_type failed")
    }
    writeLine("serverLog", "lts_compute_type [*lts_compute_type]")

    *compute_resc_role_attr = "NULL"
    get_compute_resource_role_attribute(*compute_resc_role_attr)
    if("NULL" == *compute_resc_role_attr) {
	    failmsg(-1,"get_compute_resource_role_attribute failed")
    }
    writeLine("serverLog", "compute_resc_role_attr [*compute_resc_role_attr]")

    *lts_resc_name = "NULL"
    get_resource_name_by_role(
	    *lts_resc_name,
	    *compute_resc_role_attr,
	    *lts_compute_type);
    if("NULL" == *lts_resc_name) {
	    failmsg(-1,"get_resource_name_by_role failed [*lts_resc_name][*compute_resc_role_attr][*lts_compute_type]")
    }
    writeLine("serverLog", "lts_resc_name [*lts_resc_name]")

    writeLine("serverLog", "********** obj_name [*obj_name] col_name [*col_name] attr [*compute_resc_role_attr] type [*image_compute_type]")
    *src_resc_id = "NULL"
    get_resc_id_for_data_object_reside_on_image_node(
	    *obj_name,
	    *col_name,
	    *compute_resc_role_attr,
	    *image_compute_type,
	    *src_resc_id)

    *src_phy_path = "NULL"
    get_phy_path_for_object_on_resc_id(*obj_name, *src_resc_id, *src_phy_path)
    if("NULL" == *src_phy_path) {
	failmsg(-1,"get_phy_path_for_object_on_resc_id failed for [*obj_name] [*src_resc_id]")
    }
    split_path(*src_phy_path, "/", *src_dir_name, *src_file_name)
    writeLine("serverLog", "src_phy_path [*src_phy_path] src_dir_name [*src_dir_name] src_file_name [*src_file_name]")

    *server_host = "NULL"
    get_resource_host_by_id(*src_resc_id, *server_host);
    if("NULL" == *server_host) {
	failmsg(-1,"get_resource_host_by_id failed for [*src_resc_id]")
    }
    writeLine("serverLog", "server_host [*server_host]")

    get_thumbnail_sizes(*thumb_sizes)
    foreach( *sz in *thumb_sizes ) {
        get_thumbnail_name(*obj_name, *sz, *thumbnail_name);
        *dst_obj_path = *thumb_coll_name ++ "/" ++ *thumbnail_name
writeLine( "serverLog", "XXXX - [*src_obj_path] [*sz] [*thumbnail_name] [*dst_obj_path]" )

        *sz_str = str(*sz)
        *docker_options = "-v " ++ *src_dir_name ++ ":/src -v " ++ *dst_dir_name 
        *docker_options = *docker_options ++ ":/dst -e SIZE=" ++ *sz_str
        *docker_options = *docker_options ++ " -e SOURCE_IMAGE=" ++ *src_file_name 
        *docker_options = *docker_options ++ " -e DESTINATION_IMAGE=" ++ *thumbnail_name

writeLine( "serverLog", "XXXX - docker_options [*docker_options]")

        launch_compute_container(
            *server_host,
            *guid_str,
            *dst_dir_name,
            *dst_obj_path,
            *thumbnail,
            *docker_options)

    } # for

} # launch_thumbnail_compute

launch_compute_container(
    *host_name,
    *guid_str,
    *src_phy_path,
    *dst_log_path,
    *containter_name,
    *user_docker_options) {

    # following the template method pattern we call the
    # container then call moarlock to handle the results
    # of the processing.  Moarlock provides a contract to
    # register data as well as apply metadata contained
    # in a json file.  We knit the two together based on
    # options passed to the containers

    # conventions:
    #    source location : /var/input

    # thumbnail run string
    # docker run -v /home/ubuntu/irods_training/:/src -v /tmp:/dst -e SIZE=128 -e SOURCE_IMAGE=stickers.jpg -e DESTINATION_IMAGE=stickers_thumbnail128.jpg thumbnail

    # moarlock run string
    # docker run -rm -v irodsabspathtoparent:/var/input -e "host=xxx" -e "port=xxx" -e "zone=xxx" -e "user=xxx" -e "passwd=xxxx" -e "irodsout=yyy" -e "parm1=xxx" -e "parm2=xxx" -e "parm3=xxx" -e "parm4=xxx" -e "guid=xxx" diceunc/moarlock

    # irodsout - abs logical path to a collection
    # irodsabspathtoparent - local path to directory where data is at rest

    remote(*host_name, "null") {
        # possible future pre-processing here

        # build source and destination mounts for user container
        *src_mnt = "-v " + *src_phy_path + ":/var/input"
 
        *dst_path = "/tmp/"+*guid_str
        errormsg(msiExecCmd("mkdir -p ", *dst_path, "null", "null", "null", *std_out_err), *msg);

        *dst_mnt = "-v " + *dst_path + ":/dst"


        # build the full docker option string
        *cmd_opt = *options + " " + *_container_name

  
        # call the users provided container
        *msg = ""
        errormsg(msiExecCmd("docker run", *cmd_opt, "null", "null", "null", *std_out_err), *msg);

        # post-processing with  Moarlock

    } # remote
} # launch_compute_container

