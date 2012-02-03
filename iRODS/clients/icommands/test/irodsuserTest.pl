#!/usr/bin/perl -w
#
# script to test the basic functionalities of icommands. This is similar to
# testiCommands.pl except it should be run by a non-privilege user (irodsuser)
# instead of privilege user. The resources for the test is assumed to have
# been created already and define in $testresource, $compresource and
# $resgroup.
#  

# icommands location has to be put in the PATH env variable or their PATH will be asked
# at the beginning of the execution of this script.
#
# usage:   ./irodsuserTest.pl [debug] [noprompt] [help]
#    help - Print usage messages.
#    debug - print debug messages.
#    noprompt - assumes iinit was done before running this script 
#    and will not ask for path nor password input.
# 
#
# Copyright (c), CCIN2P3
# For more information please refer to files in the COPYRIGHT directory.

use strict;
use Cwd;
use Sys::Hostname;
use File::stat;
use File::Copy;

#-- Initialization

# $doIbunZipTest - whether to do ibun test for gzip and bzip2 dataType.
# "yes" or "no". Default is no since the gzipTar and bzip2Tar dataType
# are not defined yet.
my $doIbunZipTest = "yes";

my $testresource='hpResc1';
my $compresource='compResc';
my $resgroup='compGrp';
my $testresource2='demoResc';
my $debug;
my $entry;
my @failureList;
my $i;
my $input;
my $irodsdefresource;
my $irodshost;
my $irodshome;
my $irodszone;
my $line;
my @list;
my $misc;
my $nfailure;
my $nsuccess;
my $rc;
my @returnref;
my @successlist;
my @summarylist;
my @tmp_tab;
my $username;
my @words;

# If noprompt_flag is set to 1, it assume iinit was done before running this
# script and will not ask for path and password input. A "noprompt" input 
# will set it.
my $noprompt_flag;
my $arg;

$debug = 0;
$noprompt_flag = 0;
foreach $arg (@ARGV)
{
    if ( $arg =~ "debug" ) {
        $debug = 1;
    } elsif ( $arg =~ "noprompt" ) {
	$noprompt_flag = 1;
    } elsif ( $arg =~ "help" ) {
        &printUsage ();
	exit( 0 );
    }  else {
	print ("unknown input - $arg \n");
        &printUsage ();
        exit( 1 );
    }
}

my $dir_w        = cwd();
my $myldir = $dir_w . '/ldir';
my $mylsize;
my $mysdir = '/tmp/irodssdir';
my $myssize;
my $host         = hostname();
if ( $host =~ '.' ) {
	@words = split( /\./, $host );
	$host  = $words[0];
}
my $irodsfile;
my $irodsEnvFile = $ENV{'irodsEnvFile'};
if ($irodsEnvFile) {
    $irodsfile = $irodsEnvFile;
} else {
    $irodsfile    = "$ENV{HOME}/.irods/.irodsEnv";
}
my $ntests       = 0;
my $progname     = $0;

my $outputfile   = "testSurvey_" . $host . ".log";
my $ruletestfile = "testRule_"   . $host . ".irb";
my $sfile2 = $dir_w . '/sfile2';
my $sfile2size;
system ( "cat $progname $progname > $sfile2" );
$sfile2size =  stat ($sfile2)->size;


#-- Find current working directory and make consistency path

$outputfile   = $dir_w . '/' . $outputfile;
$ruletestfile = $dir_w . '/' . $ruletestfile;

