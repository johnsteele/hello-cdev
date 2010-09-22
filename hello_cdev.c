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

module_param(hello_major, int, S_IRUGO);
module_param(hello_minor, int, S_IRUGO);
module_param(num_devices, int, S_IRUGO); 


/* 
 * Dynamically allocated char devices.
 * See driver init() for initialization.  
 */ 
struct cdev *my_devices;


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

	if (hello_major) { // If user specified a major.
		device = MKDEV(hello_major, hello_minor);
		result = register_chrdev_region(device, num_devices, "hello-cdev");
	} else {           // Otherwise get major from kernel.
		result = alloc_chrdev_region(&device, hello_minor, num_devices, 
						"hello-cdev");
			   // Set our dynamically allocated major. 
		hello_major = MAJOR(device);
	}

	if (result < 0) {  // An error occured while register device.
		printk(KERN_WARNING "hello-cdev: can't get a major.\n");
		return result;	
	}

	// Set up the char devices. 
 	for (i = 0; i < num_devices; i++) {
		

	}	
	return 0;
}


/*
 * Cleanup hello_cdev driver resources. 
 */
static void __exit driver_cleanup(void) 
{
}


/*
 * Specify initialize and cleanup functions. 
 */
module_init(driver_init);
module_exit(driver_cleanup);


MODULE_LICENSE("DUAL BSD/GPL");
MODULE_AUTHOR ("John Steele");

