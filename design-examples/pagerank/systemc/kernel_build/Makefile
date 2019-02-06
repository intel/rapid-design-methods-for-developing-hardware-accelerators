# See LICENSE for license details.
HLD_ROOT ?= ../../../../

DEBUG_FLAGS=-O3

SOURCES=sc_main.cpp 
USER_DEFS ?= -DSC_KERNEL
TARGET=ga_kernel

CFLAGS=${USER_DEFS} -g -I..
#CFLAGS=${USER_DEFS} -mavx2
include $(HLD_ROOT)/common/Makefile.inc
