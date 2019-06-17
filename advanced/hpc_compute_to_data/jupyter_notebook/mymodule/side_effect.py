def do_stuff(nm):
  with open('/tmp/'+nm,'w') as f:
     f.write('hello, {0}.\ngoodbye, {0}.\n'.format(nm))
