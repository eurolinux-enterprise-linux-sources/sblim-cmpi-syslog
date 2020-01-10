#
# runtest_pegasus.sh
#
# Copyright (c) IBM Corp. 2003, 2009
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Eclipse Public License from
# http://www.opensource.org/licenses/eclipse-1.0.php
#
# Author: R Sharada <sharada@in.ibm.com>
#
# Interface Type: Common Manageability Programming Interface ( CMPI )
#
# Description: Test Script for Linux Syslog Service Provider using Pegasus CLI
#


#This works with the pegasus CLI
#Set the pegasus install and home paths here to match your test environment
export PEGASUS_ROOT=/home/sharada/CIM/pegasus-2.2.1
export PEGASUS_HOME=/home/sharada/CIM/pegasus_home
export PATH=$PATH:$PEGASUS_HOME/bin

#Set the user and password to be used for the client connection to the server
#This is not needed for pegasus right now, so ignoring it
user=
passwd=

echo "This is the cli test for syslog log provider classes - messagelog, logrecord and recordinlog"
echo "There are two types of test for this provider"
echo "1. Instance Tests 2. Association Tests"
echo "Give your input as integer values - 1 or 2"

read n
echo "Value entered:$n"

echo "Enter the hostname on which the server is running. Give the FQDN ( Fully
Qualified Domain Name )"
read hostname
echo "HostName:$hostname"
                                                                                
#Some defined values for the keys for the classes tested here.
                                                                                
logccn=Syslog_MessageLog
reclccn=$logccn
recccn=Syslog_LogRecord

echo "For messagelog getinstance and assoc interfaces, enter a log name ( valid
log name that exists in the /etc/syslog.conf file )"
read logname
                                                                                
echo "For logrecord getinstance and assoc interfaces, enter the keys for the logrecord class ( the logname, recordid, and timestamp values ), each on a new line"
read reclogname 
read recid 
read rectime
echo "Value entered:" $reclogname $recid $rectime

cmd=CLI
inst_func='ei ni'
assoc_func='a an r rn'

logexp_string=Syslog_MessageLog
recexp_string=Syslog_LogRecord

logexp_getstring=Syslog_MessageLog.CreationClassName=@$logccn@,Name=@$logname@
recexp_getstring=Syslog_LogRecord.LogCreationClassName=@$reclccn@,LogName=@$reclogname@,CreationClassName=@$recccn@,RecordId=@$recid@,MessageTimestamp=@$rectime@
logexp_assocstring=Syslog_MessageLog.CreationClassName=@$logccn@,Name=@$logname@
recexp_assocstring=Syslog_LogRecord.LogCreationClassName=@$reclccn@,LogName=@$reclogname@,CreationClassName=@$recccn@,RecordId=@$recid@,MessageTimestamp=@$rectime@

if [ $n -eq 1 ] 
then
echo "Running Instance Tests"
for i in $inst_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $logexp_string
echo " "
$cmd $i $recexp_string
done
echo "running gi test"
$cmd gi $logexp_getstring
echo " "
$cmd gi $recexp_getstring

elif [ $n -eq 2 ] 
then
echo "Running Association Tests"
for i in $assoc_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $logexp_assocstring
echo " "
$cmd $i $recexp_assocstring
echo " "
done

else
echo "Value Not Supported"
fi


