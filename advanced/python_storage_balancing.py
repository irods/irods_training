import re
from rule_engine_constants import PYTHON_RE_RET_ARGUMENTS

def pep_resource_resolve_hierarchy_pre(rule_args, callback, rei):
    if rule_args[3] == 'CREATE':
        ret = callback.findRescType(rule_args[0], '')
        resc_type = ret[PYTHON_RE_RET_ARGUMENTS][1]
        if (resc_type == 'passthru'):
            ret = callback.findInstId(rule_args[0], '')
            inst_id = ret[PYTHON_RE_RET_ARGUMENTS][1]

            ret = callback.findBytesUsed(inst_id, '')
            bytes_used = ret[PYTHON_RE_RET_ARGUMENTS][1]

            ret = callback.findContextString(rule_args[0], '')
            context_string = ret[PYTHON_RE_RET_ARGUMENTS][1]

            max_bytes = -1
            max_bytes_index = context_string.find('max_bytes')
            if max_bytes_index != -1:
                max_bytes_re = 'max_bytes=(\d+)'
                max_bytes_search = re.search(max_bytes_re, context_string)
                max_bytes_str = max_bytes_search.group(1)
                max_bytes = max_bytes_str

            percent_full = 0.0
            if max_bytes == -1:
                percent_full = 0.0
            elif max_bytes == 0:
                percent_full = 1.0
            else:
                percent_full = float(bytes_used)/max_bytes

            write_weight = 1.0 - percent_full
            rule_args[2] = 'read=1.0;write=' + str(write_weight)


