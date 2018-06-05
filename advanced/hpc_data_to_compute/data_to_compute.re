get_resource_name_by_role(*resc_name, *attr, *value) 
{
     *resc_name =  "NULL"   # -- *resc_name = "demoResc" # old D-to-C example
     *out1 = *attr
     *out2 = *value
     if (*value == "") { # assume *attr is of form "KEY=VALUE"
       pyParseRoleSpec ( *attr, *out1, *out2 ) # see ~irods/compute/util.py
     }
 
     foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*out1' AND 
                     META_RESC_ATTR_VALUE = '*out2') {
         *resc_name = *row.DATA_RESC_NAME
     } # foreach
}

get_host_and_resource_name_by_role (*host, *resc_name, *attr, *value) 
{
    *resc_name = "NULL"    # -- *resc_name = "NULL" # old D-to-C example
    *out1=*attr
    *out2=*value
    if (*value == "") { # assume *attr is of form "KEY=VALUE"
      pyParseRoleSpec ( *attr, *out1, *out2 ) # see ~irods/compute/util.py
    }

    foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*out1' AND 
                    META_RESC_ATTR_VALUE = '*out2') {
        *resc_name = *row.DATA_RESC_NAME
    }

    foreach (*h in SELECT RESC_LOC WHERE DATA_RESC_NAME = '*resc_name' )
    {
      *host = *h.RESC_LOC;
    }
}
