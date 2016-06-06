# core.py
import os
import sys
import EXIF
def acPostProcForPut(rule_args, callback):
    phypath = callback.getSessionVar('filePath', 'dummy')[PYTHON_RE_RET_OUTPUT][1]
    objpath = callback.getSessionVar('objPath', 'dummy')[PYTHON_RE_RET_OUTPUT][1]
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
