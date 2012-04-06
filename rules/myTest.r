mytestrule{ 
   *ContInxOld = 1; 
   *Count = 0; 
   #Get current Time
   msiGetIcatTime(*Time,"unix"); 
   *Tim = int(*Time); 
   #Generate SQL query, This will get every file on the Cache resource. 256 rows in a time . 
   msiMakeGenQuery("DATA_NAME,COLL_NAME,DATA_CREATE_TIME,RESC_NAME","RESC_NAME like '*Cache'",*GenQInp);
   #Excute query. 
   msiExecGenQuery(*GenQInp,*GenQOut);
   #Get Continuation index, index is non-zero when additional rows are available
   msiGetContInxFromGenQueryOut(*GenQOut,*ContInxNew);
foreach (*GenQOut) {
	msiGetValByKey(*GenQOut,"DATA_CREATE_TIME",*Ctime);
	*Ctim = int(*Ctime);
	msiGetValByKey(*GenQOut,"DATA_NAME",*File);
	msiGetValByKey(*GenQOut,"COLL_NAME",*Col);
	writeLine("stdout","*Col *File *Ctim");
#	writeLine("stdout",*Col);
#	writeLine("stdout",*Ctim);
	}
	}
INPUT *Coll="%home%",*Cache="sweStoreCache",*Archive="sweStore" 
OUTPUT ruleExecOut 
