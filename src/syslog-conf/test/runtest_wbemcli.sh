#
# runtest_wbemcli.sh
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
# Description: Test Script for Linux Syslog Service Provider using wbemcli
#


#This works for the sblim wbemcli
echo "This is the cli test for syslog conf provider classes - configuration, setting and settingcontext"

#Set the user and password to be used to connect to the server
user=
passwd=

echo "There are two types of test for this provider"
echo "1. Instance Tests 2. Association Tests"
echo "Give your input as integer values - 1 or 2"

read n
echo "Value entered:$n"

echo "Enter the hostname on which the server is running"
read hostname
echo "HostName:$hostname"

echo "For setting getinstance, enter the key values for facility, priority and
target, each on a new line"
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
echo "Value entered:" "$newfac" "$newprio" "$newpath"
fi

cmd=wbemcli
inst_func='ei ein'
set_func='ci mi'
del_func='di'
assoc_func='ai ain ri rin'

#Check if hostname is set. If not set, print error and exit
if [ -z "$hostname" ]
then
echo "Hostname not set!"
exit
fi

#Check if user and passwd set. If not set, print error and exit
if [ -z "$user" -a -z "$passwd" ]
then
echo "Userid and Password not set! Please set them in this script and rerun it"
exit
fi 

conexp_string=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Configuration
setexp_string=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Setting

conexp_getstring=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Configuration.Name=/etc/syslog.conf
setexp_getstring=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Setting.Facility=$fac,Priority=$prio,Target=$path
setexp_crsetstring="http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Setting.Facility=$newfac,Priority=$newprio,Target=$newpath Facility=$newfac,Priority=$newprio,Target=$newpath"
setexp_delstring=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Setting.Facility=$newfac,Priority=$newprio,Target=$newpath

conexp_assocstring=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Configuration.Name=/etc/syslog.conf
setexp_assocstring=http://$user:$passwd@$hostname:5988/root/cimv2:Syslog_Setting.Facility=$fac,Priority=$prio,Target=$path

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
$cmd $i $setexp_crsetstring
done

for i in $del_func
do
echo "running $i test"
$cmd $i $setexp_delstring
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


