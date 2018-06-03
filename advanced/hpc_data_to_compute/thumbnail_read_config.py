#!/usr/bin/env python2
from __future__ import print_function
import sys, os, json, pprint
from os.path import join ,dirname

json_jobConfig_filename =join (dirname(sys.argv[0]), "..", "compute", 
                               "thumbnail_job_params.json")
with open (json_jobConfig_filename) as f:
  config = json.load(f)

if not( config and len(sys.argv) > 1):
  print ( "Need 1 argument (key of the job param to be retrieved)"\
            "\nChoose from: " + pprint.pformat(map(str, list(config.keys())) ),
          file=sys.stderr
        )
  raise SystemExit(1)

print( "{!s}".format( config.get(sys.argv[1],'') ) )
