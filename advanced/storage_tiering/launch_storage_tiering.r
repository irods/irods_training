go {
    *msg = ""
    *err = failmsg(apply_storage_tiering_policy(), *msg)
    if(*err < 0) {
        writeLine("stdout", "ERROR :: [*err] [*msg]")
        failmsg(*err, *msg)
    }

    writeLine("stdout", "done")

}

INPUT null
OUTPUT ruleExecOut
