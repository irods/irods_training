get_data_tiering_group_attribute(*attr) {
    *attr = "irods::data_tier_group"
}

get_data_tiering_time_attribute(*attr) {
    *attr = "irods::data_tier_time"
}

get_data_tiering_query(*attr) {
    *attr = "irods::data_tier_query"
}

get_restage_delay_parameters(*attr) {
    *attr = "<PLUSET>1s</PLUSET><EF>1h DOUBLE UNTIL SUCCESS OR 6 TIMES</EF>"
}

