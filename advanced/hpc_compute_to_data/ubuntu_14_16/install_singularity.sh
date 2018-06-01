#!/bin/bash

# -----------------------------------------------------------------------
#  Purpose:
#  Modify an Ubuntu system for running data-to-compute example on SLURM
#  
#  Prerequisite : iRODS server should already be installed on the system
#
#  This script installs the following software services:
#
#  Singularity - application container technology from Berkeley Labs
#    ( downloads : singularity.lbl.gov / singularity-hub.org )
#
# ------------------------------------------------------------------------

IRODS_COMPUTE=~irods/compute

# Import symbolic error codes

DIR=$(dirname "$0")
. "$DIR/errors.rc"


add_error BAD_OPTION        "Error in program option or argument"	# 1
add_error SINGULARITY_BUILD "Singularity build (or install) failed"	# 2
add_error SINGULARITY_PULL  "Pull of Singularity images failed"		# 3

# =-=-=-=-=-=-=
# Build and install from this directory:

mkdir -p ~/github

WGET=1

typeset -A dlPath=( [singularity]="singularityware/singularity"	   )\
	   dlTag=(  [singularity]="2.5.1"                          )\
	   bldReq=( [singularity]="squashfs-tools libarchive-dev autoconf" )

download() {
  local pkg="$1" 
  [ -z "$pkg" ] && exit 125
  printf $'\n\t*** Downloading and building: "%s" ***\n\n' "$pkg" >&2
  [ -d ".old.$pkg" ] && rm -fr ".old.$pkg"/
  [ -d "$pkg" ] && mv "$pkg"/ ".old.$pkg"/
  local reqs="${bldReq[$pkg]}"
  if [ -n "$reqs" ] ; then  # -- satisfy build requirements, if there are any
    echo -n "installing ( $reqs ) before download ..." >&2
    sudo apt-get -qq -y install $reqs || die NEED_PREREQ
    echo >&2
  fi
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

f_singularity_install() 
{
           OLDDIR=$(pwd)
           mkdir -p ~/github && \
           cd ~/github && download singularity && \
           cd singularity  && \
           ./autogen.sh  && \
           ./configure --prefix=/usr/local  && \
           make && \
           sudo make install
           STATUS=$?
           cd "$OLDDIR"
           [ $? -eq 0 ] || warn SINGULARITY_BUILD
}

#--------------------------------------

. "$DIR/../singularity_images.conf"

f_singularity_pull() 
{
  local images=(
    "${!Singularity_Sources[@]}" # -- sourced from 'singularity_images.conf'
  )
  for key in "${!images[@]}"
  do
    sudo su - irods -c \
    " # cd $IRODS_COMPUTE  \\
      singularity pull --name '${images[$key]}.simg'" \
         || { warn SINGULARITY_PULL ; break ; }
  done
}

# -------------------------------------------

menu() { echo >&2 \
"Menu
        :::::: Singularity set-up ::::::

        1 : install Singularity.

        2 : pull compute-to-data images to
              run under Singularity.

	Q quit "
}

# --- main install ---

if [ $# -eq 0 ] ; then  

  #-- Automatic run-through of all install stages

   f_singularity_install     || exit $?

   f_singularity_pull        || exit $?        

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
	1)
           f_singularity_install  ;;
	2)
           f_singularity_pull  ;;

	99*) echo >&2 " ** (: Nines ;) ** ";;

	[Qq]*) exit 0		  ;;

	*) menu ;;
    esac
    echo "Done.  Choice ($x) finished with status: $?" >&2
    x=""
  done
fi

