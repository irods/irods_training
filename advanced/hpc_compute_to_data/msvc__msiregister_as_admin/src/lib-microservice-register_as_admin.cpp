// =-=-=-=-=-=-=-
// irods includes
#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"

// system library includes

#include <cstring>
#include <cstdlib>
#include <string>
#include <cstring>
#include <regex.h>
#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

// iRODS includes

#include "reDataObjOpr.hpp"
#include "apiHeaderAll.h"
#include "rsApiHandler.hpp"
#include "collection.hpp"
#include "rsDataObjCreate.hpp"
#include "rsDataObjOpen.hpp"
#include "rsDataObjClose.hpp"
#include "rsDataObjRead.hpp"
#include "rsDataObjWrite.hpp"
#include "rsDataObjUnlink.hpp"
#include "rsDataObjRepl.hpp"
#include "rsDataObjCopy.hpp"
#include "rsDataObjChksum.hpp"
#include "rsDataObjLseek.hpp"
#include "rsDataObjPhymv.hpp"
#include "rsDataObjRename.hpp"
#include "rsDataObjTrim.hpp"
#include "rsCollCreate.hpp"
#include "rsRmColl.hpp"
#include "rsPhyPathReg.hpp"
#include "rsObjStat.hpp"
#include "rsDataObjRsync.hpp"
#include "rsOpenCollection.hpp"
#include "rsReadCollection.hpp"
#include "rsCloseCollection.hpp"
#include "rsExecCmd.hpp"
#include "rsCollRepl.hpp"
#include "rsModDataObjMeta.hpp"
#include "rsStructFileExtAndReg.hpp"
#include "rsModDataObjMeta.hpp"
#include "rsStructFileBundle.hpp"
#include "rsPhyBundleColl.hpp"
  
/* --------------dwm--------------- */

int
msiregister_as_admin( msParam_t *inpParam1, msParam_t *inpParam2,
                msParam_t *inpParam3, msParam_t *inpParam4, msParam_t *outParam,
                ruleExecInfo_t *rei ) {
     rsComm_t *rsComm;
     dataObjInp_t dataObjInp, *myDataObjInp = NULL;
  
     RE_TEST_MACRO( "    Calling msiPhyPathReg" )
  
     if ( rei == NULL || rei->rsComm == NULL ) {
         rodsLog( LOG_ERROR,
                  "msiPhyPathReg: input rei or rsComm is NULL" );
         return SYS_INTERNAL_NULL_INPUT_ERR;
     }
  
     rsComm = rei->rsComm;
  
     /* parse inpParam1 */
     rei->status = parseMspForDataObjInp( inpParam1, &dataObjInp,
                                          &myDataObjInp, 1 );
  
     if ( rei->status < 0 ) {
         rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                             "msiPhyPathReg: input inpParam1 error. status = %d", rei->status );
         return rei->status;
     }
  
     if ( ( rei->status = parseMspForCondInp( inpParam2, &myDataObjInp->condInput,
                          DEST_RESC_NAME_KW ) ) < 0 ) {
         rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                             "msiPhyPathReg: input inpParam2 error. status = %d", rei->status );
         return rei->status;
     }
  
     if ( ( rei->status = parseMspForCondInp( inpParam3, &myDataObjInp->condInput,
                          FILE_PATH_KW ) ) < 0 ) {
         rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                             "msiPhyPathReg: input inpParam3 error. status = %d", rei->status );
         return rei->status;
     }
  
     if ( ( rei->status = parseMspForPhyPathReg( inpParam4,
                          &myDataObjInp->condInput ) ) < 0 ) {
         rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                             "msiPhyPathReg: input inpParam4 error. status = %d", rei->status );
         return rei->status;
     }
  
     // elevate privilege level

     int temp = rei->rsComm->clientUser.authInfo.authFlag;
     rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

     rei->status = rsPhyPathReg( rsComm, myDataObjInp );
  
     // restore old privilege level
     rsComm->clientUser.authInfo.authFlag = temp;

     if ( myDataObjInp == &dataObjInp ) {
         clearKeyVal( &myDataObjInp->condInput );
     }
  
     if ( rei->status >= 0 ) {
         fillIntInMsParam( outParam, rei->status );
     }
     else {
         rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                             "msiPhyPathReg: rsPhyPathReg failed for %s, status = %d",
                             myDataObjInp->objPath,
                             rei->status );
     }
  
     return rei->status;
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(5);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        msParam_t*,
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiregister_as_admin",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiregister_as_admin));
    return msvc;
}

