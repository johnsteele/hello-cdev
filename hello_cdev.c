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

static int __init driver_init(void)
{
	int result;
	result = 0;
	return result;
}

static void __exit driver_cleanup(void) 
{
}

module_init(driver_init);
module_exit(driver_cleanup);

