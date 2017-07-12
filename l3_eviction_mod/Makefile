obj-m += l3mod.o
l3mod-objs := l3.o asm.o
KVERSION = $(shell uname -r)
CFLAGS_l3.o := -Wall

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
load:
	sudo insmod l3mod.ko
	dmesg | tail -n 1
unload:
	sudo rmmod l3mod.ko
	sudo rm /dev/l3
run:
	echo 1 | sudo tee /dev/l3
	cat /dev/l3
