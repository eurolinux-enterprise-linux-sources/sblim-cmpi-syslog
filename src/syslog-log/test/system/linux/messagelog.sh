syslog_conf=/etc/syslog.conf
if [ -f /etc/rsyslog.conf ]; then
    syslog_conf=/etc/rsyslog.conf
fi

cat ${syslog_conf} | grep '/var/log' | awk '!/^#/ {print $2}' | sed 's/\-//g'
