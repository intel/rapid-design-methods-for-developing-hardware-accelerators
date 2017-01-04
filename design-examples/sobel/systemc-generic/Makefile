# See LICENSE for license details.
.PHONY: all acc kernel clean_acc clean_kernel clean

all: acc kernel

clean: clean_acc clean_kernel

acc:
	$(MAKE) -C acc_build

kernel:
	$(MAKE) -C kernel_build

clean_acc:
	$(MAKE) -C acc_build clean

clean_kernel:
	$(MAKE) -C kernel_build clean







