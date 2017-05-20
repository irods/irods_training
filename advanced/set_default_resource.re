

object_is_image_type(*_f, *_flag) {
    *_flag = false;
    if (*_f like "*.jpg" || *_f like "*.jpeg" || *_f like "*.bmp" ||
	*_f like "*.tif" || *_f like "*.tiff" || *_f like "*.rif" ||
	*_f like "*.gif" || *_f like "*.png"  || *_f like "*.svg" ||
	*_f like "*.xpm") {
        *_flag = true;
    }
}


determine_destination_resource(*_obj_path) {
    *comp_attr = "NULL"
    get_compute_resource_role_attribute(*comp_attr);

    *image_flag = false;
    object_is_image_type(*_obj_path, *image_flag)

    *resc_name = "demoResc"
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

