#! /bin/sh
# $Id: load_hello_cdev, v 1.0.0 2010/08/23 John Exp $
module="hello-cdev"
device="hello-cdev"
mode="664"

#More than likely this driver will get a major dynamically,
#so the invocation of insmod can be replaced by this script.

#If this driver is distributed in the form of a module, 
#the user can invoke this script from the system's rc.local
#file or call it manually whenver the module is needed.

#If you get Permission denied, it's because the permissions 
#for the file first have to be set to executable, and by
#default the permissions for new files are set to read and
#write only. 
#So to read, write, and execute the file use: 
# chmod 755 load_hello_cdev will do the trick.

#Group: since distributions do it differently, look for wheel,
#or use staff.
if grep -q '^staff:' /etc/group; then
	group="staff"
else
	group="wheel"
fi

#Invoke insmod with all arguments we got, and use a path name,
#as insmod doesn't look in '.' by default.
/sbin/insmod ./$module.ko $* || exit 1

#Retrieve the major number.
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

#Remove any stale nodes and replace them,
#then give gid and permissions.
rm -f /dev/${device}0 
mknod /dev/${device}0 c $major 0 
#c means that a char device is to be created. 

#Now change the group and permission.
ln -sf ${device}0 /dev/${device}
chgrp $group /dev/${device}0
chmod $mode  /dev/${device}0
