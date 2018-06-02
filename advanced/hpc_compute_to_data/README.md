
Compute to Data - main steps
------

Data is automatically routed based on rules
   - key on file extension in $ objPath (python: sv['data_object  ']['object_path'] )
   - route operation is simulated in the exercise by a put
   - in the exercise, we also 
      * run a script to  "discover"  the valid data object
      * followed immediately by attaching meta-data to describe job via putting a 'json' config
        that describes the jobs to be done - ie thumbnail sizes
   - third command in the exercise launches custom app container on remote HPC via rmt_exec_container() rule
      * first the client user is determined: here it is 'alice' (python: (client_user)(user_name))
      * remote call to containers launched on HPC, where rods admin acts on behalf of client to
        run two containers:
           1. thumbnail_image
              - docker image run by Singularity
              - filesystem operations only (hence the .json is co-located on the HPC filesystem with the input
                stickers.jpg.
              - generate thumbnail(s) and metadata manifest
           2. metadata_addtags  
              - Singularity container running a proxy-as-user-on-PRC Singularity build
              - picks up a metadata manifest, again in .json, to add the metadata tags to the output
              - register the products (thumbnail images) and attach the metadata from the last step
              - replicate thumbnails to long term storage and trim from HPC scratch storage
