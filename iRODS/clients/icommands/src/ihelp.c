/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*
 A simple program to provide intro help to the icommands
*/

#include "rodsClient.h"
#include "parseCommandLine.h"

char *icmds[]={
   "iadmin", "ibun", "icd", "ichksum", "ichmod", "icp", "ienv",
   "ierror", "iexecmd", "iexit", "iget", "ihelp", "iinit", "ils", "ilsresc",
   "imcoll", "imeta", "imiscsvrinfo", "imkdir", "imv", "ipasswd",
   "iphybun", "iphymv", "iput", "ipwd", "iqdel", "iqmod", "iqstat",
   "iquest", "ireg", "irepl", "irm", "irmtrash", "irsync", "irule",
   "isysmeta", "itrim", "iuserinfo",
   ""};

void usage ();

void
printMainHelp() 
{  
   char *msgs[]={
"The following is a list of the icommands and a brief description of",
"what each does:",
" ",
"iadmin   - perform irods administrator operations (irods admins only).",
"ibun     - upload/download structured (tar) files.",
"icd      - change the current working directory (collection).",
"ichksum  - checksum one or more data-objects or collections.",
"ichmod   - check the access permissions to collections or data-objects.",
"icp      - copy  a data-object (file) or collection (directory) to another.",
"ienv     - display current irods environment.",
"ierror   - convert an irods error code to text.",
"iexecmd  - remotely execute special commands.",
"iexit    - exit an irods session (un-iinit).",
"iget     - get a file from iRODS.",
"ihelp    - display a synopsis list of the i-commands.",
"iinit    - initialize a session, so you don't need to retype your password.",
"ils      - list collections (directories) and data-objects (files).",
"ilsresc  - list iRODS resources and resource-groups.",
"imcoll   - manage mounted collections and associated cache.",
"imeta    - add/remove/copy/list/query user-defined metadata.",
"imiscsvrinfo - retrieve basic server information.",
"imkdir   - make an irods directory (collection).",
"imv      - move/rename an irods data-object (file) or collection (directory).",
"ipasswd  - change your irods password.",
"iphybun  - physically bundle files (admin only).",
"iphymv   - physically move a data-object to another storage resource.",
"iput     - put (store) a file into iRODS.",
"ipwd     - print the current working directory (collection) name.",
"iqdel    - remove a delayed rule (owned by you) from the queue.",
"iqmod    - modify certain values in existing delayed rules (owned by you).",
"iqstat   - show the queue status of delayed rules.",
"iquest   - issue a question (query on system/user-defined metadata).",
"ireg     - register a file or directory/files/subdirectories into iRODS.",
"irepl    - replicate a file in iRODS to another storage resource.",
"irm      - remove one or more data-objects or collections.",
"irmtrash - remove data-objects from the trash bin.",
"irsync   - synchronize collections between a local/irods or irods/irods.",
"irule    - submit a rule to be executed by the iRODS server.",
"isysmeta - show or modify system metadata.",
"itrim    - trim down the number of replicas of data-objects.",
"iuserinfo- show information about your iRODS user account.",
" ",
"For basic operations, try: iinit, ils, iput, iget, imkdir, icd, ipwd,",
"and iexit.",
" ",
"For more information, run the icommand with '-h' or run ",
"'ihelp icommand'.",
""};
  int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("ihelp");
}

int
main(int argc, char **argv) {
	int status;
	rodsArguments_t myRodsArgs;
	char *optStr;
	
	
	optStr = "h";
	
	status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);
	
	if (status < 0) {
	   printf("Use -h for help\n");
	   exit (1);
	}

	if (myRodsArgs.help==True) {
	   usage();
	   exit(0);
	}
 
        if (argc==1) {
	   printMainHelp();
	}
	if (argc==2) {
	   char myExe[100];
	   int i;
	   int OK=0;
	   for (i=0;;i++) {
	      if (strlen(icmds[i])==0) break;
	      if (strcmp(argv[1], icmds[i])==0) {
		 OK=1;
		 break;
	      }
	   }
	   if (OK==0) {
	      printf("%s is not an i-command\n", argv[1]);
	   }
	   else {
	      strncpy(myExe, argv[1], 40);
	      strncat(myExe, " -h", 40);
	      status =system(myExe);
	      if (status) printf("error %d running %s\n",status, myExe);
	   }
	}

	exit(0);
}

void
usage () {
   char *msgs[]={
"Usage : ihelp [-h] [icommand]",
"Display i-commands synopsis or a particular i-command help text",
"Options are:",
" -h  this help",
" ",
"Run with no options to display a synopsis of the i-commands",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("ihelp");
}
