# --- 
# allow normal rodsuser to register files into iRODS

acSetChkFilePathPerm { msiSetChkFilePathPerm("noChkPathPerm"); }

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
    *image_flag = false;
    object_is_image_type(*_obj_path, *image_flag)
    *resc_name = "demoResc" 
    if(true == *image_flag) {
        *resc_name = "img_resc"
    }
    msiSetDefaultResc(*resc_name,"preferred");
    *obj_path = *_obj_path
    writeLine("serverLog", "[*resc_name] was preferred for [[ *obj_path ]]. ")
}

acSetRescSchemeForCreate {
  determine_destination_resource($objPath)
}

rmtExec_singularity( *host , *arg ) 
{
  *user = ""
  get_irods_username (*user)
  
  remote("*host", "") {
    writeLine("serverLog", "rmtExec - host [*host] remote cmd [*arg] by [*user] ")
    #msiExecCmd("hello","*arg *arg2","null","null","null",*OUT)
    #remotePythonService ( *arg )
  }
}


