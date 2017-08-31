
pep_api_data_obj_close_post(*INST, *COMM, *INP) {
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(*err < 0) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

pep_api_data_obj_put_post(*INST, *COMM, *INP, *BUFF, *OPR_OUT) {
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(*err < 0) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

pep_api_data_obj_get_post(*INST, *COMM, *INP, *BUFF, *OPR_OUT) {
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(*err < 0) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    *obj_path = *INP.obj_path
    *resc_hier = *INP.resc_hier
    *err = errormsg(restage_object_to_lowest_tier(*obj_path, *resc_hier), *msg)
    if(*err < 0) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

