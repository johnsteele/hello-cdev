ifneq ($(KERNELRELEASE),)
	#m - for module
	#spcecifies files wich are built as loadable kernel modules.
	obj-m += hello-cdev.o
	#just incase we later add some more source files.
	hello-cdev-objs := hello_cdev.o

else
	KERNELDIR = /lib/modules/$(shell uname -r)/build
	PWD=$(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
endif
