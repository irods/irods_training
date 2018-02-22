#!/bin/bash

# $1 - executable
# $2 - thumbnail option
# $3 - sizing string
# $4 - source physical path
# $5 - destination physical path

/usr/bin/condor_submit /var/lib/irods/msiExecCmd_bin/thumbnail.submit -append "executable ${1}" -append "arguments ${2} ${3} ${4} ${5}"

# debug only
#echo ${1}
#echo ${2}
#echo ${3}
#echo ${4}
#echo ${5}
#echo "return code is $?"
#exit 0

