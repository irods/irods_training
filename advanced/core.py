
import re
import os
import sys
import EXIF

def pep_resource_resolve_hierarchy_pre(rule_args, callback):
    if rule_args[3] == 'CREATE':
        resc_type = ''
        ret = callback.findRescType(rule_args[0], resc_type)
        resc_type = str(ret[PYTHON_RE_RET_OUTPUT][1])
        if (resc_type == 'passthru'):
            inst_id = ''
            ret = callback.findInstId(rule_args[0], inst_id)
            inst_id = str(ret[PYTHON_RE_RET_OUTPUT][1])

            bytes_used = ''
            ret = callback.findBytesUsed(inst_id, bytes_used)
            bytes_used = str(ret[PYTHON_RE_RET_OUTPUT][1])

            max_bytes = -1.0
            context_string = ''
            ret = callback.findContextString(rule_args[0], context_string)
            context_string = str(ret[PYTHON_RE_RET_OUTPUT][1])
            max_bytes_index = context_string.find('max_bytes')
            if max_bytes_index != -1:
                max_bytes_re = 'max_bytes=(\d+)'
                max_bytes_search = re.search(max_bytes_re, context_string)
                max_bytes_str = max_bytes_search.group(1)
                max_bytes = float(max_bytes_str)

            percent_full = 0.0
            if max_bytes == -1:
                percent_full = 0.0
            elif max_bytes == 0:
                percent_full = 1.0
            else:
                percent_full = float(bytes_used)/max_bytes

            write_weight = 1.0 - percent_full
            rule_args[2] = 'read=1.0;write=' + str(write_weight)
# core.py
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
