

object_is_image_type(*_fn, *_flag) {
    *_flag = false;
    if (*_fn like "*.jpg" || *_fn like "*.jpeg" || *_fn like "*.bmp" ||
	*_fn like "*.tif" || *_fn like "*.tiff" || *_fn like "*.rif" ||
	*_fn like "*.gif" || *_fn like "*.png"  || *_fn like "*.svg" ||
	*_fn like "*.xpm") {
        *_flag = true;
    }
}


determine_destination_resource(*_obj_path) {
    *comp_attr = "NULL"
    get_compute_resource_role_attribute(*comp_attr);

    *image_flag = false;
    object_is_image_type(*_obj_path, *image_flag)

    *resc_name = "lts_resc" # discover LTS resc
    if(true == *image_flag) {
        *image_type = "NULL"
        get_image_compute_type(*image_type)

        get_resource_name_by_role(*resc_name, *comp_attr, *image_type)
    }

    msiSetDefaultResc(*resc_name,"forced");
}


acSetRescSchemeForCreate{
    determine_destination_resource($objPath)
}

