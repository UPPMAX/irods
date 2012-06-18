testVerifyChecksum{
writeLine("stdout","Running verify checksum");
   *ContInxOld = 1; 
   *Count = 0; 
   #Get current Time
   msiGetIcatTime(*Time,"unix"); 
   *Tim = int(*Time); 
   msiDataObjChksum(*Tpath,"verifyChksum",*chkSum);
   writeLine("stdout","*chkSum");

   #Generate SQL query, This will get every file on the Cache resource. 256 lines at a time. 
   msiMakeGenQuery("DATA_NAME,COLL_NAME,DATA_CREATE_TIME,RESC_NAME","RESC_NAME like '*Cache'",*GenQInp);
   #Excute query.
   msiExecGenQuery(*GenQInp,*GenQOut);
   #Get Continuation index, index is non-zero when additional rows are available 
   msiGetContInxFromGenQueryOut(*GenQOut,*ContInxNew);	
   #Loop over every chunk of 256 lines of a file
   while (*ContInxOld > 0) { 
	foreach (*GenQOut) {
		msiGetValByKey(*GenQOut,"DATA_CREATE_TIME",*Ctime);
		*Ctim = int(*Ctime);
		msiGetValByKey(*GenQOut,"DATA_NAME",*File);
		msiGetValByKey(*GenQOut,"COLL_NAME",*Col);
		        *Path="*Col" ++ "/*File";
			writeLine("stdout","uppnexReplAndTrim: *Path is too old will be replicated and removed from cache");
			#Replicate a file, and replicate as admin, the admin user can replicate other user files
			msiDataObjRepl(*Path,"rescName=*Cache++++destRescName=*Archive++++irodsAdmin=1",*Status);
#			Verify checksum
			writeLine("stdout","*chkSum");
			#If status is good and replica exists, trim(and delete) the file from Cache, as admin user.
			if (*Status == 0) {
		        	*Count = *Count + 1; 
             			msiDataObjTrim(*Path,*Cache,"null","1","1",*Status1);
		}
	}

	*ContInxOld = *ContInxNew;
	if(*ContInxOld > 0) {
		#Get more lines and start over the foreach loop
		msiGetMoreRows(*GenQInp,*GenQOut,*ContInxNew);
	}	
   }
   #Serverlog for Rule is the reLog file 
   writeLine("stdout","uppnexReplAndTrim: *Count files were archived and removed from Cache");
   
}
#Dtim is the delete time, delete if older than 24h. 
INPUT *Tpath=$"/testZone1/home/rods/unixODBC-2.2.12.tar",*Dtim="86400",*Cache="sweStoreCache",*Archive="sweStore" 
OUTPUT ruleExecOut 
