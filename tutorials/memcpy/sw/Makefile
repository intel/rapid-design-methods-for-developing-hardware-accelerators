# See LICENSE for license details.
.PHONY: all part ref fpga clean_part clean_ref clean_fpga
all: part ref fpga
clean: clean_part clean_ref clean_fpga

part:
	$(MAKE) -C part_build

ref:
	$(MAKE) -C ref_build

fpga:
	export prefix 
	$(MAKE) -C fpga_build

clean_part:
	$(MAKE) -C part_build clean

clean_ref:
	$(MAKE) -C ref_build clean

clean_fpga:
	export prefix
	$(MAKE) -C fpga_build clean






