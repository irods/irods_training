
################################
# configuration 
get_compute_resource_role_attribute(*t) {
    *t = "COMPUTE_RESOURCE_ROLE"
}

get_image_compute_type(*t) {
    *t = "IMAGE_PROCESSING"
}

get_long_term_resc_type(*t) {
    *t = "LONG_TERM_STORAGE"
}

submit_thumbnail_job(
    *server_host,
    *size_str,
    *src_phy_path,
    *dst_phy_path ) {
    remote(*server_host, "") {
	    *cmd_opt = 'convert -thumbnail *size_str *src_phy_path *dst_phy_path'
	    *err = errormsg(msiExecCmd("submit_job.sh", *cmd_opt, "", "", "", *std_out_err), *msg);
	    if(*err != 0) {
		writeLine( "serverLog", "FAILED: [*cmd_opt] [*err] [*msg]" );
		failmsg(*err,*cmd_opt)
	    }
    } # remote
}

get_thumbnail_collection_name(
    *col_name,
    *obj_name,
    *thumb_coll_name) {
    *fn = trimr(*obj_name, ".")
    *thumb_coll_name = *col_name ++ "/" ++ *fn ++ "_thumbnails"
}

get_thumbnail_physical_path(*dst_dir, *thumb_name, *phy_path) {
    *phy_path = *dst_dir ++ "/" ++ *thumb_name
}

get_thumbnail_name(*file_name, *size, *thumb_name) {
    # triim the extension
    *fn = trimr(*file_name, ".")
    *ext = substr(*file_name, strlen(*fn)+1, strlen(*file_name)) 
    *thumb_name = *fn ++ "_thumbnail_" ++ *size ++ "." ++ *ext
}

get_thumbnail_sizes(*size_list) {
    *size_list = list( "128x128", "256x256", "512x512", "1024x1024" )
    #*size_list = list( "128x128" )
}


