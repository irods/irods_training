# **Storage Tiering**

 The storage tiering rule base provides iRODS the capability of automatically moving data between identified tiers of storage.  Each storage resource is tagged as part of a group with an indicator as to where in the group the resource resides.  Each resource is also tagged with a value which describes the maximum time a data object may reside on that resource.  Should a data object's last time of access exceed this given value it will be migrated to the next resource in the tier.  This package also includes a rule base feature which tags objects with their last time of access for this computation.
 
# **Rulebase Configuration**

Add the four included rule bases above "core" in the re_rule_base_set array of the native iRODS rule language plugin:
```
"rule_engines": [
    {    
        "instance_name": "irods_rule_engine_plugin-irods_rule_language-instance",
        "plugin_name": "irods_rule_engine_plugin-irods_rule_language",
        "plugin_specific_configuration": {  
            "re_data_variable_mapping_set": [
                "core"
            ],
            "re_function_name_mapping_set": [
                "core"
            ],
            "re_rulebase_set": [
                **"access_time_configuration",
                "apply_access_time",
                "storage_tiering_configuration",
                "apply_storage_tiering",
                "example_tiering_policy",**
                "core"
            ],
            "regexes_for_supported_peps": [
                "ac[^ ]*",
                "msi[^ ]*",
                "[^ ]*pep_[^ ]*_(pre|post)"
            ]
        },
        "shared_memory_instance": "irods_rule_language_rule_engine"
    },
]
```
 
# **Creating a Tier Group**
 
Tier groups are defined via metadata attached to the resources which participate in the group.  In iRODS terminology the attribute is defined by a function in the rule base **storage_tiering_configuration.re**, which by default is **irods::storage_tier_group**.  The value of the metadata triple is the name of the tier group, and the Unit holds the numeric position of the resource within the group.  To define a tier group simply choose a name and apply metadata to the selected root resources of given compositions.  For example:
```
imeta add -R fast_resc irods::storage_tier_group example_group 0
imeta add -R medium_resc irods::storage_tier_group example_group 1
imeta add -R slow_resc irods::storage_tier_group example_group 2 
```


# **Setting Tiering Policy**

Once a tier group is defined, the age limit for each tier must also be configured via metadata.  Once a data object has remained unaccessed on a given tier for more than the configured time, it will be staged to the next tier in the group and then trimmed from the previous tier.  This is configured via the default attribute **irods::storage_tier_time**, which is also defined in the **storage_tiering_configuration.re** rulebase.  In order to configure the tiering time, apply a tag to the resource using the given attribute and a positive numeric value in seconds.  For example, to configure the _fast_resc_ to only hold data for 30 minutes:
```
imeta add -R fast_resc irods::storage_tier_time 1800
```
We can then configure the _medium_resc to hold data for 8 hours:
```
imeta add -R medium_resc irods::storage_tier_time 28800
```

# **Customizing the Violating Objects Query**

A tier within a tier group may identify data objects which are in violation by an additional mechanism beyond the built in time based constraint.  This allows the data grid administrator to take additional context into account when identifying data objects to migrate.  Data objects given a metadata attribute, a specific collection, a particular user or a project may be identified through a custom query attached to the root resource of a given tier within a group.  The default attribute **irods::storage_tier_query** is used to hold the query.  To configure the custom query, attach the query to the root resource of the tier within the tier group.  This query will be used in place of the default.

```
imeta set -R rnd1 irods::storage_tier_query "select META_DATA_ATTR_VALUE, DATA_NAME, COLL_NAME where RESC_NAME = 'ufs2' || = 'ufs3' and META_DATA_ATTR_NAME = 'irods::access_time' and META_DATA_ATTR_VALUE < 'TIME_CHECK_STRING'"
```

This example implements the default query.  Note that the string **TIME_CHECK_STRING** is used in place of an actual time.  This string will be replaced by the storage tiering framework with the appropriately computed time given the previous parameters.



