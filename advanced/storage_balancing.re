pep_resource_resolve_hierarchy_pre(*INST_NAME,*CTX,*OUT,*OP_TYPE,*HOST,*RESC_HIER,*VOTE){
   writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - status message :: [*INST_NAME] [*CTX] [*OUT] [*OP_TYPE] [*HOST] [*RESC_HIER] [*VOTE]");
   # TODO: check to apply only to passthru resource
   if( "CREATE" == *OP_TYPE ) {
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

          # if no max_bytes context string, assume infinite capacity. 0 is a do-not-write
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
          if( -1 == *MAX_BYTES ) {
              *HYP_PERCENT_FULL = 0;
          } else if( 0 == *MAX_BYTES ) {
              *HYP_PERCENT_FULL = 1;
          } else {
              *HYP_PERCENT_FULL = *HYP_BYTES_USED / *MAX_BYTES;
          }
          *WRITE_WEIGHT = 1 - *HYP_PERCENT_FULL;

          *SOURCE_LPATH = ""; 
          *THIS_LPATH = *CTX.logical_path;
          if(0 != errorcode(*MD = *CTX.metadataIncluded)) {
            *MD = "";
          }
          *THIS_METADATA = split(*MD, ";");
          for( *I = 0; *I<size(*THIS_METADATA); *I = *I + 3) {
            *ATTR = elem( *THIS_METADATA, *I );
            writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - attr *ATTR in sidecar metadata");
            if( "source_data_path" == *ATTR ) {
              *SOURCE_LPATH = elem(*THIS_METADATA,*I + 1);
              writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - found source_data_path *SOURCE_LPATH in sidecar metadata");
            }
          }
          
          if( strlen(*SOURCE_LPATH) == 0 ) {
             foreach( *ROW in SELECT META_DATA_ATTR_VALUE WHERE DATA_NAME = '*THIS_LPATH' AND META_DATA_ATTR_NAME = "source_data_path" ) {
                *SOURCE_LPATH = *ROW.META_DATA_ATTR_VALUE;
                writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - found source_data_path *SOURCE_LPATH in catalog metadata");
             }
          }

          *SOURCE_RESC = "";
          if( strlen(*SOURCE_LPATH) > 0 ) {
             *SOURCE_COLL_NAME = trimr(*SOURCE_LPATH,"/");
             *COLL_LEN = strlen(*SOURCE_COLL_NAME) + 1;
             *SOURCE_DATA_NAME = substr(*SOURCE_LPATH,*COLL_LEN,strlen(*SOURCE_LPATH));
             writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - looking for source_resc for *SOURCE_COLL_NAME and *SOURCE_DATA_NAME");
             foreach( *ROW in SELECT RESC_NAME WHERE DATA_NAME = '*SOURCE_DATA_NAME' AND COLL_NAME = '*SOURCE_COLL_NAME') {
                *SOURCE_RESC = *ROW.RESC_NAME;
                if( *STORAGE_RESC == *SOURCE_RESC ) {
                   writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - downgrading write_weight from *WRITE_WEIGHT");
                   *WRITE_WEIGHT = *WRITE_WEIGHT * 0.1; #strong preference against writing to the same resource
                }
             }
          }
          writeLine("serverLog", "pep_resource_resolve_hierarchy_pre - resource *INST_NAME applying write_weight *WRITE_WEIGHT");
          *WEIGHT_STRING = "read=1.0;write=*WRITE_WEIGHT";
          *OUT = *WEIGHT_STRING;
      } # if( "passthru"
   } # if( "CREATE"
} # pep
