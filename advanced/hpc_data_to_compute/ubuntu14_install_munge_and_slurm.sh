#!/bin/bash

# -----------------------------------------------------------------------
#  Purpose:
#  Modify an Ubuntu14 system for running data-to-compute example on SLURM
#  
#  Prerequisite : iRODS server should already be installed on the system
#
#  This script installs the following software services:
#
#  MUNGE =  Service for creating and validating credentials
#             (a requirement for running SLURM)
#
#  SLURM =  "Simple Linux Utility for Resource Management"
#             (a popular job scheduler and resource manager)
#
#  Ub-14 system files are also updated to bring up the daemons on reboot
# ------------------------------------------------------------------------

# Import symbolic error codes

DIR=$(dirname "$0")
. "$DIR/errors.rc"

# -- Check for irods service account, die unless it exists

grep '^irods:' /etc/passwd >/dev/null 2>&1 || die NO_IRODS_USER

# -- We'll build and install from this directory:

mkdir -p ~/github

# We'll download source for the build via one of two possible methods: 
#
#   - git clone and checkout using e.g.:
#       http://github.com/dun/munge/
#       http://github.com/SchedMD/slurm/
#     (followed by cd into repo and checkout of desired tag. Slower,
#     but could support easy version upgrading with minor script changes)
#
#   - wget:
#       http://github.com/dun/munge/archive/munge-0.5.13.tar.gz
#       http://github.com/SchedMD/slurm/archive/slurm-17-11-4-1.tar.gz
#
# Set WGET to 1 to prefer 'wget', else use 'git' clone & checkout 

WGET=1

# -- Dictionaries to hold repository path and preferred version info

typeset -A \
  dlPath=( [munge]="dun/munge" [slurm]="SchedMD/slurm" )\
  dlTag=(  [munge]="munge-0.5.13" [slurm]="slurm-17-11-4-1" )

# -- Helper function to download software --

download() {
  local pkg="$1" 
  [ -z "$pkg" ] && exit 125
  [ -d ".old.$pkg" ] && rm -fr ".old.$pkg"/
  [ -d "$pkg" ] && mv "$pkg"/ ".old.$pkg"/
  local fname
  if [ "$WGET" = "1" ] ; then
    fname=${dlTag[$pkg]}.tar.gz
    wget "http://github.com/${dlPath[$pkg]}/archive/$fname" >/dev/null 2>&1 &&\
    tar xf $fname && mv "$pkg"-*/ "$pkg"
  else
    git clone "http://github.com/${dlPath[$pkg]}"
  fi
}

# -------------------------------------------
# The functions that follow are the component
#   parts of the software install
# -------------------------------------------

f_munge_build () {

  # -- Download and build the MUNGE software

  # bash sub-shell to preserve CWD
  (
    cd ~/github && \
    download munge && \
    cd munge && \
    ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var && \
    make && \
    sudo make install
  )

  # Be verbose & change return status if make or install failed.
  # See the 'errors.rc' file for symbolic error codes. 
  # Note also, that:  [ $condition ] || { $statements ; }
  # is the same as:   if [ ! $condition ] ; then $statements ; fi

  [ $? -eq 0 ] || warn MUNGE_BUILD
}

# -------------------------------------------

f_munge_user_install () {

  # -- Make sure we have a munge user and group, the owner of the 'munged' daemon
  #    process and related files

  local STATUS=1
  if ! grep ^munge: /etc/passwd  >/dev/null 2>&1 ; then
    echo >&2 "Adding 'munge' system user"
    sudo adduser --system --group --no-create-home munge && \
    sudo dd if=/dev/urandom of=/etc/munge/munge.key  bs=1k count=1  && \
    sudo chmod 600 /etc/munge/munge.key  && \
    sudo mkdir -p /var/log/munge && \
    sudo chown -R munge:munge /var/run/munge /var/log/munge /etc/munge && \
    STATUS=0
  fi
  [ $STATUS -eq 0 ] || warn MUNGE_USER
}

