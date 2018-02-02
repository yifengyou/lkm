#!/bin/bash 
TIMESTAMP=`date "+%Y-%m-%d+%H:%M:%S" `
echo $TIMESTAMP
dmesg -c &> /dev/null
insmod ./`ls *.ko` &> /dev/null
if [ $? -eq 0 ];then 
    echo "insmod success!"
else
    echo "already exist!"
fi

if [ ! -d `uname -r` ]; then
    mkdir `uname -r`
fi

cd `uname -r`
mkdir mkdir-$TIMESTAMP