if ( $progname !~ '/' ) {
	$progname = $dir_w . '/' . $progname;
} else {
	if ( substr( $progname, 0, 2 ) eq './' ) {
		@words    = split( /\//, $progname );
		$i        = $#words;
		$progname = $dir_w . '/' . $words[$i];
	}
	if ( substr( $progname, 0, 1 ) ne '/' ) {
		$progname = $dir_w . '/' . $progname;
	}
}

#-- Remove ruletestfile

if ( -e $ruletestfile ) { unlink( $ruletestfile ); }

#-- Take debug level

# $debug = shift;
# if ( ! $debug ) {
# 	$debug = 0;
# } else {
# 	$debug = 1;
# }

#-- Print debug

if ( $debug ) {
	print( "\n" );
	print( "MAIN: irodsfile        = $irodsfile\n" );
	print( "MAIN: cwd              = $dir_w\n" );
	print( "MAIN: outputfile       = $outputfile\n" );
	print( "MAIN: ruletestfile     = $ruletestfile\n" );
	print( "MAIN: progname         = $progname\n" );
	print( "\n" );
}

#-- Dump content of $irodsfile to @list

my $tempFile   = "/tmp/iCommand.log";
@list = dumpFileContent( $irodsfile );

#-- Loop on content of @list
# The below parsing works in the current environment 
# but there are two shortcomings:
#   1) single quotes are removed, but if there were to be embedded ones,
#      they would be removed too.
#   2) if the name and value are separated by =, the line will not split right.
foreach $line ( @list ) {
 	chomp( $line );
	if ( ! $line ) { next; }
 	if ( $line =~ /irodsUserName/ ) {
		( $misc, $username ) = split( / /, $line );
		$username =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsHome/ ) {
		( $misc, $irodshome ) = split( / /, $line );
		$irodshome =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsZone/ ) {
		( $misc, $irodszone ) = split( / /, $line );
		$irodszone =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsHost/ ) {
		( $misc, $irodshost ) = split( / /, $line );
		$irodshost =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsDefResource/ ) {
		( $misc, $irodsdefresource ) = split( / /, $line );
		$irodsdefresource =~ s/\'//g; #remove all ' chars, if any
	}
}

#-- Print debug

if ( $debug ) {
	print( "MAIN: username         = $username\n" );
	print( "MAIN: irodshome        = $irodshome\n" );
	print( "MAIN: irodszone        = $irodszone\n" );
	print( "MAIN: irodshost        = $irodshost\n" );
	print( "MAIN: irodsdefresource = $irodsdefresource\n" );
}

#-- Environment setup and print to stdout

print( "\nThe results of the test will be written in the file: $outputfile\n\n" );
print( "Warning: you need to be a rodsadmin in order to pass successfully all the tests," );
print( " as some admin commands are being tested." );
if ( ! $noprompt_flag ) {
    print( " If icommands location has not been set into the PATH env variable," );
    print( " please give it now, else press return to proceed.\n" );
    print( "icommands location path: " );
    chomp( $input = <STDIN> );
    if ( $input ) { $ENV{'PATH'} .= ":$input"; }
    print "Please, enter the password of the iRODS user used for the test: ";
    chomp( $input = <STDIN> );
    if ( ! $input ) {
	print( "\nYou should give valid pwd.\n\n");
	exit;
    } else {
	print( "\n" );
    }


    runCmd( "iinit $input" );
}

#-- Test the icommands and eventually compare the result to what is expected.


# Simple test to increase coverage; run each of the i-commands with -h
runCmd( "ihelp -a" );
# and also run the ihelp usage function
runCmd( "ihelp -h" );

#-- basic clients commands.


# single file test

$myssize = stat ($progname)->size;
runCmd( "imiscsvrinfo" );
runCmd( "iuserinfo", "", "name:", $username );
runCmd( "ienv" );
runCmd( "icd $irodshome" );
runCmd( "ipwd",  "", "LIST", "home" );
runCmd( "ihelp ils" );
runCmd( "ierror -14000", "", "LIST", "SYS_API_INPUT_ERR" );
runCmd( "iexecmd hello", "", "LIST", "Hello world" );
runCmd( "ips -v", "", "LIST", "ips" );
runCmd( "iqstat" );
runCmd( "imkdir $irodshome/icmdtest", "", "", "", "irm -r $irodshome/icmdtest" );
# make a directory of large files
runCmd( "iput -KR $testresource2 --wlock $progname $irodshome/icmdtest/foo1", "", "", "", "irm $irodshome/icmdtest/foo1" );
runCmd( "ichksum -f $irodshome/icmdtest/foo1" );
runCmd( "iput -kf $progname $irodshome/icmdtest/foo1" );
runCmd( "ils $irodshome/icmdtest/foo1" , "", "LIST", "foo1" );
runCmd( "ils -l $irodshome/icmdtest/foo1", "", "LIST", "foo1,$myssize" );
runCmd( "ils -A $irodshome/icmdtest/foo1", "", "LIST", "$username#$irodszone:own" );
runCmd( "ichmod read rods $irodshome/icmdtest/foo1" );
runCmd( "ils -A $irodshome/icmdtest/foo1", "", "LIST", "rods#$irodszone:read" );
runCmd( "irepl -B -R $testresource --rlock $irodshome/icmdtest/foo1" );
runCmd( "ils -l $irodshome/icmdtest/foo1", "", "LIST", "1 $testresource" );
# overwrite a copy 
runCmd( "itrim -S  $testresource2 -N1 $irodshome/icmdtest/foo1" );
runCmd( "ils -L $irodshome/icmdtest/foo1", "negtest", "LIST", "$testresource2" );
runCmd( "iphymv -R  $testresource2 $irodshome/icmdtest/foo1" );
runCmd( "ils -l $irodshome/icmdtest/foo1", "", "LIST", "$testresource2" );
runCmd( "imeta add -d $irodshome/icmdtest/foo1 testmeta1 180 cm", "", "", "", "imeta rm -d $irodshome/icmdtest/foo1 testmeta1 180 cm" );
runCmd( "imeta ls -d $irodshome/icmdtest/foo1", "", "LIST", "testmeta1,180,cm" );
runCmd( "icp -K -R $testresource $irodshome/icmdtest/foo1 $irodshome/icmdtest/foo2", "", "", "", "irm $irodshome/icmdtest/foo2" );
runCmd( "ils $irodshome/icmdtest/foo2", "", "LIST", "foo2" );
runCmd( "imv $irodshome/icmdtest/foo2 $irodshome/icmdtest/foo4" );
runCmd( "ils -l $irodshome/icmdtest/foo4", "", "LIST", "foo4" );
runCmd( "imv $irodshome/icmdtest/foo4 $irodshome/icmdtest/foo2" );
runCmd( "ils -l $irodshome/icmdtest/foo2", "", "LIST", "foo2" );
runCmd( "ichksum $irodshome/icmdtest/foo2", "", "LIST", "foo2" );
runCmd( "imeta add -d $irodshome/icmdtest/foo2 testmeta1 180 cm", "", "", "", "imeta rm -d $irodshome/icmdtest/foo2 testmeta1 180 cm" );
runCmd( "imeta add -d $irodshome/icmdtest/foo1 testmeta2 hello", "", "", "", "imeta rm -d $irodshome/icmdtest/foo1 testmeta2 hello"  );
runCmd( "imeta ls -d $irodshome/icmdtest/foo1", "", "LIST", "testmeta1,hello" );
runCmd( "imeta qu -d testmeta1 = 180", "", "LIST", "foo1" );
runCmd( "imeta qu -d testmeta2 = hello", "", "dataObj:", "foo1" );
runCmd( "iget -f -K --rlock $irodshome/icmdtest/foo2 $dir_w" );
runCmd( "ls -l $dir_w/foo2", "", "LIST", "foo2,$myssize");
unlink ( "$dir_w/foo2" );
# we have foo1 in $testresource2 and foo2 in $testresource
# make a directory containing 20 small files
mksdir ();
runCmd( "irepl -B -R $testresource $irodshome/icmdtest/foo1" );
my $phypath = $dir_w . '/' . 'foo1.' .  int(rand(10000000));
runCmd( "iput -kfR $testresource2 -p $phypath $sfile2 $irodshome/icmdtest/foo1" );
# show have 2 different copies
runCmd( "ils -l $irodshome/icmdtest/foo1", "", "LIST", "foo1,$myssize,$sfile2size" );
# update all old copies
runCmd( "irepl -U $irodshome/icmdtest/foo1" );
# make sure the old size is not there
runCmd( "ils -l $irodshome/icmdtest/foo1", "negtest", "LIST", "$myssize" );
runCmd( "itrim -S $testresource2 $irodshome/icmdtest/foo1" );
# bulk test
runCmd( "iput -bIvPKr $mysdir $irodshome/icmdtest" );
# iput with a lot of options
my $rsfile = $dir_w . "/rsfile";
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iput -PkITr -X $rsfile --retries 10  $mysdir $irodshome/icmdtestw" );
runCmd( "imv $irodshome/icmdtestw $irodshome/icmdtestw1" );
runCmd( "ils -lr $irodshome/icmdtestw1", "", "LIST", "sfile10" );
runCmd( "ils -Ar $irodshome/icmdtestw1", "", "LIST", "sfile10" );
system ( "irm -rvf $irodshome/icmdtestw1" );
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vIKPfr -X rsfile --retries 10 $irodshome/icmdtest $dir_w/testx", "", "", "", "rm -r $dir_w/testx" );
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "tar -chf $dir_w/testx.tar -C $dir_w/testx .", "", "", "", "rm $dir_w/testx.tar" );
# my $phypath = $dir_w . '/' . 'testx.tar.' .  int(rand(10000000));
runCmd( "iput $dir_w/testx.tar $irodshome/icmdtestx.tar", "", "", "", "irm -f $irodshome/icmdtestx.tar" );
runCmd( "ibun -x $irodshome/icmdtestx.tar $irodshome/icmdtestx", "", "", "", "irm -rf $irodshome/icmdtestx" );
runCmd( "ils -lr $irodshome/icmdtestx", "", "LIST", "foo2,sfile10" );
runCmd( "ibun -cDtar $irodshome/icmdtestx1.tar $irodshome/icmdtestx", "", "", "", "irm -f $irodshome/icmdtestx1.tar" );
runCmd( "ils -l $irodshome/icmdtestx1.tar", "", "LIST", "testx1.tar" );
system ( "mkdir $dir_w/testx1" );
runCmd( "iget  $irodshome/icmdtestx1.tar $dir_w/testx1.tar", "",  "", "", "rm $dir_w/testx1.tar" );
runCmd( "tar -xvf $dir_w/testx1.tar -C $dir_w/testx1", "", "", "", "rm -r $dir_w/testx1" );
runCmd( "diff -r $dir_w/testx $dir_w/testx1/icmdtestx", "", "NOANSWER" );
if ( $doIbunZipTest =~ "yes" ) {
# test ibun with gzip
    runCmd( "ibun -cDgzip $irodshome/icmdtestx1.tar.gz $irodshome/icmdtestx");
    runCmd( "ibun -x $irodshome/icmdtestx1.tar.gz $irodshome/icmdtestgz");
    runCmd( "iget -vr $irodshome/icmdtestgz $dir_w");
    runCmd( "diff -r $dir_w/testx $dir_w/icmdtestgz/icmdtestx", "", "NOANSWER" );
    system ("rm -r $dir_w/icmdtestgz");
    runCmd( "ibun --add $irodshome/icmdtestx1.tar.gz $irodshome/icmdtestgz");
    system ("irm -rf $irodshome/icmdtestx1.tar.gz $irodshome/icmdtestgz");
# test ibun with bzip2
    runCmd( "ibun -cDbzip2 $irodshome/icmdtestx1.tar.bz2 $irodshome/icmdtestx");
    runCmd( "ibun -xb $irodshome/icmdtestx1.tar.bz2 $irodshome/icmdtestbz2");
    runCmd( "iget -vr $irodshome/icmdtestbz2 $dir_w");
    runCmd( "diff -r $dir_w/testx $dir_w/icmdtestbz2/icmdtestx", "", "NOANSWER" );
    system ("rm -r $dir_w/icmdtestbz2");
    system ("irm -rf $irodshome/icmdtestx1.tar.bz2");
    runCmd( "iphybun -R$resgroup -Dbzip2 $irodshome/icmdtestbz2", "failtest" );
}
system ( "irm -rf $irodshome/icmdtestbz2");
system ( "mv $sfile2 /tmp/sfile2" );
system ( "cp /tmp/sfile2 $dir_w/sfile2" );
runCmd( "ireg -KR $testresource $dir_w/sfile2  $irodshome/foo5" );
system ( "cp /tmp/sfile2 $dir_w/sfile2r" );
runCmd( "ireg -KR $compresource --repl $dir_w/sfile2r  $irodshome/foo5" );
runCmd( "iget -fK $irodshome/foo5 $dir_w/foo5", "", "", "", "rm $dir_w/foo5" );
runCmd( "diff /tmp/sfile2  $dir_w/foo5", "", "NOANSWER" );
system ( "cp -r $mysdir $dir_w/mysdir" );
runCmd( "ireg -KCR $testresource $dir_w/mysdir $irodshome/icmdtesta" );
runCmd( "iget -fvrK $irodshome/icmdtesta $dir_w/testa" );
runCmd( "diff -r $mysdir $dir_w/testa", "", "NOANSWER" );
system ( "rm -r $dir_w/testa" );
system ( "irm -f $irodshome/foo5" );

