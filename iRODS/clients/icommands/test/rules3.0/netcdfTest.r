netcdfTest () {
	if (msiNcOpen (*ncTestPath, "0", *ncid) == 0) {
	    writeLine("stdout", "msiNcOpen success, ncid = *ncid");
	} else {
	    writeLine("stdout", "msiNcOpen failed");
	    fail;
	} 
# inq longitude
	if (msiNcInqId ("longitude", 1, *ncid, *londimid) == 0) {
            writeLine("stdout", "msiNcInqId success, londimid = *londimid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        } 
        if (msiNcInqWithId (*londimid, 1, *ncid, *inqOut) == 0) {
            writeLine("stdout", "msiNcInqWithId londimid success");
	    if (msiNcGetArrayLen (*inqOut, *lonArrayLen) == 0) {
		writeLine ("stdout", "lonArrayLen = *lonArrayLen");
	    } else {
                writeLine("stdout", "msiNcGetArrayLen failed");
                fail;
	    }
        } else {
            writeLine("stdout", "msiNcInqWithId failed");
            fail;
        } 
# inq latitude
        if (msiNcInqId ("latitude", 1, *ncid, *latdimid) == 0) {
            writeLine("stdout", "msiNcInqId success, latdimid = *latdimid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        } 
        if (msiNcInqWithId (*latdimid, 1, *ncid, *inqOutl) == 0) {
            writeLine("stdout", "msiNcInqWithId latdimid success");
            if (msiNcGetArrayLen (*inqOutl, *latArrayLen) == 0) {
                writeLine ("stdout", "latArrayLen = *latArrayLen");
            } else {
                writeLine("stdout", "msiNcGetArrayLen failed");
                fail;
            }
        } else {
            writeLine("stdout", "msiNcInqWithId failed");
            fail;
        }

# variables
        if (msiNcInqId ("pressure", 0, *ncid, *pressvarid) == 0) {
            writeLine("stdout", "msiNcInqId success, pressvarid = *pressvarid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
        if (msiNcInqWithId (*pressvarid, 0, *ncid, *pressinqout) == 0) {
            writeLine("stdout", "msiNcInqWithId pressvarid success");
	    if (msiNcGetNumDim (*pressinqout, *ndim) == 0) {
		writeLine("stdout", "pressinqout ndim = *ndim");
		for(*I=0;*I<*ndim;*I=*I+1) {
		    msiNcGetElementInArray (*pressinqout, *I, *element);
		    writeLine("stdout", "dimid *I: *element");
		}
            } else {
                writeLine("stdout", "msiNcGetNumDim failed");
                fail;
            }
	    if (msiNcGetDataType (*pressinqout, *pressDataType) == 0) {
                writeLine("stdout", "msiNcGetDataType success pressDataType = *pressDataType");
            } else {
                writeLine("stdout", "msiNcGetDataType pressinqout failed");
                fail;
	    }
        }
        if (msiNcGetVarsByType (*pressDataType, *ncid, *pressvarid, *ndim, "0%0", "3%5", "1%1", *getVarsOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNcGetVarsByType pressvarid success");
            if (msiNcGetArrayLen (*getVarsOut, *pressArrayLen) == 0) {
                writeLine ("stdout", "pressArrayLen = *pressArrayLen");
               for(*I=0;*I<*pressArrayLen;*I=*I+1) {
                    msiNcGetElementInArray (*getVarsOut, *I, *element);
		    if (*pressDataType == 5) {
# float. writeLine cannot handle float yet.
			msiFloatToString (*element, *floatStr);
			writeLine("stdout", "pressure *I: *floatStr");
		    } else {
                        writeLine("stdout", "pressure *I: *element");
		    }
		}
            } else {
                writeLine("stdout", "msiNcGetArrayLen failed");
                fail;
            }
        } else {
            writeLine("stdout", "msiNcGetVarsByType pressvarid failed");
            fail;
        }
# msiNccfGetVara test
        if (msiNcInqId ("temperature", 0, *ncid, *tempvarid) == 0) {
            writeLine("stdout", "msiNcInqId success, tempvarid = *tempvarid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }

        if (msiNccfGetVara (*ncid, *tempvarid, "0", "0", "30.0", "41.0", "-120.0", "-96.0", 1000, *tempVaraOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNccfGetVara tempvarid success");
            if (msiNcGetArrayLen (*tempVaraOut, *tempArrayLen) == 0) {
                writeLine ("stdout", "tempArrayLen = *tempArrayLen");
		if (msiNcGetDataType (*tempVaraOut, *tempDataType) == 0) {
                    writeLine("stdout", "tempDataType = *tempDataType");
                } else {
                    writeLine("stdout", "msiNcGetDataType temp failed");
                    fail;
               }
               for(*I=0;*I<*tempArrayLen;*I=*I+1) {
                    msiNcGetElementInArray (*tempVaraOut, *I, *element);
                    if (*tempDataType == 5) {
# float. writeLine cannot handle float yet.
                        msiFloatToString (*element, *floatStr); 
                        writeLine("stdout", "pressure *I: *floatStr");
                    } else {
                        writeLine("stdout", "pressure *I: *element");
                    }
                }
            } else {
                writeLine("stdout", "msiNcGetArrayLen failed");
                fail;
            }
        } else {
            writeLine("stdout", "msiNccfGetVara pressvarid failed");
            fail;
        }
        if (msiNcClose (*ncid) == 0) {
            writeLine("stdout", "msiNcClose success, ncid = *ncid");
        } else {
            writeLine("stdout", "msiNcClose failed");
            fail;
        }
}
INPUT *ncTestPath="/wanZone/home/rods/netcdf/sfc_pres_temp.nc"
OUTPUT ruleExecOut,*tempVaraOut
