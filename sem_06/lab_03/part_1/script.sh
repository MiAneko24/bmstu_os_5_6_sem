#!/bin/bash
make
echo ">>>>> Loading md.ko..."
echo
sleep 0.8
sudo insmod md.ko
echo ">>>>> Module was loaded. Let's look at it: "
lsmod | grep md
echo
sleep 1
echo ">>>>> This is the last 15 lines in the system log: "
dmesg | tail -15
echo
read -n 1 -s -r -p "Press any key to continue..."
echo
echo ">>>>> Remove md..."
sudo rmmod md
echo
sleep 0.8
dmesg | tail -15