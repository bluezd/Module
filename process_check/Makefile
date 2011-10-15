ifneq ($(KERNELRELEASE),)
     obj-m := process.o
else
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:       
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf Module.markers modules.order Module.symvers
endif
