


acPostProcForPut() {
    if("ufs_cache" == $rescName ) {
        writeLine( "serverLog", "XXXX - calling delayed replication" );
        delay("<PLUSET>1s</PLUSET><EF>1h DOUBLE UNTIL SUCCESS OR 6 TIMES</EF>") {
            *CacheRescName = "comp_resc;ufs_cache";
            msisync_to_archive("*CacheRescName", $filePath, $objPath );
        }
    }
}

pep_resource_resolve_hierarchy_pre(
  *INST_NAME,*CTX,*OUT,*OP_TYPE,*HOST,*RESC_HIER,*VOTE){

    # only influence CREATE operations
    if( "CREATE" == *OP_TYPE ) {

       writeLine( "serverLog", "XXXX - pep_resource_resolve_hierarchy_pre 2 [*INST_NAME]");

       foreach ( *ROW in SELECT RESC_TYPE_NAME WHERE RESC_NAME = '*INST_NAME' ) {
           *RESC_TYPE = *ROW.RESC_TYPE_NAME;
       }
       if( "passthru" == *RESC_TYPE ) {
          *HYP_BYTES_USED = double(*CTX.file_size);

          # add up bytes used by all of the resource's children
          foreach ( *ROW in SELECT RESC_ID WHERE RESC_NAME = '*INST_NAME' ) {
            *INST_ID = int(*ROW.RESC_ID);
          }
          foreach ( *ROW1 in SELECT RESC_NAME WHERE RESC_PARENT = '*INST_ID' ) {
             *STORAGE_RESC = *ROW1.RESC_NAME;
             foreach ( *ROW2 in SELECT sum(DATA_SIZE) WHERE RESC_NAME = '*STORAGE_RESC' ) {
               *HYP_BYTES_USED = *HYP_BYTES_USED + double(*ROW2.DATA_SIZE);
             }
          }

          # if no max_bytes context string, assume infinite capacity
          # 0 is a do-not-write
          *MAX_BYTES = -1;
          foreach(*ROW in SELECT RESC_CONTEXT WHERE RESC_NAME = '*INST_NAME'){
            *CONTEXT_STRING = *ROW.RESC_CONTEXT;
          }
          foreach( *KVP_STRING in split( *CONTEXT_STRING, ";" ) ) {
            *KVP = split( *KVP_STRING, "=" );
            if( "max_bytes" == elem( *KVP, 0 )) {
              *MAX_BYTES = double(elem(*KVP,1));
            }
          }

          # compute percent full
          if( -1 == *MAX_BYTES ) {
              *HYP_PERCENT_FULL = 0;
          } else if( 0 == *MAX_BYTES ) {
              *HYP_PERCENT_FULL = 1;
          } else {
              *HYP_PERCENT_FULL = *HYP_BYTES_USED / *MAX_BYTES;
          }
          *WRITE_WEIGHT = 1 - *HYP_PERCENT_FULL;

          writeLine( "serverLog", "XXXX - [*INST_NAME] - [*WRITE_WEIGHT]");

          *WEIGHT_STRING = "read=1.0;write=*WRITE_WEIGHT";
          *OUT = *WEIGHT_STRING;

      } # if( "passthru"

   } # if( "CREATE"

} # pep

pep_resource_resolve_hierarchy_pre(
  *INST_NAME,*CTX,*OUT,*OP_TYPE,*HOST,*RESC_HIER,*VOTE){

    writeLine( "serverLog", "XXXX - pep_resource_resolve_hierarchy_pre 1 [*INST_NAME]");

    if( "CREATE" == *OP_TYPE ) {
        if( "pt1" == *INST_NAME) {
            *OUT = "read=1.0;write=0.5"
        }
        else if ( "pt2" == *INST_NAME ) {
            *OUT = "read=1.0;write=1.0"
        }
    }
}

add_metadata_to_objpath(*str, *objpath, *objtype) {
    msiString2KeyValPair(*str, *kvp);
    msiAssociateKeyValuePairsToObj(*kvp, *objpath, *objtype);
}











