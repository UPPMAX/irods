mytestrule{ 
   *ContInxOld = 1; 
   *Count = 0; 
   msiGetIcatTime(*Time,"unix"); 
   *Tim = int(*Time); 
   msiMakeGenQuery("DATA_NAME,COLL_NAME,DATA_CREATE_TIME","COLL_NAME like '*Coll'",*GenQInp); 
   msiExecGenQuery(*GenQInp,*GenQOut); 
   msiGetContInxFromGenQueryOut(*GenQOut,*ContInxNew); 
   while (*ContInxOld > 0) { 
     foreach (*GenQOut) { 
       msiGetValByKey(*GenQOut,"DATA_CREATE_TIME",*Ctime); 
       *Ctim = int(*Ctime); 
	msiGetValByKey(*GenQOut,"DATA_NAME",*File);

	writeLine("stdout","Test");
	}	
	}
	}
INPUT *Coll="/testZone1/home/rods",*Cache="sweStoreCache",*Archive="sweStore" 
OUTPUT ruleExecOut 
