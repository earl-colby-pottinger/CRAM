#!/bin/sh

# Compile source code to create device driver.
make

# Copy driver to driver folder.
cp objects*/cram1 ~/config/non-packaged/add-ons/kernel/drivers/bin/cram1

# Delete extra files left over from compiler.
rm objects*/*

# Delete folder created by compiler.
rmdir objects*

# Create directory if it does not exist.
mkdir /boot/home/config/non-packaged/add-ons/kernel/drivers/dev/disk

# Create symbolic link needed to support a driver.
ln -sf /boot/home/config/non-packaged/add-ons/kernel/drivers/bin/cram1 /boot/home/config/non-packaged/add-ons/kernel/drivers/dev/disk/cram1
