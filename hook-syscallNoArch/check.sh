#!/bin/bash 
echo "========= /proc/kallsyms ==============================="
cat /proc/kallsyms | grep  ' sys_call_table$' --color
cat /proc/kallsyms | grep  ' sys_mkdir$' --color
cat /proc/kallsyms | grep   ' sys_open$'  --color

echo "========= /boot/System.map-`uname -r` =================="

cat /boot/System.map-`uname -r` | grep  ' sys_call_table$' --color
cat /boot/System.map-`uname -r` | grep  ' sys_mkdir$' --color
cat /boot/System.map-`uname -r` | grep  ' sys_open$' --color

echo "========================================================"
