#!/bin/bash

# $1 - executable
# $2 - thumbnail option
# $3 - sizing string
# $4 - source physical path
# $5 - destination physical path

# == Carry in context from slurm_prolog :

# MUTX=$(hostname)_slurmjob_${SLURM_JOB_ID}.txt
# UUID=$(iquest '%s' "select META_DATA_ATTR_VALUE where \
# DATA_NAME = '$MUTX'  and \
# META_DATA_ATTR_NAME = 'uuid' and \
# COLL_NAME not like '%/trash/%'")
# [[ $UUID = CAT_NO_ROWS_FOUND* ]] && UUID="UUID_none"
# [ -f "/tmp/$UUID" ] && { : ; } 
# ( /tmp/$UUID could stash private, job-related 
#   details /  metadata on slurm controller )

SBATCH_OPTIONS="-o /tmp/slurm-%j.out"

SCRIPT="$1" # assume full path to executable

/usr/local/bin/sbatch $SBATCH_OPTIONS "$SCRIPT" \
	${2+"$2"} \
	${3+"$3"} \
	${4+"$4"} \
	${5+"$5"} \
	>/dev/null 2>&1

