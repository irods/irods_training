#!/bin/bash

. /var/lib/irods/compute/irods_compute_functions

EXECUTABLE=~irods/msiExecCmd_bin/wrap_singularity

check_user_pw () {
  local x=0 pw_opt_only="${1%% -u*}"
  local pw=${pw_opt_only#*-P}
  local remainder=${1:${#pw_opt_only}+3}
  while [ "${remainder:$x:1}" = " " ] ; do ((++x)); done
  local user=${remainder:$x}; user=${user%% *}
  echo " -u "$remainder
  singularity run --app auth_user_with_pw \
    /var/lib/irods/compute/metadata_addtags.simg \
    $user $pw 
}

outputDir=""

while [[ $1 = --* ]] ; do

  if [ "$1" = "--postproc" ] && test_irods_slurm_desc ; then

    #postproc_opts="$2" #-- if "-P pwd" not expected at start
    postproc_opts=$(check_user_pw "$2") 

    if [ $? -eq 0 ] ; then
      set_irods_slurm_var "post_processing" "$(head -1 <<<"$postproc_opts")"
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


