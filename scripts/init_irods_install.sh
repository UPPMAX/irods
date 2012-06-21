#!/bin/bash
#
# Samuel Lampa, SNIC-UPPMAX
#


# Create folders etc
imkdir -p /ssUppnexZone/proj
for f in "/" "/ssUppnexZone" "/ssUppnexZone/proj"; do
	  ichmod read public $f;
  done;

  iadmin mkgroup swestoreArchGrp

  iadmin mkresc swestoreArchResc "MSS universal driver" compound kali.uppmax.uu.se /arch2
  iadmin atrgwestoreArchGrp swestoreArchResc

  iadmin mkresc swestoreArchCacheResc "unix file system" cache kali.uppmax.uu.se /data/rescs/swestoreArchCacheResc
  iadmin atrgwestoreArchGrp swestoreArchCacheResc

