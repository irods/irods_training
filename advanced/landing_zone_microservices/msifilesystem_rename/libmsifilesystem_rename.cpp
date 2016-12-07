#include "objInfo.h"
#include "reDataObjOpr.hpp"
#include "irods_ms_plugin.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

int get_filesystem_path(
    msParam_t*   _full_path,
    msParam_t*   _src,
    msParam_t*   _tgt,
    std::string& _out ) {

    char* fp = parseMspForStr(_full_path);
    if(!fp) {
        rodsLog( LOG_ERROR, "%s - _full_path", __FUNCTION__ );
        return SYS_INVALID_INPUT_PARAM;
    }
    std::string logical_path = fp;

    char* src = parseMspForStr(_src);
    if(!src) {
        rodsLog( LOG_ERROR, "%s - _src", __FUNCTION__ );
        return SYS_INVALID_INPUT_PARAM;
    }
    std::string src_str = src;
    std::string::size_type pos = logical_path.find( src_str );

    if( std::string::npos != pos ) {
        char* tgt = parseMspForStr(_tgt);
        if(!tgt) {
            rodsLog( LOG_ERROR, "%s - _tgt", __FUNCTION__ );
            return SYS_INVALID_INPUT_PARAM;
        }

        logical_path = tgt + logical_path.substr( pos+src_str.size() );

    } else {
        rodsLog(
            LOG_DEBUG,
            "get_filesystem_path :: src dir not found [%s] in [%s]",
            src,
            fp );
    }

    rodsLog(
        LOG_DEBUG,
        "get_filesystem_path :: [%s]",
        logical_path.c_str() );
    _out = logical_path;

    return 0;

} // get_filesystem_path

int rename_msvc(
    msParam_t* _path,
    msParam_t* _src,
    msParam_t* _tgt,
    ruleExecInfo_t*) {

    char* path = parseMspForStr(_path);
    if(!path) {
        rodsLog( LOG_ERROR, "%s - _path", __FUNCTION__ );
        return SYS_INVALID_INPUT_PARAM;
    }

    std::string new_path;
    int ret = get_filesystem_path(
                  _path,
                  _src,
                  _tgt,
                  new_path);
    if(!ret) {
        fs::rename( path, new_path );
    }

    return ret;
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(3);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>(
                "msifilesystem_rename",
                std::function<int(
                    msParam_t*,
                    msParam_t*,
                    msParam_t*,
                    ruleExecInfo_t*)>(rename_msvc));
    return msvc;
}
