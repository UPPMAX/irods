xmlValidate {
	msiXmlDocSchemaValidate(*xmlObj, *xsdObj, *status);
	writePosInt("stdout",*status);
	writeLine("stdout","");
	writeBytesBuf("stdout",*status);
}
INPUT *xmlObj="/pho27/home/rods/meta/metadata.xml",*xsdObj="/pho27/home/rods/meta/metadata.xsd"
OUTPUT ruleExecOut