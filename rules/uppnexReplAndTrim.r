upppnexReplAndTrim{
#Get current time in unix
msiGetIcatTime(*Time,"unix");
#get human formatted date, year-month-day
msiGetFormattedSystemTime(*Cdate,"human","%d-%02d-%02d");
#Translate user input and full date, to unix time
msiHumanToSystemTime(*Cdate++"-"++*RuleStartTime,*Systime);
#Check if time has passed or not. 
if(int(*Time) >int(*Systime))
        {
	writeLine("stdout","Exekveringstid har passerat läggerpå 24h");
        *ExtimeUnix = str(int(*Systime) + 86400);
        }
	else
	{
	writeLine("stdout","Exekveringstid har inte passerat");
        *ExtimeUnix=str(*Systime);
        }
delay("<ET>"++*ExtimeUnix++"</ET><EF>24h</EF>") {
writeLine("serverLog","Running UPPNEX Rule uppnexReplAndTrim");
   *ContInxOld = 1; 
   *Count = 0; 
   #Get current Time
   msiGetIcatTime(*Time,"unix"); 
   *Tim = int(*Time); 
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
                *Dtim = int(*DeleteOlderThan);
		msiGetValByKey(*GenQOut,"DATA_NAME",*File);
		msiGetValByKey(*GenQOut,"COLL_NAME",*Col);
		if (*Tim - *Ctim > *Dtim) { 
		        *Path="*Col" ++ "/*File";
			writeLine("serverLog","uppnexReplAndTrim: *Path is too old will be replicated and removed from cache");
			#Replicate a file, and replicate as admin, the admin user can replicate other user files
			msiDataObjRepl(*Path,"rescName=*Cache++++destRescName=*Archive++++irodsAdmin=1",*Status);
#			writeLine("stdout","*Status");
			#If status is good and replica exists, trim(and delete) the file from Cache, as admin user.
			if (*Status == 0) {
		        	*Count = *Count + 1; 
             			msiDataObjTrim(*Path,*Cache,"null","1","1",*Status1);
			}
		}
	}

	*ContInxOld = *ContInxNew;
	if(*ContInxOld > 0) {
		#Get more lines and start over the foreach loop
		msiGetMoreRows(*GenQInp,*GenQOut,*ContInxNew);
	}	
   }
   #Serverlog for Rule is the reLog file 
   writeLine("serverLog","uppnexReplAndTrim: *Count files were archived and removed from Cache");
   }
}
#Dtim is the delete time, delete if older than 24h. 
INPUT *RuleStartTime=$"03:00:00",*DeleteOlderThan=$"86400",*Cache="swestoreArchCacheResc",*Archive="swestoreArchResc" 
OUTPUT ruleExecOut 
