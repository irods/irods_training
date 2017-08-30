#define RODS_SERVER 1

#include <cstddef>

#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"
#include "rsModAVUMetadata.hpp"

int msiset_avu(
    msParam_t* _item_type,
    msParam_t* _item_name,
    msParam_t* _attr_name,
    msParam_t* _attr_val,
    msParam_t* _attr_unit,
    ruleExecInfo_t* _rei ) {

    char *it_str = parseMspForStr( _item_type );
    if( !it_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    char *in_str = parseMspForStr( _item_name );
    if( !in_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    char *an_str = parseMspForStr( _attr_name );
    if( !an_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    char *av_str = parseMspForStr( _attr_val );
    if( !av_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    char *au_str = parseMspForStr( _attr_unit );
    if( !au_str ) {
        return SYS_INVALID_INPUT_PARAM;
    }

    char op[]  = "set";

    modAVUMetadataInp_t avuOp;
    memset(&avuOp, 0, sizeof(avuOp));
    avuOp.arg0 = op;
    avuOp.arg1 = it_str;
    avuOp.arg2 = in_str;
    avuOp.arg3 = an_str;
    avuOp.arg4 = av_str;
    avuOp.arg5 = au_str;

    _rei->status = rsModAVUMetadata(_rei->rsComm, &avuOp);

    return _rei->status;

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
        ruleExecInfo_t*>("msiset_avu",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiset_avu));
    return msvc;
}

