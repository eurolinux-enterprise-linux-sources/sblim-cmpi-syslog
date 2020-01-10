val=`cat /etc/syslog.conf | grep '/var/log' | awk '!/^#/ {print $2}' | sed 's/\-//g'`
count=0
for i in $val
do
num=`cat $i | wc -l`
count=`expr $count + $num`
done
count=`expr $count + 1`
echo $count
