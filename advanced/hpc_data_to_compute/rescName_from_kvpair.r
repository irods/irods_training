rrule {
  *out3 = ""
  *out2 = ""
  #*in1 = "COMPUTE_RESOURCE_ROLE=IMAGE_PROCESSING"
  *in1 = "COMPUTE_RESOURCE_ROLE=LONG_TERM_STORAGE"
  pyParseRoleSpec( *in1, *out2, *out3 )
  writeLine("stdout", "[ *out2 ] : [ *out3 ]" )
  if ((*out2 != "") && (*out3 != "")) {
     *r = ""
     get_resource_name_by_role ( *r , *out2, *out3 )
     writeLine("stdout", "[[ *r ]]" )
  }
}

INPUT null
OUTPUT ruleExecOut
