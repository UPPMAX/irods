#!/bin/bash

# Swestore path
sspath=srm://srm.swegrid.se/snic/uppnex/arch;

basepath="/swestoreArchZone/proj";

icd $basepath;
curmonth=`date +%Y%m`;
srmpath="srm://srm.swegrid.se/snic/uppnex/arch/proj";

for p in $(arcls $srmpath); do
  projpath=$srmpath/$p;
  # Check if project folder exists
  arcls $projpath &> /dev/null
  if [[ 0 == $? ]]; then
    for d in $(arcls $projpath); do    
      # Limit ireg operations to arch misssions of current month
      if [[ $d =~ $curmonth ]]; then
        dir=$basepath/$p/$d;
        # Check if directory exists in iRODS
        ils $dir &> /dev/null;
        if [[ 0 != $? ]]; then
          echo "Directory $dir missing, so creating ...";
          imkdir -p $dir;
        fi
        for f in $(arcls $sspath/proj/$p/$d); do
          ils $dir/$f &> /dev/null;
          if [[ 0 != $? ]]; then
            echo "Now ireg:ing file $f ...";
            # The actual ireg command
            ireg -R swestoreArchResc -G swestoreArchGrp /proj/$p/$d/$f /swestoreArchZone/proj/$p/$d/$f;
          fi
        done;
      fi;
    done;
  fi;
done;

icd $basepath;
# Make sure projects are only readeable by the correct group:
for p in $(ils -v); do 
  proj=$(echo $p|grep -oP "(a|b)20.*"); 
  if [[ -n $proj ]]; then 
    echo "Fixing permissions for proj $proj ...";
    # No public access
    ichmod -r null public $proj; 
    # Make the project group the owner
    ichmod -r own $proj $proj;
    # Also let rodsadmin own
    ichmod -r own rodsadmin $proj;
    # Make subfolders inherit permissions of this folder
    ichmod inherit $proj;
  fi; 
done;
