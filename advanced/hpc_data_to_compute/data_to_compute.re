
rmtExec_slurm ( *host , *arg ) 
{
  
  remote("*host", "") {
    writeLine("serverLog", "rmtExec - host [*host] remote cmd [*arg] ")
    #msiExecCmd("hello","*arg","null","null","null",*OUT)
    #remotePythonService ( *arg )
  }
}


