#!/usr/bin/env python3
from __future__ import print_function
from irods.models import Resource, ResourceMeta
import os, sys
from irods.session import iRODSSession
from getopt import getopt
import json
#sys.path.insert(0,'/var/lib/irods')
#from compute.common  import session_object
#session = session_object()

opt,args=getopt(sys.argv[1:],'r:l')
optLookup= {} ; optLookup.update (opt)

resc_spec = optLookup .get ( '-r', '' )

try:
    env_file = os.environ['IRODS_ENVIRONMENT_FILE']
except KeyError:
    env_file = os.path.expanduser('~/.irods/irods_environment.json')
session = iRODSSession(irods_env_file=env_file) 

if resc_spec.find('=') < 0 :
  resc_spec = resc_spec.strip()
  q = session.query ( Resource.name ). filter( Resource.name == resc_spec )
  1
else:
  resc_spec = map(lambda x:x.strip(), resc_spec.split('=')+['']) [:2] 
  q = session.query ( Resource.name, ResourceMeta.name, ResourceMeta.value).filter(
                      ResourceMeta.name == resc_spec[0] and \
                      ResourceMeta.value == resc_spec[1] )
try:
  resc_name = ( q.one()[Resource.name] )
except:
  resc_name = ''

print ('resc_name='+resc_name)
  
if optLookup.get('-l') is not None:
  try:
    q = session.query ( Resource.parent , Resource.name, Resource.location ) \
          .filter( Resource.name == resc_name )
    host = ( q.one()[Resource.location] )
  except:
    host = ''
  print('location='+host)

