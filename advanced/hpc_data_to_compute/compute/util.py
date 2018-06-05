from __future__ import print_function
import os, sys, json
from os.path import join, dirname,abspath
import irods
from irods.keywords import REG_CHKSUM_KW, VERIFY_CHKSUM_KW, CHKSUM_KW 
from irods.session import iRODSSession
from irods.models import Resource , ResourceMeta
from exceptions import SystemExit
from . import common as _common
import session_vars

def compute_host_from_rescName (rescName):
  pass

chksum_opt = { k:'1' for k in ( REG_CHKSUM_KW,VERIFY_CHKSUM_KW,CHKSUM_KW ) }

def pyParseRoleSpec (rule_args,callback,rei):  #-- called from iRODS rule language
    compute_resc_spec = rule_args[0]
    rule_args[1:3]= map( lambda x:x.strip() , 
                         (compute_resc_spec.split('=')+['']) [:2] )

def compute_rescName__as_rule ( compute_resc_spec, session = None, **kw ):

 #log_method = kw.get ( 'log_method', lambda x: None ):
  sess = kw.get ('session')
  callback = kw.get ('callback')
  if callback is None:
    # not called as rule, so get session if needed
    if session is None:  session = _common.session_object() 
  else:
    sess = None

  role_key = role_value = ""
  resc = "" 

  if compute_resc_spec . find("=") > 0:
    (role_key, role_value) = map( lambda x:x.strip() , 
                                  (compute_resc_spec.split('=')+['']) [:2] )
  else:
    resc = compute_resc_spec \
           if compute_resc_spec \
           else "demoResc"		#__ the default for now
					#    (should be configurable, however)
  if (role_key and role_value):
    if sess:
      q = sess.query( Resource.name , ResourceMeta.name , ResourceMeta.value )
      q.filter (ResourceMeta.name == role_key and \
                ResourceMeta.value == role_value )
      try: resc = q.one() [Resource.name]
      except:
      # log_method ("query didn't work; defaulting to resc: '{}'".format(resc))
        pass
  return str( resc )

#--------------------------------------------------------------
def compute_rescName__as_rule (args,callback,rei):
  kw = {'callback':callback, 'rei':rei}
  rescName = compute_rescName(*args[:1], **kw)
  # -- return value : resource's literal name 
  if len(args) > 1: args[1] =  rescName
