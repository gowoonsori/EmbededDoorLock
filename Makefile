obj-m   := doorlock_driver.o

KDIR :=/work/achro-em/kernel
PWD :=$(shell pwd)

all: driver app

driver:
#	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

app:
	arm-linux-gnueabihf-gcc -static -o doorklock doorklock.c

install_nfs:
	cp -a fpga_buzzer_driver.ko /nfsroot
	cp -a fpga_test_buzzer /nfsroot
	cp -a fpga_led_driver.ko /nfsroot
	cp -a fpga_test_led /nfsroot
	cp -a fpga_fnd_driver.ko /nfsroot
	cp -a fpga_test_fnd /nfsroot
	cp -a fpga_push_switch_driver.ko /nfsroot
	cp -a fpga_test_push_switch /nfsroot

install_scp:
	scp fpga_buzzer_driver.ko fpga_test_buzzer pi@192.168.0.xxx:/home/pi/Modules
	scp fpga_push_switch_driver.ko fpga_test_push_switch pi@192.168.0.xxx:/home/pi/Modules
	scp fpga_fnd_driver.ko fpga_test_fnd pi@192.168.0.xxx:/home/pi/Modules
	scp fpga_led_driver.ko fpga_test_led pi@192.168.0.xxx:/home/pi/Modules

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .buzzer*
	rm -rf .tmp*
	rm -rf .push_switch*
	rm -rf .fnd*
	rm -rf .led*
	rm -rf doorklock