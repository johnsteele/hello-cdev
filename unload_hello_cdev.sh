#! /bin/sh
# $Id: unload_hello_cdev.sh, v 1.0.0 2010/09/23 John Exp $
module="hello_cdev"
device="hello-cdev"

#Invoke rmmod with all arguments we got.
/sbin/rmmod $module $* || exit

#Remove stale nodes.
rm -f /dev/${device} /dev/${device}0


