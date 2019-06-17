#!/usr/bin/env python

from __future__ import print_function
import sys
from pprint import pformat

if sys.version_info >= (3,) : 
    unicode_ = str
else:
    unicode_ = unicode 

to_bytes   = lambda encoding : (lambda s: s.encode(encoding) if isinstance(s,unicode_) else s)
to_unicode = lambda encoding : (lambda b: b.decode(encoding) if isinstance(b,bytes) else b)

def map_strings_recursively(struc, encoding = 'utf8'):

    if callable(encoding):
        xlat = encoding
    else:
        xlat = to_bytes (encoding) # default =simplify json structure for python2.7
    
    if isinstance(struc, dict): return { xlat (k) : map_strings_recursively(v,encoding) for k,v in struc.items() }
    elif isinstance (struc, (list,tuple)): return type(struc)(map_strings_recursively(k,encoding) for k in struc)
    elif isinstance (struc, (unicode_,bytes)): return xlat (struc)
    else: return struc

if __name__ == '__main__': 
       
    test_struct = {
       b"hello" : {
          b"whats" : b"up",
          b"doc":  { b"name" : b"bugs"
                  }
       },
       u"goodbye": [4,u"",5]
    }

    print("\n\n\toriginal structure:\n\n" + pformat(test_struct))

    byte_mapped = map_strings_recursively( test_struct , 
        to_bytes("utf8") 
    )

    print("\n\n\tbytes only:\n\n" + pformat(byte_mapped))

    uni_mapped = map_strings_recursively( test_struct , 
        to_unicode("utf8") 
    )

    print("\n\n\tunicode only:\n\n" + pformat(uni_mapped))

    print("\n\n")

