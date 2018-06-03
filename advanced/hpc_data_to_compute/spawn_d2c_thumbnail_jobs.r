find_thumbnails {

    *thb_sizes = list("128x128", "256x256", "512x512", "1024x1024")

    #*err = errormsg(get_list_of_thumbnails(*src_obj_path, *thb_list), *msg)
    #if(0 != *err) { writeLine( "stdout", "FAIL: [*err] [*msg]") }

    foreach( *t in *thb_sizes ) {
        writeLine("stdout", "thumbnail size -> [*t]")
    }

}

INPUT null 
OUTPUT ruleExecOut
