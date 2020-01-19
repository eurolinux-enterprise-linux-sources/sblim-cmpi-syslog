#!/bin/sh

# test sblim-cmpi-syslog package

#******************************************************************************#

export SBLIM_TESTSUITE_RUN=1;
export PATH=$PATH:$PWD/system/linux

#******************************************************************************#

declare -a CLASSNAMES[];
CLASSNAMES=([0]=Syslog_MessageLog [1]=Syslog_LogRecord [2]=Syslog_RecordInLog) 

declare -i max=2;
declare -i i=0;

while(($i<=$max))
do
  . run.sh ${CLASSNAMES[$i]} $1 $2 $3 $4 || exit 1;
  i=$i+1;
done
