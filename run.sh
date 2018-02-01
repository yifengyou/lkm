#!/bin/bash 
make clean && make 
scp rootkit.ko root@192.168.1.225:/root/lkm/
scp -r syscall root@192.168.1.225:/root/lkm/

