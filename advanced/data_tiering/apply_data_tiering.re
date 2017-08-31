
split_path(*p,*tok,*col,*obj) {
    *col = trimr(*p, *tok)
    if(strlen(*col) == strlen(*p)) {
        *obj = *col
    } else {
        *obj = substr(*p, strlen(*col)+1, strlen(*p)) 
    }
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

    foreach(*group_name in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = 'irods::data_tier_group') {
        *val = *group_name.META_RESC_ATTR_VALUE
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
    }

    *indicies = sort(*indicies)
}

get_resource_data_tier_time(*resc, *time) {
    *time_attr = ""
    *err = errormsg(get_data_tiering_time_attribute(*time_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    foreach(*t in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = '*time_attr' and RESC_NAME = '*resc') {
        *time = *t.META_RESC_ATTR_VALUE
    }
}

get_resource_data_tier_query(*resc, *query) {
    *time_attr = ""
    *err = errormsg(get_data_tiering_query(*query_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    foreach(*t in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = '*query_attr' and RESC_NAME = '*resc') {
        *query = *t.META_RESC_ATTR_VALUE
    }
}

find_violating_data_objects_by_access_time(*resc_name, *time_offset, *query_iterator) {
    *msg = ""
    *err = errormsg(msiGetSystemTime(*time_now, ""), *msg)
    if(*err < 0) {
        writeLine("serverLog", "ERROR - msiGetSystemTime :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    *err = errormsg(get_access_time_attribute(*atime_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    *time_check = int(*time_now) - int(*time_offset)
    *time_check_str = str(*time_check)
    #writeLine("serverLog", "        now [*time_now] offset [*time_offset] time check [*time_check_str]")
    *query_iterator = select META_DATA_ATTR_VALUE, DATA_NAME, COLL_NAME where RESC_NAME = '*resc_name' and META_DATA_ATTR_NAME = '*atime_attr' and META_DATA_ATTR_VALUE < '*time_check_str'
}

find_violating_data_objects(*resc_name, *time_offset, *query_iterator) {

    *query = ""
    *err = errormsg(get_resource_data_tier_query(*resc_name, *query), *msg)
    if("" != *query) {
        *err = eval(*query ++ ``(*resc_name, *time_offset, *query_iterator)``)
    }
    else {
        *err = errormsg(find_violating_data_objects_by_access_time(*resc_name, *time_offset, *query_iterator), *msg)
        if(*err < 0) {
            failmsg(*err, *msg)
        }
    }
}

get_replica_number_for_resource(*obj_path, *resc_name, *repl_num) {
    split_path(*obj_path, "/", *coll_name, *data_name)

    foreach(*rn in select DATA_REPL_NUM where RESC_NAME = '*resc_name' and COLL_NAME = '*coll_name' and DATA_NAME = '*data_name') {
        *repl_num = *rn.DATA_REPL_NUM
    }
}

check_for_replica(*obj_path, *resc_name, *safe_flg) {
    split_path(*obj_path, "/", *coll_name, *data_name)

    *safe_flg = false
    foreach(*repl in select DATA_REPL_NUM where DATA_NAME = '*data_name' and COLL_NAME = '*coll_name' and RESC_NAME = '*resc_name') {
       *safe_flg = true
    }
}

migrate_data_object(*src_resc_name, *dst_resc_name, *obj_path) {
    *repl_num = ""
    get_replica_number_for_resource(*obj_path, *src_resc_name, *repl_num)

    *err = errormsg(msiDataObjRepl(
                        *obj_path,
                        "rescName=*src_resc_name++++destRescName=*dst_resc_name",
                        *out_param), *msg)
    if(0 != *err) {
        failmsg(*err, "msiDataObjRepl failed for [*obj_path] [*src_resc_name] [*dst_resc_name] - [*msg]")
    }

    # stat the new replica before unlinking the old
    check_for_replica(*obj_path, *dst_resc_name, *safe_flg)
    if(false == *safe_flg) {
        failmsg(-1, "failed to create replica for *obj_path to *dst_resc_name")
    }

    *err = errormsg(msiDataObjUnlink(
                        "objPath=*obj_path++++replNum=*repl_num",
                        *out_param), *msg)
    if(0 != *err) {
        failmsg(*err, "msiDataObjUnlink failed for [*obj_path] [*out_param]")
    }
}

get_tier_group_for_replica(*obj_path, *resc_name, *tier_group) {
    *tier_attr = ""
    *err = errormsg(get_data_tiering_group_attribute(*tier_attr), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    split_path(*obj_path, "/", *coll_name, *data_name)
    foreach(*idx in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = '*tier_attr' and COLL_NAME = '*coll_name' and DATA_NAME = '*data_name' and RESC_NAME = '*resc_name') {
        *tier_group  = *idx.META_RESC_ATTR_VALUE
    }

}

restage_object_to_lowest_tier(*obj_path, *resc_hier) {
    split_path(*resc_hier, ";", *parent, *src_resc_name)

    *tier_group = ""
    *err = errormsg(get_tier_group_for_replica(*obj_path, *src_resc_name, *tier_group), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }
    
    if("" != *tier_group) {
        *err = errormsg(get_tier_group_resources_and_indicies(
                            *tier_group,
                            *indicies,
                            *resources), *msg)
        if(*err < 0) {
            failmsg(*err, *msg)
        }
        
        *idx0 = elem(*indicies, 0)
        *dst_resc_name = *resources."*idx0"

        get_restage_delay_parameters(*delay_attr)
        delay(*delay_attr) {
            *err = errormsg(migrate_data_object(
                                *src_resc_name,
                                *dst_resc_name,
                                *obj_path), *msg)
            if(*err < 0) {
                writeLine("serverLog", "ERROR - failed to migrate data object [*err] [*msg]")
            }
        }
    }
}

apply_data_tiering_policy() {
    *group_list = list() 
    *err = errormsg(get_list_of_tier_groups(*group_list), *msg)
    if(*err < 0) {
        failmsg(*err, *msg)
    }

    foreach( *tier_group in *group_list) {
        writeLine("serverLog", "Processing Tier Group: [*tier_group]")

        *err = errormsg(get_tier_group_resources_and_indicies(
                            *tier_group,
                            *indicies,
                            *resources), *msg)
        if(*err < 0) {
            failmsg(*err, *msg)
        }

        *dst_idx = size(*indicies)-1
        for(*i = 0; *i < *dst_idx; *i = *i + 1) {
            *idx0 = elem(*indicies, *i)
            *idx1 = elem(*indicies, *i+1)

            *src_resc_name = *resources."*idx0"
            *dst_resc_name = *resources."*idx1"

            writeLine("serverLog", "    processing resource [*src_resc_name]")
            *err = errormsg(get_resource_data_tier_time(
                                *src_resc_name,
                                *time), *msg)
            if(*err < 0) {
                failmsg(*err, *msg)
            }

            *err = errormsg(find_violating_data_objects(
                                *src_resc_name,
                                *time,
                                *query_iterator), *msg)
            if(*err < 0) {
                failmsg(*err, *msg)
            }

            foreach(*obj in *query_iterator) {
                *obj_path = *obj.COLL_NAME ++ "/" ++ *obj.DATA_NAME
                *time_str = *obj.META_DATA_ATTR_VALUE

                writeLine("serverLog", "        violating object [*obj_path] [*time_str]")

                *err = errormsg(migrate_data_object(
                                    *src_resc_name,
                                    *dst_resc_name,
                                    *obj_path), *msg)
                if(*err < 0) {
                    writeLine("serverLog", "ERROR - failed to migrate data object [*err] [*msg]")
                }

            } # foreach obj

        } # for index

    } # foreach tier group

} # apply_data_tiering_policy



