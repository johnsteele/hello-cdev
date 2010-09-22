/*
 * hello_cdev.h -- Definitions for the hello-char module.
 *
 * Copyright (C) 2010 John Steele
 * 
 * $Id: hello_cdev.h, v 1.0.0 2010/08/22 John Exp $
 */

#ifndef _HELLO_CDEV_H_
#define _HELLO_CDEV_H_

#ifndef HELLO_MAJOR
#define HELLO_MAJOR 0 /* Use a dynamic major by default. */
#endif

#ifndef NUM_DEVICES
#define NUM_DEVICES 1 /* Create one device 'hello_cdev0' by default. */
#endif

/*
 * Some configurable parameters that can be set using on command line.
 */
extern int hello_major;  /* hello_cdev.c */
extern int num_devices;  /* hello_cdev.c */


/*
 * @filp  The type of file structure.
 * @buf   The buffer, from which the user space function (read) will read.
 * @count A counter with the number of bytes to transfer.
 * @f_pos The position of where to start reading the file.
 */ 
ssize_t device_read (struct file *filp, char __user *buf, size_t count,
			loff_t *f_pos);
ssize_t device_write(struct file *filp, const char __user *buf, size_t count, 
			loff_t *f_pos);	


#endif /* _HELLO_CDEV_H_ */ 