# mcoll test
runCmd( "imcoll -m link $irodshome/icmdtesta $irodshome/icmdtestb" );
runCmd( "ils -lr $irodshome/icmdtestb" );
runCmd( "iget -fvrK $irodshome/icmdtestb $dir_w/testb" );
runCmd( "diff -r $mysdir $dir_w/testb", "", "NOANSWER" );
runCmd( "imcoll -U $irodshome/icmdtestb" );
runCmd( "irm -rf $irodshome/icmdtestb" );
system ( "rm -r $dir_w/testb" );
runCmd( "imkdir $irodshome/icmdtestm" );
runCmd( "imcoll -m filesystem -R $testresource $dir_w/mysdir $irodshome/icmdtestm" );
runCmd( "imkdir $irodshome/icmdtestm/testmm" );
runCmd( "iput $progname $irodshome/icmdtestm/testmm/foo1" );
runCmd( "iput $progname $irodshome/icmdtestm/testmm/foo11" );
runCmd( "imv $irodshome/icmdtestm/testmm/foo1 $irodshome/icmdtestm/testmm/foo2" );
runCmd( "imv $irodshome/icmdtestm/testmm $irodshome/icmdtestm/testmm1" );
# mv to normal collection
runCmd( "imv $irodshome/icmdtestm/testmm1/foo2 $irodshome/icmdtest/foo100" );
runCmd( "ils -l $irodshome/icmdtest/foo100", "", "LIST", "foo100" );
runCmd( "imv $irodshome/icmdtestm/testmm1 $irodshome/icmdtest/testmm1" );
runCmd( "ils -lr $irodshome/icmdtest/testmm1", "", "LIST", "foo11" );
system ( "irm -rf $irodshome/icmdtest/testmm1 $irodshome/icmdtest/foo100" );
runCmd( "iget -fvrK $irodshome/icmdtesta $dir_w/testm" );
runCmd( "diff -r $mysdir $dir_w/testm", "", "NOANSWER" );
runCmd( "imcoll -U $irodshome/icmdtestm" );
runCmd( "irm -rf $irodshome/icmdtestm" );
system ( "rm -r $dir_w/testm" );
runCmd( "imkdir $irodshome/icmdtestt" );
runCmd( "imcoll -m tar $irodshome/icmdtestx.tar $irodshome/icmdtestt" );
runCmd( "ils -lr $irodshome/icmdtestt", "", "LIST", "foo2,foo1" );
runCmd( "iget -vr $irodshome/icmdtestt  $dir_w/testt" );
runCmd( "diff -r  $dir_w/testx $dir_w/testt", "", "NOANSWER" );
runCmd( "imkdir $irodshome/icmdtestt/mydirtt" );
runCmd( "iput $progname $irodshome/icmdtestt/mydirtt/foo1mt" );
runCmd( "imv $irodshome/icmdtestt/mydirtt/foo1mt $irodshome/icmdtestt/mydirtt/foo1mtx" );
mkldir ();
# test adding a large file to a mounted collection
runCmd( "iput $myldir/lfile1 $irodshome/icmdtestt/mydirtt" );
runCmd( "iget $irodshome/icmdtestt/mydirtt/lfile1 $dir_w/testt" );
runCmd( "irm -r $irodshome/icmdtestt/mydirtt" );
runCmd( "imcoll -s $irodshome/icmdtestt" );
runCmd( "imcoll -p $irodshome/icmdtestt" );
runCmd( "imcoll -U $irodshome/icmdtestt" );
runCmd( "irm -rf $irodshome/icmdtestt" );
system ( "rm -r $dir_w/testt" );
# iphybun test
runCmd( "iput -rR $testresource $mysdir $irodshome/icmdtestp" );
runCmd( "iphybun -KR$resgroup $irodshome/icmdtestp", "failtest" );
system ( "irm -rvf $irodshome/icmdtesta" );
system ( "irm -rvf $irodshome/icmdtestp" );
system ( "rm -r $dir_w/mysdir" );
system ( "rm -r $mysdir" );


