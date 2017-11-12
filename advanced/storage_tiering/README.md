# **Storage Tiering**

The storage tiering framework provides iRODS the capability of automatically moving data between identified tiers of storage.
 
To define a tier_group, target storage resources are labeled with metadata which define their place in the group and how long data should reside in that tier before being migrated to the next tier.
 
# **Rulebase Configuration**

Add the five included rule bases (two configuration files, two implementation files, and an example policy) above "core" in the `re_rulebase_set` array of the native iRODS rule language plugin:
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
                "access_time_configuration",
                "apply_access_time",
                "storage_tiering_configuration",
                "apply_storage_tiering",
                "example_tiering_policy",
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
 
Tier groups are defined via metadata AVUs attached to the resources which participate in the group.

In iRODS terminology, the `attribute` is defined by a function in the rule base **storage_tiering_configuration.re**, which by default is named **irods::storage_tier_group**.  The `value` of the metadata triple is the name of the tier group, and the `unit` holds the numeric position of the resource within the group.  To define a tier group, simply choose a name and apply metadata to the selected root resources of given compositions.

For example:
```
imeta add -R fast_resc irods::storage_tier_group example_group 0
imeta add -R medium_resc irods::storage_tier_group example_group 1
imeta add -R slow_resc irods::storage_tier_group example_group 2 
```

This example defines three tiers of the group `example_group` where data will flow from tier 0 to tier 2 as it ages.  In this example `fast_resc` is a single resource, but it could have been set to `fast_tier_root_resc` and represent the root of a resource hierarchy consisting of many resources.


# **Setting Tiering Policy**

Once a tier group is defined, the age limit for each tier must also be configured via metadata.  Once a data object has remained unaccessed on a given tier for more than the configured time, it will be staged to the next tier in the group and then trimmed from the previous tier.  This is configured via the default attribute **irods::storage_tier_time** (which itself is defined in the **storage_tiering_configuration.re** rulebase).  In order to configure the tiering time, apply an AVU to the resource using the given attribute and a positive numeric value in seconds.

For example, to configure the `fast_resc` to hold data for only 30 minutes:
```
imeta add -R fast_resc irods::storage_tier_time 1800
```
We can then configure the `medium_resc` to hold data for 8 hours:
```
imeta add -R medium_resc irods::storage_tier_time 28800
```

# **Customizing the Violating Objects Query**

A tier within a tier group may identify data objects which are in violation by an alternate mechanism beyond the built-in time-based constraint.  This allows the data grid administrator to take additional context into account when identifying data objects to migrate.

Data objects which have been labeled via particular metadata, or within a specific collection, owned by a particular user, or belonging to a particular project may be identified through a custom query.  The default attribute **irods::storage_tier_query** is used to hold this custom query.  To configure the custom query, attach the query to the root resource of the tier within the tier group.  This query will be used in place of the default time-based query for that tier.

```
imeta set -R rnd1 irods::storage_tier_query "select META_DATA_ATTR_VALUE, DATA_NAME, COLL_NAME where RESC_NAME = 'ufs2' || = 'ufs3' and META_DATA_ATTR_NAME = 'irods::access_time' and META_DATA_ATTR_VALUE < 'TIME_CHECK_STRING'"
```

The example above implements the default query.  Note that the string `TIME_CHECK_STRING` is used in place of an actual time.  This string will be replaced by the storage tiering framework with the appropriately computed time given the previous parameters.



