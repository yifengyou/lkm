#!/bin/bash 
dmesg -c &> /dev/null
insmod ./scthook.ko


