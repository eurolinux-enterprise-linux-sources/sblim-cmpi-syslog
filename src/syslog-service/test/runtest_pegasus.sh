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


#This works for the pegasus CLI
export PEGASUS_ROOT=/home/sharada/CIM/pegasus-2.2.1
export PEGASUS_HOME=/home/sharada/CIM/pegasus_home
export PATH=$PATH:$PEGASUS_HOME/bin

#Set the user and password to be used for the client connection to the server
#This is not needed for pegasus right now, so ignoring it
user=
passwd=

echo "This is the cli test for syslog service provider classes - service and serviceprocess"
echo "There are three types of test for this provider"
echo "1. Instance Tests 2. Association Tests 3. Method Tests"
echo "Give your input as integer values - 1, 2 or 3"

read n
echo "Value entered:$n"

echo "Enter the hostname on which the server is running. Give the FQDN ( Fully
Qualified Domain Name )"
read hostname
echo "HostName:$hostname"
                                                                                
#Some defined values for the keys for the classes tested here. 
                                                                                
svcsccn=CIM_UnitaryComputerSystem
svcccn=Syslog_Service
proccsccn=Linux_ComputerSystem
procosccn=Linux_OperatingSystem
procccn=Linux_UnixProcess

cmd=CLI
inst_func='ei ni'
assoc_func='a an r rn'
method_func='im'
method='StartService RestartService ReloadService CondRestartService StopService ForceReloadService TryRestartService ProbeService'

svcexp_string=Syslog_Service
svcexp_getstring=Syslog_Service.SystemCreationClassName=@$svcsccn@,SystemName=@$hostname@,CreationClassName=@$svcccn@,Name=@syslog@
svcexp_assocstring=Syslog_Service.SystemCreationClassName=@$svcsccn@,SystemName=@$hostname@,CreationClassName=@$svcccn@,Name=@syslog@

kpid=`ps -ef|grep klogd|grep -v grep| awk '{print $2}'`
spid=`ps -ef|grep syslogd|grep -v grep| awk '{print $2}'`

if [ -n "$kpid" ] && [ -n "$spid" ] 
then
procexp_assocstring1=Linux_UnixProcess.CSCreationClassName=@$proccsccn@,CSName=@$hostname@,OSCreationClassName=@$procosccn@,OSName=@$hostname@,CreationClassName=@$procccn@,Handle=@$kpid@
procexp_assocstring2=Linux_UnixProcess.CSCreationClassName=@$proccsccn@,CSName=@$hostname@,OSCreationClassName=@$procosccn@,OSName=@$hostname@,CreationClassName=@$procccn@,Handle=@$spid@
fi

if [ $n -eq 1 ] 
then
echo "Running Instance Tests"
for i in $inst_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $svcexp_string
done
echo "running gi test"
$cmd gi $svcexp_getstring

elif [ $n -eq 2 ] 
then
echo "Running Association Tests"
for i in $assoc_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $svcexp_assocstring
echo " "
if [ -n "$procexp_assocstring1" ] && [ -n "$procexp_assocstring2" ]
then
$cmd $i $procexp_assocstring1
echo " "
$cmd $i $procexp_assocstring2
echo " "
else
echo "service not running"
fi
done

elif [ $n -eq 3 ] 
then
echo "Running Method Tests"
for i in $method
do
echo " "
echo "running $i test"
echo " "
$cmd $method_func $svcexp_getstring $i
echo " "
done

else
echo "Value Not Supported"
fi


