#!/usr/bin/env python3
# --
# -  iRODS compute <-> data 
# --

from __future__ import print_function
import sys
as_module = True
min_py_version = ( (3,4,0) , "Please use this module with at least 3.4.0 of Python")

assert sys.version_info >= min_py_version[0] , min_py_version[1]

import os, time, pprint 
import logging, logging.handlers
from   irods.models import Resource, ResourceMeta
from   irods.exception import DataObjectDoesNotExist , CollectionDoesNotExist
import irods.keywords as kw
import job_params

#-- set up local logging

log_filename = (os.path.dirname(sys.argv[0]) or os.path.curdir) + \
                os.path.sep + \
                'log_irods_compute.txt'
logger = logging.getLogger('compute_log')
logger.setLevel(logging.DEBUG)
ch = logging.handlers.WatchedFileHandler(filename=log_filename)
ch.setFormatter(logging. Formatter('%(processName)s:%(process)d '
		'@ %(asctime)s - %(message)s') )
logger.addHandler(ch)

if job_params . use_checksums:
  checksum_options = {  k:'1' for k in ( kw.REG_CHKSUM_KW,
                                             kw.VERIFY_CHKSUM_KW,
                                             kw.CHKSUM_KW)          }
else :
  checksum_options = {}

# ---
#     Helper functions
# ---

# - session_object() :  get, or initially set, iRODS session object

session = None

def session_object( sess_ = None ):

  global session

  if session is None:
    from irods.session import iRODSSession
    if sess_ is not None and type(sess_) is iRODSSession:
      session = sess_
    else:
      import os
      try:
        env_file = os.environ ['IRODS_ENVIRONMENT_FILE']
      except KeyError:
        env_file = os.path.expanduser('~/.irods/irods_environment.json')
      session =  iRODSSession(irods_env_file = env_file )
  
  return session


# - check and paranoid re-checking of 

def check_replica_status( repl ,
                          use_chksum =  job_params . use_checksums ,
                          compare_to = None ,
                          wait =       job_params . repl_wait ):

  if repl is None: return False

  if type(wait) is tuple:
    wait = list( wait )
  else:
    wait = [ 0, 0 ]

  status_gated_checksum = (lambda y : y.checksum if y.status == '1' else None)
  if not(compare_to) or not(compare_to.checksum) : use_chksum = False
  get_status = lambda x : x.status == '1' if not( use_chksum ) else \
        (
            (status_gated_checksum( x ) is not None) if compare_to is None \
               else status_gated_checksum( x ) == status_gated_checksum( compare_to )
        )
  delay = max(1.0e-6, wait[1])

  cur_status = False

  while not cur_status:
    cur_status =  get_status( repl )
    if cur_status or wait[0] <= 0 : break
    else:
      wait[0] -= 1
      time.sleep(delay)

  return cur_status
    

# - get name of resource serving role described in string parameter: role

def rescName_by_compute_role (role):

  try:
    q = session_object().query( Resource.name , ResourceMeta.name, ResourceMeta.value
        ).filter(
                  ResourceMeta.name  == 'COMPUTE_RESOURCE_ROLE' ,
                  ResourceMeta.value == role
        )
    return q.one() [Resource.name]
  except: 
    pass

  return None

# -- 

def get_replNumber_to_rescName_map (object, filter_on_name = ''):

  _filter = lambda _: True 

  if filter_on_name:
    _filter = (lambda x : x == filter_on_name)

  return { r.number:r for r in object.replicas if _filter(r.resource_name) }

# -- 

def trim_all_replicas_from_resource ( object,
                                      resourceName,
                                      rescName_for_repl_status = '' ): 
  if not rescName_for_repl_status:
    map_other =  {}
  else:
    map_other = get_replNumber_to_rescName_map (object,
                   filter_on_name = rescName_for_repl_status ) 

  map = get_replNumber_to_rescName_map (o , filter_on_name = resourceName) 

  repls_to_trim = [ r for r in map ]

  old_good_replicas = [ r for r,v in map_other.items() if v.status == '1' ]

  if len(old_good_replicas) > 0:

    for r in repls_to_trim :
      # -- could include specify input parameter to narrow down which repls to delete
      del map[r]
      o.unlink( replNum = r )

  return len(map) == 0
 
# --

