#! /bin/bash
### BEGIN INIT INFO
# Provides:          slurm
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Required-Start:      munge
# Required-Stop:       munge
### END INIT INFO


# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script

DESC="Slurm basic init script"
NAME="slurm"
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME="$0"

do_clean()
{
	rm -fr /var/spool/slurm{state,d}/*
	true
}

#
# Function that starts the daemon/service
#
do_start()
{
  local X retry_sec=3 RETVAL=1 ntry=0

  /usr/local/sbin/slurmd
  /usr/local/sbin/slurmctld

  while [ $((--retry_sec)) -ge 0 ]; do
    # give SLURM procs a time to die off, if they're going to....
    ((++ntry))
    X=$(pgrep 'slurm(ctl|)d')
    if [ $(echo $X|wc -w) -eq 2 -a $ntry -gt 1 ] ; then
	RETVAL=0
	echo "$X" >$PIDFILE
	break
    fi
    sleep 1   
  done
  return $RETVAL
}

do_stop()
{
  local X retry_sec=5
  if [ -f $PIDFILE ]; then
    kill $(cat $PIDFILE)
  else
    pkill -f '/slurm'
  fi 
  rm -f $PIDFILE
  RETVAL=1
  while [ $((--retry_sec)) -ge 0 ]; do
    X=$(pgrep 'slurm(ctl|)d')
    [ $(echo "$X"|wc -w) -eq 0 ]  && { RETVAL=0; break; }
  done
  return $RETVAL
}

do_status()
{
  local x
  if [ -f $PIDFILE ]; then
    kill -0 $(cat $PIDFILE) >/dev/null 2>&1
  else
    [ $(pgrep 'slurm(ctl|)d' |wc -l) -eq 2 ]
  fi
  if [ $? -eq 0 ]; then
    echo "SLURM is up"
  else
    echo "SLURM is down"
  fi
  echo '-----'
}

case "$1" in
  start)
	do_start
	case "$?" in
		0)   echo >&2 "Started SLURM daemons"
			;;
		*) exit 1
			;;
	esac
	;;
  stop)
	do_stop
	case "$?" in
		0)   echo >&2 "Stopped SLURM daemons"
			;;
		*) exit 1
			;;
	esac
	;;
  clean)
	do_clean
	;;
  status)
	do_status
	echo -n "slurmstate files : "
	find  /var/spool/slurmstate -type f |wc -l
	echo -n "slurmd     files : "
	find  /var/spool/slurmd     -type f |wc -l
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop|status|clean}" >&2
	exit 3
	;;
esac

:
