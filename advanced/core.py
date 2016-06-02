import os

import sys

from PIL import Image

from PIL.ExifTags import TAGS

def acPostProcForPut(rule_args, callback):
    phypath = callback.getSessionVar('filePath', 'dummy')[1]
    objpath = callback.getSessionVar('objPath', 'dummy')[1]

    exiflist = []

    for (k, v) in Image.open(phypath)._getexif().iteritems():
        exifpair = '%s=%s' % (TAGS.get(k), v)
        exiflist.append(exifpair)
        exifstring = '%'.join(exiflist)
        callback.add_metadata_to_objpath(exifstring, objpath, '-d')
        callback.writeLine('serverLog', 'PYTHON - acPostProcForPut() complete')
