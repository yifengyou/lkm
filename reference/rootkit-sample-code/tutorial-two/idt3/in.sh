#!/bin/bash 
dmesg -c &> /dev/null
insmod ./sys_mkdir.ko


