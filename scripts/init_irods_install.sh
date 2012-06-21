#!/bin/bash
#
# Samuel Lampa, SNIC-UPPMAX
#


# Create folders etc
imkdir -p /ssUppnexZone/proj
for f in "/" "/ssUppnexZone" "/ssUppnexZone/proj"; do
  ichmod read public $f;
done;


