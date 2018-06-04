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

rmtExec_slurm ( *host , *arg ) 
{
  remote("*host", "") {
    writeLine("serverLog", "rmtExec - host [*host] remote cmd [*arg] ")
    #msiExecCmd("hello","*arg","null","null","null",*OUT)
  }
}



INPUT null
OUTPUT ruleExecOut
