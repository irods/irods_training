#!/bin/sh

EXECUTABLE="/usr/bin/convert"

SRUN_OPTIONS="--nodes=1-1 --ntasks=1 --cpus-per-task=1"

# Allow us time to see jobs via SLURM's squeue
sleep 10

exec srun $SRUN_OPTIONS "$EXECUTABLE" "$@"
