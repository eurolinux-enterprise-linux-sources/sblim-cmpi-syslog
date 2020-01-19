syslog_conf=/etc/syslog.conf
if [ -f /etc/rsyslog.conf ]; then
    syslog_conf=/etc/rsyslog.conf
fi

cat ${syslog_conf} | awk '!/^#/ {print $0}' |grep var | wc -l
