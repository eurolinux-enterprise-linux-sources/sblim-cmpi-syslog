cat /etc/syslog.conf | awk '!/^#/ {print $0}' |grep var | wc -l
