#!/bin/bash
echo foo bar
echo $@
echo $$ > /var/run/ppp-pppoe.pid
echo "eth0" >> /var/run/ppp-pppoe.pid
sleep 10
