testRule 
{
  *thumbnail_sizes = "128x128,256x256,512x512,1024x1024" 
  *host = ""
  *key = "COMPUTE_RESOURCE_ROLE"; *val="IMAGE_PROCESSING"
  # or even: # *key = "COMPUTE_RESOURCE_ROLE=IMAGE_PROCESSING"; *val=""
  *resc_name = ""
  get_host_and_resource_name_by_role(*host, *resc_name, *key, *val) 
  writeLine ("stdout", "host=[*host] resc=[*resc_name]")
  writeLine ("stdout", "thumbnails to generate : [ *thumbnail_sizes ]")
  *input_file_name = "stickers"
  *input_file_ext = ".jpg"
  *input_file =  "*input_file_name" ++ "*input_file_ext"
  if ("*host" == "" ) {
    writeLine ("stdout", "Host for job launch was not found.")
  } else {
    foreach (*x in select DATA_PATH where COLL_NAME = '/tempZone/home/rods' and
                   DATA_NAME = '*input_file' and RESC_NAME = '*resc_name') 
    {
     *src_phy_path = *x.DATA_PATH
    }
    remote(*host, "") {
      writeLine("serverLog", "-----> remoteExec on host [*host]:")
      foreach (*size in split (*thumbnail_sizes, ",")) {
        *dst_phy_path = "/tmp/irods/thumbnails/" ++ "*input_file_name" ++ "_thumbnail_" ++ "*size" ++ "*input_file_ext"
        writeLine("serverLog"," - thumbsize [ *size ]; convert ( *src_phy_path , *dst_phy_path )")
        *cmd_opts="/var/lib/irods/msiExecCmd_bin/convert.SLURM -thumbnail *size *src_phy_path *dst_phy_path"
        msiExecCmd("submit_thumbnail_job.sh","*cmd_opts","null","null","null",*OUT)
      } #foreach
    } #remote
  } #if-else
} # end rule

INPUT  null
OUTPUT ruleExecOut
