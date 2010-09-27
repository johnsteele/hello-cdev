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
 * A buffer for copy_*_user.
 */
static char *my_buffer;

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
ssize_t device_read(struct file *filp, char __user *buff, size_t count, 
			loff_t *f_pos) 
{

	int remaining_data, transfer_data_size;
	
	/* The bytes left to transfer. */
	remaining_data = USER_BUFFER_SIZE - (int) (*f_pos); 
	if (remaining_data == 0) 
		return 0;	

	/* Only copy what will fit. */ 
	transfer_data_size = min(remaining_data, (int) count);

	if (copy_to_user(buff /* to */, my_buffer + *f_pos /* from */, transfer_data_size) != 0) {
		printk(KERN_NOTICE "Copying to user space failed again.\n");
		return -EFAULT;	
	} else {
		*f_pos += transfer_data_size;
		return transfer_data_size; 
	}
}


/*
 * Write to this device.
 */
ssize_t device_write(struct file *filp, const char __user *buff, size_t count,
			loff_t *f_pos)
{
	int remaining_data;
	remaining_data = USER_BUFFER_SIZE - (int) (*f_pos);

	if (count > remaining_data) {
		/* Can't write past the end of the device. */ 	
		return -EIO;
	}

	if (copy_from_user(my_buffer + *f_pos/* to */, buff /* from */, count) != 0) {
		printk(KERN_NOTICE "Copying to kernel space failed.\n");
		return -EFAULT;
	} else {	
		// Otherwise, increase the position in the open file.
		*f_pos += count;
		return count;
	}
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
	if (my_devices) {
		int i;
		for (i = 0; i < num_devices; i++) {
			cdev_del(&my_devices[i]);
			printk(KERN_NOTICE "Removing hello-cdev%d device\n", i);
		}

		kfree(my_devices);
	}

	if (my_buffer) 
		kfree(my_buffer);
 	
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
		
		// The name of this device (e.g., hello-cdev) has nothing to do
		// with the name of the device in /dev. 
		// It only helps to track the different owners of devices.
		// If your module name has only one type of devices it's okay to
		// use e.g., the name of the module, as I did here. 
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

	my_devices = kmalloc(num_devices * sizeof (struct cdev), GFP_KERNEL);
	
 	if (!my_devices) {
		result = -ENOMEM;
		goto fail; 
	}

	memset(my_devices, 0, num_devices * sizeof (struct cdev));
	
	/* Set up the char devices. */ 
 	for (i = 0; i < num_devices; i++) {
		setup_cdev(&my_devices[i], i);	
		printk(KERN_NOTICE "Registering hello-cdev%d device.\n", i);	
	}	
		
	/* The user-space buffer. */
	/* GFP_KERNEL - Allocate normal kernel ram.*/ 
	my_buffer = kmalloc (USER_BUFFER_SIZE * sizeof (*my_buffer), GFP_KERNEL);
	if (!my_buffer) {
		result = -ENOMEM;
		goto fail;
	}
	
	memset (my_buffer, 0, USER_BUFFER_SIZE * sizeof (*my_buffer));	
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

