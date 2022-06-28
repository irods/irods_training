#!/usr/bin/env python3

# --
# -  iRODS compute <-> data 
# --

from __future__ import print_function
import sys, copy

def check_python_version():
  min_py_version = ( (2,7) , "This module requires >=2.7 of Python")
  assert sys.version_info >= min_py_version[0] , min_py_version[1]

def get_config ( rule_args, callback, rei ):
  callback.writeLine("serverLog", "getConfig getting args:{0!r}".format( rule_args))

import os, time, pprint, json
from os.path import ( join, curdir, dirname, sep as SEP )
from glob import glob
import argparse
import logging, logging.handlers
from   irods.models import Resource, ResourceMeta
from   irods.exception import DataObjectDoesNotExist , CollectionDoesNotExist
import irods.keywords as kw

job_params = {}
checksum_options = None
logger = None

checksum_flags = (kw.REG_CHKSUM_KW, kw.VERIFY_CHKSUM_KW, kw.CHKSUM_KW)
def generate_checksum_options() :
  return { k:'1' for k in checksum_flags }

class dummyLogger (object):
  def __init__(self): pass
  def fatal(self,s): pass
  def error(self,s): pass
  def info (self,s): pass
  def debug(self,s): pass
  def warn (self,s): pass

def computeLogger(use_dummy=False):
  global logger
  if not logger:
    try:
      log_filename =  '/tmp/log_irods_compute.txt' ;
      if not use_dummy :
        logger = logging.getLogger('compute_logger')
        logger.setLevel(logging.DEBUG)
        ch = logging.handlers.WatchedFileHandler(filename=log_filename)
        ch.setFormatter(logging. Formatter('%(filename)s:%(process)d '
          '@ %(asctime)s - %(message)s') )
        logger.addHandler(ch)
    except:
      pass
  if not logger: logger = dummyLogger()
  return logger

def jobParams( cfgFile = 'job_params.json' , argv0 = '' ):
  global job_params
  if not(job_params):
    if not argv0: argv0 = __file__
    if SEP not in cfgFile:
      cfgFile = join( (dirname(argv0) or curdir) , cfgFile )
    try:
      job_params = json.load( open( cfgFile, 'r') )
    except:
      msg =  "Could not load JSON config file at {}".format (cfgFile) 
      computeLogger().error(msg); raise SystemExit(msg)
  return job_params

def checksumOptions():
  global checksum_options 
  if checksum_options is None:
    if jobParams() ['use_checksums']:
      checksum_options = generate_checksum_options()
    else:
      checksum_options = {}
  return checksum_options

# --
# -   Helper functions
# --

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

# =-=-=-=-=

def check_replica_status( repl ,
                          use_chksum =  None ,
                          compare_to = None ,
                          wait =        None ):

  if repl is None: return False
  if use_chksum is None:   use_chksum = jobParams() ['use_checksums']
  if wait is None: 	   wait = jobParams() ['input_repl_wait']

  if type(wait) not in (tuple, list):
    wait = [ 2, 0.5 ]
    computeLogger().info ("'input_repl_wait' value in config file was incorrect "
                          "type.  need list: [nTimes, delaySec].")
    computeLogger().info ("'input_repl_wait' - using default of  {!r}".format(wait))

  #-- need mutable copy
  wait = list(wait)

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

def rescName_by_role ( role_key , role_value ):

  try:
    q = session_object().query( Resource.name , ResourceMeta.name, ResourceMeta.value
        ).filter(
                  ResourceMeta.name  == role_key , 
                  ResourceMeta.value == role_value
        )
    return q.one() [Resource.name]
  except: 
    pass

  return None

# -- 

def get_replNumber_to_rescName_map (obj, filter_on_name = ''):

  _filter = lambda _: True 

  if filter_on_name:
    _filter = (lambda x : x == filter_on_name)

  return { r.number:r for r in obj.replicas if _filter(r.resource_name) }

# -- 

def trim_all_replicas_from_resource ( obj,
                                      resourceName,
                                      rescName_for_repl_status = '',
                                      force = False
                                    ): 
  if not rescName_for_repl_status:
    map_other =  {}
  else:
    map_other = get_replNumber_to_rescName_map (obj, filter_on_name = rescName_for_repl_status ) 

  map_this = get_replNumber_to_rescName_map (obj , filter_on_name = resourceName) 

  repls_to_trim = [ r for r in map_this ]
  old_good_replicas = [ r for r,v in map_other.items() if v.status == '1' ]

  if len(old_good_replicas) == 0 and not( force ):
    computeLogger().info  ("Did not trim replicas from '{}' - no others with good status "
                           "on '{}'".format (resourceName, rescName_for_repl_status) )
    return False
  else:
    for r in repls_to_trim :
      del map_this[r]
      options = {'replNum': r, 'copies': 1}
      obj.trim(**options)

  return len(map_this) == 0
 
# --

def exists_on_resource (o, resourceName , test_status = True ):
  _filter = lambda x : x
  if test_status : 
    _filter = lambda x : x.status == '1'
  lst = [ r for r in o.replicas if r.resource_name == resourceName and _filter(r) ]
  return len(lst) > 0

# --

