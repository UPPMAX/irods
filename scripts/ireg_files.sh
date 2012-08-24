#!/bin/bash

export PATH=/opt/irods/iRODS/clients/icommands/bin:$PATH;
for f in `arcls srm://srm.swegrid.se/snic/uppnex/arch/proj/a2010002/arch_mssn-20110510-155005`; 
do
  echo "Now dumping file $f ...";
  echo "Command: ireg -f -R swestoreArchResc -G swestoreArchGrp \"/proj/a2010002/arch_mssn-20110510-155005/$f\" \"/swestoreArchZone/proj/a2010002/arch_mssn-20110510-155005/$f\"";
  ireg -f -R swestoreArchResc -G swestoreArchGrp "/proj/a2010002/arch_mssn-20110510-155005/$f" "/swestoreArchZone/proj/a2010002/arch_mssn-20110510-155005/$f";
done;
