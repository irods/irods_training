#include "objInfo.h"
#include "rcMisc.h"
#include "irods_ms_plugin.hpp"
#include <glob.h>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace bfs = boost::filesystem;
namespace bs  = boost::system;
namespace err = boost::system::errc;
typedef boost::unordered_map<std::string, uintmax_t> size_map_t;

bool is_it_done(
    const bfs::path& filepath,
    size_map_t&      sizes ) {
    bs::error_code ec;
    if ( bfs::is_regular_file( filepath, ec ) &&
            ec == err::success &&
            ( sizes.count( filepath.generic_string() ) == 0 ||
              sizes[filepath.generic_string()] != bfs::file_size( filepath ) ) ) {
        return false;
    }
    else if ( bfs::is_directory( filepath, ec ) && ec == err::success ) {
        for ( bfs::directory_iterator iter( filepath ), end_iter; iter != end_iter; iter++ ) {
            if ( !is_it_done( iter->path(), sizes ) ) {
                return false;
            }
        }
    }
    return true;
}

void add_file_sizes(
    const bfs::path& filepath,
    size_map_t&      sizes ) {
    bs::error_code ec;
    if ( bfs::is_regular_file( filepath, ec ) &&
            ec == err::success ) {
        sizes[filepath.generic_string()] = bfs::file_size( filepath );
    }
    else if ( bfs::is_directory( filepath, ec ) && ec == err::success ) {
        for ( bfs::directory_iterator iter( filepath ), end_iter; iter != end_iter; iter++ ) {
            add_file_sizes( iter->path(), sizes );
        }
    }
}

int msiget_filepaths_from_glob(
    msParam_t* _glob_string,      //str
    msParam_t* _seconds_delay,    //int
    msParam_t* _second_proc_wait, //int
    msParam_t* _filepaths,        //kvp
    ruleExecInfo_t* ) {

    char* glob_string = parseMspForStr(_glob_string);
    if(!glob_string) {
        rodsLog(LOG_ERROR, "%s - invalid _glob_string", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    const int seconds_delay = parseMspForPosInt(_seconds_delay);
    if (seconds_delay <= 0) {
        rodsLog(LOG_ERROR, "%s - invalid _seconds_delay", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    const int second_proc_wait = parseMspForPosInt(_seconds_delay);
    if (second_proc_wait <= 0) {
        rodsLog(LOG_ERROR, "%s - invalid _second_proc_wait", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    keyValPair_t* filepaths = (keyValPair_t*)malloc(sizeof(keyValPair_t));
    if (!filepaths) {
        rodsLog(LOG_ERROR, "%s - invalid _filepaths", __FUNCTION__);
        return SYS_INVALID_INPUT_PARAM;
    }

    memset(filepaths,0,sizeof(keyValPair_t));

    glob_t glob_struct;
    glob( glob_string, GLOB_MARK | GLOB_TILDE_CHECK, NULL, &glob_struct );
    
    size_map_t sizes;
    for ( size_t i = 0; i < glob_struct.gl_pathc; i++ ) {
        bfs::path filepath( glob_struct.gl_pathv[i] );
        add_file_sizes( filepath, sizes );
    }
    if ( sizes.empty() ) {
        return 0;
    }

    sleep( seconds_delay );
    for ( size_t i = 0; i < glob_struct.gl_pathc; i++ ) {
        rodsLog(
            LOG_DEBUG,
            "msiget_filepaths_from_glob - processing [%s]",
            glob_struct.gl_pathv[i] );
        bfs::path filepath( glob_struct.gl_pathv[i] );
        if ( is_it_done( filepath, sizes ) ) {
            bs::error_code ec;
            if ( bfs::is_regular_file( filepath, ec ) && ec == err::success ) {
                time_t file_time = bfs::last_write_time( filepath );
                time_t curr_time = time(0);
                time_t off_time  = file_time + second_proc_wait;
                time_t diff_time = off_time - curr_time;

                if( curr_time > off_time ) {
                    addKeyVal( filepaths, glob_struct.gl_pathv[i], "-d" );

                } else {
                    rodsLog(
                        LOG_DEBUG,
                        "msiget_filepaths_from_glob - filepath [%s] is too young by [%d] seconds",
                        glob_struct.gl_pathv[i],
                        off_time - curr_time );

                }
            }
            else if ( bfs::is_directory( filepath, ec ) && ec == err::success ) {
                addKeyVal( filepaths, glob_struct.gl_pathv[i], "-C" );
            }
        }

    }

    globfree( &glob_struct );
    
    fillMsParam( _filepaths, NULL, KeyValPair_MS_T, filepaths, NULL );

    return 0;

}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(4);
    msvc->add_operation(
        "msiget_filepaths_from_glob",
        std::function<int(
            msParam_t*,
            msParam_t*,
            msParam_t*,
            msParam_t*,
            ruleExecInfo_t*)>(msiget_filepaths_from_glob));
    return msvc;
}
