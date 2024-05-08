#!/bin/bash

# -----------------------------------------------------------------------
#  Purpose:
#  Modify an Ubuntu system for running data-to-compute example on SLURM
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
# ------------------------------------------------------------------------

SLURM_CONFIG_DIR=/etc/slurm
SLURM_ADMIN=/var/lib/slurm
IRODS_MSIEXEC=~irods/msiExecCmd_bin
IRODS_COMPUTE=~irods/compute

# Import symbolic error codes

DIR=$(dirname "$0")
. "$DIR/errors.rc"

add_error BAD_OPTION      "Error in program option or argument"			# 1
add_error NO_IRODS_USER   "No irods user (please install iRODS software)"	# 2

# -- Check for irods service account, die unless it exists

grep '^irods:' /etc/passwd >/dev/null 2>&1 || die NO_IRODS_USER

f_slurm_install () {
  sudo apt install -y slurmd slurmctld slurm-client
}

copy_scripts_ ()
{
  local TYPE DEST BASE
  case $1 in 
    epilog)
	TYPE=$1;;
    prolog)
	TYPE=$1;;
    *)
		echo >&2 "----------------------------------------------------------------------"
		echo >&2 "At this time only SLURM 'prolog' and 'epilog' scripts can be installed" 
		return ;;
  esac
  REF="${2}"

  # -- Write the root_{prolog,epilog} scripts that will call the necessary pre- and post-execution
  # -- hooks as user "irods"

  sudo dd of="$SLURM_ADMIN/root_$TYPE" <<-EOF 2>/dev/null
	#!/bin/bash
	IRODS_HOOK="$IRODS_COMPUTE"/slurm_$TYPE
	if [ -x "\$IRODS_HOOK" ]; then
	  su irods -c "\$IRODS_HOOK"
	fi
	EOF
  # ^-- tab necessary for <<-EOF!

  
  sudo chmod a+rx "$SLURM_ADMIN/root_$TYPE"

  BASE="slurm_$TYPE"
  DEST="$IRODS_COMPUTE/$BASE"
  sudo su irods -c "touch '$DEST'" && \
  [ -f "$DIR"/$REF/"$BASE" ] && \
      sudo cp -p "$DIR"/$REF/"$BASE" "$DEST" && \
  [ -f "$DEST" ] && \
      sudo chmod go+rx,u+rwx "$DEST" && \
      sudo chown irods:irods "$DEST"  
}

f_slurm_config ()
{
  # -- Generate the SLURM configuration and the epilog and prolog scripts.

  sudo env -i $(slurmd -C) \
                SLURM_ADMIN=$SLURM_ADMIN \
                perl -pe 's/\$(\w+)/$ENV{$1}/ge unless /^\s*#/' \
                < "$DIR"/../slurm.conf.template.ubuntu22        \
                > /tmp/slurm.conf && \
  sudo cp /tmp/slurm.conf $SLURM_CONFIG_DIR/. && \
  sudo mkdir -p $IRODS_COMPUTE && \
  sudo chown irods:irods $IRODS_COMPUTE && \
  sudo chmod ug=rwx,o=rx $IRODS_COMPUTE && \
  copy_scripts_ prolog .. && \
  copy_scripts_ epilog ..
  [ $? -eq 0 ] || warn SLURM_CONFIG
}

# -------------------------------------------

menu() { echo >&2 \
"Menu:
	1 f_slurm_install
	2 f_slurm_config         

	Q quit "
}

#======================== Main part of the script ========================

if [ $# -eq 0 ] ; then  

  #-- If no input arguments, then run all install stages

  f_slurm_install         || exit $?
  f_slurm_config          || exit $?

else

  #-- Interactive / Menu driven

  x="."
  if [[ $1 =~ [0-9]+ ]]
  then
    x=$1
  else
    [ $# -gt 0 -a -n "$1" ] && menu && echo "
	No argument: complete install
	Empty argument (''): menu-driven install
	Integer argument: execute one phase of install
    " >&2 && die BAD_OPTION
  fi
  while [ -n "$x" ] || read -p "->" x
  do
    case $x in 
	1) f_slurm_install         ;;
	2) f_slurm_config         ;;
	[Qq]*) exit 0		  ;;
	*) menu ;;
    esac
    echo "Done.  Choice ($x) finished with status: $?" >&2
    x=""
  done
fi

