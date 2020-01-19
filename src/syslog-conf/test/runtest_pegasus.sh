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
#Set the pegasus install and home paths here to match your test environment
export PEGASUS_ROOT=/home/sharada/CIM/pegasus-2.2.1
export PEGASUS_HOME=/home/sharada/CIM/pegasus_home
export PATH=$PATH:$PEGASUS_HOME/bin

#Set the user and password to be used for the client connection to the server
#This is not needed for pegasus right now, so ignoring it
user=
passwd=

echo "This is the cli test for syslog conf provider classes - configuration, setting and settingcontext"
echo "There are two types of test for this provider"
echo "1. Instance Tests 2. Association Tests"
echo "Give your input as integer values - 1 or 2"

read n
echo "Value entered:$n"

echo "For setting getinstance, enter the key values for facility, priority and target, each on a new line"
read fac 
read prio 
read path
echo "Value entered:" "$fac" "$prio" "$path"

if [ $n -eq 1 ]
then
echo "For setting createinstance and deleteinstance, enter the key values for facility, priority and target, each on a new line"
read newfac 
read newprio 
read newpath
echo "Value entered:" $newfac $newprio $newpath
fi

cmd=CLI
inst_func='ei ni'
set_func='ci di'
assoc_func='a an r rn'

conexp_string=Syslog_Configuration
setexp_string=Syslog_Setting

conexp_getstring=Syslog_Configuration.Name=@/etc/syslog.conf@
setexp_getstring=Syslog_Setting.Facility=@$fac@,Priority=@$prio@,Target=@$path@

conexp_assocstring=Syslog_Configuration.Name=@/etc/syslog.conf@
setexp_assocstring=Syslog_Setting.Facility=@$fac@,Priority=@$prio@,Target=@$path@

if [ $n -eq 1 ] 
then
echo "Running Instance Tests"
for i in $inst_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $conexp_string
echo " "
$cmd $i $setexp_string
done
echo "running gi test"
$cmd gi $conexp_getstring
echo " "
$cmd gi $setexp_getstring

for i in $set_func
do
echo "running $i test"
$cmd $i Syslog_Setting.Facility=@$newfac@,Priority=@$newprio@,Target=@$newpath@
done

elif [ $n -eq 2 ] 
then
echo "Running Association Tests"
for i in $assoc_func
do
echo " "
echo "running $i test"
echo " "
$cmd $i $conexp_assocstring
echo " "
$cmd $i $setexp_assocstring
echo " "
done

else
echo "Value Not Supported"
fi


