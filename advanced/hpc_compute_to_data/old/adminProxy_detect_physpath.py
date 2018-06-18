#!/usr/bin/env python2
from __future__ import print_function
import os, sys
import irods
from irods.session import iRODSSession
from os.path import expanduser, curdir, sep ,abspath, join
from sys import argv as orig_argv, stderr, stdout, exit
from getopt import GetoptError, getopt
import json

# admin_as_rodsuser.py [-z tempZone] -j admin_credential.json -u alice 

try:
  opt, argv = getopt( 
        orig_argv[1:], 'l:j:r:u:z:H:I:v:' # 'E:' <-- (disabling for now)
  )
except GetoptError:
  print ( '''Usage: %s [options]
  where options are:
  \t -j json-config ( read from admin credentials file in .JSON )
  \t -h help  (print this usage statement)
  \t -H hostname  (machine with target iRODS server for our connect)
  \t -I portN (integer - iRODS server port # to connect to )
  \t -z <iRODS_zone>
  \t -u <client_user> (client user on behalf of which we are working)
  \t -l logical path to data object
  \t -r resc on which to search for data object
  \t -v N (set verbosity level)
  ''' % (orig_argv[0],) , file = stderr )
  exit(1)

argv.insert(0,orig_argv[0])
options = {}
options.update( opt )

#-=-=-=-=-=- Change admin password file

default_admin_creds_file = expanduser('~/compute/admin_as_rodsuser.json')
jsonConfigFile = options.get('-j',default_admin_creds_file)
with open(jsonConfigFile) as f:
    jsonConfig = json.load(f)
    admin_username = str( jsonConfig.get('admin_username') )
    admin_password = str( jsonConfig.get('admin_password') )

assert ( admin_username and admin_password )

irods_server_host = options.get('-H','localhost')
irods_server_port = int(options.get('-I','1247'))

user = options.get('-u','')            # - client user
zone = options.get('-z', 'tempZone' )  # - iRODS zone for compute-to-data ex.

verbosity = int( options.get('-v','0') )
if verbosity >= 2:
  _input = None
  try:    _input = raw_input        ## Python 2
  except NameError: _input = input  ## Python 3


if not user:
  session = iRODSSession ( host= irods_server_host , port = irods_server_port,
                           user = admin_username , password = admin_password,
                           zone = zone, *options_for_session )
else:
  session = iRODSSession( host = irods_server_host , port = irods_server_port, 
                          user = admin_username , password = admin_password ,
                          zone = zone , client_user = user )

if verbosity >= 1:
  print( "\tActing for user '{}'".format(session.username) , file = stderr )

#==============================================
if not (session):
  print ("session could not be instantiated", file=sys.stderr)
  exit(1)

  
answer = 'y'

if verbosity >= 1:
  print ('username = "{}" ' .format( session.username), file = stderr)
  if verbosity >= 2:
    assert _input is not None
    answer = _input('proceed (y/n) -> ')

if answer.upper().strip() != 'Y': exit(1)

print ( '** Proceeding **' , file = stderr )

try:
    logical_data_obj_path  = options.get('-l','')
    data_object = session.data_objects.get( logical_data_obj_path)
except:
    data_object = None

if data_object: print( data_object.replicas , file = sys.stderr)

resc_name = options.get('-r','')

repls = [ r for r in data_object.replicas if \
           r.resource_name == resc_name ]

if repls: print (repls[0].path)

exit(0)
