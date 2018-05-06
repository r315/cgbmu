#!/bin/bash
#****************************************************************************
#* $Id:: LinuxNXPISP 4528 2010-08-25 23:32:22Z nxp21346 $
#* Project: Linux NXP ISP example
#*
#* Description:
#* This is a bash script tested in Ubuntu 10.04 that scans for NXP
#* LPC134X devices in USB ISP mode and updates their firmware. It
#* depends on lsusb, udev, udevadmin, and mount.
#*
#****************************************************************************
#* Software that is described herein is for illustrative purposes only
#* which provides customers with programming information regarding the
#* products. This software is supplied "AS IS" without any warranties.
#* NXP Semiconductors assumes no responsibility or liability for the
#* use of the software, conveys no license or title under any patent,
#* copyright, or mask work right to the product. NXP Semiconductors
#* reserves the right to make changes in the software without
#* notification. NXP Semiconductors also make no representation or
#* warranty that such application will be suitable for the specified
#* use without further testing or modification.
#***************************************************************************/
if [ $# != 1 ]; then
echo "This program will program new firmware into an NXP LPC134x device"
echo "in USB In-System Programming (ISP) mode. Start it with a path to"
echo "the new firmware."
echo ""
echo "$0 <path to new firmware>"
exit -1
fi

if [ ! -f "$1" ]; then
echo "Cannot open firmware file $1"
exit -1
fi

echo "Searching for NXP LPC134x MCU in USB ISP mode..."

# lsusb -d will return output like this:
# Bus 007 Device 002: ID 04cc:0003 Philips Semiconductors
lsusb -d "c251:1303" |
# Bus 007 Device 002: ID 04cc:0003 Philips Semiconductors
while IFS=" " read lab1 bus lab2 device lab3 id mfg
do
device="${device:0:3}" # remove trailing ':' from device number
echo "Found USB bus number: $bus device number: $device"

# make udev path for USB device
path="/dev/bus/usb/$bus/$device"
echo "Linux UDEV device path should be: $path"
echo "Now reading USB device info."

# read USB model ID for that device using udevadm
model=$(udevadm info -q property -n $path | grep ID_MODEL=)
model="${model:9}"
echo "Device Model ID is $model"
if [ $model != "NXP_LPC17xx_Memory" ]; then
echo "Model does not match: Not \"NXP_LPC13XX_IFLASH\""
else
echo "Correct model ID found."
echo "Searching for disk devices with matching device path..."

# convert our udev path into a much longer kernel device filename
devpath=$(udevadm info -q path -n $path)

# iterate through all of the disk devices, assuming they start with
# sd and have one letter in them.
for disk in /dev/sd[a-z]; do
echo "Checking disk $disk..."
# run udevadm again to get the kernel device filename for
# the disk device we are checking.
diskdevpath=$(udevadm info -q path -n $disk)
# test to see if the disk device's kernel file matches the
# USB device's kernel file
if [ $devpath = ${diskdevpath:0:${#devpath}} ]; then
echo "Match found: Disk device is $disk"
# We found the disk for our USB device. Now test to
# see if we can find it in the mount command output.
# /dev/sdc on /media/CRP DISABLD type vfat (rw,nosuid,nodev,uhelper=udisks,uid=1000,gid=1000,shortname=mixed,dmask=0077,utf8=1,flush)
volpath=$(mount | grep "^$disk " | cut -d' ' -f3- | cut -d'(' -f1)
# volpath should be: /media/CRP DISABLD type vfat
if [ "$volpath" = "" ]; then
echo "Found device but it appears to be unmounted."
echo "Please mount the partition and try the update again."
else
# Get the volume path from the mount command output
# make volpath look like: /media/CRP DISABLD
volpath="${volpath:0:${#volpath} - 11}"

# create file path for firmware.bin on ISP device
volpath="${volpath}/firmware.bin"
echo LPC ISP disk path: $volpath

# get the firmware sizes as a sanity check
fwsize=$(stat -c %s "$volpath")
newfwsize=$(stat -c %s "$1")
echo LPC ISP flash size / new firmware size: $fwsize $newfwsize

# dd is used to update firmware.bin in place. The cp
# command will truncate firmware.bin which will cause
# the linux vfat filesystem to reallocate the blocks
# in a different order, scrambling the firmware update
# flash writes.
echo dd bs=1024 conv=nocreat,notrunc if="$1" of="$volpath"
echo "Firmware update complete!"

# unmount device when done to ensure that all of the
# data is written to the NXP LPC ISP device
umount "$disk"
echo "Device unmounted."
fi
fi
done
fi
done


