CFLAGS:=$(CFLAGS) -g -O0 -Wall # -Werror
LDFLAGS:=$(LDFLAGS) -g
OBJS=l3.o asm.o
CC=gcc
LD=gcc

all: l3

l3: $(OBJS)
	$(LD) -O0 -o $@ $(LDFLAGS) $^

prepare:
	echo 512 | sudo tee /proc/sys/vm/nr_hugepages

run: l3
	./$^
