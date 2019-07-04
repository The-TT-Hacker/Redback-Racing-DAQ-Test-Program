# Redback-Racing-DAQ-Test-Program

USAGE

Setup virtual CAN interface
===========================

modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

OR

bash setup.sh

Install Yuria
=============

Go to the confluence page

Compile the ecu program
=======================

gcc ecu.c -o ecu

Run
===

Run yuria with the supplied config file test.rbrc

run: ./ecu
