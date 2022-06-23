#!/usr/bin/env python3
from __future__ import print_function
from irods.models import DataObject, DataObjectMeta, Collection, Resource
from irods.column import Criterion, Like
from irods.session import iRODSSession
import os, sys
from getopt import getopt,GetoptError

try:
  opt, args = getopt(sys.argv[1:],'R:n:e:')
except GetoptError as e:
  print ("""usage: %s [ -e .ext ] [ -n filestub ]
              -e defaults to ".jpg"
              -n defaults to "stickers" """ % (sys.argv[0],)) 
  sys.exit(1)

opts = {} ; opts.update (opt)

try:
    env_file = os.environ['IRODS_ENVIRONMENT_FILE']
except KeyError:
    env_file = os.path.expanduser('~/.irods/irods_environment.json')

session = iRODSSession(irods_env_file=env_file) 

object_name_stub = opts.get('-n', "stickers")
object_name_ext  = opts.get('-e','.jpg')
resc_name =  opts.get ('-R', "lts_resc" )

if not resc_name : resc_name = '%'
 
q = session.query ( Collection.name , DataObject.name, Resource.name )
q.filter( Like (DataObject.name,  object_name_stub + '%x%' + object_name_ext ),
          Like (Resource.name, resc_name) )

resultsIter = q.get_results()

print ("=== QUERY RESULTS: ===")
for result in resultsIter :
  print( result[Resource.name] + " :\t\t" +\
         result[Collection.name] + "/" + result[DataObject.name] )



