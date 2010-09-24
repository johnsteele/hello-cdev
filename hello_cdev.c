/*
 * cdev.c -- A simple char device driver.
 *
 * Copyright (C) 2010 John Steele
 *
 * $Id: cdev.c, v 1.0.0 2010/09/21 John Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/moduleparam.h>  /* command-line args */
#include <asm/uaccess.h>        /* copy_*_user */

#include <linux/types.h>        /* dev_t types */
#include <linux/cdev.h>         /* cdev */
#include <linux/fs.h>           /* file_operations */

#include "hello_cdev.h"         /* definitions */


/*
 * Optionally specified at the command line.
 *
 * @hello_major The major number of this char driver.
 * @hello_minor The minor of the first device, the
		rest will sequentially follow.
 * @num_devices The number of char devices to create.
 */
int hello_major = HELLO_MAJOR;
int hello_minor = HELLO_MINOR;
int num_devices = NUM_DEVICES;

/* (e.g., insmod ./hello_cdev.ko hello_major=222 hello_minor=1 num_devices=3) */
module_param(hello_major, int, S_IRUGO);
module_param(hello_minor, int, S_IRUGO);
module_param(num_devices, int, S_IRUGO); 


/* 
 * Dynamically allocated char devices.
 * See driver init() for initialization.  
 */ 
struct cdev *my_devices;


/*
 * Functions definded by this driver that 
 * perform various operations on the device.  
 *
 * Creates a set of file operations for our proc file.
 */
struct file_operations fops = {
	.owner = THIS_MODULE,
	.read  = device_read,
	.write = device_write,	
};


/*
 * Read from the device.
 */
ssize_t device_read(struct file *filp, char __user *buf, size_t count, 
			loff_t *f_pos) 
{
	char temp[] = "Hello from the kernel!";
	if (copy_from_user(buf, temp, strlen(temp)) != 0) 
		printk(KERN_NOTICE "Copying to user space failed.\n");

	return strlen(temp); 	
}


/*
 * Write to this device.
 */
ssize_t device_write(struct file *filp, const char __user *buf, size_t count,
			loff_t *f_pos)
{
	char *temp = kmalloc(count * sizeof(char), GFP_KERNEL); 
	if (copy_to_user(temp, buf, sizeof(temp)) != 0)
		printk(KERN_NOTICE "Copying to kernel space failed.\n");
	kfree(temp);
	return count;
}


/*
 * Creates and initializes the provided cdev.
 * 
 * @the_cdev The cdev to initialize. 
 * @index    The index of the_cdev in my_devices.  
 */
void setup_cdev(struct cdev *the_cdev, int index) 
{
	int error;
	int device_number = MKDEV(hello_major, hello_minor + index);
	cdev_init(the_cdev, &fops);
	the_cdev->owner   = THIS_MODULE;
	the_cdev->ops     = &fops;
	error = cdev_add(the_cdev, device_number, 1);
	if (error) { 
		printk(KERN_NOTICE "Can't register hello-cdev%d device.\n", index);
	}
}


/*
 * Cleanup hello_cdev driver resources. 
 */
static void __exit driver_cleanup(void) 
{
	int i;
	for (i = 0; i < num_devices; i++) {
		cdev_del(&my_devices[i]);
		printk(KERN_NOTICE "Removing hello-cdev%d device\n", i);
	}
	kfree(my_devices);
	unregister_chrdev_region(MKDEV(hello_major, hello_minor), num_devices);
}


/* 
 * Initialize the hello_cdev driver.
 *
 * @return 0 for success, failure code otherwise.
 */
static int __init driver_init(void)
{
	int result, i;
	dev_t device;
	result = 0;
	device = 0;

	if (hello_major) { 
		// If user specified a major.
		device = MKDEV(hello_major, hello_minor);
		result = register_chrdev_region(device, num_devices, "hello-cdev");
	} else {          
		 // Otherwise get major from kernel.
		result = alloc_chrdev_region(&device, hello_minor, num_devices, 
						"hello-cdev");
	  	 // Set our dynamically allocated major. 
		hello_major = MAJOR(device);
	}

	if (result < 0) { 
		 // An error occured while register device.
		printk(KERN_WARNING "hello-cdev: can't get a major.\n");
		return result;	
	}

	my_devices = kmalloc(num_devices * sizeof(struct cdev), GFP_KERNEL);
 	if (!my_devices) {
		result = -ENOMEM;
		goto fail; 
	}

	memset(my_devices, 0, num_devices * sizeof(struct cdev));

	// Set up the char devices. 
 	for (i = 0; i < num_devices; i++) {
		setup_cdev(&my_devices[i], i);	
		printk(KERN_NOTICE "Registering hello-cdev%d device.\n", i);	
	}	
	return 0;
	
	fail:
		driver_cleanup();
		printk(KERN_NOTICE "hello-cdev driver failed to register.\n");
		return result; 
}


/*
 * Specify initialize and cleanup functions. 
 */
module_init(driver_init);
module_exit(driver_cleanup);


MODULE_LICENSE("DUAL BSD/GPL");
MODULE_AUTHOR ("John Steele");

