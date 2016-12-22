# See LICENSE for license details.
HLD_ROOT = ../../../../

DEBUG_FLAGS=-O3

SOURCES=sc_main.cpp 
USER_DEFS ?= -DSC_KERNEL
TARGET=ga_kernel

SC_DIR = /p/hdk/rtl/cad/x86-64_linux26/accellera/systemc/2.3.0
SC_LIB = /p/hdk/rtl/cad/x86-64_linux26/accellera/systemc/2.3.0/release/linux_64/gcc_4.5.2/libsystemc.a

CFLAGS=${USER_DEFS} -g -I..
#CFLAGS=${USER_DEFS} -mavx2
include $(HLD_ROOT)/common/Makefile.inc