def replicate_object_to_resource ( obj, resourceName, **options ): 
  if not exists_on_resource (obj, resourceName, test_status = False):
    obj.replicate( resourceName , **options)

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

# ---

def object_path_by_resource( obj, resourceName ):

  repls_on_resc = [ repl for repl in obj.replicas 
                     if repl.resource_name == resourceName ]
  if not repls_on_resc : return (None, None)
  r = repls_on_resc[0]
  return ( r.path )

# ---

def do_replicate_input (cmd_line_args):

    success = False

    subparser=argparse.ArgumentParser();
    subparser.add_argument('--skip-if-exists',  action='store_true', default=False)
    subparser.add_argument('misc', nargs=argparse.REMAINDER)
    args = subparser.parse_args( cmd_line_args )

    input_resc = jobParams() ['imageCompute_resc']
    input_path = jobParams() ['input_path']

    sess = session_object()
    
    if type ( input_resc ) is tuple: 
      try:
        (key,val) = input_resc
        input_resc = sess.query (
          ResourceMeta.name, ResourceMeta.value, 
          Resource.name ). filter (
                             ResourceMeta.name == key,
                             ResourceMeta.value == val  ).one() [Resource.name]
      except Exception as e:
        msg = "Incorrect 'input_resc' value in config file: {!r}".format(input_resc)
        computeLogger().error(msg) ; raise SystemExit (msg)

    try:
      obj = sess.data_objects.get(input_path)
    except:
      msg =  "Object '{}' does not exist" .format(input_path) 
      computeLogger().error(msg) ; raise SystemExit (msg)

    if args.skip_if_exists and exists_on_resource(obj, input_resc, test_status = False):
      return True

    old_repl = new_repl = None

    staged_replicas = [x for x in obj.replicas if x.resource_name == 'demoResc' ]

    if staged_replicas : old_repl = staged_replicas [0]  
    else :
      computeLogger().error ("no input could be found at '{}'".format(input_path))

    try :
      obj.replicate ( input_resc , ** checksumOptions() )
      obj = sess.data_objects.get(input_path)
      new_repl = [x for x in obj.replicas if x.resource_name == input_resc ][0]
    except :
      pass
    
    if check_replica_status( new_repl , compare_to = old_repl ):
      success = True
    else: 
      computeLogger().error ("no input could be replicated at '{}'".format(input_path))
    return success

# ---

def register_replicate_and_trim_thumbnail ( size_string ):

  imageCompute_resc = rescName_by_role (*jobParams()['imageCompute_resc']) 
  phys_dir = jobParams()['phys_dir_for_output']
  c = get_collection( jobParams()['output_collection'] ) 
  result_pattern = join( phys_dir  , "*" + size_string + "*.jpg" )

  try:
    g = glob( result_pattern )
    fileName = g[0]
  except:
    msg= "Could not find match for '{}'".format(g)
    computeLogger().error ( msg )
    raise SystemExit(msg)

  sess = session_object()

  sess.data_objects.register( fileName,
    '{}/{}'.format(c.path, os.path.basename(fileName)),
    rescName = imageCompute_resc , **checksumOptions()
    )

  obj_path = c.path + '/' + jobParams()['thumbnail_filename'] % (size_string,)

  o = sess.data_objects.get(obj_path)

  replicate_object_to_resource ( o, resourceName = 'lts_resc' )
  o = sess.data_objects.get(obj_path)

  trim_all_replicas_from_resource ( o, resourceName = 'img_resc',
                                    rescName_for_repl_status = 'lts_resc' )

  # - todo - add METADATA to associate logical paths of product to job input(s)

def get_defaults (defaults_in = {'homeColl':None, 'session': None} ):

  session = defaults_in.get('session')

  if not(session): session = session_object()

  path_to_home = "/{}/home/{}".format(session.zone, session.username)

  if  type(defaults_in) is dict:
    defaults = copy.deepcopy (defaults_in)
  else:
    defaults = {}
  
  defaults["homeColl"] = session.collections.get(path_to_home)
  defaults["session"] = session
  return defaults

if __name__ == '__main__':
 
   check_python_version()
   parser = argparse.ArgumentParser( )
   parser.add_argument('--config', nargs=1, help="name of a .JSON config file", default='job_params.json')

   commands = { 'test' : 'check syntax and loading of config file',
                'replicate_input' : 'copy job_input to compute resource', 
                'reg_repl_trim_output' : 'register/trim results of computation to long term storage' }

   parser.add_argument ('command', help='one of {!s}'.format(list(commands.keys())) )
   parser.add_argument ('remainder', nargs=argparse.REMAINDER)
   args = parser.parse_args()

   params = jobParams ( cfgFile = args.config , argv0 = sys.argv[0] )

   checksum_options = checksumOptions()

   if args.command == 'replicate_input':
     do_replicate_input( args.remainder )
   elif args.command == 'reg_repl_trim_output' :
     assert len(args.remainder) == 1, "need a size string ('NxN') as argument"
     register_replicate_and_trim_thumbnail ( size_string = args.remainder[0] )
   else:
     computeLogger().info("ran {} with command argument '{}'".format(
       os.path.basename (sys.argv[0]), args.command )
     )
