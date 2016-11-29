create_thumbnails {

    *err = errormsg(create_thumbnail_collection(*src_obj_path, *dst_phy_dir), *msg)
    if(0 != *err) {
        writeLine( "stdout", "FAIL: [*err] [*msg]")
    }
}

INPUT *src_obj_path="/tempZone/home/rods/stickers.jpg",*dst_phy_dir="/tmp/irods/thumbnails"
OUTPUT ruleExecOut

