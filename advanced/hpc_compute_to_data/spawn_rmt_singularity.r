testRule 
{
  *thumbnail_sizes = "128x128,256x256,512x512,1024x1024" 
  *user = ""
  get_irods_username( *user )

  *host = ""
  *key = "COMPUTE_RESOURCE_ROLE"; *val="IMAGE_PROCESSING"

  *resc_name = ""
  get_host_and_resource_name_by_role(*host, *resc_name, *key, *val) 

  *key = "COMPUTE_RESOURCE_ROLE"; *val_LTS="LONG_TERM_STORAGE"
  get_resource_name_by_role( *long_term_storage_resc, *key, *val_LTS )

  writeLine ("stdout", "host=[*host] resc=[*resc_name]")
  writeLine ("stdout", "thumbnails to generate : [ *thumbnail_sizes ]")

  *input_file_name = "stickers"
  *input_file_ext = ".jpg"
  *input_file =  "*input_file_name" ++ "*input_file_ext"

  if ("*host" == "" || "*user" == "") {

    writeLine ("stdout", "Host or user for job launch was not found.")

  } else {

    *src_phy_path = "null"

    foreach (*x in select DATA_PATH where 
    # COLL_NAME = '/tempZone/home/*user' and
      DATA_NAME = '*input_file' and RESC_NAME = '*resc_name') 
    {
      *src_phy_path = *x.DATA_PATH
    }

    *src_phy_dir = trimr ("*src_phy_path", "/")

    remote(*host, "") {

      *tmpdir = "/tmp/irods/thumbnails"

      foreach (*size in split (*thumbnail_sizes, ",")) {

        *output_file =  "*input_file_name" ++ "_*size" ++ "*input_file_ext"

        *container_opts= "--bind *src_phy_dir:/src --bind *tmpdir:/dst " ++ 
                         " /usr/bin/python3 /make_thumbnail.py " ++
                         " *size *input_file *output_file /tempZone/home/*user "

        *postproc_opts = " --outdir *tmpdir --size *size " ++ "--postproc '" ++
                         "-P *passwd -u *user -r *resc_name -f *long_term_storage_resc " ++
                         "-p *tmpdir/stickers_*size.jpg " ++
                         "-m *tmpdir/" ++ "*size" ++ "_mdmanifest.json " ++ "'"

        msiExecCmd("submit_SLURM_job.sh",
                   "/var/lib/irods/msiExecCmd_bin/singularity.SLURM" ++
                   " *postproc_opts exec thumbnail_image *container_opts ",
                   "null","null","null",*OUT)

      } #foreach

    } #remote

  } #if-else

} # end rule

INPUT  *passwd=$"-"
OUTPUT ruleExecOut