# -------------------------------------------

f_munge_start () {

  # -- Start the munge daemon

  sudo /etc/init.d/munge start
  echo -n "starting 'munge' daemon ..." >&2
  sleep 2 ; echo >&2
  [ $? -eq 0 ] || warn MUNGED_START
}


# -------------------------------------------


f_munge_daemon_persist ()
{

  # -- Make sure the links in /etc/rc*.d/ exist to start munged
  #    on reboot

  if pgrep munged  2>/dev/null >&2
  then
    sudo update-rc.d munge defaults
  fi
  [ $? -eq 0 ] || warn MUNGED_PERSIST
}


# -------------------------------------------

f_slurm_build_install () {
 
  # -- Build and install the SLURM software

  # bash sub-shell to preserve CWD
  (
    cd ~/github && \
    download slurm && \
    cd slurm && \
    ./configure --with-munge=/usr && \
    make -j3 && \
    make check && \
    sudo make install
  )
  [ $? -eq 0 ] || warn SLURM_BUILD
}

# -------------------------------------------

f_slurm_config ()
{
  # -- Generate the SLURM config file, /usr/local/etc/slurm.conf

  sudo env -i $(/usr/local/sbin/slurmd -C) \
                USER=irods \
                perl -pe 's/\$(\w+)/$ENV{$1}/ge unless /^\s*#/' \
                < "$DIR"/slurm.conf.template                    \
                > /tmp/slurm.conf && \
  sudo cp /tmp/slurm.conf /usr/local/etc && \
  sudo mkdir -p /var/spool/slurm{d,state} && \
  sudo chown -R irods:irods /var/spool/slurm{d,state}
  [ $? -eq 0 ] || warn SLURM_CONFIG
}

# -------------------------------------------

CR=$'\n'
SLURM1="/usr/local/sbin/slurmctld"
SLURM2="/usr/local/sbin/slurmd"

f_slurm_persist ()
{
  sudo pkill 'slurm(ctl|)d' >/dev/null 2>&1
  sudo su - -c "$SLURM1 && $SLURM2" 
  sleep 2
  if [ $(pgrep 'slurm(ctl|)d' | wc -l) -eq 2 ]; then
    egrep '/slurm\w*d' /etc/rc.local >/dev/null 2>&1 && \
      echo "SLURM daemons already referenced in /etc/rc.local " >&2 || \
    sudo env -i SLURMDAEMONS="${SLURM1}${CR}${SLURM2}${CR}" \
        perl -i.orig -pe 's[(\s*exit\s+0\s*)\n*$][$ENV{SLURMDAEMONS}$1]s' \
        /etc/rc.local 
  else
    warn SLURM_START
    return
  fi

  [ $? -eq 0 ] || warn SLURM_PERSIST
}

# -------------------------------------------

menu() { echo >&2 \
"Menu:	1 f_munge_build       
	2 f_munge_user_install   
	3 f_munge_start          
	4 f_munge_daemon_persist 
	5 f_slurm_build_install  
	6 f_slurm_config         
	7 f_slurm_persist
	Q quit "
}

#======================== Main part of the script ========================

if [ $# -eq 0 ] ; then 

  f_munge_build           || exit $?

  f_munge_user_install    || exit $?

  f_munge_start           || exit $?

  f_munge_daemon_persist  || exit $?

  f_slurm_build_install   || exit $?

  f_slurm_config          || exit $?

  f_slurm_persist         || exit $?        

else 

  menu
  while read -p "->" x
  do
    case $x in 
	1) f_munge_build	  ;;
	2) f_munge_user_install   ;;
	3) f_munge_start          ;;
	4) f_munge_daemon_persist ;;
	5) f_slurm_build_install  ;;
	6) f_slurm_config         ;;
	7) f_slurm_persist        ;;
	[Qq]*) exit 0		  ;;
	*) menu ;;
    esac
    echo "Done.  Choice ($x) finished with status: $?)" >&2
  done
fi
