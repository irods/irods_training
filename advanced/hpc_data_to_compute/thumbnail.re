
################################
# service
split_path(*p,*tok,*col,*obj) {
    *col = trimr(*p, *tok)
    *obj = substr(*p, strlen(*col)+1, strlen(*p)) 
}


get_resource_name_by_role(*resc_name, *attr, *value) {
    *lts_name = "NULL"
    foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*attr' AND META_RESC_ATTR_VALUE = '*value') {
        *resc_name = *row.DATA_RESC_NAME
    } # foreach
}

get_resource_name_by_id(
    *resc_id,
    *resc_name) {
    *resc_name = "NULL"
    foreach(*row in SELECT RESC_NAME WHERE RESC_ID = '*resc_id') {
        *resc_name = *row.RESC_NAME
    } # foreach
}

get_resource_id_by_name(
    *resc_name,
    *resc_id) {
    *resc_id = "NULL"

    foreach(*row in SELECT RESC_ID WHERE RESC_NAME = '*resc_name') {
        *resc_id = *row.RESC_ID
    } # foreach
}

get_resource_host_by_id(
    *resc_id,
    *resc_host) {
    *resc_host = "NULL"
    foreach(*row in SELECT RESC_LOC WHERE RESC_ID = '*resc_id') {
        *resc_host = *row.RESC_LOC
    } # foreach
}

get_resc_id_for_data_object_reside_on_image_node(
    *obj_name,
    *col_name,
    *compute_resc_role_attr,
    *image_compute_type,
    *src_resc_id) {
    *src_resc_id = "NULL"

    #*image_resc_id = "NOT_FOUND"
    foreach(*row in SELECT DATA_RESC_ID WHERE DATA_NAME = '*obj_name' AND COLL_NAME = '*col_name') {
        *id = *row.DATA_RESC_ID
        foreach(*v in SELECT META_RESC_ATTR_VALUE WHERE RESC_ID = '*id' and META_RESC_ATTR_NAME = '*compute_resc_role_attr' ) {
            if(*image_compute_type == *v.META_RESC_ATTR_VALUE) {
                *src_resc_id = *id
                break
            }
        } # values
    } # out_ids
}

get_phy_path_for_object_on_resc_id(
    *obj_name,
    *resc_id,
    *phy_path ) {
    *phy_path = "NULL"
    #writeLine("serverLog", "XXXX - obj_name [*obj_name] resc_id [*resc_id]")
    foreach(*row in SELECT DATA_PATH WHERE DATA_NAME = '*obj_name' AND RESC_ID = '*resc_id') {
        *phy_path = *row.DATA_PATH;
    }
}

replicate_object_to_image_node(
    *src_obj_path,
    *compute_resc_role_attr,
    *image_compute_type,
    *img_resc_name,
    *src_resc_id ) {
    get_resource_name_by_role(
	    *img_resc_name,
	    *compute_resc_role_attr,
	    *image_compute_type);
    if("NULL" == *img_resc_name) {
        failmsg(-1,"get_resource_name_by_role failed [*lts_resc_name][*compute_resc_role_attr][*image_compute_type]")
    }

    # "Take the Data to the Compute" - replicate to an image compute node
    *out_param = 0
    *err = errormsg(msiDataObjRepl(
		   *src_obj_path,
		   "destRescName=*img_resc_name",
		   *out_param), *msg)
    if(0 != *err) {
        failmsg(*err, "msiDataObjRepl failed for [*src_obj_path] [*img_resc_name] - [*out_param]")
    }

    *src_resc_id = "NULL"
    # set the src resc id to the new image compute node id
    get_resource_id_by_name(*img_resc_name, *src_resc_id)
}

register_and_replicate_thumbnail(
    *server_host,
    *obj_path,
    *src_resc_name,
    *phy_path,
    *dst_resc_name) {

    delay( "<EF>5s REPEAT UNTIL SUCCESS</EF>") {
	remote(*server_host, "") {
	    *long_term_resource = "demoResc"

	    writeLine("serverLog", "register_and_replicate_thumbnail :: [*obj_path] [*src_resc_name] [*phy_path] [*dst_resc_name]");
	    *err = errormsg(msiPhyPathReg(*obj_path, *src_resc_name, *phy_path, "null", *status), *msg);
	    if(0 != *err) {
		failmsg(*err, "msiPhyPathReg failed for [*obj_path] [*src_resc_name] [*phy_path] [*status]")
	    }

	    *err = errormsg(msiDataObjRepl(
		       *obj_path,
		       "destRescName=*dst_resc_name",
		       *out_param), *msg)
	    if(0 != *err) {
		failmsg(*err, "msiDataObjRepl failed for [*obj_path] [*dst_resc_name] - [*out_param]")
	    }

            *err = errormsg(msiDataObjUnlink(
                       "objPath=*obj_path++++replNum=0++++unreg=",
                       *out_param), *msg)
	    if(0 != *err) {
		failmsg(*err, "msiDataObjUnlink failed for [*obj_path] [*out_param]")
	    }
	} # remote
    } 
}

create_thumbnail_impl(
    *src_obj_path,
    *dst_obj_path,
    *dst_phy_path,
    *size_str) {

    split_path(*src_obj_path, "/", *col_name, *obj_name)

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

    *src_resc_id = "NULL"
    get_resc_id_for_data_object_reside_on_image_node(
	*obj_name,
	*col_name,
	*compute_resc_role_attr,
	*image_compute_type,
	*src_resc_id)
    writeLine("serverLog", "src_resc_id [*src_resc_id]")

    *img_resc_name = "NULL"
    # does data object not reside on the required resource?
    if("NULL" == *src_resc_id) {
        replicate_object_to_image_node(
            *src_obj_path,
            *compute_resc_role_attr,
            *image_compute_type,
            *img_resc_name,
            *src_resc_id )
	if("NULL" == *src_resc_id) {
	    failmsg(-1, "get_resource_id_by_name failed for [*img_resc_name]")
	}
    }
    writeLine("serverLog", "src_resc_id [*src_resc_id]")

    *src_resc_name = "NULL"
    get_resource_name_by_id(*src_resc_id, *src_resc_name)
    if("NULL" == *src_resc_name) {
	failmsg(-1,"get_resource_name_by_id failed for [*src_resc_id]")
    }
    writeLine("serverLog", "src_resc_name [*src_resc_name]")

    *src_phy_path = "NULL"
    get_phy_path_for_object_on_resc_id(*obj_name, *src_resc_id, *src_phy_path)
    if("NULL" == *src_phy_path) {
	failmsg(-1,"get_phy_path_for_object_on_resc_id failed for [*obj_name] [*src_resc_id]")
    }
    writeLine("serverLog", "src_phy_path [*src_phy_path]")

    *server_host = "NULL"
    get_resource_host_by_id(*src_resc_id, *server_host);
    if("NULL" == *server_host) {
	failmsg(-1,"get_resource_host_by_id failed for [*src_resc_id]")
    }
    writeLine("serverLog", "server_host [*server_host]")

    # launch image computation job
    submit_thumbnail_job(
        *server_host,
        *size_str,
        *src_phy_path,
        *dst_phy_path)

    # launch registration and replication 
    register_and_replicate_thumbnail(
               *server_host,
	       *dst_obj_path,
	       *src_resc_name,
	       *dst_phy_path,
	       *lts_resc_name);

    
}

get_list_of_thumbnails(
    *src_obj_path,
    *thumbnail_list ) {

    *thumbnail_list = list()
    split_path(*src_obj_path, "/", *col_name, *obj_name)

    # derive a collection name from the logical path
    *thumb_coll_name = "NULL"
    get_thumbnail_collection_name(*col_name, *obj_name, *thumb_coll_name);
    #writeLine( "serverLog", "XXXX - get_list_of_thumbnails :: thumb_coll_name [*thumb_coll_name]" )

    # get the list of possible sizes
    get_thumbnail_sizes(*thumb_sizes)
    foreach( *sz in *thumb_sizes ) {
        get_thumbnail_name(*obj_name, *sz, *thumbnail_name);
        *dst_obj_path = *thumb_coll_name ++ "/" ++ *thumbnail_name
        #writeLine( "serverLog", "XXXX - get_list_of_thumbnails :: [*src_obj_path] [*sz] [*thumbnail_name] [*dst_obj_path]" )

        # does the thumbnail exist
        *err = errormsg(msiObjStat(*dst_obj_path,*obj_stat), *msg);
        if( 0 == *err ) {
            # it does exist, add it to the list
            *thumbnail_list = cons(*dst_obj_path, *thumbnail_list)
        }
    }
}


create_thumbnail(
    *src_obj_path,
    *dst_obj_path,
    *dst_phy_path,
    *size_str) {
    *err = errormsg(msiObjStat(*dst_obj_path,*obj_stat), *msg);
    if(0 == *err) {
        writeLine("serverLog", "Thumbnail exists for [*dst_obj_path]")
    }
    else {
        writeLine( "serverLog", "Create Thumbnail [*src_obj_path] [*dst_obj_path] [*size_str]" )
        create_thumbnail_impl(
            *src_obj_path,
            *dst_obj_path,
            *dst_phy_path,
            *size_str );
    }
} # thumbnail

create_thumbnail_collection(
    *src_obj_path,
    *dst_phy_dir ) {

    split_path(*src_obj_path, "/", *col_name, *obj_name)

    *thumb_coll_name = "NULL"
    get_thumbnail_collection_name(*col_name, *obj_name, *thumb_coll_name);
    #writeLine( "serverLog", "XXXX - thumb_coll_name [*thumb_coll_name]" )

    *err = errormsg(msiCollCreate(*thumb_coll_name, 1, *out), *msg)
    if( *err < 0 ) {
        writeLine("serverLog", "msiCollCreate failed: [*err] [*msg] [*out]")
        failmsg(*err, *msg)
    }

    get_thumbnail_sizes(*thumb_sizes)
    foreach( *sz in *thumb_sizes ) {
        get_thumbnail_name(*obj_name, *sz, *thumbnail_name);
        *dst_obj_path = *thumb_coll_name ++ "/" ++ *thumbnail_name

        *dst_phy_path = "NULL"
        get_thumbnail_physical_path(*dst_phy_dir, *thumbnail_name, *dst_phy_path)

        create_thumbnail(
            *src_obj_path,
            *dst_obj_path,
            *dst_phy_path,
            *sz)
    }
}