# resource group test
runCmd( "iput -KR $resgroup $progname $irodshome/icmdtest/foo6", "", "", "", "irm $irodshome/icmdtest/foo6" );
runCmd( "ils -l $irodshome/icmdtest/foo6", "", "LIST", "foo6,$testresource" );
runCmd( "irepl -a $irodshome/icmdtest/foo6" );
runCmd( "ils -l $irodshome/icmdtest/foo6", "", "LIST", "$compresource,$testresource" );
runCmd( "itrim -S $testresource -N1 $irodshome/icmdtest/foo6" );
runCmd( "ils -l $irodshome/icmdtest/foo6", "negtest", "LIST", "$testresource" );
runCmd( "iget -f $irodshome/icmdtest/foo6 $dir_w/foo6" );
runCmd( "ils -l $irodshome/icmdtest/foo6", "", "LIST", "$compresource,$testresource" );
runCmd( "diff  $progname $dir_w/foo6", "", "NOANSWER" );
runCmd( "itrim -S $testresource -N1 $irodshome/icmdtest/foo6" );
runCmd( "iput -fR $irodsdefresource $progname $irodshome/icmdtest/foo6" );
runCmd( "irepl -UR $compresource $irodshome/icmdtest/foo6" );
runCmd( "ils -l $irodshome/icmdtest/foo6", "", "LIST", "$compresource,$testresource" );
runCmd( "iget -f $irodshome/icmdtest/foo6 $dir_w/foo6" );
runCmd( "diff  $progname $dir_w/foo6", "", "NOANSWER" );
system ( "rm $dir_w/foo6" );

