main_rule {

    #*host = ""
    #*value = ""
    #get_host_and_resource_name_by_role (*host, *resc_spec, *attr, *value) {
    #writeLine("stdout", "resc_name -> [ *vlauehost -> [*host]")
    *thb_sizes = list("128x128", "256x256", "512x512", "1024x1024")
    foreach( *t in *thb_sizes ) {
        writeLine("stdout", "thumbnail size -> [*t]")
    }
}


INPUT null 
OUTPUT ruleExecOut
