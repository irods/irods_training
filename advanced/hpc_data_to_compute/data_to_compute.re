
get_resource_name_by_role(*resc_name, *attr, *value) {

    *resc_name = "demoResc"    # -- *resc_name = "NULL" # old D-to-C example
    if (*value == "") { # assume *attr is of form "KEY=VALUE"
      *out1 = ""
      pyParseRoleSpec ( *attr, *out1, *out2 ) # see ~irods/compute/util.py
    }
    foreach(*row in SELECT DATA_RESC_NAME WHERE META_RESC_ATTR_NAME = '*attr' AND 
                    META_RESC_ATTR_VALUE = '*value') {
        *resc_name = *row.DATA_RESC_NAME
    } # foreach
}

rmtExec_slurm ( *host , *arg ) 
{
  
  remote("*host", "") {
    writeLine("serverLog", "rmtExec - host [*host] remote cmd [*arg] ")
    #msiExecCmd("hello","*arg","null","null","null",*OUT)
    #remotePythonService ( *arg )
  }
}


