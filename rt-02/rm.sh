#!/bin/bash 
rmmod `ls *.ko | cut -d "." -f1`
