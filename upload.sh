#!/bin/bash 
cp rootkit.ko syscall/
scp -r syscall root@192.168.79.139:/root/test/

