#!/bin/bash
#/bin/device_added.sh
mkdir /media/vagesha/$1
mount /dev/$1 /media/vagesha/$1
cp /var/log/boot.log /media/vagesha/$1
umount /media/vagesha/$1
rmdir /media/vagesha/$1

