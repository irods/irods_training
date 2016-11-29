find_thumbnails {
    *thb_list = list()

    *err = errormsg(get_list_of_thumbnails(*src_obj_path, *thb_list), *msg)
    if(0 != *err) {
        writeLine( "stdout", "FAIL: [*err] [*msg]")
    }

    foreach( *t in *thb_list ) {
        writeLine("stdout", "thumbnail [*t]")
    }

}

INPUT *src_obj_path="/tempZone/home/rods/stickers.jpg"
OUTPUT ruleExecOut
