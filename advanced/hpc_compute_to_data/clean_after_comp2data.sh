#!/bin/bash

echo >&2 "* Clearing out iRODS trash directories  ... "

irmtrash
sudo su irods -c irmtrash 

echo >&2 "* Getting rid of thumbnails in top level collection ..."

icd

{ for SZ in 128 256 512 1024 ; do
    irm -f stickers_${SZ}x${SZ}.jpg
  done
} >/dev/null 2>&1

#------------------------------------------------------------------------------

echo >&2 "* Clearing out any compute results left on file system ..."

sudo rm -fr /tmp/irods/thumbnails

#------------------------------------------------------------------------------
echo >&2 "* Getting rid of old SLURM job id's in iRODS ... "

TO_DELETE=($( sudo su - irods -c "iquest '%s/%s' \"select COLL_NAME,DATA_NAME 
              where DATA_NAME like '%slurmjob%' 
              and COLL_NAME not like '%/trash/%'\" "))

if [[ $TO_DELETE = *CAT_NO_ROWS_FOUND*  || $TO_DELETE = '' ]] ; then
   true
else
   TO_DELETE_ARRAY=($TO_DELETE)
   OLDIFS="$IFS"; IFS=" "
   sudo su irods -c 'irm -f '"${TO_DELETE_ARRAY[*]}"
   IFS="$OLDIFS"
fi

#------------------------------------------------------------------------------
echo >&2 "* recycling Slurm daemon and clearing out all state information ... " 

for  cmd in stop clean status start 
do
  sudo /etc/init.d/slurm $cmd
done
