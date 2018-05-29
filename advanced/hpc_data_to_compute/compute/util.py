from __future__ import print_function
import os, sys, json
from os.path import join, dirname,abspath
import irods
from irods.keywords import REG_CHKSUM_KW, VERIFY_CHKSUM_KW, CHKSUM_KW 
from irods.session import iRODSSession
from irods.models import Resource , ResourceMeta
from exceptions import SystemExit

def compute_host_from_rescName (rescName ):
  pass

chksum_opt = { k:'1' for k in ( REG_CHKSUM_KW,VERIFY_CHKSUM_KW,CHKSUM_KW ) }

def put_dest_from_fileName (fileName, sess = None):
  abs_module_path = abspath(dirname(__file__))
  configFile =  join( abs_module_path , "put_data_to_compute.json" )
  j = { "dest_resource":{}, "checksums": False }

  if os.path.isfile(configFile):
    with open(configFile) as f:
      j = json.load( f )

  ext = fileName.split(".")[-1:]
  spec = ''
  if ext and ext[0]:
    spec = j["dest_resource"].get( ext[0], '' )

  role_key = role_value = ''

  resc = "" 

  if spec:
    if spec.find("=") > 0:
      (role_key, role_value) = map(lambda x:x.strip() , (spec.split('=')+['']) [:2] )
    else:
      resc = spec

  if sess and role_key:
    q = sess.query( Resource.name , ResourceMeta.name , ResourceMeta.value )
    q.filter (ResourceMeta.name == role_key and \
                 ResourceMeta.value == role_value )
    try: resc = q.one() [Resource.name]
    except: pass

  return str( resc )

