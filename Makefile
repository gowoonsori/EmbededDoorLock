obj-m   := doorlock_driver.o

KDIR :=/work/achro-em/kernel
PWD :=$(shell pwd)

all: app

app:
	arm-linux-gnueabihf-gcc -static -o doorlock doorlock.c -lm -lpthread

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf doorlock
