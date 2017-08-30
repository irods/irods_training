
test_fcn() {
writeLine("serverLog", "XXXXXXXXXXXXXXXXXX")
}



append(*a, *b) =
   if size(*a) == 0 then *b else cons(hd(*a),append(tl(*a),*b))

splitByPivot(*a,*b) =
   if size(*b) == 0
     then (list(), list())
     else let *hd = hd(*b) in
          let *tl = tl(*b) in
          let (*tlb, *tlc) = splitByPivot(*a, *tl) in
              if *hd < *a
                  then (cons(*hd, *tlb), *tlc)
                  else (*tlb, cons(*hd, *tlc))

sort(*a) =
   if size(*a) <= 1 then *a
   else let *pivot = hd(*a) in
        let (*b, *c) = splitByPivot(*pivot,tl(*a)) in
        let *bs = sort(*b) in
        let *cs = sort(*c) in
        append(*bs, cons(*pivot, *cs))


get_list_of_tier_groups(*group_list) {
    *tier_attr = ""
    *err = errormsg(get_data_tiering_group_attribute(*tier_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    foreach(*g in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = 'irods::data_tier_group') {
        *val = *g.META_RESC_ATTR_VALUE
        *group_list = cons(*val, *group_list)
    }
}

get_tier_group_resources_and_indicies(*group_name, *indicies, *resources) {
    *tier_attr = ""
    *err = errormsg(get_data_tiering_group_attribute(*tier_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    *indicies = list()
    foreach(*idx in select RESC_NAME, META_RESC_ATTR_UNITS where META_RESC_ATTR_NAME = '*tier_attr' and META_RESC_ATTR_VALUE = '*group_name') {
        *resc = *idx.RESC_NAME
        *val  = *idx.META_RESC_ATTR_UNITS
        *indicies = cons(*val, *indicies)

        *resources."*val" = *resc
        writeLine("serverLog", "RESC_NAME *resc, IDX *val")
    }

    *indicies = sort(*indicies)
}

get_resource_data_tier_time(*resc, *time) {
    *time_attr = ""
    *err = errormsg(get_data_tiering_time_attribute(*time_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    writeLine("serverLog", "time_attr [*time_attr]")

    foreach(*t in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = '*time_attr' and RESC_NAME = '*resc') {
        *time = *t.META_RESC_ATTR_VALUE
    }
            
    writeLine("serverLog", "XXXX - resc name [*resc] - time [*time]")
}

find_violating_data_objects(*resc_name, *time_offset, *query_iterator) {
    *msg = ""
    *err = errormsg(msiGetSystemTime(*time_now, ""), *msg)
    if(*err < 0) {
        writeLine("stdout", "get_violating_data_objects - msiGetSystemTime :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    *err = errormsg(get_access_time_attribute(*atime_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    *time_offset = 10
    *time_check = int(*time_now) - int(*time_offset)
    *time_check_str = str(*time_check)
    *query_iterator = select DATA_NAME, COLL_NAME, DATA_REPL_NUM where RESC_NAME = '*resc_name' and META_DATA_ATTR_NAME = '*atime_attr' and META_DATA_ATTR_VALUE <= '*time_check_str'
}

migrate_violating_data_object(*src_resc_name, *dst_resc_name, *obj_path, *repl_num) {
    *err = errormsg(msiDataObjRepl(
                        *obj_path,
                        "rescName=*src_resc_name++++destRescName=*dst_resc_name",
                        *out_param), *msg)
    if(0 != *err) {
        failmsg(*err, "msiDataObjRepl failed for [*obj_path] [*src_resc_name] [*dst_resc_name] - [*msg]")
    }

    *err = errormsg(msiDataObjUnlink(
                        "objPath=*obj_path++++replNum=*repl_num",
                        *out_param), *msg)
    if(0 != *err) {
        failmsg(*err, "msiDataObjUnlink failed for [*obj_path] [*out_param]")
    }
}

migrate_violating_data_objects() {
    writeLine("serverLog", "START")

    *group_list = list() 
    *err = errormsg(get_list_of_tier_groups(*group_list), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    foreach( *g in *group_list) {
        writeLine("stdout", "group: [*g]")

        *err = errormsg(get_tier_group_resources_and_indicies(*g, *indicies, *resources), *msg)
        if(*err < 0) {
            failmsg(*err, *msg)
        }

        writeLine("serverLog", "*resources")

        *dst_idx = size(*indicies)-1
        for(*i = 0; *i < *dst_idx; *i = *i + 1) {
            *idx0 = elem(*indicies, *i)
            *idx1 = elem(*indicies, *i+1)

            writeLine("serverLog", "index: [*idx0] [*idx1]")
            *src_resc_name = *resources."*idx0"
            *dst_resc_name = *resources."*idx1"
            writeLine("serverLog", "resc names: [*src_resc_name] [*dst_resc_name]")

            *err = errormsg(get_resource_data_tier_time(*src_resc_name, *time), *msg)
            if(*err < 0) {
                failmsg(*err, *msg)
            }

            *err = errormsg(find_violating_data_objects(*src_resc_name, *time, *query_iterator), *msg)
            if(*err < 0) {
                failmsg(*err, *msg)
            }

            foreach(*obj in *query_iterator) {
                *obj_path = *obj.COLL_NAME ++ "/" ++ *obj.DATA_NAME
                *repl_num = *obj.DATA_REPL_NUM
                writeLine("serverLog", "violating object [*obj_path]")

                *err = errormsg(migrate_violating_data_object(*src_resc_name, *dst_resc_name, *obj_path, *repl_num), *msg)
                if(*err < 0) {
                    writeLine("serverLog", "ERROR - failed to migrate data object [*err] [*msg]")
                }
            }
        }
    }

    writeLine("serverLog", "DONE")
}



