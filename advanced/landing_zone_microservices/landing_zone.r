ingest_rule {
    #delay( "<PLUSET>30s</PLUSET><EF>5m REPEAT FOR EVER</EF>" ) {
    #    remote( "resource1.example.org", "null" ) {
            *err = errorcode( msiget_filepaths_from_glob( *lz_glob, *delay_seconds, *file_age_seconds, *the_files ) );
            if( *err == 0 ) {
                foreach( *f in *the_files ) {
                    writeLine( "stdout", "processing file=[*f]" );
                    *err = errorcode( msiput_dataobj_or_coll( *f, *resource_name, "forceFlag=", *tgt_coll, *real_path ) );
                    if( *err==0 ) {
                        if (*f like "*.jpg" || *f like "*.jpeg" || *f like "*.bmp" || 
                            *f like "*.tif" || *f like "*.tiff" || *f like "*.rif" || 
                            *f like "*.gif" || *f like "*.png"  || *f like "*.svg" || 
                            *f like "*.xpm") {
                                
                            writeLine("stdout", "gathering metadata for [*f]");
                            msiget_image_meta(*f, *meta);
                            
                            writeLine("stdout", "image metadata: " ++ *meta);
                            msiString2KeyValPair(*meta, *meta_kvp);

                            msiAssociateKeyValuePairsToObj(*meta_kvp, *real_path, "-d");
                        }

                        writeLine( "stdout", "processed file=[*f]" );
                        *err = errorcode( msifilesystem_rename( *f, *lz_raw, *lz_proc ) );
                        writeLine( "stdout", "move [*f], from [*lz_raw], to [*lz_proc]" ); 

                    } # if err

                } # foreach

            } # if err
            else {
                writeLine( "stdout", "error in msiget_filepaths_from_glob - *err" );

            }
    #   } # remote
    #} # delay
} # ingest_rule

INPUT *lz_raw="/tmp/landing_zone/new", *lz_proc="/tmp/landing_zone/processed/", *resource_name="demoResc", *delay_seconds=1, *lz_glob="/tmp/landing_zone/new/*", *tgt_coll="/tempZone/home/rods/", *file_age_seconds=1
OUTPUT ruleExecOut

