#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"

int msiget_leaf_resources(
    msParam_t*      _resc_name,
    msParam_t*      _leaves,
    ruleExecInfo_t* _rei ) {

    char *it_str = parseMspForStr( _resc_name );
    if( !it_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    return _rei->status;

}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(2);

    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiset_avu",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiget_leaf_resources));
    return msvc;
}

