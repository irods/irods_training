#!/usr/bin/env python
from __future__ import print_function
from irods.models import DataObject, DataObjectMeta, Collection, Resource
from irods.column import Criterion, Like
import os, sys
from irods.session import iRODSSession
from irods.meta import iRODSMeta
from getopt import getopt
import json

opt,args=getopt(sys.argv[1:],'m:')
opts= {} ; opts.update (opt)

try:
    env_file = os.environ['IRODS_ENVIRONMENT_FILE']
except KeyError:
    env_file = os.path.expanduser('~/.irods/irods_environment.json')

session = iRODSSession(irods_env_file=env_file) 

defaultCollection = "/{}/home/{}".format(session.zone,session.username)
rf = []
object_path = args[0]
resc_name = "" if len(args)<2 else args[1]

try:
  if '/' in object_path:
    fullpath =  object_path
  else :
    fullpath = defaultCollection + "/" + object_path

  obj = session.data_objects.get( fullpath )
  repls = obj.replicas
  rf= [r for r in repls if not resc_name or r.resource_name == rescname ] 
except:
  pass

if rf:
  print ( 'object exists (checksum field is: {!r})'.format(rf[0].checksum), file=sys.stderr )

if opts.has_key('-m'):
  print ( "***", file=sys.stderr )
  with open(opts['-m'],'r') as f:
    config = json.load(f)
    AVUs = config['add_avu']
    for triplet in AVUs:
      obj.metadata.add( iRODSMeta(*triplet) )

