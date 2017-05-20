// =-=-=-=-=-=-=-
// irods includes
#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"

// =-=-=-=-=-=-=-
// stl includes
#include <string>

int msiexample_microservice(
    msParam_t*      _string_param,
    msParam_t*      _int_param,
    msParam_t*      _double_param,
    ruleExecInfo_t* _rei ) {

    char *string_param = parseMspForStr( _string_param );
    if( !string_param ) {
        std::cout << "null _string_param" << std::endl;
        return SYS_INVALID_INPUT_PARAM;
    }

    int int_param = parseMspForPosInt( _int_param );
    if( int_param < 0 ) {
        std::cout << "invalid _int_param" << std::endl;
        return SYS_INVALID_INPUT_PARAM;
    }

    double double_param = 0.0;
    int ret = parseMspForDouble( _double_param, &double_param );
    if( ret < 0 ) {
        std::cout << "invalid _double_param" << std::endl;
        return SYS_INVALID_INPUT_PARAM;
    }

    std::cout << __FUNCTION__ << " string [" << string_param << "] int [" << int_param << "] double [" << double_param << "]" << std::endl;

    return 0;
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(3);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiexample_microservice",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiexample_microservice));
    return msvc;
}