# test --purgec option
runCmd( "iput -R $resgroup --purgec $progname $irodshome/icmdtest/foo7", "", "", "", "irm $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "", "LIST", "$compresource" );
runCmd( "irepl -a $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "", "LIST", "$compresource,$testresource" );
runCmd( "iput -fR $resgroup --purgec $progname $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "irepl -a $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "", "LIST", "$compresource,$testresource" );
runCmd( "irepl -R $compresource --purgec $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "irepl -a $irodshome/icmdtest/foo7" );
runCmd( "itrim -S $compresource -N1 $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "", "LIST", "$testresource" );
runCmd( "irepl -R $compresource --purgec $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "iget -f $irodshome/icmdtest/foo7 --purgec $dir_w/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "irepl -a $irodshome/icmdtest/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "", "LIST", "$compresource,$testresource" );
runCmd( "iget -f $irodshome/icmdtest/foo7 --purgec $dir_w/foo7" );
runCmd( "ils -l $irodshome/icmdtest/foo7", "negtest", "LIST", "$testresource" );
runCmd( "diff  $progname $dir_w/foo7", "", "NOANSWER" );
system ( "rm $dir_w/foo7" );


#-- Test a simple rule from the rule test file

$rc = makeRuleFile();
if ( $rc ) {
	print( "Problem with makeRuleFile. Rc = $rc\n" );
} else {
	runCmd( "irule -F $ruletestfile", "", "", "", "irm $irodshome/icmdtest/foo3" );
}

runCmd( "irsync $ruletestfile i:$irodshome/icmdtest/foo100" );
runCmd( "irsync i:$irodshome/icmdtest/foo100 $dir_w/foo100" );
runCmd( "irsync i:$irodshome/icmdtest/foo100 i:$irodshome/icmdtest/foo200" );
system ("irm -f $irodshome/icmdtest/foo100 $irodshome/icmdtest/foo200");
runCmd( "iput -R $testresource $ruletestfile $irodshome/icmdtest/foo100");
runCmd( "irsync $ruletestfile i:$irodshome/icmdtest/foo100" );
runCmd( "iput -R $testresource $ruletestfile $irodshome/icmdtest/foo200");
runCmd( "irsync i:$irodshome/icmdtest/foo100 i:$irodshome/icmdtest/foo200" );
system ("rm  $dir_w/foo100" );

# do test using xml protocol
$ENV{'irodsProt'} = 1;
runCmd( "imiscsvrinfo" );
runCmd( "iuserinfo", "", "name:", $username );
runCmd( "ienv" );
runCmd( "icd $irodshome" );
runCmd( "ipwd",  "", "LIST", "home" );
runCmd( "ihelp ils" );
runCmd( "ierror -14000", "", "LIST", "SYS_API_INPUT_ERR" );
runCmd( "iexecmd hello", "", "LIST", "Hello world" );
runCmd( "ips -v", "", "LIST", "ips" );
runCmd( "iqstat" );
runCmd( "imkdir $irodshome/icmdtest1", "", "", "", "irm -r $irodshome/icmdtest1" );
# make a directory of large files
runCmd( "iput -kf $progname $irodshome/icmdtest1/foo1" );
runCmd( "ils -l $irodshome/icmdtest1/foo1", "", "LIST", "foo1, $myssize" );
runCmd( "ichmod read rods $irodshome/icmdtest1/foo1" );
runCmd( "ils -A $irodshome/icmdtest1/foo1", "", "LIST", "rods#$irodszone:read" );
runCmd( "irepl -B -R $testresource $irodshome/icmdtest1/foo1" );
# overwrite a copy
runCmd( "itrim -S  $testresource2 -N1 $irodshome/icmdtest1/foo1" );
runCmd( "iphymv -R  $testresource2 $irodshome/icmdtest1/foo1" );
runCmd( "imeta add -d $irodshome/icmdtest1/foo1 testmeta1 180 cm", "", "", "", "imeta rm -d $irodshome/icmdtest1/foo1 testmeta1 180 cm" );
runCmd( "imeta ls -d $irodshome/icmdtest1/foo1", "", "LIST", "testmeta1,180,cm" );
runCmd( "icp -K -R $testresource $irodshome/icmdtest1/foo1 $irodshome/icmdtest1/foo2", "", "", "", "irm $irodshome/icmdtest1/foo2" );
runCmd( "imv $irodshome/icmdtest1/foo2 $irodshome/icmdtest1/foo4" );
runCmd( "imv $irodshome/icmdtest1/foo4 $irodshome/icmdtest1/foo2" );
runCmd( "ichksum -K $irodshome/icmdtest1/foo2", "", "LIST", "foo2" );
runCmd( "iget -f -K $irodshome/icmdtest1/foo2 $dir_w" );
unlink ( "$dir_w/foo2" );
system ( "irm $irodshome/icmdtest/foo3" );
runCmd( "irule -F $ruletestfile" );
runCmd( "irsync $ruletestfile i:$irodshome/icmdtest1/foo1" );
runCmd( "irsync i:$irodshome/icmdtest1/foo1 /tmp/foo1" );
runCmd( "irsync i:$irodshome/icmdtest1/foo1 i:$irodshome/icmdtest1/foo2" );
system ( "rm /tmp/foo1" );
if ( -e $ruletestfile ) { unlink( $ruletestfile ); }
$ENV{'irodsProt'} = 0;

# do the large files tests
# mkldir ();
my $lrsfile = $dir_w . "/lrsfile";
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iput -vbPKr --retries 10 --wlock -X $rsfile --lfrestart $lrsfile -N 2 $myldir $irodshome/icmdtest/testy" );
runCmd( "ichksum -rK $irodshome/icmdtest/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "irepl -BvrPT -R $testresource --rlock $irodshome/icmdtest/testy" );
runCmd( "itrim -vrS $testresource2 --dryrun --age 1 -N1 $irodshome/icmdtest/testy" );
runCmd( "itrim -vrS $testresource2 -N1 $irodshome/icmdtest/testy" );
runCmd( "icp -vKPTr -N2 $irodshome/icmdtest/testy $irodshome/icmdtest/testz" );
runCmd( "irsync -r i:$irodshome/icmdtest/testy i:$irodshome/icmdtest/testz" );
system ( "irm -vrf $irodshome/icmdtest/testy" );
runCmd( "iphymv -vrS $testresource2 -R $testresource  $irodshome/icmdtest/testz" );

if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vPKr --retries 10 -X $rsfile --lfrestart $lrsfile --rlock -N 2 $irodshome/icmdtest/testz $dir_w/testz" );
runCmd( "irsync -r $dir_w/testz i:$irodshome/icmdtest/testz" );
runCmd( "irsync -r i:$irodshome/icmdtest/testz $dir_w/testz" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "diff -r $dir_w/testz $myldir", "", "NOANSWER" );
# test -N0 transfer
runCmd( "iput -N0 -R $testresource $myldir/lfile1 $irodshome/icmdtest/testz/lfoo100" );
runCmd( "iget -N0 $irodshome/icmdtest/testz/lfoo100 $dir_w/lfoo100" );
runCmd( "diff $myldir/lfile1 $dir_w/lfoo100", "", "NOANSWER" );

system ( "rm -r $dir_w/testz" );
system ( "rm $dir_w/lfoo100" );
system ( "irm -vrf $irodshome/icmdtest/testz" );

# do the large files tests using RBUDP

runCmd( "iput -vQPKr --retries 10 -X $rsfile --lfrestart $lrsfile $myldir $irodshome/icmdtest/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "irepl -BQvrPT -R $testresource $irodshome/icmdtest/testy" );
runCmd( "itrim -vrS $testresource2 -N1 $irodshome/icmdtest/testy" );
runCmd( "icp -vQKPTr $irodshome/icmdtest/testy $irodshome/icmdtest/testz" );
system ( "irm -vrf $irodshome/icmdtest/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vQPKr --retries 10 -X $rsfile --lfrestart $lrsfile $irodshome/icmdtest/testz $dir_w/testz" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "diff -r $dir_w/testz $myldir", "", "NOANSWER" );
system ( "rm -r $dir_w/testz" );
system ( "irm -vrf $irodshome/icmdtest/testz" );
system ( "rm -r $myldir" );

#-- Execute rollback commands

if ( $debug ) { print( "\nMAIN ########### Roll back ################\n" ); }

for ( $i = $#returnref; $i >= 0; $i-- ) {
	undef( @tmp_tab );
	$line     = $returnref[$i];
	@tmp_tab = @{$line};	
	runCmd( $tmp_tab[0], $tmp_tab[1], $tmp_tab[2], $tmp_tab[3] );
}

#-- Execute last commands before leaving

if ( $debug ) { print( "\nMAIN ########### Last ################\n" ); }

runCmd( "irmtrash" );
if ( ! $noprompt_flag ) {
    runCmd( "iexit full" );
}
`/bin/rm -rf /tmp/foo`;# remove the vault for the $testresource; needed in case
                       # another unix login runs this test on this host
`/bin/rm -rf /tmp/comp`;
#-- print the result of the test into testSurvey.log

$nsuccess = @successlist;
$nfailure = @failureList;

open( FILE, "> $outputfile" ) or die "unable to open $outputfile for writing\n";
print( FILE "===========================================\n" );
print( FILE "number of successfull tested commands = $nsuccess\n" );
print( FILE "number of failed tested commands      = $nfailure\n" );
print( FILE "===========================================\n\n" );
print( FILE "Summary of the consecutive commands which have been tested:\n\n" );

$i = 1;
foreach $line ( @summarylist ) {
  print( FILE "$i - $line\n" );
  $i++;
}
print( FILE "\n\nList of successfull tests:\n\n" );
foreach $line ( @successlist ) { print( FILE "$line" ); }
print( FILE "\n\nList of failed tests:\n\n" );
foreach $line ( @failureList ) { print( FILE "$line" ); }
print( FILE "\n" );
close( FILE );
exit;

##########################################################################################################################
# runCmd needs at least 8 arguments: 
#   1- command name + arguments
#   2- specify if it is a negative test by providing the "negtest" value, ie it is successfull if the test fails (optional).
#   If this value is "failtest", this command is expected to fail.
#   3- output line of interest (optional), if equal to "LIST" then match test the entire list of answers provided in 4-. if equal to "NOANSWER" then expect no answer.
#   4- expected list of results separeted by ',' (optional: must be given if second argument provided else it will fail).
#	5- command name to go back to first situation
#	6- same as 2 but for 5
#	7- same as 3 but for 5
#	8- same as 4 but for 5

sub runCmd {
 	my ( $cmd, $testtype, $stringToCheck, $expResult, $r_cmd, $r_testtype, $r_stringToCheck, $r_expResult ) = @_;

	my $rc = 0;
	my @returnList;
	my @words;
	my $line;

 	my $answer     = "";
 	my @list       = "";
 	my $numinlist  = 0;
 	my $numsuccess = 0;
 	my $negtest    = 0;
 	my $failtest    = 0;
 	my $result     = 1; 		# used only in the case where the answer of the command has to be compared to an expected answer.

#-- Check inputs

	if ( ! $cmd ) {
		print( "No command given to runCmd; Exit\n" );
		exit;
	}
	if ( ! $testtype ) {
		$testtype = 0;
		$negtest  = 0;
	} else {
		if ( $testtype eq "negtest" ) {
			$negtest = 1;
                } elsif ( $testtype eq "failtest" ) {
                        $failtest = 1;
		} else {
			$negtest = 0;
		}	
	}
	if ( ! $stringToCheck   ) { $stringToCheck = ""; }
	if ( ! $expResult       ) { $expResult = ""; }
	if ( ! $r_cmd           ) { $r_cmd = ""; }
	if ( ! $r_testtype      ) { $r_testtype = ""; }
	if ( ! $r_stringToCheck ) { $r_stringToCheck = ""; }
	if ( ! $r_expResult     ) { $r_expResult = ""; }

#-- Update counter

	$ntests++;

#-- Print debug
	
	if ( $debug ) { print( "\n" ); }
	printf( "%3d - cmd executed: $cmd\n", $ntests );
	if ( $debug ) { print( "DEBUG: input to runCMd: $cmd, $testtype, $stringToCheck, $expResult.\n" ); }

#-- Push return command in list

	undef( @returnList );
	if ( $r_cmd ){
		$returnList[0] = $r_cmd;
		$returnList[1] = $r_testtype;
		$returnList[2] = $r_stringToCheck;
		$returnList[3] = $r_expResult;
		push( @returnref, \@returnList );
		
		if ( $debug ) { print( "DEBUG: roll back:       $returnList[0], $returnList[1], $returnList[2], $returnList[3].\n" ); }
	} else {
		if ( $debug ) { print( "DEBUG: roll back:       no.\n" ); }		
	}
	
#-- Push icommand in @summarylist

	push( @summarylist, "$cmd" );

#-- Execute icommand

	$rc = system( "$cmd > $tempFile" );

#-- check that the list of answers is part of the result of the command.

	if (( $rc == 0 ) and $stringToCheck ) {
		@words     = split( ",", $expResult );
		$numinlist = @words;
		@list      = dumpFileContent( $tempFile );
		
		if ( $debug ) {
			print( "DEBUG: numinlist = $numinlist\n" );
			print( "DEBUG: list =\n@list\n" );
		}
				
#---- If LIST is given as 3rd element: compare output of icommand to list in 4th argument
		
		if ( $stringToCheck eq "LIST" ) {
			foreach $line ( @list ) {
				chomp( $line );
				$line =~ s/^\s+//;
				$line =~ s/\s+$//;
				$answer .= "$line ";
			}
			chomp( $answer );
			$answer =~ s/^\s+//;
			$answer =~ s/\s+$//;

			if ( $debug ) { print( "DEBUG: answer    = $answer\n" ); }

			foreach $entry ( @words ) {
				if ( $answer =~ /$entry/ ) { $numsuccess++; }
			}
			
			if ( $numsuccess >= $numinlist ) {
				$result = 1;
			} else {
				$result = 0;
			}
		} elsif ( $stringToCheck eq "NOANSWER" ) {
			my $numanswer = @list;
			if ($numanswer == 0) {
				$result = 1;
                        } else {
                                $result = 0;
                        }
		} else {
			if ( $debug ) { print( "DEBUG: stringToCheck = $stringToCheck\n" ); }
			foreach $line ( @list ) {
				chomp( $line );
				if ( $debug ) { print( "DEBUG: line = $line\n" ); }
				if ( $line =~ /$stringToCheck/ ) {
					( $misc, $answer ) = split( /$stringToCheck/, $line );
					$answer =~ s/^\s+//;		# remove blanks
					$answer =~ s/\s+$//;		# remove blanks
					last;
				}
			}
			
			if ( $answer eq $words[0] ) {
				$result = 1;
			} else {
				$result = 0;
			}
		}
	}
	
        if ( $rc != 0 and $failtest == 1) {
                push( @successlist, "$ntests - $cmd  ====> OK\n" );
                $result = 1;
	} elsif ( $rc == 0 and ( $result ^ $negtest ) ) {
		push( @successlist, "$ntests - $cmd  ====> OK\n" );
		$result = 1;
	} else {
		push( @failureList, "$ntests - $cmd  ====> error code = $rc\n" );
		$result = 0;
	}

	if ( $debug ) { print( "DEBUG: result    = $result (1 is OK).\n" ); }			
	
	unlink( $tempFile );
	return();
}
##########################################################################################################################
# dumpFileContent: open a file in order to dump its content to an array

sub dumpFileContent {
	my $file = shift;
	my @filecontent;
	my $line;

	open( DUMP, $file ) or die "Unable to open the file $file in read mode.\n";
	foreach $line ( <DUMP> ) {
		$line =~ s/^\s+//;		# remove blanks
		$line =~ s/\s+$//;		# remove blanks
		push( @filecontent, $line );
	}
	close( DUMP );
	return( @filecontent );
}
##########################################################################################################################
# makeRuleFile: make a rule test file

sub makeRuleFile {
	my $rc;
	
	$rc = open( FILE2, ">$ruletestfile" );
	if ( ! $rc ) {
		print( "Impossible to open the file $ruletestfile in write mode.\n" );
		return( 1 );
	}

	print( FILE2 "# This is an example of an input for the irule command.\n" );
	print( FILE2 "# This first input line is the rule body.\n" );
	print( FILE2 "# The second input line is the input parameter in the format of:\n" );
	print( FILE2 "#   label=value. e.g., *A=$irodshome/icmdtest/foo1\n" );
	print( FILE2 "# Multiple inputs can be specified using the \'\%\' character as the seperator.\n" );
	print( FILE2 "# The third input line is the output description. Multiple outputs can be specified\n" );
	print( FILE2 "# using the \'\%\' character as the seperator.\n" );
	print( FILE2 "#\n" );
	print( FILE2 "myTestRule||msiDataObjOpen(*A,*S_FD)" );
	print( FILE2 "##msiDataObjCreate(*B,\'$testresource2\',*D_FD)" );
	print( FILE2 "##msiDataObjLseek(*S_FD,10,\'SEEK_SET\',*junk1)" );
	print( FILE2 "##msiDataObjRead(*S_FD,10000,*R_BUF)" );
	print( FILE2 "##msiDataObjWrite(*D_FD,*R_BUF,*W_LEN)" );
	print( FILE2 "##msiDataObjClose(*S_FD,*junk2)" );
	print( FILE2 "##msiDataObjClose(*D_FD,*junk3)" );
	print( FILE2 "##msiDataObjCopy(*B,*C,\'$testresource2\',*junk4)" );
#	print( FILE2 "##delayExec(msiDataObjRepl(*C,$testresource2,*junk5),<A></A>)" );
	print( FILE2 "##msiDataObjUnlink(*B,*junk6)|null" );
	print( FILE2 "\n" );
	print( FILE2 "*A=\"$irodshome/icmdtest/foo1\"\%*B=\"$irodshome/icmdtest/foo4\"\%*C=\"$irodshome/icmdtest/foo3\"" );
	print( FILE2 "\n" );
	print( FILE2 "*R_BUF\%*W_LEN" );
	print( FILE2 "\n" );
	close( FILE2 );
	return( 0 );
}

# make a directory of 2 large files and 2 small fles
sub mkldir
{
    my $i;
    my $count = 5; 
    my $fcount = 2; 
    my $mylfile;
    my $mysfile;
    my $lfile = $dir_w . "/lfile";
    my $lfile1 = $dir_w . "/lfile1";
    system( "echo 012345678901234567890123456789012345678901234567890123456789012 > $lfile" );
    for ( $i = $count; $i >= 0; $i-- ) {
      system ( "cat $lfile $lfile $lfile $lfile $lfile $lfile $lfile $lfile $lfile > $lfile1" );
	rename ( $lfile1, $lfile );
    }
    $mylsize = stat ($lfile)->size;
    system ( "mkdir $myldir" );
    for ( $i = $fcount; $i > 0; $i-- ) {
        $mylfile = $myldir . '/' . 'lfile' . $i;
        $mysfile = $myldir . '/' . 'sfile' . $i;
	if ($i != 1) {
	    copy ( $lfile, $mylfile );
	} else { 
	    rename ( $lfile, $mylfile );
	}
	copy ( $progname, $mysfile );
    }
}

# make a directory of small files and $sfile2
sub mksdir
{
    my $i;
    my $count = 20;
    my $mysfile;
    system ( "mkdir $mysdir" );
    for ( $i = $count; $i > 0; $i-- ) {
        $mysfile = $mysdir . '/' . 'sfile' . $i;
	copy ( $progname, $mysfile );
    }
}

# given a sub file path, get the path of the bundle file
sub getBunpathOfSubfile ()
{
    my $subfilepath = shift;
    my $line;
    my @list;
    my @words;
    my $numwords;

    system  ("ils -L $subfilepath > $tempFile" );
    @list      = dumpFileContent( $tempFile );
    unlink( $tempFile );
# bundle path is in 2nd line
    @words = split( / /, $list[1] );
    $numwords = @words;
# bundle path is in the last entry of the line
    return ( $words[$numwords - 1] );
}

sub printUsage ()
{
    print ("usage: $0 [help] [debug] [noprompt]\n");
    print ("  help - Print usage messages.\n");
    print ("  debug - Print debug messages.\n");
    print ("  noprompt -  Assumes iinit was done before running this script and\n");
    print ("    will not ask for password nor path input.\n");
}

