// =-=-=-=-=-=-=-
// irods includes
#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"

// =-=-=-=-=-=-=-
// stl includes
#include <string>


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

int msiget_uuid(
    msParam_t*      _string_param,
    ruleExecInfo_t* _rei ) {

    boost::uuids::uuid u;
    std::string us = boost::uuids::to_string(u);

    fillMsParam( _string_param, NULL, STR_MS_T, (void*)us.c_str(), NULL );

    return 0;
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(1);
    msvc->add_operation<
        msParam_t*,
        ruleExecInfo_t*>("msiget_uuid",
                         std::function<int(
                             msParam_t*,
                             ruleExecInfo_t*)>(msiget_uuid));
    return msvc;
}

