from genquery  import row_iterator, AS_DICT, AS_LIST
import session_vars

## --------------- utility functions --------------

def make_logger (callback,strm='serverLog'):
    return  lambda s: callback.writeLine( strm, s )

def is_collection_R( rule_args, callback, rei):
    pr = make_logger(callback,'stdout')
    pr (
      str(is_collection (callback, rule_args[0]))
    )

def is_collection( callback, colln ):
    return 0 != len([col[0] for col in row_iterator ( "COLL_NAME", "COLL_NAME = '{}'".format(colln) , AS_LIST, callback)][:1])

def _irods_error_identifier(e):
      import re
      m_status = re.search( 'status\s*\[\s*(\S*?)\s*\]', e.message )
      return m_status.groups(1)[0] if m_status is not None else ''

def _irods_error_number(e):
      import re
      m_errcode = re.search ('\[iRods__Error__Code:\s*(\S*?)\s*\]', e.message)
      return int(m_errcode.groups(1)[0]) if m_errcode is not None else ''

def create_collection_R (rule_args,callback,rei):
    coll_name = rule_args[0]
    rule_args[1] = "1" if create_collection (callback, coll_name)  else ""

def create_collection (callback, coll_name ):
    pr = make_logger(callback,'stdout')
    rv = None
    status = ""
    try:
        rv = callback.msiCollCreate (coll_name , "0", 0)
    except RuntimeError as e: # - log exact error code 
        status = _irods_error_identifier(e)
        errcode = _irods_error_number(e)
        pr ("status = {status}; errcode  = {errcode}".format(**locals()))
    return rv['status'] if rv else False

def set_acl_inherit (args,callback,rei):
    objpath = args[0]
    user = args[1]
    rv = callback.msiSetACL ("recursive", "admin:inherit", user, objpath)

def set_acl_own (args,callback,rei):
    pr = make_logger (callback)
    objpath = args[0]
    user = args[1]
    rv = callback.msiSetACL ("default", "admin:own", user, objpath)

#==================
#    utility
#==================

def split_irods_path( s ):
    elem = s.split("/")
    return "/".join(elem[:-1]),elem[-1]

def this_host_tied_to_resc_R( args, callback, rei ): # test rule
    pr = make_logger(callback,'stdout')
    yn = this_host_tied_to_resc(callback, args[0] )
    pr ('tied to resc {} -> {}' .format(args[0],yn))


def this_host_tied_to_resc(callback, resc ):
    import socket
    this_host = socket.gethostname()
    tied_host = ""
    for rescvault in row_iterator( 'RESC_LOC',"RESC_NAME = '{resc}'".format(**locals()), AS_DICT,callback):
        tied_host = rescvault['RESC_LOC']
    return this_host == tied_host


def data_object_physical_path_in_vault_R(args, callback, rei):  # - test function (call as rule)
    pr = make_logger(callback,'stdout')
    v_retn = {}
    phy = data_object_physical_path_in_vault (callback, *args[:3] , vault_validate = v_retn)
    pr('phy = %r'%phy)
    pr('v_retn = %r'%v_retn)


#--------
# Find (opt. force instantiation of) a data object path on the desired [resc] .
# In the case of an output directory, we'll have to create a dummy data object because collection's
#  corresponding directories do not autovivify in a vault without a data replica to drive the process.
#
# Iff 'vault_relative_path' is a key in [vault_validate] upon return, the data object can be assumed local
# and accessible at the given path, which is relative to the vault path returned by the function.
#
# NB this function does not create the collection itself; it is assumed to have been created already.

def data_object_physical_path_in_vault(callback, logical_path, resc, force_creation_on_resc, vault_validate = None):

    #pr = make_logger(callback,'stderr')

    collection, dataobj = split_irods_path( logical_path )

    status = _data_object_exists_targeting_resc( callback, resc, collection, dataobj )

    if status != 'already-exists' and force_creation_on_resc.upper() not in ('', 'N','NO','0') :
        close_rv = {} ; repl_rv  = {} ; close_rv = {}
        if (status == 'need-repl'):
            repl_rv = callback.msiDataObjRepl(logical_path, "destRescName={}".format(resc), 0)
            #pr("repl  status = %r arg-retn %r " % (repl_rv['status'],repl_rv['arguments'][2]))
        else:
            create_rv = callback.msiDataObjCreate(logical_path, "forceFlag=++++destRescName={}".format(resc), 0)
            descriptor = create_rv['arguments'][2]
            if type(descriptor) is int and descriptor > 0:
                close_rv =  callback.msiDataObjClose(descriptor,0)
                #pr("close(%r) status = %r arg-retn %r " % (descriptor,close_rv['status'],close_rv['arguments'][1]))
    v = {}

    if type(vault_validate) is dict:    # - get vault path to match against data object phys. path
        v = vault_validate

    resource_vault_path = ""
    for rescvault in row_iterator( 'RESC_VAULT_PATH',"RESC_NAME = '{resc}'".format(**locals()), AS_DICT,callback ):
        resource_vault_path = rescvault['RESC_VAULT_PATH']

    phys_path = ''
    for p in row_iterator("DATA_PATH",
                          "DATA_RESC_NAME = '{resc}' and DATA_NAME = '{dataobj}' and COLL_NAME = '{collection}' ".format(**locals()),
                          AS_DICT,callback):
        phys_path = p['DATA_PATH']

    if resource_vault_path != '' and phys_path.startswith( resource_vault_path ):
        v['vault_relative_path'] = phys_path [len(resource_vault_path):].lstrip('/')

    return resource_vault_path

