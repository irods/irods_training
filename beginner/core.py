# core.py
import os
import session_vars
import sys
import EXIF
def acPostProcForPut(rule_args, callback, rei):
    sv = session_vars.get_map(rei)
    phypath = sv['data_object']['file_path']
    objpath = sv['data_object']['object_path']
    exiflist = []
    with open(phypath, 'rb') as f:
        tags = EXIF.process_file(f, details=False)
        for (k, v) in tags.iteritems():
            if k not in ('JPEGThumbnail', 'TIFFThumbnail', 'Filename', 'EXIF MakerNote'):
                exifpair = '{0}={1}'.format(k, v)
                exiflist.append(exifpair)
    exifstring = '%'.join(exiflist)
    callback.add_metadata_to_objpath(exifstring, objpath, '-d')
    callback.writeLine('serverLog', 'PYTHON - acPostProcForPut() complete')
