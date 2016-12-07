//#include "reFuncDefs.h"
#include "irods_ms_plugin.hpp"
#include "objInfo.h"
#include "reDataObjOpr.hpp"
#include "collCreate.h"
#include "dataObjPut.h"
#include "collection.hpp"
#include "rcMisc.h"


#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;



void strip_trailing_slash(
    std::string& _path ) {
    if( *_path.rbegin() == '/' )
        _path = _path.substr( 0, _path.size()-1 );

}

std::string get_logical_path(
    const std::string& _full_path,
    const std::string& _root_path,
    const std::string& _tgt_coll ) {

    std::string logical_path;
    fs::path fp = fs::canonical( fs::path( _full_path ) );
    fs::path rp = fs::canonical( fs::path( _root_path ) );

    // ensure target collection ends in an irods
    // separator
    std::string tg = _tgt_coll;
    if( *tg.rbegin() != '/' )
        tg += "/";

    logical_path = tg + fp.filename().string();
    if( !_root_path.empty() ) {
        std::string::size_type pos = _full_path.find( _root_path );
        if( std::string::npos != pos ) {
           // need the +1 to skip first slash in root path
           logical_path = tg + _full_path.substr( pos+_root_path.size()+1 );

        }

    }

    strip_trailing_slash( logical_path );

    rodsLog(
        LOG_DEBUG,
        "get_logical_path :: [%s]",
        logical_path.c_str() );

    return logical_path;

} // get_logical_path



irods::error put_a_file(
    rcComm_t*          _comm,
    const std::string& _file_path,
    const std::string& _logical_path,
    const std::string& _resc,
    const std::string& _opts ) {

    rodsLog(
        LOG_DEBUG,
        "put_a_file :: put [%s] to [%s]",
        _file_path.c_str(),
        _logical_path.c_str() );

    dataObjInp_t inp;
    memset( &inp, 0, sizeof( inp ) );
    strncpy(
        inp.objPath,
        _logical_path.c_str(),
        MAX_NAME_LEN );

    int status = rcDataObjPut(
                     _comm,
                     &inp,
                     (char*)_file_path.c_str() );
    if( status < 0 ) {
        std::string msg( "rcDataObjPut failed for [" );
        msg += _file_path;
        msg += "]";
        return ERROR( status, msg );
    }

    return SUCCESS();

} // put_a_file




irods::error put_all_the_files(
    rcComm_t*          _comm,
    const fs::path&    _path,
    const std::string& _root,
    const std::string& _resc,
    const std::string& _opts,
    const std::string& _tgt_coll,
    std::string&       _out_path ) {
    irods::error final_error = SUCCESS();

    try {
        if ( fs::is_directory( _path ) ) {
            // create a matching collection
            std::string logical_path = get_logical_path(
                                           _path.string(),
                                           _root,
                                           _tgt_coll );
            rodsLog(
                LOG_DEBUG,
                "put_all_the_files :: create coll [%s]",
                logical_path.c_str() );

            collInp_t coll_inp;
            memset( &coll_inp, 0, sizeof( coll_inp ) );
            strncpy( coll_inp.collName, logical_path.c_str(), MAX_NAME_LEN );
            int status = rcCollCreate( _comm, &coll_inp );
            if( status < 0 && status != CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME ) {
                rodsLog(
                    LOG_ERROR,
                    "put_all_the_files :: rsCollCreate failed for [%s] with [%d]",
                    logical_path.c_str(),
                    status );

            } else {
                fs::directory_iterator end_iter;
                fs::directory_iterator dir_itr( _path );
                for( ;
                     dir_itr != end_iter;
                     ++dir_itr ) {

                    // recurse on this new directory
                    irods::error ret = put_all_the_files(
                                           _comm,
                                           dir_itr->path(),
                                           _root,
                                           _resc,
                                           _opts,
                                           _tgt_coll,
                                           _out_path );
                    if ( !ret.ok() ) {
                        std::stringstream msg;
                        msg << "failed on [";
                        msg << dir_itr->path().string();
                        msg << "]";
                        final_error = PASSMSG( msg.str(), final_error );
                    }

                } // for dir_itr

            } // else

        }
        else if ( fs::is_regular_file( _path ) ) {
            std::string logical_path = get_logical_path(
                                           _path.string(),
                                           _root,
                                           _tgt_coll );
            irods::error ret = put_a_file(
                                   _comm,
                                   _path.string(),
                                   logical_path,
                                   _resc,
                                   _opts );
            if( !ret.ok() ) {
                return PASS( ret );

            }

        }
        else {
            std::stringstream msg;
            msg << "unhandled entry [";
            msg << _path.filename();
            msg << "]";
            rodsLog( LOG_NOTICE, msg.str().c_str() );
        }

    }
    catch ( const std::exception & ex ) {
        std::stringstream msg;
        msg << "caught exception [";
        msg << ex.what();
        msg << "] for directory entry [";
        msg << _path.filename();
        msg << "]";
        return ERROR( -1, msg.str() );

    }

    _out_path = get_logical_path(
                    _path.string(),
                    _root,
                    _tgt_coll );
    return final_error;

} // put_all_the_files



