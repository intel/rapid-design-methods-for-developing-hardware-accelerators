# See LICENSE for license details.
ref:
	$(MAKE) -C ref_build

fpga:
	export prefix 
	$(MAKE) -C fpga_build

part:
	$(MAKE) -C part_build


clean_ref:
	$(MAKE) -C ref_build clean

clean_fpga:
	export prefix
	$(MAKE) -C fpga_build clean

clean_part:
	$(MAKE) -C part_build clean


clean: clean_ref clean_fpga

all: ref fpga


