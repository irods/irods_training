#!/bin/bash

. /var/lib/irods/compute/irods_compute_functions

EXECUTABLE="/usr/bin/convert"

SRUN_OPTIONS="--nodes=1-1 --ntasks=1 --cpus-per-task=1"

if test_irods_slurm_desc ; then
  set_irods_slurm_var  "size_string"  "$2"
fi

# Allow us time to see jobs via SLURM's squeue
sleep 10

exec srun $SRUN_OPTIONS "$EXECUTABLE" "$@"
