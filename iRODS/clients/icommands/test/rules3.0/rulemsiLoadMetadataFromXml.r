myTestRule {
# Input parameters
#  targetObj     - iRODS target file that metadata will be attached to, null if Target is specified
#  xmlObj        - iRODS path to XML file that metadata is drawn from
#
#    xmlObj is assumed to be in AVU-format
#    This format is created by transforming the original XML file
#    using an appropriate style sheet as shown in rulemsiXsltApply.r
#
#    This microservice uses libxml2.
#

  # call the microservice
  msiLoadMetadataFromXml(*targetObj, *xmlObj);

  # write message to the log file
  writeLine("serverLog","Extracted metadata from *xmlObj and attached to *targetObj");

  # write message to stdout
  writeLine"stdout","Extracted metadata from *xmlObj and attached to *targetObj");
}
INPUT *xmlObj="tempZone/home/rods/XML/sample-processed.xml", *targetObj=""
OUTPUT ruleExecOut