def exists_on_resource (o, resourceName , test_status = True ):
  _filter = lambda x : x
  if test_status : 
    _filter = lambda x : x.status == '1'
  lst = [ r for r in o.replicas if r.resource_name == resourceName and _filter(r) ]
  return len(lst) > 0

# --

def replicate_object_to_resource ( object, resourceName ): 
  if not exists_on_resource (object, resourceName, test_status = False):
    o.replicate( resourceName )

# --

def get_collection (collection_name, create = True):
    c = None
    sess = session_object()
    coll_path = collection_name if '/' in collection_name else \
        '/{sess.zone}/home/{sess.username}/{}'.format(collection_name,**locals())
    try:
      c = sess.collections.get( coll_path )
    except CollectionDoesNotExist as e:
      if create: c = sess.collections.create( coll_path )
    return c
  
# --

def object_path_by_resource( o, resourceName ):

  repls_on_resc = [ repl for repl in o.replicas 
                     if repl.resource_name == resourceName ]
  if not repls_on_resc : return (None, None)
  r = repls_on_resc[0]
  return ( r.path )


if __name__ == '__main__':

  as_module = False

  args = tuple(sys.argv[1:])

  if (args[:1]==('role',)):

    sess = session_object()
    dobj_name  = args[1] if args[1:2] else 'foo2.dat'
    o = sess.data_objects.get('/{sess.zone}/home/{sess.username}/{}'.format(
          dobj_name,**locals() 
        ))
    print( rescName_by_compute_role ('LONG_TERM_STORAGE') )
    print( rescName_by_compute_role ('IMAGE_PROCESSING') )

  elif (args[:1]==('coll',)):

    thumbnail_collection = 'stickers_thumbnails'

    c = get_collection ( thumbnail_collection )
    if c:
      print ('thumbnail collection ID is', c.id)
      print ( '-------------------- ')
      print ( 'collection contains: ')
      pprint.pprint ( c.data_objects)

  elif (args[:1]==('replicate_input',)):

    inputR = job_params . input_resc
    input_path = job_params . input_path
    sess = session_object()

    if type ( inputR ) is tuple: 
      try:
        (key,val) = inputR
        inputR = sess.query (
          ResourceMeta.name, ResourceMeta.value, 
          Resource.name ). filter (
                             ResourceMeta.name == key,
                             ResourceMeta.value == val  ).one() [Resource.name]
      except Exception as e:
        print (" {!r} ".format (e))
        pass
      except: pass

    object = sess.data_objects.get(input_path)

    old_repl = new_repl = None

    staged_replicas = [x for x in object.replicas if x.resource_name == 'demoResc' ]

    if staged_replicas : old_repl = staged_replicas [0]  
    else :
      logger.error ("no input could be found at '{}'".format(input_path))

    try :
      object.replicate ( inputR , ** checksum_options )
      object = sess.data_objects.get(input_path)
      new_repl = [x for x in object.replicas if x.resource_name == inputR ][0]
    except :
      pass
    
    if not check_replica_status( new_repl , compare_to = old_repl ):
      logger.error ("no input could be replicated at '{}'".format(input_path))

  elif (args[:1]==('register_outputs',)):

      thumbnail_collection = 'stickers_thumbnails'

      c = get_collection ( thumbnail_collection )
      ( resourceName , fileName ) = args[1:]
      session_object().data_objects.register( fileName,
                                              '{}/{}'.format(c.path, os.path.basename(fileName)),
                                              rescName = resourceName )
  elif (args[:1]==('loc',)):

    sess = session_object()
    o = sess.data_objects.get(
      '/{sess.zone}/home/{sess.username}/{}'.format('foo2.dat',**locals())
    )
    resource = rescName_by_compute_role ('LONG_TERM_STORAGE') 
    p = object_path_by_resource( o, resource )

  elif (args[:1] == ('repltrim',))  :

    sess = session_object()
    o = sess.data_objects.get(
      '/{sess.zone}/home/{sess.username}/{}'.format('foo2.dat',**locals()))
    # m = get_replNumber_to_rescName_map (o, filter_on_name = 'fooResc')
    m = None

    replicate_object_to_resource ( o,    resourceName = 'lts_resc' )

    trim_all_replicas_from_resource ( o, resourceName = 'img_resc',
                                      rescName_for_repl_status = 'lts_resc' )
  else:
    print("in else clause, as_module = {}".format(as_module), file=sys.stderr)
    ses = session_object()
    # print("--- locals ---", ) ; pprint.pprint (locals())
    pass



