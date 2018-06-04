rrule {
  *out3 = ""
  *out2 = ""
  # -- try:
  #   irule -F rescName_from_kvpair.r "*in1='COMPUTE_RESOURCE_ROLE=LONG_TERM_STORAGE'"
  # -- or to just get 'Default value':
  #   irule -F rescName_from_kvpair.r <<<"" | tail -1 |sed 's/^.*=//'
  #
  # or as a general scheme for use in sh/Bash scripts:
  #
  # find_compute_resc() { irule -F rescName_from_kvpair.r <<<"\"COMPUTE_RESOURCE_ROLE=$1\"" | tail -1 |sed 's/^.*=//'; }
  # # ...
  # resc_name=$(find_compute_resc IMAGE_PROCESSING)
  pyParseRoleSpec( *in1, *out2, *out3 )
  *r = "NULL"
  if ((*out2 != "") && (*out3 != "")) {
     get_resource_name_by_role ( *r , *out2, *out3 )
  }
  if (*r == "NULL") {
    *r="demoResc"
  }
  writeLine("stdout", "*r" )
}

INPUT  *in1=$"COMPUTE_RESOURCE_ROLE=IMAGE_PROCESSING"
OUTPUT ruleExecOut
