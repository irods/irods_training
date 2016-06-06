# training.re
getSessionVar(*name, *output) {
    *output = eval("str($"++*name++")");
}
add_metadata_to_objpath(*str, *objpath, *objtype) {
    msiString2KeyValPair(*str, *kvp);
    msiAssociateKeyValuePairsToObj(*kvp, *objpath, *objtype);
}
