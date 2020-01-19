#
# syslog-service.sh
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
# Description: Wrapper script around Linux distribution specific syslog script
# for the CIM Linux Syslog Service Provider
#


# This is the script to run the syslog service on various distros

if [ -f /etc/redhat-release ]
then
if [ -x /etc/init.d/syslog ]
then
   initscript=syslog
elif [ -x /etc/init.d/rsyslog ]
then
   initscript=rsyslog
else
   initscript=""
fi

if [ -n "$initscript" ]
then
case "$1" in
 start)
   /etc/init.d/$initscript start
   ;;
 stop)
   /etc/init.d/$initscript stop
   ;;
 restart|reload)
   /etc/init.d/$initscript restart
   ;;
 condrestart)
   /etc/init.d/$initscript condrestart
   ;;
 status)
   /etc/init.d/$initscript status | awk '{print $3 " " $1}' | tr -d '=)='
   ;;
 *)
    echo "Unsupported method for RedHat!"
    exit 0
esac
elif [ -f /lib/systemd/system/rsyslog.service ];
then
case "$1" in
 start)
   systemctl start rsyslog.service
   ;;
 stop)
   systemctl stop rsyslog.service
   ;;
 restart|reload)
   systemctl restart rsyslog.service
   ;;
 condrestart)
   systemctl condrestart rsyslog.service
   ;;
 status)
   output=`systemctl status rsyslog.service`
   if echo "$output" | grep Active: | grep inactive > /dev/null 2>&1; then
       echo "stopped"
   else
       pid=`echo "$output" | sed -n -e 's/^[ \t]\+Main PID:[ \t]\+\([0-9]\+\).*/\1/p'`
       echo "$pid rsyslog" 
   fi
   ;;
 *)
    echo "Unsupported method for RedHat!"
    exit 0
esac
else
    echo "Neither syslog nor rsyslog found!"
    exit 0
fi
elif [ -f /etc/SuSE-release ] 
then
case "$1" in
 start)
   /etc/init.d/syslog start
   ;;
 stop)
   /etc/init.d/syslog stop
   ;;
 try-restart)
   /etc/init.d/syslog try-restart
   ;;
 restart)
   /etc/init.d/syslog restart
   ;;
 reload|force-reload)
   /etc/init.d/syslog reload
   ;;
 status)
   val=`/etc/init.d/syslog status | awk '{print $4}' | cut -b10-16`
   if [ "$val" = "running" ] ; then
   syslogd=`ps -ef|grep syslogd | grep -v grep | awk '{print $2}'`
   klogd=`ps -ef|grep klogd | grep -v grep | awk '{print $2}'`
   echo $syslogd syslogd 
   echo $klogd klogd
   else
   echo stopped syslogd
   echo stopped klogd
   fi
   ;;
 probe)
   /etc/init.d/syslog probe
   ;;
 *)
   echo "Unsupported method for Suse!"
   exit 0
   ;;
esac

else
echo "Unsupported Linux Distribution!"
exit 1
fi
