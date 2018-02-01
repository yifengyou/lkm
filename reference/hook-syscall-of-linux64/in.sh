#!/bin/bash 
dmesg -c &> /dev/null
insmod ./`ls *.ko`


