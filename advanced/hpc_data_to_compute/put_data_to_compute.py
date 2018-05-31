#!/usr/bin/env python2

import os, sys
import compute.common
from irods.session import iRODSSession
from irods.models import Resource, ResourceMeta

import compute.util
import compute.common

session = common.session_object()

#try:
#  env_file = os.environ['IRODS_ENVIRONMENT_FILE']
#except KeyError:
#  env_file = os.path.expanduser('~/.irods/irods_environment.json')
#session = iRODSSession(irods_env_file=env_file) 

tst = util.put_dest_from_fileName( sys.argv[1] , session )

print ( "{!s}".format(tst) )

