#!/bin/sh

## Copyright (c) 2009 Data Intensive Cyberinfrastructure Foundation. All rights reserved.
## For full copyright notice please refer to files in the COPYRIGHT directory
## Written by Jean-Yves Nief of CCIN2P3 and copyright assigned to Data Intensive Cyberinfrastructure Foundation

# This script is a template which must be updated if one wants to use the universal MSS driver.
# Your working version should be in this directory server/bin/cmd/univMSSInterface.sh.
# Functions to modify: syncToArch, stageToCache, mkdir, chmod, rm, stat
# These functions need one or two input parameters which should be named $1 and $2.
# If some of these functions are not implemented for your MSS, just let this function as it is.
#

# function for the synchronization of file $1 on local disk resource to file $2 in the MSS
syncToArch () {
	date >> /tmp/univmss
	echo "syncToArch $1 $2" /usr/local/bin/arccp -R 5 "$1" "srm://srm.swegrid.se/snic/uppnex$2" >> /tmp/univmss


	


	# <your command or script to copy from cache to MSS> $1 $2 
	# e.g: /usr/local/bin/rfcp $1 rfioServerFoo:$2
	# Get vault path for cache resource this is needed to get chksum from ICAT
	# ilsresc -l sweStoreCache |grep vault|awk '{print $2}'
	# Hardcode to minimize operations. 
        VAULT="\/data\/rescs\/swestoreArchCacheResc"
	# Hack to translate system path to irods filespace path	
	iPATH=`echo "$1" | sed 's/'$VAULT'/\/ssUppnexZone/'`       
        md5=`isysmeta ls -l "$iPATH" |grep data_checksum |awk '{print $3}' |head -1`


	if /usr/local/bin/arcls -m -n "srm://srm.swegrid.se/snic/uppnex/$2" 2>/dev/null >/dev/null; then
	    # File exists already?

	adler=`python -c 'import zlib;import sys
s=1
while True:
 r=sys.stdin.read(65536) 
 s=zlib.adler32(r,s)
 if len(r) != 65536:    
   print "%x" % s 
   raise SystemExit' < "$iPATH"`

	size=`stat -c%s "$iPATH"`

        # arcls leftpads with 0s to get even numbers (bytes)
	if (( ${#adler} % 2 )); then
	    adler=0$adler
	fi
	
	tmpfile=/tmp/univmss_verify.$$

	
	/usr/local/bin/arcls -m -n "srm://srm.swegrid.se/snic/uppnex/$2" 2>/dev/null >$tmpfile
	
	if grep -q "^size:$size\$" "$tmpfile" ; then
	    :
	else
	    rm -f "$tmpfile"
	    return 1
	fi
	
	if grep -q "^checksum:adler32:$adler\$" "$tmpfile" ; then
	    :
	else
	    rm -f "$tmpfile"
	    return 1
	fi
	
	rm -f "$tmpfile"
	return 0
	
	fi


        if [ -n "$md5" ]
        then
	  /usr/local/bin/arccp -R 5 "$1" "srm://srm.swegrid.se/snic/uppnex/$2:checksumtype=md5:checksumvalue=$md5"
	else
	  # md5sum do not exist in iCAT calculate with ichksum
	  # Iadmin cant chksum other users file
	  #Rods user is now member of all collections and can calculate and register md5sum
	  md5=`ichksum -K "$iPATH" | awk '{print $2}'|head -1`
	  #forced to calculate md5sum on file..
	  #md5=`md5sum "$1"|awk '{print $1}'` 
  	  /usr/local/bin/arccp -R 5 "$1" "srm://srm.swegrid.se/snic/uppnex/$2:checksumtype=md5:checksumvalue=$md5"
	fi      
        return
}

# function for staging a file $1 from the MSS to file $2 on disk
stageToCache () {
        date >> /tmp/univmss
        echo "stageToCache $1 $2"    /usr/local/bin/arccp -R 5 "srm://srm.swegrid.se/snic/uppnex$1" "$2" >> /tmp/univmss


	# <your command to stage from MSS to cache> $1 $2	
	# e.g: /usr/local/bin/rfcp rfioServerFoo:$1 $2
	if [ -e "$2" ]; 
		then 
		rm -rf "$2"; 
		fi
#	echo $1,$2 >/opt/irods/debugstage.txt;
	/usr/local/bin/arccp -R 5 "srm://srm.swegrid.se/snic/uppnex$1" "$2"
        return
}

# function to create a new directory $1 in the MSS logical name space
mkdir () {
	date >> /tmp/univmss
	echo "mkdir $1"   /usr/local/bin/arcmkdir "srm://srm.swegrid.se/snic/uppnex$1"  >> /tmp/univmss
	# <your command to make a directory in the MSS> $1
	# e.g.: /usr/local/bin/rfmkdir -p rfioServerFoo:$1
	#/opt/d-cache/srm/bin/srmmkdir srm://srm.swegrid.se/snic/uppnex$1
        /usr/local/bin/arcmkdir "srm://srm.swegrid.se/snic/uppnex$1"
	return
}

# function to modify ACLs $2 (octal) in the MSS logical name space for a given directory $1 
chmod () {
	# <your command to modify ACL> $1 $2
	# e.g: /usr/local/bin/rfchmod $2 rfioServerFoo:$1

	return
}

# function to remove a file $1 from the MSS
rm () {
	# <your command to remove a file from the MSS> $1
	# e.g: /usr/local/bin/rfrm rfioServerFoo:$1
#        echo $1,$2 >/opt/irods/debugrm.txt;
	/usr/local/bin/arcrm -t 30 "srm://srm.swegrid.se/snic/uppnex$1"
	return
}

# function to rename a file $1 into $2 in the MSS
mv () {
	date >> /tmp/univmss
        echo "mv $1 $2" /usr/local/bin/arcrename "srm://srm.swegrid.se/snic/uppnex$1" "srm://srm.swegrid.se/snic/uppnex$2"  >> /tmp/univmss

       # <your command to rename a file in the MSS> $1 $2
       # e.g: /usr/local/bin/rfrename rfioServerFoo:$1 rfioServerFoo:$2


	/usr/local/bin/arcmkdir -p `dirname "srm://srm.swegrid.se/snic/uppnex$2"` 
	/usr/local/bin/arcrename "srm://srm.swegrid.se/snic/uppnex$1" "srm://srm.swegrid.se/snic/uppnex$2"
       return
}

# function to do a stat on a file $1 stored in the MSS
stat () {
	date >> /tmp/univmss
	echo "stat $1" /usr/local/bin/arcls -nl srm://srm.swegrid.se/snic/uppnex$1 >>/tmp/univmss
	# <your command to retrieve stats on the file> $1
        output=`/usr/local/bin/arcls -nl srm://srm.swegrid.se/snic/uppnex$1`
#	echo $output
	error=$?
	if [ $error != 0 ] # if file does not exist or information not available
	then
		return $error
	fi
	# parse the output.
	# Parameters to retrieve: device ID of device containing file("device"), 
	#                         file serial number ("inode"), ACL mode in octal ("mode"),
	#                         number of hard links to the file ("nlink"),
	#                         user id of file ("uid"), group id of file ("gid"),
	#                         device id ("devid"), file size ("size"), last access time ("atime"),
	#                         last modification time ("mtime"), last change time ("ctime"),
	#                         block size in bytes ("blksize"), number of blocks ("blkcnt")
	device=0
	inode=0
	mode=0
	nlink=0
	uid=0
	gid=0
	devid=0
	blksize=0
	blkcnt=0
	atime=0
	mtime=0
	ctime=`echo "$output" |grep "$1" | awk '{print $4"-"$5}' |sed 's/:/./g'`	
	if [ -z "$ctime" ]
        then
          ctime=0
	fi
	size=`echo "$output" |grep "$1" | awk '{print $3}'`	
	if [ -z "$size" ]
        then
          size=0
	fi
	# Note 1: if some of these parameters are not relevant, set them to 0.
	# Note 2: the time should have this format: YYYY-MM-dd-hh.mm.ss with: 
	#                                           YYYY = 1900 to 2xxxx, MM = 1 to 12, dd = 1 to 31,
	#                                           hh = 0 to 24, mm = 0 to 59, ss = 0 to 59
	echo "$device:$inode:$mode:$nlink:$uid:$gid:$devid:$size:$blksize:$blkcnt:$atime:$mtime:$ctime"
	return
}

#############################################
# below this line, nothing should be changed.
#############################################

case "$1" in
	syncToArch ) "$1" "$2" "$3" ;;
	stageToCache ) "$1" "$2" "$3" ;;
	mkdir ) "$1" "$2" ;;
	chmod ) "$1" "$2" "$3" ;;
	rm ) "$1" "$2" ;;
	mv ) "$1" "$2" "$3" ;;
	stat ) "$1" "$2" ;;
esac

exit $?
