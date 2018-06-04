testRule 
{
  *hostname = "null"
  #---
  *key = "COMPUTE_RESOURCE_ROLE = IMAGE_PROCESSING" ; *val = ""
  writeLine ("stdout", "key->[*key]  val->[*val] ")
  *resc_name = ""
  get_host_and_resource_name_by_role(*hostname, *resc_name, *key, *val) 
  writeLine ("stdout", "resc->[*resc_name] host->[*hostname] ")
  #---
  *key = "COMPUTE_RESOURCE_ROLE"; *val = "IMAGE_PROCESSING"
  writeLine ("stdout", "OLD VSN key->[*key]  val->[*val] ")
  *resc_name = ""
  get_resource_name_by_role(*resc_name, *key, *val) 
  writeLine ("stdout", "resc->[*resc_name] ")
}

###############################################################################
# OLDER VERSION
# get_resource_name_by_role(*resc_name, *attr, *value) {
# 
#     *resc_name =  "NULL"   # -- *resc_name = "demoResc" # old D-to-C example
#     #*value = ""
#     *out1 = *attr
#     *out2 = *value
#     if (*value == "") { # assume *attr is of form "KEY=VALUE"
#       pyParseRoleSpec ( *attr, *out1, *out2 ) # see ~irods/compute/util.py
#     }
# 
#     foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*out1' AND 
#                     META_RESC_ATTR_VALUE = '*out2') {
#         *resc_name = *row.DATA_RESC_NAME
#     } # foreach
# }
# 
###############################################################################
# NEWER VERSION
get_host_and_resource_name_by_role (*host, *resc_name, *attr, *value) {

    *resc_name = "NULL"    # -- *resc_name = "NULL" # old D-to-C example
    #*value = ""

    *out1=*attr
    *out2=*value
    if (*value == "") { # assume *attr is of form "KEY=VALUE"
      pyParseRoleSpec ( *attr, *out1, *out2 ) # see ~irods/compute/util.py
      writeLine ("stdout", " [ *out1 ] -> [ *out2 ] ")
    }

    foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*out1' AND 
                    META_RESC_ATTR_VALUE = '*out2') {
        *resc_name = *row.DATA_RESC_NAME
    } # foreach

    foreach (*h in SELECT RESC_LOC WHERE DATA_RESC_NAME = '*resc_name' )
    {
      *host = *h.RESC_LOC;
    }
}

INPUT null
OUTPUT ruleExecOut
