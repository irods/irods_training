
update_access_time_for_data_object(*_logical_path) {
    # the system time is reported in seconds
    *err = errormsg(msiGetSystemTime(*time, ""), *msg)
    if(0 != *err) {
        writeLine("serverLog", "msiGetSystemTime :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    # the attribute representing the access time is abstracted into a
    # configuration rule base to keep concerns separated
    get_access_time_attribute(*atime_attr)

    *err = errormsg(msiset_avu("-d", *_logical_path, *atime_attr, *time, ""), *msg)
    if(0 != *err) {
        writeLine("serverLog", "msiset_avu :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    writeLine("serverLog", "XXXX - update_access_time_for_data_object --- [*time] --- *_logical_path");
}

pep_api_data_obj_close_post(*INST, *COMM, *INP) {
    writeLine("serverLog", "XXXX - *INP")
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(0 != *err) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

pep_api_data_obj_put_post(*INST, *COMM, *INP, *BUFF, *OPR_OUT) {
    writeLine("serverLog", "XXXX - *INP")
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(0 != *err) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

pep_api_data_obj_get_post(*INST, *COMM, *INP, *BUFF, *OPR_OUT) {
    writeLine("serverLog", "XXXX - *INP")
    *err = errormsg(update_access_time_for_data_object(*INP.obj_path), *msg)
    if(0 != *err) {
        writeLine("serverLog", "pep_api_data_obj_close_post :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}



