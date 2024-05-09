# example_delay.r
RepeatHello {
  writeLine("serverLog", "Adding delay rule to queue.");
  delay("<PLUSET>30s</PLUSET><EF>1m</EF><INST_NAME>irods_rule_engine_plugin-irods_rule_language-instance</INST_NAME>") 
  {
    writeLine("serverLog", "Hello iRODS World!");
  }
}
INPUT null
OUTPUT ruleExecOut
