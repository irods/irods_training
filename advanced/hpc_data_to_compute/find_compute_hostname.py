#!/usr/bin/env python
from __future__ import print_function
from irods.models import DataObject, DataObjectMeta, Collection, Resource
from irods.column import Criterion, Like
import os, sys
from irods.session import iRODSSession
from irods.meta import iRODSMeta
from getopt import getopt
import json

# =-=-=-=-=-=-=
# detect a data object path is valid in iRODS and 
#   attach metadata from a .json config-file if desired
# =-=-=-=-=-=-=

opt,args=getopt(sys.argv[1:],'r:')
optLookup= {} ; optLookup.update (opt)

resc_spec = optLookup .get ( '-r', '' )

try:
    env_file = os.environ['IRODS_ENVIRONMENT_FILE']
except KeyError:
    env_file = os.path.expanduser('~/.irods/irods_environment.json')

session = iRODSSession(irods_env_file=env_file) 

q = session.query ( Resource.parent , Resource.name, Resource.location ).filter( Resource.name == resc_spec )
# filter out all but the root node resource with a matching name
q = q.filter ( Resource.parent == '') .filter( Resource.name == resc_spec )

try:
  print ( q.one()[Resource.location] )
except:
  print ( "localhost" )

