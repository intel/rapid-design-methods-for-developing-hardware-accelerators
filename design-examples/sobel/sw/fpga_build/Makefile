# See LICENSE for license details.
HLD_ROOT = ../../../..

SOURCES=tb.cpp 
TARGET=sobel_fpga
CFLAGS =
LDFLAGS =

USER_DEFS ?= -DFPGA_ACC -D__AAL_USER__

ifeq ($(prefix), )
$(error FPGA mode requires prefix parameter set to point to aalsdk installation location)
endif
CCI_MPF_SW_ROOT ?= /nfs/site/disks/scl.work.38/ppt/aayupov/systemc/atd_fpga_app-qa_bblocks/BBB_cci_mpf/sw
CFLAGS += -I$(prefix)/include -I$(CCI_MPF_SW_ROOT)/include
LDFLAGS += -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib \
        -L$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(prefix)/lib64 \
        $(HLD_ROOT)/fpga_sw/5.0.3/AcclAppVtp/libAcclApp.so -Wl,-rpath -Wl,$(HLD_ROOT)/fpga_sw/5.0.3/AcclAppVtp/ \
        -Wl,-rpath -Wl,$(CCI_MPF_SW_ROOT)/
            

CFLAGS+=${USER_DEFS} -I../../systemc -I${HLD_ROOT}/fpga_sw/5.0.3/AcclAppVtp/
include ${HLD_ROOT}/common/Makefile.inc
