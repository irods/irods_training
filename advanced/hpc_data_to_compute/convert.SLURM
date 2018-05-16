#!/bin/sh

EXECUTABLE="/usr/bin/convert"

SRUN_OPTIONS="--nodes=1-1 --ntasks=1 --cpus-per-task=1"

MUTX=${SLURM_CLUSTER_NAME}_slurmjob_${SLURM_JOB_ID}.txt
UUID=$(iquest '%s' "select META_DATA_ATTR_VALUE where \
 DATA_NAME = '$MUTX'  and \
 META_DATA_ATTR_NAME = 'uuid' and \
 COLL_NAME not like '%/trash/%'")
[[ $UUID = CAT_NO_ROWS_FOUND* ]] && UUID="UUID_none"
if [ -f "/tmp/$UUID" ] ; then
  imeta add -d $MUTX size_string $2
else
  echo "couldn't locate the UUID"
fi

# Allow us time to see jobs via SLURM's squeue
sleep 10

exec srun $SRUN_OPTIONS "$EXECUTABLE" "$@"