int msiput_dataobj_or_coll(
    msParam_t* _path,
    msParam_t* _resc,
    msParam_t* _opts,
    msParam_t* _tgt_coll,
    msParam_t* _out_path,
    ruleExecInfo_t* rei) {

    char* path = parseMspForStr(_path);
    if(!path) {
        rodsLog(LOG_ERROR, "%s null _path", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    char* resc = parseMspForStr(_resc);
    if(!resc) {
        rodsLog(LOG_ERROR, "%s null _resc", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    char* opts = parseMspForStr(_opts);
    if(!opts) {
        rodsLog(LOG_ERROR, "%s null _opts", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    char* tgt_coll = parseMspForStr(_tgt_coll);
    if(!tgt_coll) {
        rodsLog(LOG_ERROR, "%s null _tgt_coll", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    std::string file_name;
    fs::path inp_path( path );
    if( fs::is_directory( inp_path ) ) {
        file_name = fs::canonical( inp_path ).parent_path( ).string();

    }

    std::string tmp_coll( tgt_coll );
    strip_trailing_slash( tmp_coll );
    int status = rsMkCollR(
                     rei->rsComm,
                     "/",
                     tmp_coll.c_str() );
    if( status < 0 ) {
        rodsLog(
            LOG_ERROR,
            "msiput_dataobj_or_coll - failed to make collection [%s]",
            _tgt_coll );
        return status;

    }

    rcComm_t* comm = rcConnect(
                     rei->rsComm->myEnv.rodsHost,
                     rei->rsComm->myEnv.rodsPort,
                     rei->rsComm->myEnv.rodsUserName,
                     rei->rsComm->myEnv.rodsZone,
                     NO_RECONN, 0 );
    if( !comm ) {
        rodsLog(
            LOG_ERROR,
            "msiput_dataobj_or_coll - rcConnect failed" );
        return 0;
    }

    status = clientLogin(
                 comm,
                 0,
                 rei->rsComm->myEnv.rodsAuthScheme );
    if ( status != 0 ) {
        rcDisconnect( comm );
        rodsLog(
            LOG_ERROR,
            "msiput_dataobj_or_coll - client login failed %d",
            status );
        return 0;
    }

    std::string out_path;
    irods::error ret = put_all_the_files(
                           comm,
                           inp_path,
                           file_name,
                           resc,
                           opts,
                           tgt_coll,
                           out_path );

    rcDisconnect( comm );

    if( !ret.ok() ) {
        addRErrorMsg(
            &rei->rsComm->rError,
            ret.code(),
            ret.result().c_str() );
    }

    fillMsParam( _out_path, NULL, STR_MS_T, (void*)out_path.c_str(), NULL );

    return ret.code();

}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(5);
    msvc->add_operation(
        "msiput_dataobj_or_coll",
        std::function<int(
            msParam_t*,
            msParam_t*,
            msParam_t*,
            msParam_t*,
            msParam_t*,
            ruleExecInfo_t*)>(msiput_dataobj_or_coll));
    return msvc;
}
