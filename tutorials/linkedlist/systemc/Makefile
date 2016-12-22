# See LICENSE for license details.
acc:
	$(MAKE) -C acc_build

kernel:
	$(MAKE) -C kernel_build

clean_acc:
	$(MAKE) -C acc_build clean

clean_kernel:
	$(MAKE) -C kernel_build clean


clean: clean_acc clean_kernel

all: acc kernel


