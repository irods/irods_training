#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"
#include "irods_hierarchy_parser.hpp"
#include "rsGlobalExtern.hpp"

namespace irods {
    int msiget_leaf_resources(
        msParam_t*      _root_name,
        msParam_t*      _leaves,
        ruleExecInfo_t* _rei ) {

        char *root_name = parseMspForStr( _root_name );
        if( !root_name ) {
            rodsLog(
                LOG_ERROR,
                "%s:%d - invalid root resource name",
                __FUNCTION__,
                __LINE__);
            return SYS_INVALID_INPUT_PARAM;
        }

        // if the resource has no children then simply return
        resource_ptr root_resc;
        error ret = resc_mgr.resolve(root_name, root_resc);
        if(!ret.ok()) {
            irods::log(PASS(ret));
            return ret.code();
        }

        // otherwise capture the resource id's
        // and convert them to names, then aggregate
        // them into one big string
        std::string leaf_string;
        try {
            std::vector<resource_manager::leaf_bundle_t> leaf_bundles = resc_mgr.gather_leaf_bundles_for_resc(root_name);
            for(const auto & leaves : leaf_bundles) {
                for(const auto & leaf_id : leaves) {
                    std::string hier;
                    ret = resc_mgr.leaf_id_to_hier(leaf_id, hier);
                    if(!ret.ok()) {
                        irods::log(PASS(ret));
                        continue;
                    }

                    irods::hierarchy_parser parser;
                    parser.set_string(hier);

                    std::string leaf_name;
                    parser.last_resc(leaf_name);

                    if(!leaf_string.empty()) {
                        leaf_string += ",";
                    } 

                    leaf_string += leaf_name;
                }
            }
        }
        catch( const exception & _e ) {
            irods::log(_e);
            return _e.code();
        }

        // if there is no hierarchy
        if(leaf_string.empty()) {
            leaf_string = root_name;
        }

        char* tmp_arr = new char[leaf_string.size()+1];
        strcpy(tmp_arr, leaf_string.c_str());
        fillMsParam( _leaves, NULL, STR_MS_T, tmp_arr, NULL );

        return _rei->status;

    }
}; // namespace irods

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(2);

    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiget_leaf_resources",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(irods::msiget_leaf_resources));
    return msvc;
}

