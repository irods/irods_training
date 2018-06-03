mainRule {
  *key = "COMPUTE_RESOURCE_ROLE = IMAGE_PROCESSING"
  *hostname = ""
  *resc_name = ""
#
  get_host_and_resource_name_by_role(*hostname, *resc_name, *key, *value) 
  #*out1 = ""; *out2 = "" ; pyParseRoleSpec ( *key, *out1, *out2 )
  writeLine ("stdout", "resc->[*resc_name] host->[*hostname] ")
}

get_host_and_resource_name_by_role (*host, *resc_name, *attr, *value) {

    *resc_name = "demoResc"    # -- *resc_name = "NULL" # old D-to-C example
    *value = ""

    if (*value == "") { # assume *attr is of form "KEY=VALUE"
      *out1=""
      *out2=""
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
