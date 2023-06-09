#!/bin/bash

# $1 - executable
# $2 - thumbnail option
# $3 - sizing string
# $4 - source physical path
# $5 - destination physical path

SBATCH_OPTIONS="-o /tmp/slurm-%j.out"

SCRIPT="$1" # assume full path to executable

sbatch $SBATCH_OPTIONS "$SCRIPT" \
	${2+"$2"} \
	${3+"$3"} \
	${4+"$4"} \
	${5+"$5"} \
	>/dev/null 2>&1