def data_object_exists_targeting_resc_R( rule_args, callback, rei): # test rule
    pr = make_logger(callback,'stdout')
    pr ( "data obj exists = %r" %  data_object_exists_on_resc(callback, *rule_args[:3]) )

def _data_object_exists_targeting_resc( callback, resc, coll, base_data_name ):
    rescs_having_obj = [ col['RESC_NAME'] for col in row_iterator ( "RESC_NAME" , "COLL_NAME = '{}' and DATA_NAME = '{}'".format(coll,base_data_name),
                                                                    AS_DICT, callback)]
    if len( rescs_having_obj ) > 0 :
        return 'need-repl' if (resc not in rescs_having_obj) else 'already-exists'
    return '' # - did not find  data object on any resc

#==================
# other necessaries
#==================

def user_id_for_name(callback, username):

    user_id=""
    for i in row_iterator( 'USER_ID,USER_NAME', "USER_NAME = '{}'".format(username), AS_DICT, callback):
        if 0 == len(user_id):
          user_id = i['USER_ID']
    return user_id

def get_user_name (callback,rei):
    u = ''
    try:
        u = session_vars.get_map(rei)['client_user']['user_name']
    except: pass
    return u

def user_has_access_R(rule_args, callback, rei ): # test rule

    pr = make_logger(callback,'stdout')
    username = rule_args[0]
    if username == "" : username = get_user_name(callback,rei)

    if username != "" :
        access_type_name = rule_args[1]
        datobj = rule_args[2]
        colln = rule_args[3]
        a = user_has_access  (callback, rei, username, access_type_name, data_object_path=datobj, collection_path=colln)
        pr ('access = ' + repr(a))
    else:
        pr ("username could not be determined")

def user_has_access (callback, rei, username, access_type_name, data_object_path='', collection_path=''):

    access = False
    access_types = { 'write':'1120', 'read':'1050', 'own':'1200' }
    user_ID = user_id_for_name (callback,username)

    do_query = (user_ID != '')

    if data_object_path and not collection_path :

        coll_name , data_name = split_irods_path (data_object_path)

        condition = "DATA_NAME = '{0}' and  COLL_NAME = '{1}' "\
                    "and DATA_ACCESS_USER_ID = '{2}' and DATA_ACCESS_TYPE >= '{3}'".format(
                    data_name, coll_name, user_ID, access_types[access_type_name] )

    elif collection_path and not data_object_path:

        condition = "COLL_NAME = '{0}' and COLL_ACCESS_USER_ID = '{1}' and COLL_ACCESS_TYPE >= '{2}' ".format(
                    collection_path, user_ID, access_types[access_type_name] )

    else :
        do_query = False

    if do_query:
        for i in row_iterator( "COLL_NAME", condition, AS_LIST, callback):
            access = True

    return access

#===========================================================================

def replicate_data_objects( rule_args , callback , rei):

    from_object = rule_args[0]
    to_resource = rule_args[1]
    from_resource = rule_args[2]
    trim_after_replication = rule_args[3]

    if len(to_resource) == 0 or len(from_object) == 0: return

    path,obj = split_irods_path (from_object)

    condition = "COLL_NAME = '{0}' and DATA_NAME = '{1}' ".format(path,obj)
    if from_resource:
        condition += " and DATA_RESC_NAME = '{}' ".format(from_resource)

    data_objects = list(row_iterator('DATA_NAME,COLL_NAME,DATA_RESC_NAME,DATA_REPL_NUM', condition, AS_DICT, callback))

    if not(data_objects):
        condition = "COLL_NAME = '{0}' || like '{0}/%' " .format (from_object)
        if from_resource:
            condition += " and DATA_RESC_NAME = '{}'".format(from_resource)
        data_objects = list(row_iterator('DATA_NAME,COLL_NAME,DATA_RESC_NAME,DATA_REPL_NUM', condition, AS_DICT, callback))

    replicated = {}

    for dobj in data_objects:

        full_path = "{COLL_NAME}/{DATA_NAME}".format(**dobj)
        if dobj['DATA_RESC_NAME'] == to_resource:
            replicated[full_path] = True
        else:
            old_replication_status = replicated.get(full_path, False)

            if not old_replication_status:
                #callback.writeLine("stderr", "replicating: \n" + pprint.pformat(dobj))
                retval = callback.msiDataObjRepl( full_path, "destRescName={0}".format(to_resource),0)
                new_replication_status = retval['status']
                replicated [full_path] = new_replication_status

            if new_replication_status and not(old_replication_status) and trim_after_replication and \
             dobj['DATA_RESC_NAME'] == from_resource != "":

                trim_retval = callback.msiDataObjTrim( "{COLL_NAME}/{DATA_NAME}".format(**dobj), "null",
                                                       dobj['DATA_REPL_NUM'], "1", "null", 0)

