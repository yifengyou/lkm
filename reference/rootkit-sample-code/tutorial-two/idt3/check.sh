#!/bin/bash 
echo "========================================================"
cat /proc/kallsyms | grep sys_open
echo "========================================================"

cat /proc/kallsyms | grep  sys_call_table

echo "========================================================"

cat /proc/kallsyms | grep sys_mkdir
