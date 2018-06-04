def create_thumbnails( rule_args, callback, rei ):
  import sys, os
  #sys.path.insert(0,'/var/lib/irods')
  #import compute.common as common
  callback.writeLine ('serverLog', 'calling from PYTHON rule')

#create_thumbnails {
#   *src_obj_path="/tempZone/home/rods/stickers.jpg"
#    *dst_phy_dir="/tmp/irods/thumbnails"
#    *err = errormsg(create_thumbnail_collection(*src_obj_path, *dst_phy_dir), *msg)
#    if(0 != *err) {
#        writeLine( "stdout", "FAIL: [*err] [*msg]")
#    }
#}

INPUT null
OUTPUT ruleExecOut

