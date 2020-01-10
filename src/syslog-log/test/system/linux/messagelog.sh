cat /etc/syslog.conf | grep '/var/log' | awk '!/^#/ {print $2}' | sed 's/\-//g'
