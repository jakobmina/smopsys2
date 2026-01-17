# Kernel Module Makefile

# Define module name
obj-m += smopsys.o

# Link core objects into the module
smopsys-y := smopsys_mod.o ../../kernel/qcore_math.o ../../kernel/qcore_scheduler.o ../../kernel/qcore_asm.o ../../kernel/qcore_bios.o

# Add include directory to CFLAGS
ccflags-y := -I$(src)/../../include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
