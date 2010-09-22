ifneq ($(KERNELRELEASE),)
	obj-m := hello_cdev.o

else
	KERNELDIR = /lib/modules/$(shell uname -r)/build
	PWD=$(shell pwd)

defaule:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 

endif
