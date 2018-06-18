#!/bin/bash

. /var/lib/irods/compute/irods_compute_functions

EXECUTABLE=~irods/msiExecCmd_bin/wrap_singularity

outputDir=""

while [[ $1 = --* ]] ; do

 if [ "$1" = "--postproc" ]; then
   if test_irods_slurm_desc ; then
     set_irods_slurm_var  "post_processing"  "$2"
   fi
 fi

  if [ "$1" = "--outdir" ]; then
    outputDir="$2"
  fi 

  if [ "$1" = "--size" ]; then
    sizeStr="$2"
  fi 

  shift 2

done 

if [ -n "$outputDir" ]; then
  mkdir -p "$outputDir"
else
  outputDir="/tmp"
fi

#SRUN_OPTIONS="--nodes=1-1 --ntasks=1 --cpus-per-task=1" 
#exec #srun $SRUN_OPTIONS  "EXECUTABLE" "$@"

"$EXECUTABLE" "$@"

#[ -f "${outputDir}/mdmanifest.json" -a -n "$sizeStr" ] && \
#  mv "${outputDir}/mdmanifest.json" "${outputDir}/${sizeStr}_mdmanifest.json"

