
update_access_time_for_data_object(*_logical_path) {
    # the system time is reported in seconds
    *err = errormsg(msiGetSystemTime(*time, ""), *msg)
    if(0 != *err) {
        writeLine("serverLog", "msiGetSystemTime :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    # wash the time through an int to fix formatting
    *time_int = int(*time)
    *time_str = str(*time_int)

    # the attribute representing the access time is abstracted into a
    # configuration rule base to keep concerns separated
    get_access_time_attribute(*atime_attr)

    *err = errormsg(msiset_avu("-d", *_logical_path, *atime_attr, *time_str, ""), *msg)
    if(0 != *err) {
        writeLine("serverLog", "msiset_avu :: [*err] [*msg]")
        failmsg(*err, *msg)
    }
}

