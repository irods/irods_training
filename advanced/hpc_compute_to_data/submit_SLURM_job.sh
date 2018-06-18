#!/bin/bash

set -e 

# $1 - executable
# $2 - thumbnail option
# $3 - sizing string
# $4 - source physical path
# $5 - destination physical path

SBATCH_OPTIONS="-o /tmp/slurm-%j.out"

SCRIPT="$1" # assume full path to executable
shift

/usr/local/bin/sbatch $SBATCH_OPTIONS "$SCRIPT" "$@" 2>&1 
# >/dev/null 2>&1

